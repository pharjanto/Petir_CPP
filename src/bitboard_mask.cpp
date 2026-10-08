// Faithful translation of bitboard_mask.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_bitboard_mask {
pas::Array<std::uint64_t, 0, 63> bit2nmask{};
pas::Array<std::uint64_t, 0, 63> bit2nmasknot{};
pas::Array<std::uint64_t, 0, 63> bit2nmask90{};
pas::Array<std::uint64_t, 0, 63> bit2nmaska8h1{};
pas::Array<std::uint64_t, 0, 63> bit2nmaskh8a1{};
pas::Array<std::uint64_t, 0, 63> w_pion_bebas_mask{};
pas::Array<std::uint64_t, 0, 63> b_pion_bebas_mask{};
pas::Array<std::uint64_t, 0, 63> knightmask{};
pas::Array<std::uint64_t, 0, 63> kingmask{};
std::uint64_t mask_left{};
std::uint64_t mask_right{};
pas::Array<pas::Array<std::uint64_t, 0, 63>, 0, 63> horzmask2{};
pas::Array<pas::Array<std::uint64_t, 0, 63>, 0, 63> vertmask2{};
pas::Array<pas::Array<std::uint64_t, 0, 255>, 0, 63> diaga8h1mask{};
pas::Array<pas::Array<std::uint64_t, 0, 255>, 0, 63> diagh8a1mask{};
pas::Array<std::uint8_t, 0, 65535> firstbitmask{};
pas::Array<std::uint64_t, 0, 63> w_outpost_mask{};
pas::Array<std::uint64_t, 0, 63> b_outpost_mask{};
pas::Array<std::uint64_t, 0, 63> w_pawn_attack{};
pas::Array<std::uint64_t, 0, 63> b_pawn_attack{};
pas::Array<std::uint64_t, 0, 7> file_mask{};
pas::Array<std::uint64_t, 0, 7> rank_mask{};
pas::Array<pas::Array<std::uint64_t, 0, 63>, 0, 63> pathmask{};
pas::Array<pas::Array<std::int8_t, 0, 63>, 0, 63> direction{};
pas::Array<pas::Array<std::int8_t, 0, 63>, 0, 63> direction2{};
pas::Array<std::uint64_t, 0, 63> rook_attack_up_mask{};
pas::Array<std::uint64_t, 0, 63> rook_attack_down_mask{};
pas::Array<std::uint64_t, 0, 63> rook_attack_left_mask{};
pas::Array<std::uint64_t, 0, 63> rook_attack_right_mask{};
pas::Array<std::uint64_t, 0, 63> bishop_attack_plus7_mask{};
pas::Array<std::uint64_t, 0, 63> bishop_attack_plus9_mask{};
pas::Array<std::uint64_t, 0, 63> bishop_attack_min7_mask{};
pas::Array<std::uint64_t, 0, 63> bishop_attack_min9_mask{};
pas::Array<std::uint64_t, 0, 63> bishop_attack_up_mask{};
pas::Array<std::uint64_t, 0, 63> bishop_attack_down_mask{};
// bitboard_mask.pas:139
std::int32_t firstbitp(petir_bitboard_mask::pword p) {
    std::uint64_t bits;
    std::memcpy(&bits, p, sizeof(bits));
    // Preserve the legacy empty-board result; countr_zero(0) returns 64.
    return bits ? std::countr_zero(bits) : 0;
}

// bitboard_mask.pas:108
std::int32_t lastbitp(petir_bitboard_mask::pword p) {
    std::int32_t return_value{};
    (p = pas::increment(p, 3));
    if (((*p) != 0)) {
        return_value = pas::add(petir_bitboard_mask::firstbitmask[(*p)], 48);
        return return_value;
    }
    (p = pas::decrement(p, 1));
    if (((*p) != 0)) {
        return_value = pas::add(petir_bitboard_mask::firstbitmask[(*p)], 32);
        return return_value;
    }
    (p = pas::decrement(p, 1));
    if (((*p) != 0)) {
        return_value = pas::add(petir_bitboard_mask::firstbitmask[(*p)], 16);
        return return_value;
    }
    (p = pas::decrement(p, 1));
    if (((*p) != 0)) {
        return_value = petir_bitboard_mask::firstbitmask[(*p)];
        return return_value;
    }
    return_value = pas::negate(1);
    return return_value;
}

// bitboard_mask.pas:96
std::uint64_t attackbishop(petir_header::tdata& data, std::int32_t pos) {
    std::uint64_t return_value{};
    return_value = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
    return_value = (return_value | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
    return return_value;
}

// bitboard_mask.pas:102
std::uint64_t attackrook(petir_header::tdata& data, std::int32_t pos) {
    std::uint64_t return_value{};
    return_value = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
    return_value = (return_value | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
    return return_value;
}

}
