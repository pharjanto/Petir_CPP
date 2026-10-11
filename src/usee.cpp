// Faithful translation of usee.pas; Delphi source is in original/.
// see() is restructured for speed, preserving the original board sources,
// attacker order, signed 8-bit swaps and x-ray behavior.
#include "petir.hpp"
namespace petir_usee {
namespace {
// Delphi's firstbitp on a nonzero board: the lowest bit.
inline std::int32_t first_bit(std::uint64_t bits) {
#if defined(_MSC_VER) && defined(_M_X64)
    unsigned long index;
    _BitScanForward64(&index, bits);
    return static_cast<std::int32_t>(index);
#else
    return std::countr_zero(bits);
#endif
}

// Delphi's lastbitp on a nonzero board: the lowest bit of the highest occupied
// 16-bit word.
inline std::int32_t last_bit(std::uint64_t bits) {
#if defined(_MSC_VER) && defined(_M_X64)
    unsigned long highest, lowest;
    _BitScanReverse64(&highest, bits);
    const auto word_offset = highest & ~15UL;
    _BitScanForward64(&lowest, bits >> word_offset);
    return static_cast<std::int32_t>(word_offset + lowest);
#else
    const int word_offset = (63 - std::countl_zero(bits)) & ~15;
    return word_offset + std::countr_zero(bits >> word_offset);
#endif
}

// One capture of the exchange for one side, exactly as the original loop body
// did it: the side's least valuable attacker (White scans with lastbitp except
// for its king, Black with firstbitp except for its king), the swap value, the
// x-ray behind the attacker and its removal. The value comes from papan and
// every attacker is x-rayed, as in the original: search can reach boards where
// a captured king's bitboard is stale, and SEE must score them as before.
// Returns false when the side has no attacker left.
using ExchangeList = pas::Array<std::int8_t, 0, 32>;
template <bool White>
inline bool capture(std::uint64_t& attack, ExchangeList& list, std::int32_t& n, std::int32_t& piece, std::int32_t sto, const petir_header::tdata& data) {
    std::uint64_t temp;
    std::int32_t square;
    if ((temp = (attack & (White ? data.pawnwhite : data.pawnblack))) != 0
        || (temp = (attack & (White ? data.knightwhite : data.knightblack))) != 0
        || (temp = (attack & (White ? data.bishopwhite : data.bishopblack))) != 0
        || (temp = (attack & (White ? data.rookwhite : data.rookblack))) != 0
        || (temp = (attack & (White ? data.queenwhite : data.queenblack))) != 0) {
        square = White ? last_bit(temp) : first_bit(temp);
    } else if ((temp = (attack & (White ? data.kingwhite : data.kingblack))) != 0) {
        square = White ? first_bit(temp) : last_bit(temp);
    } else {
        return false;
    }
    list[n] = static_cast<std::int8_t>(piece - list[n - 1]);
    piece = petir_usee::nilai_piece[data.papan[square]];
    petir_usee::xray(attack, square, sto, data);
    attack &= ~petir_bitboard_mask::bit(square);
    ++n;
    return true;
}
}

// usee.pas:83
std::int32_t see(std::int32_t giliran, const petir_header::tdata& data, std::int32_t sfrom, std::int32_t sto) {
    using namespace petir_bitboard_mask;
    // Every entry used by the backward pass is assigned during the forward pass.
    ExchangeList list;
    std::uint64_t attack = (knightmask[sto] & (data.knightblack | data.knightwhite))
        | (w_pawn_attack[sto] & data.pawnblack) | (b_pawn_attack[sto] & data.pawnwhite)
        | (kingmask[sto] & (data.kingblack | data.kingwhite));
    attack |= (horzmask2[sto][(data.allpieces >> ((sto & 56) + 1)) & 63] | vertmask2[sto][(data.allpiecesr90 >> vershiftmask[sto]) & 63]) & data.rookqueen;
    attack |= (diaga8h1mask[sto][(data.allpiecesa8h1 >> a8h1shiftmask[sto]) & 255] | diagh8a1mask[sto][(data.allpiecesh8a1 >> h8a1shiftmask[sto]) & 255]) & data.bishopqueen;
    list[0] = static_cast<std::int8_t>(petir_usee::nilai_piece[data.papan[sto]]);
    petir_usee::xray(attack, sfrom, sto, data);
    attack &= ~bit(sfrom);
    std::int32_t piece = petir_usee::nilai_piece[data.papan[sfrom]];
    std::int32_t n = 1;
    // The opponent recaptures first; the two sides then alternate.
    if (giliran == petir_header::_sisiputih) goto black;
    for (;;) {
        if (!capture<true>(attack, list, n, piece, sto, data)) break;
        black:
        if (!capture<false>(attack, list, n, piece, sto, data)) break;
    }
    // Backward pass: each side may stop capturing when that is better for it.
    for (n = n - 1; n > 0; --n) {
        if (list[n] > -list[n - 1]) list[n - 1] = static_cast<std::int8_t>(-list[n]);
    }
    return list[0];
}

// usee.pas:21
void xray(std::uint64_t& attack, std::int32_t square, std::int32_t target, const petir_header::tdata& data) {
    using namespace petir_bitboard_mask;
    // direction2 pairs opposite rays on the same axis. Select the ray without
    // walking eight direction branches, then share the lookup for each pair.
    static constexpr const pas::Array<std::uint64_t, 0, 63>* rays[] = {
        nullptr, &rook_attack_down_mask, &rook_attack_up_mask,
        &rook_attack_left_mask, &rook_attack_right_mask,
        &bishop_attack_min9_mask, &bishop_attack_plus9_mask,
        &bishop_attack_min7_mask, &bishop_attack_plus7_mask
    };
    const int direction = direction2[target][square];
    if (direction == 0) return;
    const auto candidates = (*rays[direction])[square]
        & (direction > 4 ? data.bishopqueen : data.rookqueen);
    if (candidates == 0) return;
    // Keep the original occupancy source for each axis: the rotated boards
    // need not agree with allpieces on the boards reached during search.
    switch ((direction - 1) >> 1) {
    case 0:
        attack |= candidates & vertmask2[square][(data.allpiecesr90 >> vershiftmask[square]) & 63];
        break;
    case 1:
        attack |= candidates & horzmask2[square][(data.allpieces >> ((square & 56) + 1)) & 63];
        break;
    case 2:
        attack |= candidates & diagh8a1mask[square][(data.allpiecesh8a1 >> h8a1shiftmask[square]) & 255];
        break;
    case 3:
        attack |= candidates & diaga8h1mask[square][(data.allpiecesa8h1 >> a8h1shiftmask[square]) & 255];
        break;
    }
}

}
