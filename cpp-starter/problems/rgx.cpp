#include "rgx.hpp"

#include <fstream>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

namespace {

const string HEAD = R"(^\S+ \S+ +\d+ +\d+ )";

const vector<regex> TASK1_PATTERNS = {
    regex(R"(^10-15 )"),
    regex(R"(^10-15 10:18:51\.)"),
    regex(R"(^\S+ \d\d:\d\d:51\.)"),
    regex(HEAD + R"([IDVEFAW] lowmemorykiller *:)"),
    regex(R"(^\S+ 10:18:51\.\d+ +221 +\d+ )"),
    regex(HEAD + R"(E )"),
    regex(HEAD + R"(W PackageManager:)"),
    regex(HEAD + R"(D .*ExoPlayer.*)"),
    regex(HEAD + R"([EW] .*\.java:)"),
    regex(HEAD + R"([IDVEFAW] (.+?):.*Thread.*)")
};

const regex TASK2_PATTERN(HEAD + R"(E .*\((\w+\.java):\d+\))");
const regex TASK3_PATTERN(R"(^\S+ \S+ +(\d+) )");
const regex TASK4_PATTERN(R"(^\S+ \S+ +(\d+) +\d+ E )");
const regex TASK5_PATTERN(R"(^\S+ \S+ +(\d+) +(\d+) E )");

vector<string> task1(const vector<string>& lines) {
    vector<string> result;
    for (const regex& pattern : TASK1_PATTERNS) {
        int count = 0;
        for (const string& line : lines) {
            if (regex_search(line, pattern)) {
                ++count;
            }
        }
        result.push_back(to_string(count));
    }
    return result;
}

vector<string> task2(const vector<string>& lines) {
    vector<string> result;
    smatch match;
    for (const string& line : lines) {
        if (regex_search(line, match, TASK2_PATTERN)) {
            result.push_back(match[1].str());
        }
    }
    return result;
}

vector<string> task3(const vector<string>& lines) {
    unordered_set<string> seen;
    vector<string> result;
    smatch match;
    for (const string& line : lines) {
        if (regex_search(line, match, TASK3_PATTERN)) {
            string pid = match[1].str();
            if (seen.insert(pid).second) {
                result.push_back(pid);
            }
        }
    }
    return result;
}

vector<string> task4(const vector<string>& lines) {
    unordered_map<string, int> counts;
    vector<string> pidOrder;
    smatch match;

    for (const string& line : lines) {
        if (regex_search(line, match, TASK4_PATTERN)) {
            string pid = match[1].str();
            if (counts.find(pid) == counts.end()) {
                pidOrder.push_back(pid);
            }
            ++counts[pid];
        }
    }

    if (pidOrder.empty()) {
        throw runtime_error("Nincs E szintu bejegyzes a 4. feladathoz.");
    }

    string selectedPid = pidOrder[0];
    for (const string& pid : pidOrder) {
        if (counts[pid] > counts[selectedPid]) {
            selectedPid = pid;
        }
    }

    return {"PID=" + selectedPid + " (" + to_string(counts[selectedPid]) + " errors)"};
}

vector<string> task5(const vector<string>& lines) {
    int count = 0;
    smatch match;
    for (const string& line : lines) {
        if (regex_search(line, match, TASK5_PATTERN) &&
            match[1].str() == match[2].str()) {
            ++count;
        }
    }
    return {to_string(count)};
}

} // namespace

void RegexProblem::initialize_parser(cxxopts::Options& options) {
    options.add_options()
        ("rgx", "Solve regex problems (1-5)", cxxopts::value<string>());
}

bool RegexProblem::is_chosen_problem(const cxxopts::ParseResult& args) {
    return args.count("rgx") > 0;
}

int RegexProblem::run(const cxxopts::ParseResult& args) {
    try {
        if (args.count("input") == 0 || args.count("output") == 0) {
            throw runtime_error("Add meg a --input es --output parametereket.");
        }

        string taskText = args["rgx"].as<string>();
        size_t used = 0;
        int taskNumber = stoi(taskText, &used);
        if (used != taskText.size() || taskNumber < 1 || taskNumber > 5) {
            throw runtime_error("A --rgx erteke 1 es 5 kozott legyen.");
        }

        string inputFile = args["input"].as<string>();
        string outputFile = args["output"].as<string>();

        ifstream input(inputFile);
        if (!input) {
            throw runtime_error("Nem sikerult megnyitni: " + inputFile);
        }

        vector<string> lines;
        string line;
        while (getline(input, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            lines.push_back(line);
        }

        vector<string> result;
        switch (taskNumber) {
            case 1: result = task1(lines); break;
            case 2: result = task2(lines); break;
            case 3: result = task3(lines); break;
            case 4: result = task4(lines); break;
            case 5: result = task5(lines); break;
        }

        ofstream output(outputFile);
        if (!output) {
            throw runtime_error("Nem sikerult letrehozni: " + outputFile);
        }
        for (const string& value : result) {
            output << value << '\n';
        }
        if (result.empty()) {
            output << '\n';
        }
        if (!output) {
            throw runtime_error("Hiba a kimeneti fajl irasakor.");
        }
        return 0;
    } catch (const exception& e) {
        cerr << "Regex hiba: " << e.what() << '\n';
        return 1;
    }
}
