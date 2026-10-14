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
//   eval_trace --lazy FENS
//       Counts lazy exits on the wrong side of the window: for each position,
//       null windows from 1300 below to 1300 above the full evaluation in
//       steps of 10, where the null-window result and the full evaluation
//       fall on different sides of alpha.
// Each form takes --weights FILE: the weights are first set to the values in
// FILE, written like --list (a fit's output), instead of the engine's own.
#include "weights.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
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

// The weights, with the values from --weights when given. The evaluation does
// not read the piece values but the position's running material score, so
// changed piece values are kept in `material` and applied to each position.
struct Weights {
    std::vector<texel::Weight> list = texel::evalWeights();
    int material[6] = {};

    petir::Position applied(const petir::Position& position) const {
        return texel::withMaterialChange(position, material);
    }
};

// Sets the weights to the values in a file written like --list. Every weight
// must be there, by name and in the same order, within its range.
bool loadWeights(Weights& weights, const std::string& fileName) {
    std::ifstream file(fileName);
    if (!file) {
        std::cerr << "cannot open " << fileName << '\n';
        return false;
    }
    std::string line;
    std::size_t index = 0;
    while (std::getline(file, line)) {
        if (line.empty() || line == "\r") continue;
        std::istringstream fields(line);
        std::size_t listed;
        std::string name;
        int value;
        if (!(fields >> listed >> name >> value) || index >= weights.list.size() || listed != index
            || name != weights.list[index].name) {
            std::cerr << fileName << ": line " << index + 1 << " does not match weight " << index << " ("
                      << (index < weights.list.size() ? weights.list[index].name : "none") << ")\n";
            return false;
        }
        auto& weight = weights.list[index];
        if (value < weight.minimum || value > weight.maximum) {
            std::cerr << fileName << ": " << name << " = " << value << " is outside " << weight.minimum << ".."
                      << weight.maximum << '\n';
            return false;
        }
        if (weight.piece) weights.material[weight.piece] = value - weight.get();
        weight.set(value);
        ++index;
    }
    if (index != weights.list.size()) {
        std::cerr << fileName << ": " << index << " weights, expected " << weights.list.size() << '\n';
        return false;
    }
    return true;
}

int listWeights(const Weights& weights, const std::string& outName) {
    std::ofstream out(outName);
    for (std::size_t index = 0; index < weights.list.size(); ++index) {
        const auto& weight = weights.list[index];
        out << index << '\t' << weight.name << '\t' << weight.get() << '\t' << weight.minimum << '\t' << weight.maximum << '\n';
    }
    std::cerr << weights.list.size() << " weights\n";
    return 0;
}

int tracePositions(Weights& weights, const std::string& fenName, const std::string& outName, int step, bool everyWeight) {
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
        position = weights.applied(position);
        const int eval = texel::whiteEval(position, sideToMove);
        out << eval;
        for (const auto& [index, coefficient] : texel::coefficients(weights.list, position, sideToMove, eval, step, everyWeight))
            out << '\t' << index << ':' << coefficientText(coefficient);
        out << '\n';
        if (++count % 10000 == 0) std::cerr << "  " << count << " positions\n";
    }
    return 0;
}

int checkLinearModel(Weights& weights, const std::string& fenName, int maximumChange, int step, unsigned seed) {
    std::ifstream fens(fenName);
    if (!fens) {
        std::cerr << "cannot open " << fenName << '\n';
        return 2;
    }
    auto& list = weights.list;
    std::mt19937 random(seed);
    std::uniform_int_distribution<int> changeOf(-maximumChange, maximumChange);
    std::vector<int> original(list.size()), change(list.size());
    for (std::size_t index = 0; index < list.size(); ++index) original[index] = list[index].get();

    long positions = 0, exact = 0, withinOne = 0, withinFour = 0;
    double errorSum = 0, movedSum = 0, worstError = -1;
    std::string fen, worstFen;
    while (std::getline(fens, fen)) {
        if (fen.empty() || fen == "\r") continue;
        petir::Position position{};
        int sideToMove = 1;
        if (!readPosition(fen, position, sideToMove)) continue;
        position = weights.applied(position);
        const int eval = texel::whiteEval(position, sideToMove);
        const auto terms = texel::coefficients(list, position, sideToMove, eval, step, false);

        int material[6] = {};
        for (std::size_t index = 0; index < list.size(); ++index) {
            auto& weight = list[index];
            int changed = original[index] + changeOf(random);
            changed = std::clamp(changed, weight.minimum, weight.maximum);
            change[index] = changed - original[index];
            if (weight.piece) material[weight.piece] = change[index];
            else weight.set(changed);
        }
        const int changedEval = texel::whiteEval(texel::withMaterialChange(position, material), sideToMove);
        for (std::size_t index = 0; index < list.size(); ++index)
            if (!list[index].piece) list[index].set(original[index]);

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

int auditLazyExits(Weights& weights, const std::string& fenName) {
    std::ifstream fens(fenName);
    if (!fens) {
        std::cerr << "cannot open " << fenName << '\n';
        return 2;
    }
    constexpr int fullWindow = 20500, reach = 1300, stride = 10;
    long positions = 0, positionsWrong = 0;
    long long windows = 0, wrong = 0;
    std::string fen;
    while (std::getline(fens, fen)) {
        if (fen.empty() || fen == "\r") continue;
        petir::Position position{};
        int sideToMove = 1;
        if (!readPosition(fen, position, sideToMove)) continue;
        position = weights.applied(position);
        const int full = petir_ueval::evaluate(position, sideToMove, -fullWindow, fullWindow);
        long wrongHere = 0;
        for (int alpha = full - reach; alpha <= full + reach; alpha += stride) {
            const int narrow = petir_ueval::evaluate(position, sideToMove, alpha, alpha + 1);
            wrongHere += (narrow > alpha) != (full > alpha);
            ++windows;
        }
        ++positions;
        wrong += wrongHere;
        positionsWrong += wrongHere > 0;
    }
    std::printf("%ld positions, %lld null windows: %lld wrong lazy exits (%.4f%%), in %ld positions (%.3f%%)\n", positions,
                windows, wrong, 100.0 * wrong / std::max(1LL, windows), positionsWrong,
                100.0 * positionsWrong / std::max(1L, positions));
    return 0;
}

int main(int argc, char** argv) {
    // No opening book is needed: a directory without book files skips it.
    petir::initialize("no-book");
    std::vector<std::string> positional;
    std::string weightsFile;
    std::string step = "16", change = "2", seed = "1";
    bool everyWeight = false, list = false, check = false, lazy = false, valid = true;
    for (int index = 1; index < argc; ++index) {
        const std::string arg = argv[index];
        const bool hasValue = index + 1 < argc;
        if (arg == "--every-weight") everyWeight = true;
        else if (arg == "--list") list = true;
        else if (arg == "--check") check = true;
        else if (arg == "--lazy") lazy = true;
        else if (arg == "--weights" && hasValue) weightsFile = argv[++index];
        else if (arg == "--step" && hasValue) step = argv[++index];
        else if (arg == "--change" && hasValue) change = argv[++index];
        else if (arg == "--seed" && hasValue) seed = argv[++index];
        else if (arg.rfind("--", 0) == 0) valid = false;
        else positional.push_back(arg);
    }

    Weights weights;
    if (!weightsFile.empty() && !loadWeights(weights, weightsFile)) return 2;
    if (valid && lazy && !list && !check && positional.size() == 1) return auditLazyExits(weights, positional[0]);
    if (valid && list && !check && positional.size() == 1) return listWeights(weights, positional[0]);
    if (valid && check && !list && positional.size() == 1)
        return checkLinearModel(weights, positional[0], std::stoi(change), std::stoi(step),
                                static_cast<unsigned>(std::stoul(seed)));
    if (valid && !list && !check && positional.size() == 2)
        return tracePositions(weights, positional[0], positional[1], std::stoi(step), everyWeight);
    std::cerr << "usage: eval_trace --list OUT [--weights FILE]\n"
                 "       eval_trace FENS OUT [--step N] [--every-weight] [--weights FILE]\n"
                 "       eval_trace --check FENS [--change N] [--step N] [--seed S] [--weights FILE]\n"
                 "       eval_trace --lazy FENS [--weights FILE]\n";
    return 2;
}
