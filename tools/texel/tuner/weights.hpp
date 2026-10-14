// The weights of Petir's evaluation that the Texel tuner fits. Built against
// Petir with PETIR_EVAL_TUNING (see CMakeLists.txt), which turns the scalar
// weights into variables and makes the pawn caches recompute every entry.
#pragma once
#include "engine.hpp"
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace texel {

// One tunable number. It can stand for several stored values that must stay
// equal: a White table entry and the mirrored Black one, the kingside and
// queenside endgame king tables, or every square of a file in the rook table.
// For a tapered table (see taper in petir.hpp) it is the middlegame (name.mg)
// or the endgame (name.eg) value of those stored values.
struct Weight {
    std::string name;
    // The range the stored values can hold. A weight that the evaluation
    // halves with a shift must stay non-negative.
    int minimum = -1000, maximum = 1000;
    std::function<int()> get;
    std::function<void(int)> set;
    // False when the weight cannot count in the position: a square-table entry
    // whose square holds no such piece. Empty when it may always count.
    std::function<bool(const petir::Position&)> mayCount;
    // Material weights (1 pawn ... 5 queen) are not read by the evaluation but
    // kept in the position's running material score; get and set only hold the
    // value.
    int piece = 0;
};

// Every tunable weight, in a fixed order. Exits with a message when tables
// that a weight ties together are not mirrored as expected.
std::vector<Weight> evalWeights();

// Petir's static evaluation from White's view with the full window.
int whiteEval(const petir::Position& position, int sideToMove);

// The position with its material score changed as if the piece values had
// changed by `change` (indexed by piece, 1 pawn ... 5 queen).
petir::Position withMaterialChange(const petir::Position& position, const int change[6]);

// How much the evaluation (White's view) changes per point of each weight, as
// (weight index, coefficient) pairs for the weights whose coefficient is not
// zero. Each weight is changed by `step` (or by -step at the top of its
// range), and the evaluation difference is divided by the step. With
// everyWeight false, weights whose mayCount is false are skipped.
std::vector<std::pair<int, double>> coefficients(std::vector<Weight>& weights, const petir::Position& position,
                                                 int sideToMove, int baseEval, int step, bool everyWeight);

}
