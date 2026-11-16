
#include <iostream>  // Konzolra torteno kiiras
#include <string>    // Szoveges valtozok kezelese
#include <vector>    // Dinamikusan bovitheto tomb

// Kulso konyvtar a parancssori argumentumok feldolgozasahoz
#include "cxxopts.hpp"

// Az elso hazi feladat fejlecfile-ja
#include "problems/dfa.hpp"

// A masodik hazi feladat fejlecfile-ja
#include "problems/det.hpp"

// A harmadik hazi feladat fejlecfile-ja
#include "problems/mini.hpp"

// A projektben eredetileg is szereplo osszeadasi feladat
#include "problems/sum.hpp"
#include "problems/rgx.hpp"


// Ez a fuggveny iranyitja az egesz program mukodeset.
//
// argc: a parancssorban megadott argumentumok szama
// argv: a parancssorban megadott argumentumok tombje
//
// Pelda:
// project --input input1.txt --output output1.txt --det
//
// A fuggveny visszateresi erteke:
// 0 = sikeres futas
// 1 = hiba

int runProblem(int argc, char* argv[]) {

    // Letrehozunk egy vektort, amely a feladatok
    // objektumaira mutato pointereket tarolja.
    //
    // A Problem egy kozos ososztaly.
    // Ennek segitsegevel az egyes feladatokat
    // egyseges modon tudjuk kezelni.

    std::vector<Problem *> problems;

    // Letrehozzuk es regisztraljuk a feladatokat.

    // Eredeti osszeadasi pelda
    problems.push_back(new SumProblem());

    // 1. hazi: szavak ellenorzese DFA-val
    problems.push_back(new DfaProblem());

    // 2. hazi: NFA determinizalasa
    problems.push_back(new DetProblem());

    // 3. hazi: DFA minimalizalasa
// Az oran tanult haromszogmatrixos algoritmussal
problems.push_back(new MiniProblem());
    problems.push_back(new RegexProblem());

    // Letrehozzuk a parancssori argumentumokat
    // feldolgozo objektumot.
    //
    // A "project" a program neve.
    // A masodik parameter a program leirasa.

    cxxopts::Options options(
        "project",
        "Run the specific problem"
    );


    // Definialjuk a minden feladathoz hasznalhato
    // parancssori kapcsolokat.

    options.add_options()

        // -i vagy --input:
        // a bemeneti fajl neve
        ("i,input", "Input file name",
         cxxopts::value<std::string>())

        // -o vagy --output:
        // a kimeneti fajl neve
        ("o,output", "Output file name",
         cxxopts::value<std::string>())

        // -h vagy --help:
        // segitseg megjelenitese
        ("h,help", "Print usage");


    // Minden feladat sajat kapcsoloit regisztraljuk.
    //
    // Pelda:
    // DfaProblem -> --check
    // DetProblem -> --det
    //
    // A for ciklus vegigmegy az osszes feladaton.

    for (Problem *p : problems) {

        // Meghivjuk az aktualis feladat
        // initialize_parser() metodusat.

        p->initialize_parser(options);
    }


    // Feldolgozzuk a parancssorban megadott adatokat.
    //
    // Az eredmenyt az args valtozoban taroljuk.

    cxxopts::ParseResult args =
        options.parse(argc, argv);


    // Megnezzuk, hogy a felhasznalo
    // melyik feladatot szeretne futtatni.

    for (Problem *p : problems) {

        // Az is_chosen_problem() ellenorzi,
        // hogy az adott feladat kapcsoloja szerepel-e.
        //
        // Pelda:
        // Ha --det szerepel, akkor a DetProblem
        // lesz a kivalasztott feladat.

        if (p->is_chosen_problem(args)) {

            // Elinditjuk a kivalasztott feladatot.
            //
            // A run() metodus tartalmazza
            // a feladat tenyleges megoldasat.

            return p->run(args);
        }
    }


    // Ha nem valasztottunk feladatot,
    // megjelenitjuk a hasznalati utmutatot.

    std::cout << options.help() << std::endl;

    return 0;
}


// A main() minden C++ program belepesi pontja.
// A program futtatasa mindig innen indul.

int main(int argc, char* argv[]) {

    // A try blokkban a program normal mukodeset
    // probaljuk vegrehajtani.

    try {

        // Meghivjuk a programot iranyito fuggvenyt.

        return runProblem(argc, argv);

    }

    // Ha a parancssori argumentumok feldolgozasakor
    // hiba tortenik, azt itt kezeljuk.

    catch (const cxxopts::exceptions::exception &e) {

        // Kiirjuk a hiba szoveget a konzolra.

        std::cerr
            << "Error parsing options: "
            << e.what()
            << std::endl;

        // Hibakoddal fejezzuk be a programot.

        return 1;
    }
}
