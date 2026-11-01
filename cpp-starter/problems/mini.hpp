
#pragma once

// =====================================================
// 3. HAZI FELADAT
// DETERMINISZTIKUS AUTOMATA MINIMALIZALASA
// =====================================================

// A kozos Problem ososztaly
#include "../problem.hpp"

// A MiniProblem osztaly a harmadik hazi
// feladat mukodeset valositja meg.

class MiniProblem : public Problem {
public:

    // Regisztralja a --mini kapcsolot
    void initialize_parser(
        cxxopts::Options& options
    ) override;

    // Ellenorzi, hogy a --mini kapcsolot
    // adtak-e meg a parancssorban
    bool is_chosen_problem(
        const cxxopts::ParseResult& args
    ) override;

    // Beolvassa az automatat,
    // elvegzi a minimalizalast,
    // es fajlba irja az eredmenyt
    int run(
        const cxxopts::ParseResult& args
    ) override;
};
