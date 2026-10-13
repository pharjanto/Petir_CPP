// Records how Petir's evaluation depends on each tunable weight, for the Texel
// tuner. The evaluation is linear in almost all of its weights, so a position's
// score with changed weights w' is predicted as
//   eval(w') = eval(w) + sum of coefficient[i] * (w'[i] - w[i]).
// A coefficient is measured by changing one weight and evaluating again, so it
// follows the evaluation code as it is, including scaling and draw rules.
//
// Usage:
//   eval_trace --list OUT
//       Writes the weights, one per line: index, name, value, minimum, maximum.
//   eval_trace FENS OUT [--step N] [--every-weight]
//       For each FEN writes one line: the static evaluation from White's view,
//       then index:coefficient for each weight whose coefficient is not zero,
//       each a tab apart. A coefficient is measured by changing the weight by N
//       points (default 16). --every-weight also measures the square-table
//       entries whose square is empty (for checking the shortcut). An
//       unreadable FEN gives the line "error".
//   eval_trace --check FENS [--change N] [--step N] [--seed S]
//       Tests the linear model: for each position, changes every weight by a
//       random amount of at most N points (default 2) and compares the
//       evaluation with the one the coefficients predict.
#include "weights.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace {
// Reads one FEN. Returns false when Petir cannot use it.
bool readPosition(std::string fen, petir::Position& position, int& sideToMove) {
    if (!fen.empty() && fen.back() == '\r') fen.pop_back();
    try {
        position = petir::fromFen(fen, sideToMove);
    } catch (const std::exception&) {
        return false;
    }
    return petir::isConsistent(position);
}

std::string coefficientText(double coefficient) {
    char text[32];
    std::snprintf(text, sizeof text, "%.6g", coefficient);
    return text;
}

int listWeights(const char* outName) {
    auto weights = texel::evalWeights();
    std::ofstream out(outName);
    for (std::size_t index = 0; index < weights.size(); ++index) {
        const auto& weight = weights[index];
        out << index << '\t' << weight.name << '\t' << weight.get() << '\t' << weight.minimum << '\t' << weight.maximum << '\n';
    }
    std::cerr << weights.size() << " weights\n";
    return 0;
}

int tracePositions(const char* fenName, const char* outName, int step, bool everyWeight) {
    auto weights = texel::evalWeights();
    std::ifstream fens(fenName);
    std::ofstream out(outName);
    if (!fens || !out) {
        std::cerr << "cannot open " << (fens ? outName : fenName) << '\n';
        return 2;
    }
    std::string fen;
    long count = 0;
    while (std::getline(fens, fen)) {
        if (fen.empty() || fen == "\r") continue;
        petir::Position position{};
        int sideToMove = 1;
        if (!readPosition(fen, position, sideToMove)) {
            out << "error\n";
            continue;
        }
        const int eval = texel::whiteEval(position, sideToMove);
        out << eval;
        for (const auto& [index, coefficient] : texel::coefficients(weights, position, sideToMove, eval, step, everyWeight))
            out << '\t' << index << ':' << coefficientText(coefficient);
        out << '\n';
        if (++count % 10000 == 0) std::cerr << "  " << count << " positions\n";
    }
    return 0;
}

int checkLinearModel(const char* fenName, int maximumChange, int step, unsigned seed) {
    auto weights = texel::evalWeights();
    std::ifstream fens(fenName);
    if (!fens) {
        std::cerr << "cannot open " << fenName << '\n';
        return 2;
    }
    std::mt19937 random(seed);
    std::uniform_int_distribution<int> changeOf(-maximumChange, maximumChange);
    std::vector<int> original(weights.size()), change(weights.size());
    for (std::size_t index = 0; index < weights.size(); ++index) original[index] = weights[index].get();

    long positions = 0, exact = 0, withinOne = 0, withinFour = 0;
    double errorSum = 0, movedSum = 0, worstError = -1;
    std::string fen, worstFen;
    while (std::getline(fens, fen)) {
        if (fen.empty() || fen == "\r") continue;
        petir::Position position{};
        int sideToMove = 1;
        if (!readPosition(fen, position, sideToMove)) continue;
        const int eval = texel::whiteEval(position, sideToMove);
        const auto terms = texel::coefficients(weights, position, sideToMove, eval, step, false);

        int material[6] = {};
        for (std::size_t index = 0; index < weights.size(); ++index) {
            auto& weight = weights[index];
            int changed = original[index] + changeOf(random);
            changed = std::clamp(changed, weight.minimum, weight.maximum);
            change[index] = changed - original[index];
            if (weight.piece) material[weight.piece] = change[index];
            else weight.set(changed);
        }
        const int changedEval = texel::whiteEval(texel::withMaterialChange(position, material), sideToMove);
        for (std::size_t index = 0; index < weights.size(); ++index)
            if (!weights[index].piece) weights[index].set(original[index]);

        double predicted = eval;
        for (const auto& [index, coefficient] : terms) predicted += coefficient * change[index];
        const double error = std::abs(changedEval - predicted);
        ++positions;
        errorSum += error;
        movedSum += std::abs(changedEval - eval);
        exact += error < 0.5;
        withinOne += error <= 1.5;
        withinFour += error <= 4.5;
        if (error > worstError) {
            worstError = error;
            worstFen = fen;
        }
    }
    if (!positions) {
        std::cerr << "no positions\n";
        return 2;
    }
    std::printf("%ld positions; mean change of the evaluation %.1f cp\n", positions, movedSum / positions);
    std::printf("prediction error: mean %.2f cp; within 0: %.1f%%, within 1: %.1f%%, within 4: %.1f%%\n",
                errorSum / positions, 100.0 * exact / positions, 100.0 * withinOne / positions,
                100.0 * withinFour / positions);
    std::printf("largest error %.1f cp: %s\n", worstError, worstFen.c_str());
    return 0;
}
}

int main(int argc, char** argv) {
    std::vector<std::string> args(argv + 1, argv + argc);
    // No opening book is needed: a directory without book files skips it.
    petir::initialize("no-book");
    auto option = [&](const std::string& name, int fallback) {
        for (std::size_t index = 0; index + 1 < args.size(); ++index)
            if (args[index] == name) return std::stoi(args[index + 1]);
        return fallback;
    };
    auto flag = [&](const std::string& name) {
        for (const auto& arg : args)
            if (arg == name) return true;
        return false;
    };
    if (args.size() == 2 && args[0] == "--list") return listWeights(args[1].c_str());
    if (args.size() >= 2 && args[0] == "--check")
        return checkLinearModel(args[1].c_str(), option("--change", 2), option("--step", 16),
                                static_cast<unsigned>(option("--seed", 1)));
    if (args.size() >= 2 && args[0].rfind("--", 0) != 0 && args[1].rfind("--", 0) != 0)
        return tracePositions(args[0].c_str(), args[1].c_str(), option("--step", 16), flag("--every-weight"));
    std::cerr << "usage: eval_trace --list OUT\n"
                 "       eval_trace FENS OUT [--step N] [--every-weight]\n"
                 "       eval_trace --check FENS [--change N] [--step N] [--seed S]\n";
    return 2;
}
