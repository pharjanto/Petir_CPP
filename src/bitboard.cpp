// Faithful translation of bitboard.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_bitboard {
// bitboard.pas:110
void init_mask() {
    std::int32_t a{};
    std::int32_t b{};
    std::int32_t x{};
    std::int32_t y{};
    std::int32_t z{};
    std::int32_t ld{};
    std::int32_t bld{};
    std::int32_t c{};
    std::int32_t d{};
    std::int32_t e{};
    std::int32_t ld2{};
    std::int32_t bld2{};
    std::uint64_t n{};
    std::int32_t pas_temp_1{};
    std::int32_t pas_temp_2{};
    std::int32_t pas_temp_3{};
    std::int32_t pas_temp_4{};
    std::int32_t pas_temp_5{};
    std::int32_t pas_temp_6{};
    std::int32_t pas_temp_7{};
    std::int32_t pas_temp_8{};
    std::int32_t pas_temp_9{};
    std::int32_t pas_temp_10{};
    std::int32_t pas_temp_11{};
    std::int32_t pas_temp_12{};
    std::int32_t pas_temp_13{};
    std::int32_t pas_temp_14{};
    std::int32_t pas_temp_15{};
    std::int32_t pas_temp_16{};
    std::int32_t pas_temp_17{};
    std::int32_t pas_temp_18{};
    std::int32_t pas_temp_19{};
    std::int32_t pas_temp_20{};
    std::int32_t pas_temp_21{};
    n = 1;
    pas_temp_1 = 63;
    for (a = 0; a <= pas_temp_1; ++a) {
        petir_bitboard_mask::bit2nmask[a] = n;
        petir_bitboard_mask::bit2nmasknot[a] = (~n);
        n = (n * 2);
    }
    pas_temp_2 = 63;
    for (a = 0; a <= pas_temp_2; ++a) {
        d = a;
        x = (d % 8);
        petir_bitboard_mask::rook_attack_right_mask[a] = 0;
        while ((x < 7)) {
            (d = pas::increment(d, 1));
            petir_bitboard_mask::rook_attack_right_mask[a] = (petir_bitboard_mask::rook_attack_right_mask[a] | petir_bitboard_mask::bit2nmask[d]);
            x = (d % 8);
        }
        d = a;
        x = (d % 8);
        petir_bitboard_mask::rook_attack_left_mask[a] = 0;
        while ((x > 0)) {
            (d = pas::increment(d, pas::negate(1)));
            petir_bitboard_mask::rook_attack_left_mask[a] = (petir_bitboard_mask::rook_attack_left_mask[a] | petir_bitboard_mask::bit2nmask[d]);
            x = (d % 8);
        }
        d = a;
        x = (d % 8);
        y = (d / 8);
        petir_bitboard_mask::bishop_attack_plus9_mask[a] = 0;
        while (((x < 7) && (y < 7))) {
            (d = pas::increment(d, 9));
            petir_bitboard_mask::bishop_attack_plus9_mask[a] = (petir_bitboard_mask::bishop_attack_plus9_mask[a] | petir_bitboard_mask::bit2nmask[d]);
            x = (d % 8);
            y = (d / 8);
        }
        d = a;
        x = (d % 8);
        y = (d / 8);
        petir_bitboard_mask::bishop_attack_plus7_mask[a] = 0;
        while (((x > 0) && (y < 7))) {
            (d = pas::increment(d, 7));
            petir_bitboard_mask::bishop_attack_plus7_mask[a] = (petir_bitboard_mask::bishop_attack_plus7_mask[a] | petir_bitboard_mask::bit2nmask[d]);
            x = (d % 8);
            y = (d / 8);
        }
        d = a;
        x = (d % 8);
        y = (d / 8);
        petir_bitboard_mask::bishop_attack_min9_mask[a] = 0;
        while (((x > 0) && (y > 0))) {
            (d = pas::increment(d, pas::negate(9)));
            petir_bitboard_mask::bishop_attack_min9_mask[a] = (petir_bitboard_mask::bishop_attack_min9_mask[a] | petir_bitboard_mask::bit2nmask[d]);
            x = (d % 8);
            y = (d / 8);
        }
        d = a;
        x = (a % 8);
        y = (a / 8);
        petir_bitboard_mask::bishop_attack_min7_mask[a] = 0;
        while (((x < 7) && (y > 0))) {
            (d = pas::increment(d, pas::negate(7)));
            petir_bitboard_mask::bishop_attack_min7_mask[a] = (petir_bitboard_mask::bishop_attack_min7_mask[a] | petir_bitboard_mask::bit2nmask[d]);
            x = (d % 8);
            y = (d / 8);
        }
        petir_bitboard_mask::bishop_attack_up_mask[a] = (petir_bitboard_mask::bishop_attack_plus7_mask[a] | petir_bitboard_mask::bishop_attack_plus9_mask[a]);
        petir_bitboard_mask::bishop_attack_down_mask[a] = (petir_bitboard_mask::bishop_attack_min7_mask[a] | petir_bitboard_mask::bishop_attack_min9_mask[a]);
    }
    pas_temp_3 = 63;
    for (a = 0; a <= pas_temp_3; ++a) {
        pas_temp_4 = 63;
        for (b = 0; b <= pas_temp_4; ++b) {
            petir_bitboard_mask::direction[a][b] = 0;
            if (((a % 8) == (b % 8))) {
                if ((a > b)) {
                    petir_bitboard_mask::direction[a][b] = pas::negate(8);
                } else {
                    if ((b > a)) {
                        petir_bitboard_mask::direction[a][b] = 8;
                    }
                }
            }
            if (((a / 8) == (b / 8))) {
                if ((a > b)) {
                    petir_bitboard_mask::direction[a][b] = pas::negate(1);
                } else {
                    if ((b > a)) {
                        petir_bitboard_mask::direction[a][b] = 1;
                    }
                }
            }
            if ((pas::add((a / 8), (a % 8)) == pas::add((b / 8), (b % 8)))) {
                if ((a > b)) {
                    petir_bitboard_mask::direction[a][b] = pas::negate(7);
                } else {
                    if ((b > a)) {
                        petir_bitboard_mask::direction[a][b] = 7;
                    }
                }
            }
            if ((pas::sub((a / 8), (a % 8)) == pas::sub((b / 8), (b % 8)))) {
                if ((a > b)) {
                    petir_bitboard_mask::direction[a][b] = pas::negate(9);
                } else {
                    if ((b > a)) {
                        petir_bitboard_mask::direction[a][b] = 9;
                    }
                }
            }
        }
    }
    pas_temp_5 = 63;
    for (a = 0; a <= pas_temp_5; ++a) {
        pas_temp_6 = 63;
        for (b = 0; b <= pas_temp_6; ++b) {
            petir_bitboard_mask::direction2[a][b] = 0;
            if ((petir_bitboard_mask::direction[a][b] == 7)) {
                petir_bitboard_mask::direction2[a][b] = 8;
            } else {
                if ((petir_bitboard_mask::direction[a][b] == pas::negate(7))) {
                    petir_bitboard_mask::direction2[a][b] = 7;
                } else {
                    if ((petir_bitboard_mask::direction[a][b] == 9)) {
                        petir_bitboard_mask::direction2[a][b] = 6;
                    } else {
                        if ((petir_bitboard_mask::direction[a][b] == pas::negate(9))) {
                            petir_bitboard_mask::direction2[a][b] = 5;
                        } else {
                            if ((petir_bitboard_mask::direction[a][b] == 1)) {
                                petir_bitboard_mask::direction2[a][b] = 4;
                            } else {
                                if ((petir_bitboard_mask::direction[a][b] == pas::negate(1))) {
                                    petir_bitboard_mask::direction2[a][b] = 3;
                                } else {
                                    if ((petir_bitboard_mask::direction[a][b] == 8)) {
                                        petir_bitboard_mask::direction2[a][b] = 2;
                                    } else {
                                        if ((petir_bitboard_mask::direction[a][b] == pas::negate(8))) {
                                            petir_bitboard_mask::direction2[a][b] = 1;
                                        } else {
                                            ;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    pas_temp_7 = 63;
    for (a = 0; a <= pas_temp_7; ++a) {
        pas_temp_8 = 63;
        for (b = 0; b <= pas_temp_8; ++b) {
            petir_bitboard_mask::pathmask[a][b] = 0;
            if ((((a % 8) == (b % 8)) && (pas::abs(pas::sub(a, b)) >= 16))) {
                if ((b > a)) {
                    c = a;
                    d = b;
                } else {
                    c = b;
                    d = a;
                }
                (c = pas::increment(c, 8));
                while ((c < d)) {
                    petir_bitboard_mask::pathmask[a][b] = (petir_bitboard_mask::pathmask[a][b] | petir_bitboard_mask::bit2nmask[c]);
                    (c = pas::increment(c, 8));
                }
            }
            if ((((a / 8) == (b / 8)) && (pas::abs(pas::sub(a, b)) >= 2))) {
                if ((b > a)) {
                    c = a;
                    d = b;
                } else {
                    c = b;
                    d = a;
                }
                (c = pas::increment(c, 1));
                while ((c < d)) {
                    petir_bitboard_mask::pathmask[a][b] = (petir_bitboard_mask::pathmask[a][b] | petir_bitboard_mask::bit2nmask[c]);
                    (c = pas::increment(c, 1));
                }
            }
            if (((pas::add((a / 8), (a % 8)) == pas::add((b / 8), (b % 8))) && (pas::abs(pas::sub(a, b)) >= 14))) {
                if ((b > a)) {
                    c = a;
                    d = b;
                } else {
                    c = b;
                    d = a;
                }
                (c = pas::increment(c, 7));
                while ((c < d)) {
                    petir_bitboard_mask::pathmask[a][b] = (petir_bitboard_mask::pathmask[a][b] | petir_bitboard_mask::bit2nmask[c]);
                    (c = pas::increment(c, 7));
                }
            }
            if (((pas::sub((a / 8), (a % 8)) == pas::sub((b / 8), (b % 8))) && (pas::abs(pas::sub(a, b)) >= 18))) {
                if ((b > a)) {
                    c = a;
                    d = b;
                } else {
                    c = b;
                    d = a;
                }
                (c = pas::increment(c, 9));
                while ((c < d)) {
                    petir_bitboard_mask::pathmask[a][b] = (petir_bitboard_mask::pathmask[a][b] | petir_bitboard_mask::bit2nmask[c]);
                    (c = pas::increment(c, 9));
                }
            }
        }
    }
    pas_temp_9 = 63;
    for (a = 0; a <= pas_temp_9; ++a) {
        petir_bitboard_mask::bit2nmask90[a] = petir_bitboard_mask::bit2nmask[petir_bitboard::r90mask[a]];
        petir_bitboard_mask::bit2nmaska8h1[a] = petir_bitboard_mask::bit2nmask[petir_bitboard::a8h1mask[a]];
        petir_bitboard_mask::bit2nmaskh8a1[a] = petir_bitboard_mask::bit2nmask[petir_bitboard::h8a1mask[a]];
    }
    pas_temp_10 = 63;
    for (a = 0; a <= pas_temp_10; ++a) {
        n = 0;
        pas_temp_11 = 8;
        for (b = 1; b <= pas_temp_11; ++b) {
            x = (a % 8);
            (x = pas::increment(x, petir_bitboard::x_kuda_direction[b]));
            y = (a / 8);
            (y = pas::increment(y, petir_bitboard::y_kuda_direction[b]));
            if ((((x >= 0 && x <= 7)) && ((y >= 0 && y <= 7)))) {
                n = (n | petir_bitboard_mask::bit2nmask[pas::add(pas::mul(y, 8), x)]);
            }
        }
        petir_bitboard_mask::knightmask[a] = n;
    }
    pas_temp_12 = 63;
    for (a = 0; a <= pas_temp_12; ++a) {
        n = 0;
        pas_temp_13 = 8;
        for (b = 1; b <= pas_temp_13; ++b) {
            x = (a % 8);
            (x = pas::increment(x, petir_bitboard::x_king_direction[b]));
            y = (a / 8);
            (y = pas::increment(y, petir_bitboard::y_king_direction[b]));
            if ((((x >= 0 && x <= 7)) && ((y >= 0 && y <= 7)))) {
                n = (n | petir_bitboard_mask::bit2nmask[pas::add(pas::mul(y, 8), x)]);
            }
        }
        petir_bitboard_mask::kingmask[a] = n;
    }
    pas_temp_14 = 63;
    for (b = 0; b <= pas_temp_14; ++b) {
        pas_temp_15 = 255;
        for (a = 0; a <= pas_temp_15; ++a) {
            n = 0;
            x = (b % 8);
            y = (b / 8);
            z = x;
            if ((x > 0)) {
                (x = pas::decrement(x, 1));
                while (((x >= 0) && ((a & petir_bitboard_mask::bit2nmask[x]) == 0))) {
                    n = (n | petir_bitboard_mask::bit2nmask[pas::sub(b, pas::sub(z, x))]);
                    (x = pas::decrement(x, 1));
                }
                if ((x >= 0)) {
                    n = (n | petir_bitboard_mask::bit2nmask[pas::sub(b, pas::sub(z, x))]);
                }
            }
            x = z;
            if ((x < 7)) {
                (x = pas::increment(x, 1));
                while (((x <= 7) && ((a & petir_bitboard_mask::bit2nmask[x]) == 0))) {
                    n = (n | petir_bitboard_mask::bit2nmask[pas::add(b, pas::sub(x, z))]);
                    (x = pas::increment(x, 1));
                }
                if ((x <= 7)) {
                    n = (n | petir_bitboard_mask::bit2nmask[pas::add(b, pas::sub(x, z))]);
                }
            }
            petir_bitboard_mask::horzmask2[b][(pas::shr(a, 1) & 63)] = n;
            n = 0;
            z = y;
            if ((y > 0)) {
                (y = pas::decrement(y, 1));
                while (((y >= 0) && ((a & petir_bitboard_mask::bit2nmask[y]) == 0))) {
                    n = (n | petir_bitboard_mask::bit2nmask[pas::sub(b, pas::mul(pas::sub(z, y), 8))]);
                    (y = pas::decrement(y, 1));
                }
                if ((y >= 0)) {
                    n = (n | petir_bitboard_mask::bit2nmask[pas::sub(b, pas::mul(pas::sub(z, y), 8))]);
                }
            }
            y = z;
            if ((y < 7)) {
                (y = pas::increment(y, 1));
                while (((y <= 7) && ((a & petir_bitboard_mask::bit2nmask[y]) == 0))) {
                    n = (n | petir_bitboard_mask::bit2nmask[pas::add(b, pas::mul(pas::sub(y, z), 8))]);
                    (y = pas::increment(y, 1));
                }
                if ((y <= 7)) {
                    n = (n | petir_bitboard_mask::bit2nmask[pas::add(b, pas::mul(pas::sub(y, z), 8))]);
                }
            }
            petir_bitboard_mask::vertmask2[b][(pas::shr(a, 1) & 63)] = n;
        }
    }
    pas_temp_16 = petir_header::h8;
    for (b = petir_header::a1; b <= pas_temp_16; ++b) {
        ld = pas::sub(petir_bitboard::diag_a8h1_length[b], 1);
        bld = (petir_bitboard_mask::bit2nmask[pas::add(ld, 1)] - 1);
        ld2 = pas::sub(6, ld);
        bld2 = (petir_bitboard_mask::bit2nmask[pas::add(ld2, 1)] - 1);
        pas_temp_17 = bld;
        for (a = 0; a <= pas_temp_17; ++a) {
            n = 0;
            x = petir_bitboard::diag_a8h1_posx[b];
            z = x;
            if ((x > 0)) {
                (x = pas::decrement(x, 1));
                while (((x >= 0) && ((a & petir_bitboard_mask::bit2nmask[x]) == 0))) {
                    n = (n | petir_bitboard_mask::bit2nmask[pas::sub(b, pas::mul(pas::sub(z, x), 7))]);
                    (x = pas::decrement(x, 1));
                }
                if ((x >= 0)) {
                    n = (n | petir_bitboard_mask::bit2nmask[pas::sub(b, pas::mul(pas::sub(z, x), 7))]);
                }
            }
            x = z;
            if ((x < ld)) {
                (x = pas::increment(x, 1));
                while (((x <= ld) && ((a & petir_bitboard_mask::bit2nmask[x]) == 0))) {
                    n = (n | petir_bitboard_mask::bit2nmask[pas::add(b, pas::mul(pas::sub(x, z), 7))]);
                    (x = pas::increment(x, 1));
                }
                if ((x <= ld)) {
                    n = (n | petir_bitboard_mask::bit2nmask[pas::add(b, pas::mul(pas::sub(x, z), 7))]);
                }
            }
            petir_bitboard_mask::diaga8h1mask[b][a] = n;
            if ((ld2 >= 0)) {
                pas_temp_18 = bld2;
                for (c = 0; c <= pas_temp_18; ++c) {
                    petir_bitboard_mask::diaga8h1mask[b][pas::add(pas::shl(c, pas::add(ld, 1)), a)] = n;
                }
            }
        }
        ld = pas::sub(petir_bitboard::diag_h8a1_length[b], 1);
        bld = (petir_bitboard_mask::bit2nmask[pas::add(ld, 1)] - 1);
        ld2 = pas::sub(6, ld);
        bld2 = (petir_bitboard_mask::bit2nmask[pas::add(ld2, 1)] - 1);
        pas_temp_19 = bld;
        for (a = 0; a <= pas_temp_19; ++a) {
            n = 0;
            x = petir_bitboard::diag_h8a1_posx[b];
            z = x;
            if ((x > 0)) {
                (x = pas::decrement(x, 1));
                while (((x >= 0) && ((a & petir_bitboard_mask::bit2nmask[pas::sub(ld, x)]) == 0))) {
                    n = (n | petir_bitboard_mask::bit2nmask[pas::sub(b, pas::mul(pas::sub(z, x), 9))]);
                    (x = pas::decrement(x, 1));
                }
                if ((x >= 0)) {
                    n = (n | petir_bitboard_mask::bit2nmask[pas::sub(b, pas::mul(pas::sub(z, x), 9))]);
                }
            }
            x = z;
            if ((x < ld)) {
                (x = pas::increment(x, 1));
                while (((x <= ld) && ((a & petir_bitboard_mask::bit2nmask[pas::sub(ld, x)]) == 0))) {
                    n = (n | petir_bitboard_mask::bit2nmask[pas::add(b, pas::mul(pas::sub(x, z), 9))]);
                    (x = pas::increment(x, 1));
                }
                if ((x <= ld)) {
                    n = (n | petir_bitboard_mask::bit2nmask[pas::add(b, pas::mul(pas::sub(x, z), 9))]);
                }
            }
            petir_bitboard_mask::diagh8a1mask[b][a] = n;
            if ((ld2 >= 0)) {
                pas_temp_20 = bld2;
                for (c = 0; c <= pas_temp_20; ++c) {
                    petir_bitboard_mask::diagh8a1mask[b][pas::add(pas::shl(c, pas::add(ld, 1)), a)] = n;
                }
            }
        }
    }
    pas_temp_21 = 65535;
    for (a = 1; a <= pas_temp_21; ++a) {
        b = a;
        n = 1;
        while (((b & 1) == 0)) {
            b = pas::shr(b, 1);
            (n = pas::increment(n, 1));
        }
        petir_bitboard_mask::firstbitmask[a] = (n - 1);
    }
    petir_bitboard_mask::firstbitmask[0] = 0;
}

}
