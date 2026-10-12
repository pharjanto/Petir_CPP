// Shared by both eval_diff harnesses.
#pragma once
#include "petir.hpp"

namespace eval_diff {
// Whether any bitboard disagrees with the board array. Search can reach such
// boards: a captured king's bitboard stays set while the board shows the
// capturing piece on that square (see "stale king bitboards" in MEMORY.md).
inline bool isStaleBoard(const petir_header::tdata& position) {
    std::uint64_t byCode[13] = {}; // index: board code + 6
    for (int square = 0; square < 64; ++square) byCode[position.board[square] + 6] |= std::uint64_t(1) << square;
    const std::uint64_t white = byCode[7] | byCode[8] | byCode[9] | byCode[10] | byCode[11] | byCode[12];
    const std::uint64_t black = byCode[0] | byCode[1] | byCode[2] | byCode[3] | byCode[4] | byCode[5];
    return byCode[7] != position.whitePawns || byCode[8] != position.whiteKnights || byCode[9] != position.whiteBishops
        || byCode[10] != position.whiteRooks || byCode[11] != position.whiteQueens || byCode[12] != position.whiteKingBoard
        || byCode[5] != position.blackPawns || byCode[4] != position.blackKnights || byCode[3] != position.blackBishops
        || byCode[2] != position.blackRooks || byCode[1] != position.blackQueens || byCode[0] != position.blackKingBoard
        || white != position.whitePieces || black != position.blackPieces || (white | black) != position.allPieces;
}
}
