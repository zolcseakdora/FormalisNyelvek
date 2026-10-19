
#include "det.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

using StateSet = std::set<std::string>;

// A beolvasott automata adatai
struct Automaton {
    std::vector<std::string> alphabet;
    std::string start;
    StateSet finals;

    // forras -> szimbolum -> celallapotok
    std::map<std::string,
        std::map<std::string, StateSet>> transitions;
};

// Egy sor feldarabolasa szokozok menten
std::vector<std::string> split(const std::string& line) {
    std::vector<std::string> result;
    std::stringstream ss(line);
    std::string token;

    while (ss >> token) {
        result.push_back(token);
    }

    return result;
}

// Automata beolvasasa fajlbol
bool readAutomaton(const std::string& filename, Automaton& a) {
    std::ifstream fin(filename);

    if (!fin) {
        std::cerr << "Hiba: Nem nyithato meg a bemeneti fajl.\n";
        return false;
    }

    std::string line;

    // 1. sor: allapotok
    if (!std::getline(fin, line)) {
        std::cerr << "Hiba: Hianyzik az allapotok sora.\n";
        return false;
    }

    auto states = split(line);
    StateSet validStates(states.begin(), states.end());

    if (states.empty() || states.size() != validStates.size()) {
        std::cerr << "Hiba: Hibas vagy ismetlodo allapot.\n";
        return false;
    }

    // 2. sor: abece
    if (!std::getline(fin, line)) {
        std::cerr << "Hiba: Hianyzik az abece sora.\n";
        return false;
    }

    a.alphabet = split(line);

    std::set<std::string> validSymbols(
        a.alphabet.begin(), a.alphabet.end()
    );

    if (a.alphabet.empty() ||
        a.alphabet.size() != validSymbols.size()) {
        std::cerr << "Hiba: Hibas vagy ismetlodo szimbolum.\n";
        return false;
    }

    std::sort(a.alphabet.begin(), a.alphabet.end());

    // 3. sor: kezdoallapot
    if (!std::getline(fin, line)) {
        std::cerr << "Hiba: Hianyzik a kezdoallapot sora.\n";
        return false;
    }

    auto starts = split(line);

    if (starts.size() != 1 || !validStates.count(starts[0])) {
        std::cerr << "Hiba: Ervenytelen kezdoallapot.\n";
        return false;
    }

    a.start = starts[0];

    // 4. sor: vegallapotok
    if (!std::getline(fin, line)) {
        std::cerr << "Hiba: Hianyzik a vegallapotok sora.\n";
        return false;
    }

    for (const auto& q : split(line)) {
        if (!validStates.count(q)) {
            std::cerr << "Hiba: Ervenytelen vegallapot: "
                      << q << "\n";
            return false;
        }
        a.finals.insert(q);
    }

    // 5. sortol: atmenetek
    int lineNumber = 4;

    while (std::getline(fin, line)) {
        ++lineNumber;
        auto t = split(line);

        if (t.empty()) continue;

        if (t.size() != 3 ||
            !validStates.count(t[0]) ||
            !validSymbols.count(t[1]) ||
            !validStates.count(t[2])) {

            std::cerr << "Hiba: Ervenytelen atmenet a(z) "
                      << lineNumber << ". sorban.\n";
            return false;
        }

        a.transitions[t[0]][t[1]].insert(t[2]);
    }

    if (fin.bad()) {
        std::cerr << "Hiba: Nem sikerult beolvasni a fajlt.\n";
        return false;
    }

    return true;
}

// NFA -> DFA atalakitasa
int determinize(const Automaton& a, const std::string& filename) {

    std::vector<StateSet> dfaStates;

    // Az elso DFA allapot az eredeti kezdoallapot
    dfaStates.push_back({a.start});

    // Minden allapothalmazhoz egy s azonosito tartozik
    std::map<StateSet, size_t> ids;
    ids[dfaStates[0]] = 0;

    // DFA atmenetek
    std::vector<std::tuple<size_t, std::string, size_t>> edges;

    // Az uj allapotokat sorban dolgozzuk fel
    for (size_t i = 0; i < dfaStates.size(); ++i) {

        for (const auto& symbol : a.alphabet) {

            StateSet target;

            // Az aktualis halmaz osszes allapotabol
            // megkeressuk a szimbolummal elerheto allapotokat
            for (const auto& state : dfaStates[i]) {

                auto fromIt = a.transitions.find(state);

                if (fromIt == a.transitions.end()) continue;

                auto symbolIt = fromIt->second.find(symbol);

                if (symbolIt != fromIt->second.end()) {
                    target.insert(
                        symbolIt->second.begin(),
                        symbolIt->second.end()
                    );
                }
            }

            // Ures halmazhoz nem irunk atmenetet
            if (target.empty()) continue;

            auto existing = ids.find(target);
            size_t targetId;

            if (existing == ids.end()) {
                // Uj DFA allapot
                targetId = dfaStates.size();
                ids[target] = targetId;
                dfaStates.push_back(target);
            } else {
                // Mar ismert DFA allapot
                targetId = existing->second;
            }

            edges.emplace_back(i, symbol, targetId);
        }
    }

    // Eredmeny fajlba irasa
    std::ofstream fout(filename);

    if (!fout) {
        std::cerr << "Hiba: Nem hozhato letre a kimeneti fajl.\n";
        return 1;
    }

    // 1. sor: uj allapotok
    for (size_t i = 0; i < dfaStates.size(); ++i) {
        if (i) fout << ' ';
        fout << 's' << i;
    }
    fout << '\n';

    // 2. sor: abece
    for (size_t i = 0; i < a.alphabet.size(); ++i) {
        if (i) fout << ' ';
        fout << a.alphabet[i];
    }
    fout << '\n';

    // 3. sor: kezdoallapot
    fout << "s0\n";

    // 4. sor: vegallapotok
    bool first = true;

    for (size_t i = 0; i < dfaStates.size(); ++i) {
        for (const auto& state : dfaStates[i]) {
            if (a.finals.count(state)) {
                if (!first) fout << ' ';
                fout << 's' << i;
                first = false;
                break;
            }
        }
    }
    fout << '\n';

    // Tovabbi sorok: atmenetek
    for (const auto& edge : edges) {
        fout << 's' << std::get<0>(edge) << ' '
             << std::get<1>(edge) << " s"
             << std::get<2>(edge) << '\n';
    }

    if (!fout) {
        std::cerr << "Hiba: Sikertelen fajlba iras.\n";
        return 1;
    }

    return 0;
}

// Parancssori kapcsolo regisztralasa
void DetProblem::initialize_parser(cxxopts::Options &options) {
    options.add_options()
        ("det", "Automata determinizalasa");
}

// Akkor indul, ha megadtuk a --det kapcsolot
bool DetProblem::is_chosen_problem(
    const cxxopts::ParseResult &args
) {
    return args.count("det") > 0;
}

// A feladat futtatasa
int DetProblem::run(const cxxopts::ParseResult &args) {

    if (!args.count("input") || !args.count("output")) {
        std::cerr << "Hiba: Hianyzik az --input vagy --output!\n";
        return 1;
    }

    std::string inputFile = args["input"].as<std::string>();
    std::string outputFile = args["output"].as<std::string>();

    Automaton automaton;

    if (!readAutomaton(inputFile, automaton)) {
        return 1;
    }

    return determinize(automaton, outputFile);
}
