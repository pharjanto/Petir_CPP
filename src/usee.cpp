// Faithful translation of usee.pas; Delphi source is in original/.
// see() is restructured for speed, preserving the original board sources,
// attacker order, signed 8-bit swaps and x-ray behavior.
#include "petir.hpp"
#if defined(_MSC_VER)
#define PETIR_SEE_INLINE __forceinline
#else
#define PETIR_SEE_INLINE inline __attribute__((always_inline))
#endif
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

// Lookups for xray, filled by init_xray from the bitboard_mask tables: the ray
// beyond a square for each direction2 value (row 0 is empty, so direction 0
// finds no candidates), and for each axis the original occupancy board, its
// shift and the attack table row. Each axis keeps its original occupancy
// source: the rotated boards need not agree with allpieces during search.
struct XrayAxis { const std::uint64_t* row; std::uint64_t petir_header::tdata::* occupancy; std::uint8_t shift, mask; };
std::uint64_t xray_ray[9][64];
XrayAxis xray_axis[4][64];

// usee.pas:21. Forced inline so see() keeps attack in a register; MSVC
// otherwise calls it out of line at all three sites.
PETIR_SEE_INLINE void xray_inline(std::uint64_t& attack, std::int32_t square, std::int32_t target, const petir_header::tdata& data) {
    const int direction = petir_bitboard_mask::direction2[target][square];
    const auto candidates = xray_ray[direction][square] & (direction > 4 ? data.bishopqueen : data.rookqueen);
    if (candidates == 0) return;
    const XrayAxis& axis = xray_axis[(direction - 1) >> 1][square];
    attack |= candidates & axis.row[((data.*axis.occupancy) >> axis.shift) & axis.mask];
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
    xray_inline(attack, square, sto, data);
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
    xray_inline(attack, sfrom, sto, data);
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
    xray_inline(attack, square, target, data);
}

// Called once after petir_bitboard::init_mask and petir_evalmask::initevalmask,
// which fills the vertical ray masks.
void init_xray() {
    using namespace petir_bitboard_mask;
    using petir_header::tdata;
    const pas::Array<std::uint64_t, 0, 63>* rays[] = {
        nullptr, &rook_attack_down_mask, &rook_attack_up_mask,
        &rook_attack_left_mask, &rook_attack_right_mask,
        &bishop_attack_min9_mask, &bishop_attack_plus9_mask,
        &bishop_attack_min7_mask, &bishop_attack_plus7_mask
    };
    for (int square = 0; square < 64; ++square) {
        xray_ray[0][square] = 0;
        for (int direction = 1; direction < 9; ++direction) xray_ray[direction][square] = (*rays[direction])[square];
        xray_axis[0][square] = {&vertmask2[square][0], &tdata::allpiecesr90, vershiftmask[square], 63};
        xray_axis[1][square] = {&horzmask2[square][0], &tdata::allpieces, static_cast<std::uint8_t>((square & 56) + 1), 63};
        xray_axis[2][square] = {&diagh8a1mask[square][0], &tdata::allpiecesh8a1, h8a1shiftmask[square], 255};
        xray_axis[3][square] = {&diaga8h1mask[square][0], &tdata::allpiecesa8h1, a8h1shiftmask[square], 255};
    }
}

}
#undef PETIR_SEE_INLINE
