// Helpers for the main search; see search_tools.hpp.
#include "search_tools.hpp"

namespace petir_search {
bool isInCheck(std::int32_t side, petir_header::tdata& position) {
    if (side == petir_header::whiteSide) return petir_tools::whiteInCheck(position);
    return petir_tools::blackInCheck(position);
}

void makeSideMove(std::int32_t side, std::int32_t move, petir_header::tdata& position) {
    if (side == petir_header::whiteSide) {
        petir_makemove::makeWhiteMove(move, position);
    } else {
        petir_makemove::makeBlackMove(move, position);
    }
}

bool leavesKingAttacked(std::int32_t side, petir_header::tdata& position, std::int32_t move, std::uint64_t pinnedMask) {
    const std::int32_t fromSquare = moveFromSquare(move);
    const std::int32_t toSquare = moveToSquare(move);
    const bool isPinned = (pinnedMask & petir_bitboard_mask::squareBit(fromSquare)) != 0;
    if (side == petir_header::whiteSide) {
        if (position.board[fromSquare] == petir_header::whiteKing) {
            return petir_tools::attackedByBlack(position, toSquare);
        }
        return isPinned && petir_tools::exposesWhiteKing(position, move);
    }
    if (position.board[fromSquare] == petir_header::blackKing) {
        return petir_tools::attackedByWhite(position, toSquare);
    }
    return isPinned && petir_tools::exposesBlackKing(position, move);
}

// exposesBlackKing asks whether moving the piece uncovers an attack on the
// black king; for a White move that is a discovered check.
bool givesDiscoveredCheck(std::int32_t side, petir_header::tdata& position, std::int32_t move) {
    if (side == petir_header::whiteSide) return petir_tools::exposesBlackKing(position, move);
    return petir_tools::exposesWhiteKing(position, move);
}

bool forksOrWinsPawn(std::int32_t side, std::int32_t square, petir_header::tdata& position) {
    if (side == petir_header::whiteSide) return petir_tools::whiteForksOrWinsPawn(square, position);
    return petir_tools::blackForksOrWinsPawn(square, position);
}
}
