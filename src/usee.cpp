// Faithful translation of usee.pas; Delphi source is in original/.
// staticExchange() is restructured for speed, preserving the original board sources,
// attacker order, signed 8-bit swaps and x-ray behavior.
#include "petir.hpp"
#include <algorithm>
#if defined(_MSC_VER)
#define PETIR_SEE_INLINE __forceinline
#else
#define PETIR_SEE_INLINE inline __attribute__((always_inline))
#endif
namespace petir_usee {
namespace {
// Delphi's firstBit on a nonzero board: the lowest bit.
inline std::int32_t lowestSetBit(std::uint64_t bits) {
#if defined(_MSC_VER) && defined(_M_X64)
    unsigned long bitIndex;
    _BitScanForward64(&bitIndex, bits);
    return static_cast<std::int32_t>(bitIndex);
#else
    return std::countr_zero(bits);
#endif
}

// Delphi's lastBit on a nonzero board: the lowest bit of the highest occupied
// 16-bit word.
inline std::int32_t legacyLastBit(std::uint64_t bits) {
#if defined(_MSC_VER) && defined(_M_X64)
    unsigned long highestBit, lowestBit;
    _BitScanReverse64(&highestBit, bits);
    const auto wordOffset = highestBit & ~15UL;
    _BitScanForward64(&lowestBit, bits >> wordOffset);
    return static_cast<std::int32_t>(wordOffset + lowestBit);
#else
    const int wordOffset = (63 - std::countl_zero(bits)) & ~15;
    return wordOffset + std::countr_zero(bits >> wordOffset);
#endif
}

// Lookups for addXray, filled by initXray from the bitboard_mask tables: the ray
// beyond a square for each rayDirectionCode value (row 0 is empty, so direction 0
// finds no candidates), and for each axis the original occupancy board, its
// shift and the attack table row. Each axis keeps its original occupancy
// source: the rotated boards need not agree with allPieces during search.
struct XrayAxis { const std::uint64_t* attackRow; std::uint64_t petir_header::tdata::* occupancyBoard; std::uint8_t shift, mask; };
std::uint64_t xrayRays[9][64];
XrayAxis xrayAxes[4][64];

// usee.pas:21. Forced inline so staticExchange() keeps attackers in a register; MSVC
// otherwise calls it out of line at all three sites.
PETIR_SEE_INLINE void addXrayInline(std::uint64_t& attackers, std::int32_t square, std::int32_t target, const petir_header::tdata& position) {
    const int direction = petir_bitboard_mask::rayDirectionCode[target][square];
    const auto candidates = xrayRays[direction][square] & (direction > 4 ? position.bishopsAndQueens : position.rooksAndQueens);
    if (candidates == 0) return;
    const XrayAxis& axis = xrayAxes[(direction - 1) >> 1][square];
    attackers |= candidates & axis.attackRow[((position.*axis.occupancyBoard) >> axis.shift) & axis.mask];
}

// One capture of the exchange for one side, exactly as the original loop body
// did it: the side's least valuable attacker (White scans with lastBit except
// for its king, Black with firstBit except for its king), the swap value, the
// x-ray behind the attacker and its removal. The value comes from board and
// every attacker is x-rayed, as in the original: search can reach boards where
// a captured king's bitboard is stale, and SEE must score them as before.
// Returns false when the side has no attacker left.
using ExchangeList = pas::Array<std::int8_t, 0, 32>;
template <bool White>
inline bool captureStep(std::uint64_t& attackers, ExchangeList& swapList, std::int32_t& swapCount, std::int32_t& targetValue, std::int32_t targetSquare, const petir_header::tdata& position) {
    std::uint64_t candidates;
    std::int32_t attackerSquare;
    if ((candidates = (attackers & (White ? position.whitePawns : position.blackPawns))) != 0
        || (candidates = (attackers & (White ? position.whiteKnights : position.blackKnights))) != 0
        || (candidates = (attackers & (White ? position.whiteBishops : position.blackBishops))) != 0
        || (candidates = (attackers & (White ? position.whiteRooks : position.blackRooks))) != 0
        || (candidates = (attackers & (White ? position.whiteQueens : position.blackQueens))) != 0) {
        attackerSquare = White ? legacyLastBit(candidates) : lowestSetBit(candidates);
    } else if ((candidates = (attackers & (White ? position.whiteKingBoard : position.blackKingBoard))) != 0) {
        attackerSquare = White ? lowestSetBit(candidates) : legacyLastBit(candidates);
    } else {
        return false;
    }
    swapList[swapCount] = static_cast<std::int8_t>(targetValue - swapList[swapCount - 1]);
    targetValue = petir_usee::pieceSeeValue[position.board[attackerSquare]];
    // Rays exclude their source square, so removal can overlap the x-ray loads.
    attackers &= ~petir_bitboard_mask::squareBit(attackerSquare);
    addXrayInline(attackers, attackerSquare, targetSquare, position);
    ++swapCount;
    return true;
}
}

// usee.pas:83
std::int32_t staticExchange(std::int32_t sideToMove, const petir_header::tdata& position, std::int32_t fromSquare, std::int32_t toSquare) {
    using namespace petir_bitboard_mask;
    // Every entry used by the backward pass is assigned during the forward pass.
    ExchangeList swapList;
    std::uint64_t attackers = (knightAttacks[toSquare] & (position.blackKnights | position.whiteKnights))
        | (whitePawnAttacks[toSquare] & position.blackPawns) | (blackPawnAttacks[toSquare] & position.whitePawns)
        | (kingAttacks[toSquare] & (position.blackKingBoard | position.whiteKingBoard));
    attackers |= (horizontalAttacks[toSquare][(position.allPieces >> ((toSquare & 56) + 1)) & 63] | verticalAttacks[toSquare][(position.allPiecesRotated90 >> verticalShift[toSquare]) & 63]) & position.rooksAndQueens;
    attackers |= (diagonalA8H1Attacks[toSquare][(position.allPiecesA8H1 >> diagonalA8H1Shift[toSquare]) & 255] | diagonalH8A1Attacks[toSquare][(position.allPiecesH8A1 >> diagonalH8A1Shift[toSquare]) & 255]) & position.bishopsAndQueens;
    swapList[0] = static_cast<std::int8_t>(petir_usee::pieceSeeValue[position.board[toSquare]]);
    attackers &= ~squareBit(fromSquare);
    addXrayInline(attackers, fromSquare, toSquare, position);
    std::int32_t targetValue = petir_usee::pieceSeeValue[position.board[fromSquare]];
    std::int32_t swapCount = 1;
    // The opponent recaptures first; the two sides then alternate.
    if (sideToMove == petir_header::whiteSide) goto black;
    for (;;) {
        if (!captureStep<true>(attackers, swapList, swapCount, targetValue, toSquare, position)) break;
        black:
        if (!captureStep<false>(attackers, swapList, swapCount, targetValue, toSquare, position)) break;
    }
    // Each side may stop capturing. Fold the swaps in a register: earlier
    // entries are read once, and no backward-pass array writes are needed.
    // min(signed byte, -result) stays in [-128, 127], even for result == -128.
    std::int32_t result = swapList[swapCount - 1];
    while (--swapCount > 0) result = std::min<std::int32_t>(swapList[swapCount - 1], -result);
    return result;
}

// usee.pas:21
void addXray(std::uint64_t& attackers, std::int32_t square, std::int32_t target, const petir_header::tdata& position) {
    addXrayInline(attackers, square, target, position);
}

// Called once after petir_bitboard::initMasks and petir_evalmask::initEvalMasks,
// which fills the vertical ray masks.
void initXray() {
    using namespace petir_bitboard_mask;
    using petir_header::tdata;
    const pas::Array<std::uint64_t, 0, 63>* rayMasks[] = {
        nullptr, &rookRayDownMask, &rookRayUpMask,
        &rookRayLeftMask, &rookRayRightMask,
        &bishopRayMinus9Mask, &bishopRayPlus9Mask,
        &bishopRayMinus7Mask, &bishopRayPlus7Mask
    };
    for (int square = 0; square < 64; ++square) {
        xrayRays[0][square] = 0;
        for (int direction = 1; direction < 9; ++direction) xrayRays[direction][square] = (*rayMasks[direction])[square];
        xrayAxes[0][square] = {&verticalAttacks[square][0], &tdata::allPiecesRotated90, verticalShift[square], 63};
        xrayAxes[1][square] = {&horizontalAttacks[square][0], &tdata::allPieces, static_cast<std::uint8_t>((square & 56) + 1), 63};
        xrayAxes[2][square] = {&diagonalH8A1Attacks[square][0], &tdata::allPiecesH8A1, diagonalH8A1Shift[square], 255};
        xrayAxes[3][square] = {&diagonalA8H1Attacks[square][0], &tdata::allPiecesA8H1, diagonalA8H1Shift[square], 255};
    }
}

}
#undef PETIR_SEE_INLINE
