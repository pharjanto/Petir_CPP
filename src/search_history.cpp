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
