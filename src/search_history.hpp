#pragma once
// History heuristics for quiet moves, used by the main search (search.cpp).
//
// - Butterfly counters, historyTried and historySucceeded [piece][square]: how
//   often a quiet move was searched and how often it caused a cutoff, each
//   weighted by plies left squared. The search reduces moves that rarely cut off.
// - History scores, whiteHistory and blackHistory [piece][to-square], and the
//   continuation-history tables (keyed by the previous one or two moves): they
//   order quiet moves in the move picker. updateHistory and updateContinuation
//   keep them within +/-historyLimit.
//
// The tables and the functions other modules call (clearHistoryTables,
// addHistory, updateHistory, updateContinuation, penaliseQuiets) are declared
// in petir.hpp. The helpers only the search needs are declared here.
#include "petir.hpp"

namespace petir_search {
// The bonus for a quiet move that was best or cut off with plies left: the
// original plies*plies step, capped well inside the history limit.
std::int32_t historyBonus(std::int32_t plies);

// The continuation-history table of the move made at ply; none above the
// root or after a null move.
petir_header::piece_to_history* continuationTable(std::int32_t ply);

// The square under which historyTried and historySucceeded file a quiet move:
// the king's destination for castling, the to-square otherwise.
std::int32_t historySquare(std::int32_t side, std::int32_t toSquare);

// Static-evaluation correction: the average error of the static evaluation
// (search score minus evaluation) seen at earlier nodes, from two tables keyed
// by the side to move and either the pawn structure or the material signature.
// correctedEval adds it to staticEval, a static evaluation of position from
// sideToMove's side; a dead-draw score is left alone. updateEvalCorrection
// moves the entries toward score - staticEval, faster with more plies
// searched. evalCorrection is the correction alone.
std::int32_t evalCorrection(std::int32_t sideToMove, const petir_header::tdata& position);
std::int32_t correctedEval(std::int32_t sideToMove, const petir_header::tdata& position, std::int32_t staticEval);
void updateEvalCorrection(std::int32_t sideToMove, const petir_header::tdata& position, std::int32_t staticEval, std::int32_t score, std::int32_t plies);
}
