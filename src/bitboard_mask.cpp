// Faithful translation of bitboard_mask.pas; Delphi source is in original/.
#include "petir.hpp"
#if defined(_MSC_VER) && defined(_M_X64)
#include <intrin.h>
#endif
namespace petir_bitboard_mask {
pas::Array<std::uint64_t, 0, 63> squareMask{};
pas::Array<std::uint64_t, 0, 63> squareMaskInverted{};
pas::Array<std::uint64_t, 0, 63> rotated90SquareMask{};
pas::Array<std::uint64_t, 0, 63> rotatedA8H1SquareMask{};
pas::Array<std::uint64_t, 0, 63> rotatedH8A1SquareMask{};
pas::Array<std::uint64_t, 0, 63> whitePassedPawnMask{};
pas::Array<std::uint64_t, 0, 63> blackPassedPawnMask{};
pas::Array<std::uint64_t, 0, 63> knightAttacks{};
pas::Array<std::uint64_t, 0, 63> kingAttacks{};
std::uint64_t notFileAMask{};
std::uint64_t notFileHMask{};
pas::Array<pas::Array<std::uint64_t, 0, 63>, 0, 63> horizontalAttacks{};
pas::Array<pas::Array<std::uint64_t, 0, 63>, 0, 63> verticalAttacks{};
pas::Array<pas::Array<std::uint64_t, 0, 255>, 0, 63> diagonalA8H1Attacks{};
pas::Array<pas::Array<std::uint64_t, 0, 255>, 0, 63> diagonalH8A1Attacks{};
pas::Array<std::uint8_t, 0, 65535> firstBitTable{};
pas::Array<std::uint64_t, 0, 63> whiteOutpostMask{};
pas::Array<std::uint64_t, 0, 63> blackOutpostMask{};
pas::Array<std::uint64_t, 0, 63> whitePawnAttacks{};
pas::Array<std::uint64_t, 0, 63> blackPawnAttacks{};
pas::Array<std::uint64_t, 0, 7> fileMask{};
pas::Array<std::uint64_t, 0, 7> rankMask{};
pas::Array<pas::Array<std::uint64_t, 0, 63>, 0, 63> betweenMask{};
pas::Array<pas::Array<std::int8_t, 0, 63>, 0, 63> rayDirection{};
pas::Array<pas::Array<std::int8_t, 0, 63>, 0, 63> rayDirectionCode{};
pas::Array<std::uint64_t, 0, 63> rookRayUpMask{};
pas::Array<std::uint64_t, 0, 63> rookRayDownMask{};
pas::Array<std::uint64_t, 0, 63> rookRayLeftMask{};
pas::Array<std::uint64_t, 0, 63> rookRayRightMask{};
pas::Array<std::uint64_t, 0, 63> bishopRayPlus7Mask{};
pas::Array<std::uint64_t, 0, 63> bishopRayPlus9Mask{};
pas::Array<std::uint64_t, 0, 63> bishopRayMinus7Mask{};
pas::Array<std::uint64_t, 0, 63> bishopRayMinus9Mask{};
pas::Array<std::uint64_t, 0, 63> bishopRaysUpMask{};
pas::Array<std::uint64_t, 0, 63> bishopRaysDownMask{};
// bitboard_mask.pas:139
std::int32_t firstBit(const std::uint16_t* bitboard) {
    std::uint64_t bits;
    std::memcpy(&bits, bitboard, sizeof(bits));
#if defined(_MSC_VER) && defined(_M_X64)
    // BSF is available on every x64 CPU; avoid the STL's per-call ISA dispatch.
    unsigned long bitIndex;
    return _BitScanForward64(&bitIndex, bits) ? static_cast<int>(bitIndex) : 0;
#else
    // Preserve the legacy empty-board result; countr_zero(0) returns 64.
    return bits ? std::countr_zero(bits) : 0;
#endif
}

// bitboard_mask.pas:108
std::int32_t lastBit(const std::uint16_t* bitboard) {
    std::uint64_t bits;
    std::memcpy(&bits,bitboard,sizeof(bits));
    if(!bits)return -1;
    // Delphi selects the least bit in the highest occupied 16-bit word.
#if defined(_MSC_VER) && defined(_M_X64)
    unsigned long highestBit, lowestBit;
    _BitScanReverse64(&highestBit, bits);
    const auto wordOffset = highestBit & ~15UL;
    _BitScanForward64(&lowestBit, bits >> wordOffset);
    return static_cast<int>(wordOffset + lowestBit);
#else
    const int wordOffset=(63-std::countl_zero(bits))&~15;
    return wordOffset+std::countr_zero(bits>>wordOffset);
#endif
}

// bitboard_mask.pas:96
std::uint64_t bishopAttacks(const petir_header::tdata& position, std::int32_t square) {
    std::uint64_t result{};
    result = petir_bitboard_mask::diagonalA8H1Attacks[square][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[square]) & 255)];
    result = (result | petir_bitboard_mask::diagonalH8A1Attacks[square][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[square]) & 255)]);
    return result;
}

// bitboard_mask.pas:102
std::uint64_t rookAttacks(petir_header::tdata& position, std::int32_t square) {
    std::uint64_t result{};
    result = petir_bitboard_mask::horizontalAttacks[square][(pas::shiftRight(position.allPieces, pas::add((square & 56), 1)) & 63)];
    result = (result | petir_bitboard_mask::verticalAttacks[square][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[square]) & 63)]);
    return result;
}

}
