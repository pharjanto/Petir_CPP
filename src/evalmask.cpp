// Faithful translation of evalmask.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_evalmask {
pas::Array<std::uint64_t, 0, 63> w_outpost_mask{};
pas::Array<std::uint64_t, 0, 63> b_outpost_mask{};
std::uint64_t abcd_mask{};
std::uint64_t efgh_mask{};
std::uint64_t abc_mask{};
std::uint64_t fgh_mask{};
pas::Array<std::uint64_t, 0, 63> pion_gantung_mask{};
pas::Array<std::uint8_t, 0, 65535> mobmask{};
pas::Array<pas::Array<bool, 0, 63>, 0, 63> dekatbaris{};
pas::Array<pas::Array<std::uint8_t, 0, 63>, 0, 63> tabel_edistance{};
pas::Array<pas::Array<std::uint8_t, 0, 63>, 0, 63> tabel_edistance2{};
pas::Array<pas::Array<std::uint8_t, 0, 63>, 0, 63> tabel_distance_gajah{};
// evalmask.pas:108
void initevalmask() {
    std::int32_t a{};
    std::int32_t b{};
    std::int32_t x{};
    std::int32_t n{};
    std::int32_t pas_temp_1{};
    std::int32_t pas_temp_2{};
    std::int32_t pas_temp_3{};
    std::int32_t pas_temp_4{};
    std::int32_t pas_temp_5{};
    std::int32_t pas_temp_6{};
    petir_evalmask::initevaltable();
    pas_temp_1 = 63;
    for (a = 0; a <= pas_temp_1; ++a) {
        pas_temp_2 = 63;
        for (b = 0; b <= pas_temp_2; ++b) {
            n = petir_evalmask::distance_gajah(a, b);
            petir_evalmask::tabel_distance_gajah[a][b] = pas::sub(7, n);
            if ((petir_evalmask::tabel_distance_gajah[a][b] <= 3)) {
                petir_evalmask::tabel_distance_gajah[a][b] = 0;
            } else {
                (petir_evalmask::tabel_distance_gajah[a][b] = pas::decrement(petir_evalmask::tabel_distance_gajah[a][b], 3));
            }
            petir_evalmask::dekatbaris[a][b] = ((pas::abs(pas::sub((a % 8), (b % 8))) == 0) || (pas::abs(pas::sub((a % 8), (b % 8))) == 1));
        }
    }
    pas_temp_3 = 65535;
    for (a = 1; a <= pas_temp_3; ++a) {
        b = a;
        n = 0;
        while ((b != 0)) {
            if (((b & 1) == 1)) {
                (n = pas::increment(n, 1));
            }
            b = pas::shr(b, 1);
        }
        petir_evalmask::mobmask[a] = n;
    }
    petir_evalmask::mobmask[0] = 0;
    pas_temp_4 = 63;
    for (a = 0; a <= pas_temp_4; ++a) {
        petir_evalmask::w_outpost_mask[a] = 0;
        petir_evalmask::b_outpost_mask[a] = 0;
        petir_bitboard_mask::w_pion_bebas_mask[a] = 0;
        petir_bitboard_mask::b_pion_bebas_mask[a] = 0;
        petir_bitboard_mask::w_pawn_attack[a] = 0;
        petir_bitboard_mask::b_pawn_attack[a] = 0;
        if (((a % 8) < 7)) {
            if ((a <= pas::sub(63, 9))) {
                petir_bitboard_mask::w_pawn_attack[a] = (petir_bitboard_mask::w_pawn_attack[a] | petir_bitboard_mask::bit2nmask[pas::add(a, 9)]);
            }
            if ((a >= 7)) {
                petir_bitboard_mask::b_pawn_attack[a] = (petir_bitboard_mask::b_pawn_attack[a] | petir_bitboard_mask::bit2nmask[pas::sub(a, 7)]);
            }
            b = pas::add(a, 9);
            while ((b <= petir_header::h8)) {
                petir_evalmask::w_outpost_mask[a] = (petir_evalmask::w_outpost_mask[a] | petir_bitboard_mask::bit2nmask[b]);
                petir_bitboard_mask::w_pion_bebas_mask[a] = (petir_bitboard_mask::w_pion_bebas_mask[a] | petir_bitboard_mask::bit2nmask[b]);
                (b = pas::increment(b, 8));
            }
            b = pas::sub(a, 7);
            while ((b >= petir_header::a1)) {
                petir_evalmask::b_outpost_mask[a] = (petir_evalmask::b_outpost_mask[a] | petir_bitboard_mask::bit2nmask[b]);
                petir_bitboard_mask::b_pion_bebas_mask[a] = (petir_bitboard_mask::b_pion_bebas_mask[a] | petir_bitboard_mask::bit2nmask[b]);
                (b = pas::decrement(b, 8));
            }
        }
        if (((a % 8) > 0)) {
            if ((a <= pas::sub(63, 7))) {
                petir_bitboard_mask::w_pawn_attack[a] = (petir_bitboard_mask::w_pawn_attack[a] | petir_bitboard_mask::bit2nmask[pas::add(a, 7)]);
            }
            if ((a >= 9)) {
                petir_bitboard_mask::b_pawn_attack[a] = (petir_bitboard_mask::b_pawn_attack[a] | petir_bitboard_mask::bit2nmask[pas::sub(a, 9)]);
            }
            b = pas::add(a, 7);
            while ((b <= petir_header::h8)) {
                petir_evalmask::w_outpost_mask[a] = (petir_evalmask::w_outpost_mask[a] | petir_bitboard_mask::bit2nmask[b]);
                petir_bitboard_mask::w_pion_bebas_mask[a] = (petir_bitboard_mask::w_pion_bebas_mask[a] | petir_bitboard_mask::bit2nmask[b]);
                (b = pas::increment(b, 8));
            }
            b = pas::sub(a, 9);
            while ((b >= petir_header::a1)) {
                petir_evalmask::b_outpost_mask[a] = (petir_evalmask::b_outpost_mask[a] | petir_bitboard_mask::bit2nmask[b]);
                petir_bitboard_mask::b_pion_bebas_mask[a] = (petir_bitboard_mask::b_pion_bebas_mask[a] | petir_bitboard_mask::bit2nmask[b]);
                (b = pas::decrement(b, 8));
            }
        }
        petir_bitboard_mask::rook_attack_up_mask[a] = 0;
        b = pas::add(a, 8);
        while ((b <= petir_header::h8)) {
            petir_bitboard_mask::rook_attack_up_mask[a] = (petir_bitboard_mask::rook_attack_up_mask[a] | petir_bitboard_mask::bit2nmask[b]);
            petir_bitboard_mask::w_pion_bebas_mask[a] = (petir_bitboard_mask::w_pion_bebas_mask[a] | petir_bitboard_mask::bit2nmask[b]);
            (b = pas::increment(b, 8));
        }
        b = pas::sub(a, 8);
        petir_bitboard_mask::rook_attack_down_mask[a] = 0;
        while ((b >= petir_header::a1)) {
            petir_bitboard_mask::rook_attack_down_mask[a] = (petir_bitboard_mask::rook_attack_down_mask[a] | petir_bitboard_mask::bit2nmask[b]);
            petir_bitboard_mask::b_pion_bebas_mask[a] = (petir_bitboard_mask::b_pion_bebas_mask[a] | petir_bitboard_mask::bit2nmask[b]);
            (b = pas::decrement(b, 8));
        }
    }
    pas_temp_5 = 7;
    for (a = 0; a <= pas_temp_5; ++a) {
        b = a;
        petir_bitboard_mask::file_mask[a] = 0;
        while ((b <= petir_header::h8)) {
            petir_bitboard_mask::file_mask[a] = (petir_bitboard_mask::file_mask[a] | petir_bitboard_mask::bit2nmask[b]);
            (b = pas::increment(b, 8));
        }
    }
    petir_evalmask::abcd_mask = (((petir_bitboard_mask::file_mask[0] | petir_bitboard_mask::file_mask[1]) | petir_bitboard_mask::file_mask[2]) | petir_bitboard_mask::file_mask[3]);
    petir_evalmask::efgh_mask = (((petir_bitboard_mask::file_mask[4] | petir_bitboard_mask::file_mask[5]) | petir_bitboard_mask::file_mask[6]) | petir_bitboard_mask::file_mask[7]);
    petir_evalmask::abc_mask = (petir_bitboard_mask::file_mask[0] | petir_bitboard_mask::file_mask[1]);
    petir_evalmask::fgh_mask = (petir_bitboard_mask::file_mask[6] | petir_bitboard_mask::file_mask[7]);
    petir_bitboard_mask::mask_left = (~petir_bitboard_mask::file_mask[0]);
    petir_bitboard_mask::mask_right = (~petir_bitboard_mask::file_mask[7]);
    pas::fillchar(petir_bitboard_mask::rank_mask, sizeof(petir_bitboard_mask::rank_mask), 0);
    pas_temp_6 = 63;
    for (a = 0; a <= pas_temp_6; ++a) {
        x = (a % 8);
        if ((x == 0)) {
            petir_evalmask::pion_gantung_mask[a] = petir_bitboard_mask::file_mask[1];
        } else {
            if ((x == 7)) {
                petir_evalmask::pion_gantung_mask[a] = petir_bitboard_mask::file_mask[6];
            } else {
                petir_evalmask::pion_gantung_mask[a] = (petir_bitboard_mask::file_mask[pas::sub(x, 1)] | petir_bitboard_mask::file_mask[pas::add(x, 1)]);
            }
        }
        petir_bitboard_mask::rank_mask[(a / 8)] = (petir_bitboard_mask::rank_mask[(a / 8)] | petir_bitboard_mask::bit2nmask[a]);
    }
}

// evalmask.pas:77
std::int32_t distancemax(std::int32_t p, std::int32_t q) {
    std::int32_t return_value{};
    std::int32_t x1{};
    std::int32_t y1{};
    std::int32_t x2{};
    std::int32_t y2{};
    std::int32_t dx{};
    std::int32_t dy{};
    x1 = (p & 7);
    x2 = (q & 7);
    y1 = pas::shr(p, 3);
    y2 = pas::shr(q, 3);
    dx = pas::abs(pas::sub(x1, x2));
    dy = pas::abs(pas::sub(y1, y2));
    if ((dx > dy)) {
        return_value = dx;
    } else {
        return_value = dy;
    }
    return return_value;
}

// evalmask.pas:42
std::int32_t distance_gajah(std::int32_t p, std::int32_t q) {
    std::int32_t return_value{};
    std::int32_t x1{};
    std::int32_t y1{};
    std::int32_t x2{};
    std::int32_t y2{};
    std::int32_t d1{};
    std::int32_t d2{};
    x1 = (p % 8);
    x2 = (q % 8);
    y1 = (p / 8);
    y2 = (q / 8);
    d1 = pas::abs(pas::sub(pas::add(x1, y1), pas::add(x2, y2)));
    d2 = pas::abs(pas::sub(pas::sub(x1, y1), pas::sub(x2, y2)));
    if ((d1 < d2)) {
        return_value = d1;
    } else {
        return_value = d2;
    }
    return return_value;
}

// evalmask.pas:64
std::int32_t distance(std::int32_t p, std::int32_t q) {
    std::int32_t return_value{};
    std::int32_t x1{};
    std::int32_t y1{};
    std::int32_t x2{};
    std::int32_t y2{};
    std::int32_t dx{};
    std::int32_t dy{};
    x1 = (p % 8);
    x2 = (q % 8);
    y1 = (p / 8);
    y2 = (q / 8);
    dx = pas::abs(pas::sub(x1, x2));
    dy = pas::abs(pas::sub(y1, y2));
    return_value = pas::sub(10, pas::round(pas::sqrt(pas::add(pas::mul(dx, dx), pas::mul(dy, dy)))));
    return return_value;
}

// evalmask.pas:93
void initevaltable() {
    std::int32_t a{};
    std::int32_t b{};
    std::int32_t n{};
    std::int32_t pas_temp_1{};
    std::int32_t pas_temp_2{};
    pas_temp_1 = 63;
    for (a = 0; a <= pas_temp_1; ++a) {
        pas_temp_2 = 63;
        for (b = 0; b <= pas_temp_2; ++b) {
            n = petir_evalmask::distance(a, b);
            if ((n >= 5)) {
                petir_evalmask::tabel_edistance[a][b] = (n / 2);
            } else {
                petir_evalmask::tabel_edistance[a][b] = 0;
            }
            petir_evalmask::tabel_edistance2[a][b] = n;
        }
    }
}

}
