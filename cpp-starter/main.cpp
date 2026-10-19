#include <iostream>
#include <string>
#include <vector>

// Külső könyvtár a parancssori argumentumok (pl. -i, -o) kényelmes feldolgozásához
#include "cxxopts.hpp"
// A megvalósított feladatok (osztályok) fejlécfájljainak beillesztése
#include "problems/dfa.hpp"
#include "problems/det.hpp"

// Ide kerülnek az újonnan létrehozott feladatok fejlécei
#include "problems/sum.hpp"

// Ez a függvény fogja össze a program logikáját
int runProblem(int argc, char* argv[]) {

    // 1. FELADATOK REGISZTRÁLÁSA
    // Létrehozunk egy dinamikus tömböt (vektort), amely a Problem ősosztályból származó objektumokat tárolja.
    std::vector<Problem *> problems;

    // Példányosítjuk és listához adjuk a konkrét feladatokat.
    // Ha új feladatot készítesz, azt itt kell beregisztrálni (pl. new SajatFeladat()).
    problems.push_back(new SumProblem());
    problems.push_back(new DfaProblem());
    problems.push_back(new DetProblem());

    // 2. PARANCSSORI OPCIÓK BEÁLLÍTÁSA
    // Létrehozzuk a parser (elemző) objektumot a projekt nevével és rövid leírásával.
    cxxopts::Options options("project", "Run the specific problem");

    // Definiáljuk az alapvető, globális kapcsolókat, amiket minden feladat használhat:
    // -i vagy --input: bemeneti fájl
    // -o vagy --output: kimeneti fájl
    // -h vagy --help: súgó megjelenítése
    options.add_options()
        ("i,input", "Input file name", cxxopts::value<std::string>())
        ("o,output", "Output file name", cxxopts::value<std::string>())
        ("h,help", "Print usage");

    // 3. FELADATSPECIFIKUS OPCIÓK INICIALIZÁLÁSA
    // Végigmegyünk a beregisztrált problémákon, így mindegyik hozzáadhatja a saját speciális parancssori kapcsolóit az 'options' objektumhoz.
    for (Problem *p : problems) {
        p->initialize_parser(options);
    }

    // 4. ARGUMENTUMOK FELDOLGOZÁSA
    // A program itt értelmezi, hogy mit gépelt be a felhasználó a parancssorba futtatáskor (pl. "./project -i adat.txt").
    cxxopts::ParseResult args = options.parse(argc, argv);

    // 5. A MEGFELELŐ FELADAT KIVÁLASZTÁSA ÉS FUTTATÁSA
    for (Problem *p : problems) {
        // Megkérdezzük az aktuális feladatot, hogy a megadott paraméterek alapján őt akarják-e futtatni.
        if (p->is_chosen_problem(args)) {
            // Ha igen, elindítjuk a feladat tényleges megoldását végző 'run' metódust.
             return p->run(args);
        }
    }

    // 6. SÚGÓ MEGJELENÍTÉSE
    // Ha egyik feladatot sem választották (vagy a -h kapcsolót adták meg), a program automatikusan kiírja a használati útmutatót.
    std::cout << options.help() << std::endl;
    return 0;
}

// A program tényleges belépési pontja
int main(int argc, char* argv[]) {
    // A try-catch blokk (kivételkezelés) arra szolgál, hogy ha hibás parancssori paramétert adnak meg,
    // a program ne omoljon össze, hanem elegánsan kezelje azt.
    try {
        // Meghívjuk a fent megírt fő logikát.
        return runProblem(argc, argv);
    } catch (const cxxopts::exceptions::exception &e) {
        // Ha az argumentum-elemző (cxxopts) hibát észlel (pl. hiányzó érték egy kapcsoló után),
        // itt elkapjuk a hibát, és kulturáltan kiírjuk a felhasználónak, majd hiba-kóddal (1) kilépünk.
        std::cerr << "Error parsing options: " << e.what() << std::endl;
        return 1;
    }
}