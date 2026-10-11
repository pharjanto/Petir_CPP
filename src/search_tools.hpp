#pragma once
// Small helpers for the main search (search.cpp): move encoding, sides, and
// side-specific board tests behind a single call.
#include "petir.hpp"

namespace petir_search {
// Search depths are counted in eighths of a ply (plyUnits is 8): a depth of 8
// is one ply, and an extension of 6 is three quarters of a ply.
inline std::int32_t wholePlies(std::int32_t depth) {return pas::shiftRight(depth, 3);}

// A move holds its from-square in bits 0-6 and its to-square in bits 7-13. A
// to-square above 63 is a code instead of a square: shortCastleTarget,
// longCastleTarget or enPassantTarget.
inline std::int32_t moveFromSquare(std::int32_t move) {return move & 127;}
inline std::int32_t moveToSquare(std::int32_t move) {return pas::shiftRight(move, 7) & 127;}

// Sides are whiteSide (1) and blackSide (2).
inline std::int32_t opponentOf(std::int32_t side) {return 3 - side;}

bool isInCheck(std::int32_t side, petir_header::tdata& position);
void makeSideMove(std::int32_t side, std::int32_t move, petir_header::tdata& position);

// The move generators produce pseudo-legal moves. When the side to move is not
// in check, a move can leave its own king attacked in two ways: the king steps
// onto an attacked square, or a pinned piece leaves its pin line. (En passant
// can also expose the king; the search tests that after making the move.)
bool leavesKingAttacked(std::int32_t side, petir_header::tdata& position, std::int32_t move, std::uint64_t pinnedMask);

// Whether moving this piece opens a line from one of the mover's sliders to the
// enemy king, that is, whether the move gives a discovered check.
bool givesDiscoveredCheck(std::int32_t side, petir_header::tdata& position, std::int32_t move);

// Whether the piece on square (just moved there) forks two pieces or wins a pawn.
bool forksOrWinsPawn(std::int32_t side, std::int32_t square, petir_header::tdata& position);
}
