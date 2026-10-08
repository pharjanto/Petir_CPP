// Faithful translation of notation.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_notation {
// notation.pas:256
pas::String movetonotation(std::int32_t move, std::int32_t ep, std::int32_t giliran) {
    pas::String return_value{};
    std::int32_t sfrom{};
    std::int32_t sto{};
    std::int32_t promosi{};
    std::int32_t n{};
    return_value = pas::String("");
    sfrom = (move & 127);
    sto = (pas::shr(move, 7) & 127);
    promosi = (pas::shr(move, 14) & 7);
    if ((sto == petir_header::_rokadependek)) {
        if ((giliran == petir_header::_sisiputih)) {
            return_value = pas::String("e1g1");
        } else {
            return_value = pas::String("e8g8");
        }
    } else {
        if ((sto == petir_header::_rokadepanjang)) {
            if ((giliran == petir_header::_sisiputih)) {
                return_value = pas::String("e1c1");
            } else {
                return_value = pas::String("e8c8");
            }
        } else {
            if ((sto == petir_header::_en_passant)) {
                if ((giliran == petir_header::_sisiputih)) {
                    if ((ep > (sfrom % 8))) {
                        sto = pas::add(sfrom, 9);
                    } else {
                        sto = pas::add(sfrom, 7);
                    }
                } else {
                    if ((ep > (sfrom % 8))) {
                        sto = pas::sub(sfrom, 7);
                    } else {
                        sto = pas::sub(sfrom, 9);
                    }
                }
                return_value = (petir_notation::getnotasi(sfrom) + petir_notation::getnotasi(sto));
            } else {
                return_value = (petir_notation::getnotasi(sfrom) + petir_notation::getnotasi(sto));
                if ((promosi == petir_header::promosi_mentri)) {
                    return_value = (return_value + 'q');
                } else {
                    if ((promosi == petir_header::promosi_kuda)) {
                        return_value = (return_value + 'n');
                    } else {
                        if ((promosi == petir_header::promosi_gajah)) {
                            return_value = (return_value + 'b');
                        } else {
                            if ((promosi == petir_header::promosi_benteng)) {
                                return_value = (return_value + 'r');
                            }
                        }
                    }
                }
            }
        }
    }
    return return_value;
}

// notation.pas:195
std::int32_t notationtomove(pas::String notasi, petir_header::tdata data, std::uint8_t giliran) {
    std::int32_t return_value{};
    std::uint8_t x{};
    std::uint8_t y{};
    std::uint8_t sfrom{};
    std::uint8_t sto{};
    if (((notasi[1] >= 'a' && notasi[1] <= 'h'))) {
        x = pas::sub(pas::ord(notasi[1]), 97);
        y = pas::sub(pas::strtoint(notasi[2]), 1);
        sfrom = pas::add(pas::mul(y, 8), x);
        x = pas::sub(pas::ord(notasi[3]), 97);
        y = pas::sub(pas::strtoint(notasi[4]), 1);
        sto = pas::add(pas::mul(y, 8), x);
        if ((((sfrom == petir_header::e1) && (sto == petir_header::g1)) && (data.papan[petir_header::e1] == petir_header::_rajaputih))) {
            return_value = pas::add(sfrom, pas::shl(petir_header::_rokadependek, 7));
            return return_value;
        }
        if ((((sfrom == petir_header::e1) && (sto == petir_header::c1)) && (data.papan[petir_header::e1] == petir_header::_rajaputih))) {
            return_value = pas::add(sfrom, pas::shl(petir_header::_rokadepanjang, 7));
            return return_value;
        }
        if ((((sfrom == petir_header::e8) && (sto == petir_header::g8)) && (data.papan[petir_header::e8] == petir_header::_rajahitam))) {
            return_value = pas::add(sfrom, pas::shl(petir_header::_rokadependek, 7));
            return return_value;
        }
        if ((((sfrom == petir_header::e8) && (sto == petir_header::c8)) && (data.papan[petir_header::e8] == petir_header::_rajahitam))) {
            return_value = pas::add(sfrom, pas::shl(petir_header::_rokadepanjang, 7));
            return return_value;
        }
        if ((((data.papan[sfrom] == petir_header::_pionputih) && ((pas::sub(sto, sfrom) == 7) || (pas::sub(sto, sfrom) == 9))) && (data.papan[sto] == 0))) {
            return_value = pas::add(sfrom, pas::shl(petir_header::_en_passant, 7));
            return return_value;
        }
        if ((((data.papan[sfrom] == petir_header::_pionhitam) && ((pas::sub(sfrom, sto) == 7) || (pas::sub(sfrom, sto) == 9))) && (data.papan[sto] == 0))) {
            return_value = pas::add(sfrom, pas::shl(petir_header::_en_passant, 7));
            return return_value;
        }
        return_value = pas::add(sfrom, pas::shl(sto, 7));
        if ((pas::length(notasi) > 4)) {
            if ((notasi[5] == 'q')) {
                return_value = pas::add(return_value, pas::shl(petir_header::promosi_mentri, 14));
            } else {
                if ((notasi[5] == 'r')) {
                    return_value = pas::add(return_value, pas::shl(petir_header::promosi_benteng, 14));
                } else {
                    if ((notasi[5] == 'b')) {
                        return_value = pas::add(return_value, pas::shl(petir_header::promosi_gajah, 14));
                    } else {
                        if ((notasi[5] == 'n')) {
                            return_value = pas::add(return_value, pas::shl(petir_header::promosi_kuda, 14));
                        }
                    }
                }
            }
        }
    } else {
        if (((pas::copy(notasi, 1, 3) == pas::String("O-O")) && (giliran == petir_header::_sisiputih))) {
            return_value = pas::add(petir_header::e1, pas::shl(petir_header::_rokadependek, 7));
        } else {
            if (((pas::copy(notasi, 1, 3) == pas::String("O-O")) && (giliran == petir_header::_sisihitam))) {
                return_value = pas::add(petir_header::e8, pas::shl(petir_header::_rokadependek, 7));
            } else {
                if (((pas::copy(notasi, 1, 5) == pas::String("O-O-O")) && (giliran == petir_header::_sisihitam))) {
                    return_value = pas::add(petir_header::e8, pas::shl(petir_header::_rokadepanjang, 7));
                } else {
                    if (((pas::copy(notasi, 1, 5) == pas::String("O-O-O")) && (giliran == petir_header::_sisiputih))) {
                        return_value = pas::add(petir_header::e1, pas::shl(petir_header::_rokadepanjang, 7));
                    }
                }
            }
        }
    }
    return return_value;
}

// notation.pas:93
pas::String shortnotationtolong(pas::String notasi, petir_header::tdata data, std::uint8_t giliran) {
    pas::String return_value{};
// notation.pas:95
    auto notasi_pion = [&]() -> pas::String {
    pas::String return_value{};
    std::int32_t sfrom{};
    std::int32_t sto{};
    std::int32_t x{};
    if ((pas::length(notasi) == 2)) {
        sto = petir_notation::squaretobinary(notasi);
        if ((giliran == petir_header::_sisiputih)) {
            if ((data.papan[pas::sub(sto, 8)] == petir_header::_pionputih)) {
                sfrom = pas::sub(sto, 8);
            } else {
                sfrom = pas::sub(sto, 16);
            }
        } else {
            if ((data.papan[pas::add(sto, 8)] == petir_header::_pionhitam)) {
                sfrom = pas::add(sto, 8);
            } else {
                sfrom = pas::add(sto, 16);
            }
        }
    } else {
        x = pas::sub(pas::ord(notasi[1]), 97);
        sto = petir_notation::squaretobinary(pas::copy(notasi, 3, 2));
        if ((giliran == petir_header::_sisiputih)) {
            if ((x < (sto % 8))) {
                sfrom = pas::sub(sto, 9);
            } else {
                sfrom = pas::sub(sto, 7);
            }
        } else {
            if ((x < (sto % 8))) {
                sfrom = pas::add(sto, 7);
            } else {
                sfrom = pas::add(sto, 9);
            }
        }
    }
    return_value = (petir_notation::getnotasi(sfrom) + petir_notation::getnotasi(sto));
    return return_value;
};

// notation.pas:127
    auto notasi_perwira = [&]() -> pas::String {
    pas::String return_value{};
    std::int32_t sfrom{};
    std::int32_t sto{};
    std::int32_t x{};
    std::int32_t y{};
    std::uint64_t a{};
    std::int32_t pas_temp_1{};
    x = 9;
    y = 9;
    if ((pas::length(notasi) == 3)) {
        sto = petir_notation::squaretobinary(pas::copy(notasi, 2, 2));
    } else {
        sto = petir_notation::squaretobinary(pas::copy(notasi, pas::sub(pas::length(notasi), 1), 2));
        if ((notasi[2] != 'x')) {
            if (((notasi[2] >= 'a' && notasi[2] <= 'h'))) {
                x = pas::sub(pas::ord(notasi[2]), 97);
            } else {
                y = pas::sub(pas::strtoint(notasi[2]), 1);
            }
        }
    }
    pas_temp_1 = notasi[1];
    if (((pas_temp_1 == 'N'))) {
        a = petir_bitboard_mask::knightmask[sto];
        if ((giliran == petir_header::_sisiputih)) {
            a = (a & data.knightwhite);
        } else {
            a = (a & data.knightblack);
        }
    }
    else if (((pas_temp_1 == 'B'))) {
        a = petir_bitboard_mask::diaga8h1mask[sto][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[sto]) & 255)];
        a = (a | petir_bitboard_mask::diagh8a1mask[sto][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[sto]) & 255)]);
        if ((giliran == petir_header::_sisiputih)) {
            a = (a & data.bishopwhite);
        } else {
            a = (a & data.bishopblack);
        }
    }
    else if (((pas_temp_1 == 'R'))) {
        a = petir_bitboard_mask::horzmask2[sto][(pas::shr(data.allpieces, pas::add((sto & 56), 1)) & 63)];
        a = (a | petir_bitboard_mask::vertmask2[sto][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[sto]) & 63)]);
        if ((giliran == petir_header::_sisiputih)) {
            a = (a & data.rookwhite);
        } else {
            a = (a & data.rookblack);
        }
    }
    else if (((pas_temp_1 == 'Q'))) {
        a = petir_bitboard_mask::horzmask2[sto][(pas::shr(data.allpieces, pas::add((sto & 56), 1)) & 63)];
        a = (a | petir_bitboard_mask::vertmask2[sto][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[sto]) & 63)]);
        a = (a | petir_bitboard_mask::diaga8h1mask[sto][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[sto]) & 255)]);
        a = (a | petir_bitboard_mask::diagh8a1mask[sto][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[sto]) & 255)]);
        if ((giliran == petir_header::_sisiputih)) {
            a = (a & data.queenwhite);
        } else {
            a = (a & data.queenblack);
        }
    }
    else if (((pas_temp_1 == 'K'))) {
        a = petir_bitboard_mask::kingmask[sto];
        if ((giliran == petir_header::_sisiputih)) {
            a = (a & data.kingwhite);
        } else {
            a = (a & data.kingblack);
        }
    }
    if ((x != 9)) {
        a = (a & petir_bitboard_mask::file_mask[x]);
    } else {
        if ((y != 9)) {
            a = (a & petir_bitboard_mask::rank_mask[y]);
        }
    }
    sfrom = petir_bitboard_mask::firstbitp(pas::address(a));
    return_value = (petir_notation::getnotasi(sfrom) + petir_notation::getnotasi(sto));
    return return_value;
};

    if (((notasi[1] >= 'a' && notasi[1] <= 'h'))) {
        return_value = notasi_pion();
    } else {
        if (((notasi[1] == 'N') || (notasi[1] == 'B') || (notasi[1] == 'R') || (notasi[1] == 'Q') || (notasi[1] == 'K'))) {
            return_value = notasi_perwira();
        }
    }
    return return_value;
}

// notation.pas:22
pas::String getnotasi(std::int32_t square) {
    pas::String return_value{};
    std::int32_t pas_temp_1{};
    pas_temp_1 = square;
    if (((pas_temp_1 == petir_header::a1))) {
        return_value = pas::String("a1");
    }
    else if (((pas_temp_1 == petir_header::a2))) {
        return_value = pas::String("a2");
    }
    else if (((pas_temp_1 == petir_header::a3))) {
        return_value = pas::String("a3");
    }
    else if (((pas_temp_1 == petir_header::a4))) {
        return_value = pas::String("a4");
    }
    else if (((pas_temp_1 == petir_header::a5))) {
        return_value = pas::String("a5");
    }
    else if (((pas_temp_1 == petir_header::a6))) {
        return_value = pas::String("a6");
    }
    else if (((pas_temp_1 == petir_header::a7))) {
        return_value = pas::String("a7");
    }
    else if (((pas_temp_1 == petir_header::a8))) {
        return_value = pas::String("a8");
    }
    else if (((pas_temp_1 == petir_header::b1))) {
        return_value = pas::String("b1");
    }
    else if (((pas_temp_1 == petir_header::b2))) {
        return_value = pas::String("b2");
    }
    else if (((pas_temp_1 == petir_header::b3))) {
        return_value = pas::String("b3");
    }
    else if (((pas_temp_1 == petir_header::b4))) {
        return_value = pas::String("b4");
    }
    else if (((pas_temp_1 == petir_header::b5))) {
        return_value = pas::String("b5");
    }
    else if (((pas_temp_1 == petir_header::b6))) {
        return_value = pas::String("b6");
    }
    else if (((pas_temp_1 == petir_header::b7))) {
        return_value = pas::String("b7");
    }
    else if (((pas_temp_1 == petir_header::b8))) {
        return_value = pas::String("b8");
    }
    else if (((pas_temp_1 == petir_header::c1))) {
        return_value = pas::String("c1");
    }
    else if (((pas_temp_1 == petir_header::c2))) {
        return_value = pas::String("c2");
    }
    else if (((pas_temp_1 == petir_header::c3))) {
        return_value = pas::String("c3");
    }
    else if (((pas_temp_1 == petir_header::c4))) {
        return_value = pas::String("c4");
    }
    else if (((pas_temp_1 == petir_header::c5))) {
        return_value = pas::String("c5");
    }
    else if (((pas_temp_1 == petir_header::c6))) {
        return_value = pas::String("c6");
    }
    else if (((pas_temp_1 == petir_header::c7))) {
        return_value = pas::String("c7");
    }
    else if (((pas_temp_1 == petir_header::c8))) {
        return_value = pas::String("c8");
    }
    else if (((pas_temp_1 == petir_header::d1))) {
        return_value = pas::String("d1");
    }
    else if (((pas_temp_1 == petir_header::d2))) {
        return_value = pas::String("d2");
    }
    else if (((pas_temp_1 == petir_header::d3))) {
        return_value = pas::String("d3");
    }
    else if (((pas_temp_1 == petir_header::d4))) {
        return_value = pas::String("d4");
    }
    else if (((pas_temp_1 == petir_header::d5))) {
        return_value = pas::String("d5");
    }
    else if (((pas_temp_1 == petir_header::d6))) {
        return_value = pas::String("d6");
    }
    else if (((pas_temp_1 == petir_header::d7))) {
        return_value = pas::String("d7");
    }
    else if (((pas_temp_1 == petir_header::d8))) {
        return_value = pas::String("d8");
    }
    else if (((pas_temp_1 == petir_header::e1))) {
        return_value = pas::String("e1");
    }
    else if (((pas_temp_1 == petir_header::e2))) {
        return_value = pas::String("e2");
    }
    else if (((pas_temp_1 == petir_header::e3))) {
        return_value = pas::String("e3");
    }
    else if (((pas_temp_1 == petir_header::e4))) {
        return_value = pas::String("e4");
    }
    else if (((pas_temp_1 == petir_header::e5))) {
        return_value = pas::String("e5");
    }
    else if (((pas_temp_1 == petir_header::e6))) {
        return_value = pas::String("e6");
    }
    else if (((pas_temp_1 == petir_header::e7))) {
        return_value = pas::String("e7");
    }
    else if (((pas_temp_1 == petir_header::e8))) {
        return_value = pas::String("e8");
    }
    else if (((pas_temp_1 == petir_header::f1))) {
        return_value = pas::String("f1");
    }
    else if (((pas_temp_1 == petir_header::f2))) {
        return_value = pas::String("f2");
    }
    else if (((pas_temp_1 == petir_header::f3))) {
        return_value = pas::String("f3");
    }
    else if (((pas_temp_1 == petir_header::f4))) {
        return_value = pas::String("f4");
    }
    else if (((pas_temp_1 == petir_header::f5))) {
        return_value = pas::String("f5");
    }
    else if (((pas_temp_1 == petir_header::f6))) {
        return_value = pas::String("f6");
    }
    else if (((pas_temp_1 == petir_header::f7))) {
        return_value = pas::String("f7");
    }
    else if (((pas_temp_1 == petir_header::f8))) {
        return_value = pas::String("f8");
    }
    else if (((pas_temp_1 == petir_header::g1))) {
        return_value = pas::String("g1");
    }
    else if (((pas_temp_1 == petir_header::g2))) {
        return_value = pas::String("g2");
    }
    else if (((pas_temp_1 == petir_header::g3))) {
        return_value = pas::String("g3");
    }
    else if (((pas_temp_1 == petir_header::g4))) {
        return_value = pas::String("g4");
    }
    else if (((pas_temp_1 == petir_header::g5))) {
        return_value = pas::String("g5");
    }
    else if (((pas_temp_1 == petir_header::g6))) {
        return_value = pas::String("g6");
    }
    else if (((pas_temp_1 == petir_header::g7))) {
        return_value = pas::String("g7");
    }
    else if (((pas_temp_1 == petir_header::g8))) {
        return_value = pas::String("g8");
    }
    else if (((pas_temp_1 == petir_header::h1))) {
        return_value = pas::String("h1");
    }
    else if (((pas_temp_1 == petir_header::h2))) {
        return_value = pas::String("h2");
    }
    else if (((pas_temp_1 == petir_header::h3))) {
        return_value = pas::String("h3");
    }
    else if (((pas_temp_1 == petir_header::h4))) {
        return_value = pas::String("h4");
    }
    else if (((pas_temp_1 == petir_header::h5))) {
        return_value = pas::String("h5");
    }
    else if (((pas_temp_1 == petir_header::h6))) {
        return_value = pas::String("h6");
    }
    else if (((pas_temp_1 == petir_header::h7))) {
        return_value = pas::String("h7");
    }
    else if (((pas_temp_1 == petir_header::h8))) {
        return_value = pas::String("h8");
    }
    return return_value;
}

// notation.pas:14
std::int32_t squaretobinary(pas::String s) {
    std::int32_t return_value{};
    std::uint8_t x{};
    std::uint8_t y{};
    x = pas::sub(pas::ord(s[1]), 97);
    y = pas::sub(pas::strtoint(s[2]), 1);
    return_value = pas::add(pas::mul(y, 8), x);
    return return_value;
}

}
