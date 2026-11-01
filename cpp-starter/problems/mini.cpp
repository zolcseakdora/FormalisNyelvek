
#include "mini.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

using namespace std;

// A segedfuggvenyeket elkulonitjuk,
// hogy ne utkozhessenek a korabbi
// hazi feladatok fuggvenyeivel.
namespace {

// =====================================================
// 1. AZ AUTOMATA ADATAINAK TAROLASA
// =====================================================

struct Automata {
    vector<string> allapotok;
    vector<string> abc;
    string kezdo;
    set<string> vegallapotok;

    // DFA eseteben egy allapotbol egy adott
    // betuvel legfeljebb egy celallapot lehet.
    map<pair<string, string>, string> atmenetek;
};

// A sor szavakra bontasa.
vector<string> szavak(const string& sor) {
    istringstream be(sor);
    vector<string> eredmeny;
    string szo;

    while (be >> szo) {
        eredmeny.push_back(szo);
    }

    return eredmeny;
}

// Az allapot nevenek szamresze.
// Pelda: q12 -> 12.
unsigned long long sorszam(const string& nev) {
    size_t elso_szam =
        nev.find_first_of("0123456789");

    if (elso_szam == string::npos ||
        elso_szam == 0 ||
        !all_of(
            nev.begin(),
            nev.begin() + elso_szam,
            [](unsigned char c) {
                return (c >= 'a' && c <= 'z') ||
                       (c >= 'A' && c <= 'Z');
            }
        ) ||
        !all_of(
            nev.begin() + elso_szam,
            nev.end(),
            [](unsigned char c) {
                return c >= '0' && c <= '9';
            }
        )) {
        throw runtime_error(
            "Hibas allapotnev: " + nev
        );
    }

    try {
        return stoull(nev.substr(elso_szam));
    } catch (const exception&) {
        throw runtime_error(
            "Tul nagy allapotsorszam: " + nev
        );
    }
}

// Az allapotok sorszam szerinti rendezese.
bool allapot_sorrend(
    const string& a,
    const string& b
) {
    auto ai = sorszam(a);
    auto bi = sorszam(b);

    if (ai != bi) {
        return ai < bi;
    }

    return a < b;
}

// q3 -> s3
string uj_nev(const string& eredeti) {
    return "s" + to_string(sorszam(eredeti));
}

// =====================================================
// 2. AZ AUTOMATA BEOLVASASA
// =====================================================

Automata beolvas(const string& fajlnev) {
    ifstream fajl(fajlnev);

    if (!fajl) {
        throw runtime_error(
            "Nem nyithato meg a bemeneti fajl."
        );
    }

    Automata a;
    string sor;

    // Elso sor: allapotok.
    if (!getline(fajl, sor)) {
        throw runtime_error(
            "Hianyzik az allapotok sora."
        );
    }

    a.allapotok = szavak(sor);

    if (a.allapotok.empty()) {
        throw runtime_error(
            "Legalabb egy allapot szukseges."
        );
    }

    set<string> allapot_halmaz(
        a.allapotok.begin(),
        a.allapotok.end()
    );

    if (allapot_halmaz.size() !=
        a.allapotok.size()) {
        throw runtime_error(
            "Ismetlodo allapot."
        );
    }

    set<string> uj_nevek;

    for (const auto& allapot : a.allapotok) {
        if (!uj_nevek.insert(
                uj_nev(allapot)
            ).second) {
            throw runtime_error(
                "Azonos uj allapotnevek."
            );
        }
    }

    // Masodik sor: bemeneti abece.
    if (!getline(fajl, sor)) {
        throw runtime_error(
            "Hianyzik az abece sora."
        );
    }

    a.abc = szavak(sor);

    if (a.abc.empty()) {
        throw runtime_error(
            "Az abece nem lehet ures."
        );
    }

    set<string> abc_halmaz(
        a.abc.begin(),
        a.abc.end()
    );

    if (abc_halmaz.size() != a.abc.size()) {
        throw runtime_error(
            "Ismetlodo abecejel."
        );
    }

    // Harmadik sor: kezdoallapot.
    if (!getline(fajl, sor)) {
        throw runtime_error(
            "Hianyzik a kezdoallapot."
        );
    }

    auto kezdok = szavak(sor);

    if (kezdok.size() != 1 ||
        !allapot_halmaz.count(kezdok[0])) {
        throw runtime_error(
            "Hibas kezdoallapot."
        );
    }

    a.kezdo = kezdok[0];

    // Negyedik sor: vegallapotok.
    if (!getline(fajl, sor)) {
        throw runtime_error(
            "Hianyzik a vegallapotok sora."
        );
    }

    for (const auto& v : szavak(sor)) {
        if (!allapot_halmaz.count(v)) {
            throw runtime_error(
                "Ismeretlen vegallapot: " + v
            );
        }

        a.vegallapotok.insert(v);
    }

    // A tovabbi sorok: atmenetek.
    int sorszam_sor = 4;

    while (getline(fajl, sor)) {
        ++sorszam_sor;

        const auto adatok = szavak(sor);

        if (adatok.empty()) {
            continue;
        }

        if (adatok.size() != 3) {
            throw runtime_error(
                "Hibas atmeneti sor: " +
                to_string(sorszam_sor)
            );
        }

        const string& honnan = adatok[0];
        const string& betu = adatok[1];
        const string& hova = adatok[2];

        // Ellenorizzuk az allapotokat.
        if (!allapot_halmaz.count(honnan) ||
            !allapot_halmaz.count(hova)) {
            throw runtime_error(
                "Ismeretlen allapot az atmenetben."
            );
        }

        // Ellenorizzuk a bemeneti jelet.
        if (!abc_halmaz.count(betu)) {
            throw runtime_error(
                "Ismeretlen bemeneti jel."
            );
        }

        auto kulcs = make_pair(honnan, betu);
        auto korabbi = a.atmenetek.find(kulcs);

        // Determinisztikus automata ellenorzese.
        if (korabbi != a.atmenetek.end() &&
            korabbi->second != hova) {
            throw runtime_error(
                "Nem determinisztikus atmenet."
            );
        }

        a.atmenetek[kulcs] = hova;
    }

    sort(
        a.allapotok.begin(),
        a.allapotok.end(),
        allapot_sorrend
    );

    sort(a.abc.begin(), a.abc.end());

    return a;
}

// =====================================================
// 3. ELERHETO ALLAPOTOK MEGHATAROZASA
// =====================================================

// Csak a kezdoallapotbol elerheto
// allapotokat tartjuk meg.

vector<string> elerheto_allapotok(
    const Automata& a
) {
    set<string> latott{a.kezdo};
    queue<string> sor;

    sor.push(a.kezdo);

    while (!sor.empty()) {
        string akt = sor.front();
        sor.pop();

        for (const string& betu : a.abc) {
            auto it = a.atmenetek.find(
                {akt, betu}
            );

            if (it != a.atmenetek.end() &&
                latott.insert(it->second).second) {
                sor.push(it->second);
            }
        }
    }

    return vector<string>(
        latott.begin(),
        latott.end()
    );
}

// =====================================================
// 4. MINIMALIZALAS HAROMSZOGMATRIXSZAL
// =====================================================

// Az oran tanult algoritmus:
//
// 1. Haromszogmatrix felepitese.
// 2. Vegallapot - nem vegallapot parok
//    megjelolese csillaggal.
// 3. Ujabb parok megjelolese az
//    atmenetek alapjan.
// 4. A meg nem jelolt allapotparok
//    osszevonasa.
// 5. Az uj allapotnevek meghatarozasa.

map<string, string> minimalizalt_nevek(
    const Automata& a
) {
    vector<string> allapotok =
        elerheto_allapotok(a);

    sort(
        allapotok.begin(),
        allapotok.end(),
        allapot_sorrend
    );

    int eredeti_db =
        static_cast<int>(allapotok.size());

    // A minimalizalas teljes DFA-n tortenik.
    // A hianyzo atmenetekhez ideiglenes
    // nyeloallapotot hasznalunk.
    // Ezt nem irjuk ki a kimeneti fajlba.

    bool kell_nyelo = false;

    for (const auto& q : allapotok) {
        for (const auto& betu : a.abc) {
            if (!a.atmenetek.count({q, betu})) {
                kell_nyelo = true;
            }
        }
    }

    const int nyelo = eredeti_db;

    const int n =
        eredeti_db + (kell_nyelo ? 1 : 0);

    map<string, int> index;

    for (int i = 0; i < eredeti_db; ++i) {
        index[allapotok[i]] = i;
    }

    // Atmeneti tablazat keszitese.
    vector<vector<int>> delta(
        n,
        vector<int>(a.abc.size(), nyelo)
    );

    for (int i = 0; i < eredeti_db; ++i) {
        for (size_t j = 0; j < a.abc.size(); ++j) {
            auto it = a.atmenetek.find(
                {allapotok[i], a.abc[j]}
            );

            if (it != a.atmenetek.end()) {
                delta[i][j] = index.at(it->second);
            }
        }
    }

    if (kell_nyelo) {
        for (size_t j = 0; j < a.abc.size(); ++j) {
            delta[nyelo][j] = nyelo;
        }
    }

    // Megnezzuk, mely allapotok vegallapotok.
    vector<bool> veg(n, false);

    for (int i = 0; i < eredeti_db; ++i) {
        veg[i] =
            a.vegallapotok.count(allapotok[i]) != 0;
    }

    // ---------------------------------------------
    // 1. LEPES: HAROMSZOGMATRIX
    // ---------------------------------------------

    // false = meg nincs megkulonboztetve
    // true  = biztosan nem ekvivalens

    vector<vector<bool>> megjelolt(
        n,
        vector<bool>(n, false)
    );

    // ---------------------------------------------
    // 2. LEPES: KEZDETI CSILLAGOZAS
    // ---------------------------------------------

    // Ha az egyik allapot vegallapot,
    // a masik nem, akkor nem ekvivalensek.

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (veg[i] != veg[j]) {
                megjelolt[i][j] = true;
            }
        }
    }

    // ---------------------------------------------
    // 3. LEPES: ITERATIV CSILLAGOZAS
    // ---------------------------------------------

    // Ha egy allapotpar valamely bemeneti
    // jelre mar megkulonboztetett parba jut,
    // akkor az aktualis par sem ekvivalens.

    bool valtozott;

    do {
        valtozott = false;

        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {

                if (megjelolt[i][j]) {
                    continue;
                }

                for (
                    size_t betu = 0;
                    betu < a.abc.size();
                    ++betu
                ) {
                    int u = delta[i][betu];
                    int v = delta[j][betu];

                    if (u == v) {
                        continue;
                    }

                    if (u > v) {
                        swap(u, v);
                    }

                    if (megjelolt[u][v]) {
                        megjelolt[i][j] = true;
                        valtozott = true;
                        break;
                    }
                }
            }
        }

    } while (valtozott);

    // ---------------------------------------------
    // 4. LEPES: EKVIVALENS ALLAPOTOK OSSZEVONASA
    // ---------------------------------------------

    // A csillag nelkul maradt
    // allapotparok ekvivalensek.

    vector<int> osztaly(n, -1);
    int osztaly_db = 0;

    for (int i = 0; i < n; ++i) {
        if (osztaly[i] != -1) {
            continue;
        }

        osztaly[i] = osztaly_db;

        for (int j = i + 1; j < n; ++j) {
            if (!megjelolt[i][j]) {
                osztaly[j] = osztaly_db;
            }
        }

        ++osztaly_db;
    }

    // ---------------------------------------------
    // 5. LEPES: UJ ALLAPOTNEVEK
    // ---------------------------------------------

    // Az osszevont allapotok kozul
    // a legkisebb sorszamu nevet tartjuk meg.
    //
    // Pelda:
    // q0 es q3 -> s0
    // q1 es q5 -> s1

    vector<string> osztalynevek(osztaly_db);

    for (int i = 0; i < eredeti_db; ++i) {
        if (osztalynevek[osztaly[i]].empty()) {
            osztalynevek[osztaly[i]] =
                uj_nev(allapotok[i]);
        }
    }

    map<string, string> uj;

    for (int i = 0; i < eredeti_db; ++i) {
        uj[allapotok[i]] =
            osztalynevek[osztaly[i]];
    }

    return uj;
}

// =====================================================
// 5. A MINIMALIZALT AUTOMATA KIIRASA
// =====================================================

struct Atmenet {
    string honnan;
    string betu;
    string hova;
};

void kiiras(
    const Automata& a,
    const map<string, string>& nev,
    const string& fajlnev
) {
    set<string, decltype(&allapot_sorrend)>
        allapotok(&allapot_sorrend);

    set<string, decltype(&allapot_sorrend)>
        vegek(&allapot_sorrend);

    for (const auto& elem : nev) {
        allapotok.insert(elem.second);

        if (a.vegallapotok.count(elem.first)) {
            vegek.insert(elem.second);
        }
    }

    vector<Atmenet> atmenetek;

    for (const auto& bejegyzes : a.atmenetek) {
        auto from = nev.find(
            bejegyzes.first.first
        );

        auto to = nev.find(
            bejegyzes.second
        );

        if (from != nev.end() &&
            to != nev.end()) {
            atmenetek.push_back({
                from->second,
                bejegyzes.first.second,
                to->second
            });
        }
    }

    // Rendezesi sorrend:
    // 1. Kiindulasi allapot.
    // 2. Bemeneti jel.
    // 3. Celallapot.

    sort(
        atmenetek.begin(),
        atmenetek.end(),
        [](const Atmenet& a, const Atmenet& b) {
            if (a.honnan != b.honnan) {
                return allapot_sorrend(
                    a.honnan, b.honnan
                );
            }

            if (a.betu != b.betu) {
                return a.betu < b.betu;
            }

            return allapot_sorrend(
                a.hova, b.hova
            );
        }
    );

    // Ismetlodo atmenetek eltavolitasa.
    atmenetek.erase(
        unique(
            atmenetek.begin(),
            atmenetek.end(),
            [](const Atmenet& a, const Atmenet& b) {
                return tie(a.honnan, a.betu, a.hova) ==
                       tie(b.honnan, b.betu, b.hova);
            }
        ),
        atmenetek.end()
    );

    ofstream ki(fajlnev);

    if (!ki) {
        throw runtime_error(
            "Nem nyithato meg a kimeneti fajl."
        );
    }

    // Allapotok.
    bool elso = true;

    for (const auto& q : allapotok) {
        if (!elso) {
            ki << ' ';
        }

        ki << q;
        elso = false;
    }

    ki << '\n';

    // Abece.
    for (size_t i = 0; i < a.abc.size(); ++i) {
        if (i) {
            ki << ' ';
        }

        ki << a.abc[i];
    }

    // Kezdoallapot.
    ki << '\n' << nev.at(a.kezdo) << '\n';

    // Vegallapotok.
    elso = true;

    for (const auto& q : vegek) {
        if (!elso) {
            ki << ' ';
        }

        ki << q;
        elso = false;
    }

    ki << '\n';

    // Atmenetek.
    for (const auto& t : atmenetek) {
        ki << t.honnan << ' '
           << t.betu << ' '
           << t.hova << '\n';
    }
}

} // namespace

// =====================================================
// 6. A PROBLEM OSOSZTALY FUGGVENYEI
// =====================================================

// A --mini parancssori kapcsolo regisztralasa.
void MiniProblem::initialize_parser(
    cxxopts::Options& options
) {
    options.add_options()
        ("mini", "Minimalize a deterministic automaton");
}

// Ellenorizzuk, hogy ezt a feladatot valasztottuk-e.
bool MiniProblem::is_chosen_problem(
    const cxxopts::ParseResult& args
) {
    return args.count("mini") > 0;
}

// A feladat elinditasa.
int MiniProblem::run(
    const cxxopts::ParseResult& args
) {
    try {
        if (!args.count("input") ||
            !args.count("output")) {
            cerr
                << "Hiba: --input es --output szukseges.\n";
            return 1;
        }

        string bemenet =
            args["input"].as<string>();

        string kimenet =
            args["output"].as<string>();

        Automata a = beolvas(bemenet);

        map<string, string> nevek =
            minimalizalt_nevek(a);

        kiiras(a, nevek, kimenet);

        cout
            << "Sikeres minimalizalas: "
            << kimenet << '\n';

        return 0;

    } catch (const exception& hiba) {
        cerr << "Hiba: "
             << hiba.what()
             << '\n';

        return 1;
    }
}
