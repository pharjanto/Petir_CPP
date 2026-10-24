// History heuristics for quiet moves; see search_history.hpp.
#include "search_history.hpp"
#include "search_tools.hpp"
#include <algorithm>
#include <cstdlib>

namespace petir_search {
pas::Array<pas::Array<std::int32_t, 0, 63>, pas::negate(6), 6> historyTried{};
pas::Array<pas::Array<std::int32_t, 0, 63>, pas::negate(6), 6> historySucceeded{};

// The change for a bonus or penalty: it moves the entry toward +historyLimit or
// -historyLimit and shrinks as the entry nears that limit.
static std::int32_t historyStep(std::int32_t entry, std::int32_t bonus) {return bonus - entry * std::abs(bonus) / petir_header::historyLimit;}

std::int32_t historyBonus(std::int32_t plies) {return std::min(pas::mul(plies, plies), petir_header::historyLimit / 4);}

petir_header::piece_to_history* continuationTable(std::int32_t ply) {
    if ((ply < 0) || (currentMove[ply] == petir_header::noMove)) return nullptr;
    return &petir_header::continuationHistory[currentPiece[ply]][moveToSquare(currentMove[ply])];
}

std::int32_t historySquare(std::int32_t side, std::int32_t toSquare) {
    const bool isWhite = (side == petir_header::whiteSide);
    if (toSquare == petir_header::longCastleTarget) return isWhite ? petir_header::c1 : petir_header::c8;
    if (toSquare == petir_header::shortCastleTarget) return isWhite ? petir_header::g1 : petir_header::g8;
    return toSquare;
}

// search.pas:44
void clearHistoryTables() {
    pas::fillChar(historyTried, sizeof(historyTried), 0);
    pas::fillChar(historySucceeded, sizeof(historySucceeded), 0);
}

// Correction entries hold the error times correctionGrain, within
// +/-correctionLimit (64 points of evaluation). Two tables, one keyed by the
// pawn structure and one by the material signature (the count of each piece
// type for each side), are read as one correction: the sum of their entries,
// within the same limit.
static constexpr std::int32_t correctionIndexBits = 14;
static constexpr std::int32_t correctionGrain = 256;
static constexpr std::int32_t correctionLimit = 64 * correctionGrain;
using CorrectionTable = pas::Array<pas::Array<std::int16_t, 0, (1 << correctionIndexBits) - 1>, petir_header::whiteSide, petir_header::blackSide>;
static CorrectionTable evalCorrectionTable{};
static CorrectionTable materialCorrectionTable{};

static std::int16_t& correctionEntry(std::int32_t sideToMove, const petir_header::tdata& position) {
    const std::uint64_t pawnKey = (position.whitePawns * UINT64_C(0x9e3779b97f4a7c15)) ^ (position.blackPawns * UINT64_C(0xc2b2ae3d27d4eb4f));
    return evalCorrectionTable[sideToMove][static_cast<std::int32_t>(pawnKey >> (64 - correctionIndexBits))];
}

static std::int16_t& materialCorrectionEntry(std::int32_t sideToMove, const petir_header::tdata& position) {
    return materialCorrectionTable[sideToMove][static_cast<std::int32_t>((position.materialSignature * UINT64_C(0x9e3779b97f4a7c15)) >> (64 - correctionIndexBits))];
}

void clearEvalCorrection() {
    pas::fillChar(evalCorrectionTable, sizeof(evalCorrectionTable), 0);
    pas::fillChar(materialCorrectionTable, sizeof(materialCorrectionTable), 0);
}

std::int32_t evalCorrection(std::int32_t sideToMove, const petir_header::tdata& position) {
    const std::int32_t sum = correctionEntry(sideToMove, position) + materialCorrectionEntry(sideToMove, position);
    return std::clamp(sum, -correctionLimit, correctionLimit) / correctionGrain;
}

std::int32_t correctedEval(std::int32_t sideToMove, const petir_header::tdata& position, std::int32_t staticEval) {
    if (staticEval == petir_header::drawScore) return staticEval;
    return staticEval + evalCorrection(sideToMove, position);
}

// Each table moves toward the part of the error the other one leaves.
void updateEvalCorrection(std::int32_t sideToMove, const petir_header::tdata& position, std::int32_t staticEval, std::int32_t score, std::int32_t plies) {
    if (staticEval == petir_header::drawScore) return;
    auto& pawnEntry = correctionEntry(sideToMove, position);
    auto& materialEntry = materialCorrectionEntry(sideToMove, position);
    const std::int32_t weight = std::min(plies + 1, 16);
    const std::int32_t error = (score - staticEval) * correctionGrain;
    const std::int32_t pawnTarget = std::clamp(error - materialEntry, -correctionLimit, correctionLimit);
    const std::int32_t materialTarget = std::clamp(error - pawnEntry, -correctionLimit, correctionLimit);
    pawnEntry = static_cast<std::int16_t>((pawnEntry * (256 - weight) + pawnTarget * weight) / 256);
    materialEntry = static_cast<std::int16_t>((materialEntry * (256 - weight) + materialTarget * weight) / 256);
}

// Moves the entry toward +historyLimit for a bonus and toward -historyLimit for
// a penalty; the step shrinks as the entry nears that limit.
void updateHistory(std::int32_t sideToMove, std::int32_t piece, std::int32_t toSquare, std::int32_t bonus) {
    auto& entry = (sideToMove == petir_header::whiteSide ? petir_header::whiteHistory : petir_header::blackHistory)[piece][toSquare];
    entry += historyStep(entry, bonus);
}

// The same update for the move's entry in each continuation-history table.
void updateContinuation(petir_header::piece_to_history* const* continuationTables, std::int32_t piece, std::int32_t toSquare, std::int32_t bonus) {
    for (std::int32_t tableIndex = 0; tableIndex < 2; ++tableIndex) {
        if (continuationTables[tableIndex] != nullptr) {
            auto& entry = (*continuationTables[tableIndex])[piece][toSquare];
            entry = static_cast<std::int16_t>(entry + historyStep(entry, bonus));
        }
    }
}

// The other quiet moves searched at the node lose what the best one gains.
void penaliseQuiets(std::int32_t plies, std::int32_t sideToMove, const petir_header::tdata& position, petir_header::piece_to_history* const* continuationTables, const std::int32_t* quietMoves, std::int32_t quietCount, std::int32_t bestMove) {
    for (std::int32_t index = 0; index < quietCount; ++index) {
        if (quietMoves[index] != bestMove) {
            const std::int32_t piece = position.board[moveFromSquare(quietMoves[index])];
            const std::int32_t toSquare = moveToSquare(quietMoves[index]);
            updateHistory(sideToMove, piece, toSquare, -historyBonus(plies));
            updateContinuation(continuationTables, piece, toSquare, -historyBonus(plies));
        }
    }
}

// search.pas:51
// Credits a quiet move that caused a cutoff and makes it the killer for its ply.
void addHistory(std::int32_t plies, std::int32_t ply, std::int32_t move, std::int32_t sideToMove, std::int32_t piece) {
    updateHistory(sideToMove, piece, moveToSquare(move), historyBonus(plies));
    petir_header::firstKillers[ply] = move;
}
}
