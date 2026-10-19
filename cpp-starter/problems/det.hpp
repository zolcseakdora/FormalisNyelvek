
#ifndef PROBLEMS_DET_H
#define PROBLEMS_DET_H

#include "../problem.hpp"
#include "../cxxopts.hpp"

class DetProblem : public Problem {
public:
    void initialize_parser(cxxopts::Options &options) override;
    bool is_chosen_problem(const cxxopts::ParseResult &args) override;
    int run(const cxxopts::ParseResult &args) override;
};

#endif
