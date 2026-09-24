#include "dfa.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>

void DfaProblem::initialize_parser(cxxopts::Options &options) {
    // A feladatkiírás alapján a --check kapcsolóval kapjuk a szavakat
    options.add_options()
        ("check", "Ellenorizendo szavak (vesszovel elvalasztva)", cxxopts::value<std::string>());
}

bool DfaProblem::is_chosen_problem(const cxxopts::ParseResult &args) {
    // Ha a parancssorban szerepel a --check, ezt a feladatot futtatjuk
    return args.count("check") > 0;
}

int DfaProblem::run(const cxxopts::ParseResult &args) {
    // Fájlnevek és ellenőrzendő szavak kinyerése
    if (!args.count("input") || !args.count("output") || !args.count("check")) {
        std::cerr << "Hiba: Hianyzik az --input, --output vagy a --check parameter!" << std::endl;
        return 1;
    }

    std::string inputFile = args["input"].as<std::string>();
    std::string outputFile = args["output"].as<std::string>();
    std::string words_str = args["check"].as<std::string>();

    // 1. DFA adatainak beolvasása a megadott formátum alapján
    std::ifstream fin(inputFile);
    if (!fin.is_open()) {
        std::cerr << "Hiba: Nem sikerult megnyitni a bemeneti fajlt!" << std::endl;
        return 1;
    }

    std::string line;
    
    // 1. sor: Állapotok (csak átlépjük)
    std::getline(fin, line); 
    // 2. sor: Ábécé (csak átlépjük)
    std::getline(fin, line);

    // 3. sor: Kezdőállapot
    std::getline(fin, line);
    std::stringstream ss_start(line);
    std::string start_state;
    ss_start >> start_state;

    // 4. sor: Végállapotok
    std::getline(fin, line);
    std::stringstream ss_finals(line);
    std::set<std::string> final_states;
    std::string fs;
    while (ss_finals >> fs) {
        final_states.insert(fs);
    }

    // További sorok: Átmenetek (formátum: forrás_állapot szimbólum cél_állapot)
    // Térkép felépítése: jelenlegi_állapot -> (szimbólum -> következő_állapot)
    std::map<std::string, std::map<char, std::string>> transitions;
    std::string from_state, to_state;
    char symbol;
    while (fin >> from_state >> symbol >> to_state) {
        transitions[from_state][symbol] = to_state;
    }
    fin.close();

    // 2. Ellenőrzendő szavak szétválasztása vesszők mentén
    std::vector<std::string> words;
    std::stringstream ss_words(words_str);
    std::string word;
    while (std::getline(ss_words, word, ',')) {
        words.push_back(word);
    }

    // 3. Szimuláció és kimenet generálása
    std::ofstream fout(outputFile);
    if (!fout.is_open()) {
        std::cerr << "Hiba: Nem sikerult letrehozni a kimeneti fajlt!" << std::endl;
        return 1;
    }

    for (const std::string& w : words) {
        std::string current_state = start_state;
        bool valid = true;
        
        // Végigmegyünk a szó karakterein
        for (char c : w) {
            // Ha létezik átmenet az adott szimbólumra
            if (transitions[current_state].find(c) != transitions[current_state].end()) {
                current_state = transitions[current_state][c];
            } else {
                valid = false; // Ha nincs átmenet, az automata elakad
                break; 
            }
        }

        // Ha hibátlanul végigmentünk ÉS a jelenlegi állapot elfogadó állapot
        if (valid && final_states.find(current_state) != final_states.end()) {
            fout << "IGEN\n";
        } else {
            fout << "NEM\n";
        }
    }
    fout.close();

    return 0;
}