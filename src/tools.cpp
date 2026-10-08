// Faithful translation of tools.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_tools {
std::int32_t anu{};
// tools.pas:1620
bool white_attacked(petir_header::tdata& data, std::int32_t pos) {
    bool return_value{};
    std::uint64_t attack{};
    if ((pos > 63)) {
        return_value = false;
        return return_value;
    }
    if (((petir_bitboard_mask::knightmask[pos] & data.knightblack) != 0)) {
        return_value = true;
        return return_value;
    }
    attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
    attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
    if (((attack & (data.queenblack | data.rookblack)) != 0)) {
        return_value = true;
        return return_value;
    }
    attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
    attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
    if (((attack & (data.queenblack | data.bishopblack)) != 0)) {
        return_value = true;
        return return_value;
    }
    if (((((pos & 7) > 0) && (pos <= pas::sub(63, 7))) && (data.papan[pas::add(pos, 7)] == petir_header::_pionhitam))) {
        return_value = true;
        return return_value;
    }
    if (((((pos & 7) < 7) && (pos <= pas::sub(63, 9))) && (data.papan[pas::add(pos, 9)] == petir_header::_pionhitam))) {
        return_value = true;
        return return_value;
    }
    if (((petir_bitboard_mask::kingmask[pos] & data.kingblack) != 0)) {
        return_value = true;
        return return_value;
    }
    return_value = false;
    return return_value;
}

// tools.pas:1662
bool black_attacked(petir_header::tdata& data, std::int32_t pos) {
    bool return_value{};
    std::uint64_t attack{};
    if ((pos > 63)) {
        return_value = false;
        return return_value;
    }
    if (((petir_bitboard_mask::knightmask[pos] & data.knightwhite) != 0)) {
        return_value = true;
        return return_value;
    }
    attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
    attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
    if (((attack & (data.queenwhite | data.rookwhite)) != 0)) {
        return_value = true;
        return return_value;
    }
    attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
    attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
    if (((attack & (data.queenwhite | data.bishopwhite)) != 0)) {
        return_value = true;
        return return_value;
    }
    if (((((pos & 7) < 7) && (pos > 6)) && (data.papan[pas::sub(pos, 7)] == petir_header::_pionputih))) {
        return_value = true;
        return return_value;
    }
    if (((((pos & 7) > 0) && (pos > 8)) && (data.papan[pas::sub(pos, 9)] == petir_header::_pionputih))) {
        return_value = true;
        return return_value;
    }
    if (((petir_bitboard_mask::kingmask[pos] & data.kingwhite) != 0)) {
        return_value = true;
        return return_value;
    }
    return_value = false;
    return return_value;
}

// tools.pas:1545
bool white_checked(petir_header::tdata& data) {
    bool return_value{};
    std::uint64_t attack{};
    std::int32_t pos{};
    pos = petir_bitboard_mask::firstbitp(pas::address(data.kingwhite));
    if (((petir_bitboard_mask::knightmask[pos] & data.knightblack) != 0)) {
        return_value = true;
        return return_value;
    }
    attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
    attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
    if (((attack & (data.queenblack | data.rookblack)) != 0)) {
        return_value = true;
        return return_value;
    }
    attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
    attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
    if (((attack & (data.queenblack | data.bishopblack)) != 0)) {
        return_value = true;
        return return_value;
    }
    if (((petir_bitboard_mask::w_pawn_attack[pos] & data.pawnblack) != 0)) {
        return_value = true;
        return return_value;
    }
    if (((petir_bitboard_mask::kingmask[pos] & data.kingblack) != 0)) {
        return_value = true;
        return return_value;
    }
    return_value = false;
    return return_value;
}

// tools.pas:1581
bool black_checked(petir_header::tdata& data) {
    bool return_value{};
    std::uint64_t attack{};
    std::int32_t pos{};
    pos = petir_bitboard_mask::lastbitp(pas::address(data.kingblack));
    if (((petir_bitboard_mask::knightmask[pos] & data.knightwhite) != 0)) {
        return_value = true;
        return return_value;
    }
    attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
    attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
    if (((attack & (data.queenwhite | data.rookwhite)) != 0)) {
        return_value = true;
        return return_value;
    }
    attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
    attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
    if (((attack & (data.queenwhite | data.bishopwhite)) != 0)) {
        return_value = true;
        return return_value;
    }
    if (((petir_bitboard_mask::b_pawn_attack[pos] & data.pawnwhite) != 0)) {
        return_value = true;
        return return_value;
    }
    if (((petir_bitboard_mask::kingmask[pos] & data.kingwhite) != 0)) {
        return_value = true;
        return return_value;
    }
    return_value = false;
    return return_value;
}

// tools.pas:1514
void clearkiller() {
    std::int32_t a{};
    std::int32_t pas_temp_1{};
    pas_temp_1 = 63;
    for (a = 0; a <= pas_temp_1; ++a) {
        petir_header::killer0[a] = petir_header::_no_move;
        petir_header::killer1[a] = petir_header::_no_move;
    }
}

// tools.pas:1326
bool white_open_check(petir_header::tdata& data, std::uint16_t moves) {
    bool return_value{};
    std::int32_t dn{};
    std::int32_t pos2{};
    std::int32_t adn{};
    std::int32_t post{};
    std::int32_t posraja{};
    std::uint64_t temp{};
    pos2 = (moves & 127);
    posraja = petir_bitboard_mask::firstbitp(pas::address(data.kingwhite));
    dn = petir_bitboard_mask::direction[posraja][pos2];
    if ((dn == 0)) {
        return_value = false;
        return return_value;
    }
    adn = pas::abs(dn);
    post = (pas::shr(moves, 7) & 127);
    if ((post == petir_header::_en_passant)) {
        if ((data.ep > (pos2 & 7))) {
            post = pas::sub(pos2, 7);
        } else {
            post = pas::sub(pos2, 9);
        }
    } else {
        if ((post > 63)) {
            return_value = false;
            return return_value;
        }
    }
    if ((pas::abs(petir_bitboard_mask::direction[pos2][post]) == adn)) {
        return_value = false;
        return return_value;
    }
    if ((adn == 7)) {
        temp = (data.allpiecesa8h1 & (~petir_bitboard_mask::bit2nmaska8h1[pos2]));
        if (((petir_bitboard_mask::diaga8h1mask[posraja][(pas::shr(temp, petir_bitboard_mask::a8h1shiftmask[posraja]) & 255)] & (data.queenblack | data.bishopblack)) != 0)) {
            return_value = true;
            return return_value;
        }
    } else {
        if ((adn == 9)) {
            temp = (data.allpiecesh8a1 & (~petir_bitboard_mask::bit2nmaskh8a1[pos2]));
            if (((petir_bitboard_mask::diagh8a1mask[posraja][(pas::shr(temp, petir_bitboard_mask::h8a1shiftmask[posraja]) & 255)] & (data.queenblack | data.bishopblack)) != 0)) {
                return_value = true;
                return return_value;
            }
        } else {
            if ((adn == 8)) {
                temp = (data.allpiecesr90 & (~petir_bitboard_mask::bit2nmask90[pos2]));
                if (((petir_bitboard_mask::vertmask2[posraja][(pas::shr(temp, petir_bitboard_mask::vershiftmask[posraja]) & 63)] & (data.queenblack | data.rookblack)) != 0)) {
                    return_value = true;
                    return return_value;
                }
            } else {
                if ((adn == 1)) {
                    temp = (data.allpieces & (~petir_bitboard_mask::bit2nmask[pos2]));
                    if (((petir_bitboard_mask::horzmask2[posraja][(pas::shr(temp, pas::add((posraja & 56), 1)) & 63)] & (data.queenblack | data.rookblack)) != 0)) {
                        return_value = true;
                        return return_value;
                    }
                }
            }
        }
    }
    return_value = false;
    return return_value;
}

// tools.pas:1396
bool white_open_check2(petir_header::tdata& data, std::uint16_t pos2, std::uint16_t post, std::uint16_t posraja) {
    bool return_value{};
    std::int32_t dn{};
    std::int32_t adn{};
    std::uint64_t temp{};
    dn = petir_bitboard_mask::direction[posraja][pos2];
    if ((dn == 0)) {
        return_value = false;
        return return_value;
    }
    adn = pas::abs(dn);
    if ((pas::abs(petir_bitboard_mask::direction[pos2][post]) == adn)) {
        return_value = false;
        return return_value;
    }
    if ((adn == 7)) {
        temp = (data.allpiecesa8h1 & (~petir_bitboard_mask::bit2nmaska8h1[pos2]));
        if (((petir_bitboard_mask::diaga8h1mask[posraja][(pas::shr(temp, petir_bitboard_mask::a8h1shiftmask[posraja]) & 255)] & (data.queenwhite | data.bishopwhite)) != 0)) {
            return_value = true;
            return return_value;
        }
    } else {
        if ((adn == 9)) {
            temp = (data.allpiecesh8a1 & (~petir_bitboard_mask::bit2nmaskh8a1[pos2]));
            if (((petir_bitboard_mask::diagh8a1mask[posraja][(pas::shr(temp, petir_bitboard_mask::h8a1shiftmask[posraja]) & 255)] & (data.queenwhite | data.bishopwhite)) != 0)) {
                return_value = true;
                return return_value;
            }
        } else {
            if ((adn == 8)) {
                temp = (data.allpiecesr90 & (~petir_bitboard_mask::bit2nmask90[pos2]));
                if (((petir_bitboard_mask::vertmask2[posraja][(pas::shr(temp, petir_bitboard_mask::vershiftmask[posraja]) & 63)] & (data.queenwhite | data.rookwhite)) != 0)) {
                    return_value = true;
                    return return_value;
                }
            } else {
                if ((adn == 1)) {
                    temp = (data.allpieces & (~petir_bitboard_mask::bit2nmask[pos2]));
                    if (((petir_bitboard_mask::horzmask2[posraja][(pas::shr(temp, pas::add((posraja & 56), 1)) & 63)] & (data.queenwhite | data.rookwhite)) != 0)) {
                        return_value = true;
                        return return_value;
                    }
                }
            }
        }
    }
    return_value = false;
    return return_value;
}

// tools.pas:1454
bool black_open_check2(petir_header::tdata& data, std::uint16_t pos2, std::uint16_t post, std::uint16_t posraja) {
    bool return_value{};
    std::int32_t dn{};
    std::int32_t adn{};
    std::uint64_t temp{};
    dn = petir_bitboard_mask::direction[posraja][pos2];
    if ((dn == 0)) {
        return_value = false;
        return return_value;
    }
    adn = pas::abs(dn);
    if ((pas::abs(petir_bitboard_mask::direction[pos2][post]) == adn)) {
        return_value = false;
        return return_value;
    }
    if ((adn == 7)) {
        temp = (data.allpiecesa8h1 & (~petir_bitboard_mask::bit2nmaska8h1[pos2]));
        if (((petir_bitboard_mask::diaga8h1mask[posraja][(pas::shr(temp, petir_bitboard_mask::a8h1shiftmask[posraja]) & 255)] & (data.queenblack | data.bishopblack)) != 0)) {
            return_value = true;
            return return_value;
        }
    } else {
        if ((adn == 9)) {
            temp = (data.allpiecesh8a1 & (~petir_bitboard_mask::bit2nmaskh8a1[pos2]));
            if (((petir_bitboard_mask::diagh8a1mask[posraja][(pas::shr(temp, petir_bitboard_mask::h8a1shiftmask[posraja]) & 255)] & (data.queenblack | data.bishopblack)) != 0)) {
                return_value = true;
                return return_value;
            }
        } else {
            if ((adn == 8)) {
                temp = (data.allpiecesr90 & (~petir_bitboard_mask::bit2nmask90[pos2]));
                if (((petir_bitboard_mask::vertmask2[posraja][(pas::shr(temp, petir_bitboard_mask::vershiftmask[posraja]) & 63)] & (data.queenblack | data.rookblack)) != 0)) {
                    return_value = true;
                    return return_value;
                }
            } else {
                if ((adn == 1)) {
                    temp = (data.allpieces & (~petir_bitboard_mask::bit2nmask[pos2]));
                    if (((petir_bitboard_mask::horzmask2[posraja][(pas::shr(temp, pas::add((posraja & 56), 1)) & 63)] & (data.queenblack | data.rookblack)) != 0)) {
                        return_value = true;
                        return return_value;
                    }
                }
            }
        }
    }
    return_value = false;
    return return_value;
}

// tools.pas:1253
bool black_open_check(petir_header::tdata& data, std::uint16_t moves) {
    bool return_value{};
    std::int32_t dn{};
    std::int32_t pos2{};
    std::int32_t adn{};
    std::int32_t post{};
    std::int32_t posraja{};
    std::uint64_t temp{};
    pos2 = (moves & 127);
    posraja = petir_bitboard_mask::lastbitp(pas::address(data.kingblack));
    dn = petir_bitboard_mask::direction[posraja][pos2];
    if ((dn == 0)) {
        return_value = false;
        return return_value;
    }
    post = (pas::shr(moves, 7) & 127);
    if ((post == petir_header::_en_passant)) {
        if ((data.ep > (pos2 & 7))) {
            post = pas::add(pos2, 9);
        } else {
            post = pas::add(pos2, 7);
        }
    } else {
        if ((post > 63)) {
            return_value = false;
            return return_value;
        }
    }
    adn = pas::abs(dn);
    if ((pas::abs(petir_bitboard_mask::direction[pos2][post]) == adn)) {
        return_value = false;
        return return_value;
    }
    if ((adn == 7)) {
        temp = (data.allpiecesa8h1 & (~petir_bitboard_mask::bit2nmaska8h1[pos2]));
        if (((petir_bitboard_mask::diaga8h1mask[posraja][(pas::shr(temp, petir_bitboard_mask::a8h1shiftmask[posraja]) & 255)] & (data.queenwhite | data.bishopwhite)) != 0)) {
            return_value = true;
            return return_value;
        }
    } else {
        if ((adn == 9)) {
            temp = (data.allpiecesh8a1 & (~petir_bitboard_mask::bit2nmaskh8a1[pos2]));
            if (((petir_bitboard_mask::diagh8a1mask[posraja][(pas::shr(temp, petir_bitboard_mask::h8a1shiftmask[posraja]) & 255)] & (data.queenwhite | data.bishopwhite)) != 0)) {
                return_value = true;
                return return_value;
            }
        } else {
            if ((adn == 8)) {
                temp = (data.allpiecesr90 & (~petir_bitboard_mask::bit2nmask90[pos2]));
                if (((petir_bitboard_mask::vertmask2[posraja][(pas::shr(temp, petir_bitboard_mask::vershiftmask[posraja]) & 63)] & (data.queenwhite | data.rookwhite)) != 0)) {
                    return_value = true;
                    return return_value;
                }
            } else {
                if ((adn == 1)) {
                    temp = (data.allpieces & (~petir_bitboard_mask::bit2nmask[pos2]));
                    if (((petir_bitboard_mask::horzmask2[posraja][(pas::shr(temp, pas::add((posraja & 56), 1)) & 63)] & (data.queenwhite | data.rookwhite)) != 0)) {
                        return_value = true;
                        return return_value;
                    }
                }
            }
        }
    }
    return_value = false;
    return return_value;
}

// tools.pas:1203
bool isextension(petir_header::tdata& data, std::int32_t& newlevel, std::int32_t giliran, std::int32_t moves, std::int32_t& cext) {
    bool return_value{};
    std::uint8_t pos{};
    std::int8_t d{};
    return_value = false;
    pos = (moves & 127);
    if ((giliran == petir_header::_sisihitam)) {
        pos = pas::sub(63, pos);
    }
    if ((pos <= petir_header::h4)) {
        return return_value;
    }
    if ((pos <= petir_header::h5)) {
        if (((data.nilai_perwira_putih > 15) && (data.nilai_perwira_hitam > 15))) {
            return return_value;
        }
        if ((newlevel > 24)) {
            return return_value;
        }
        if ((giliran == petir_header::_sisiputih)) {
            if (((data.papan[pas::add(pos, 16)] != 0) || (data.papan[pas::add(pos, 24)] != 0))) {
                return return_value;
            }
            if ((((pos & 7) < 7) && (data.papan[pas::add(pos, 17)] == petir_header::_pionhitam))) {
                return return_value;
            }
            if ((((pos & 7) > 0) && (data.papan[pas::add(pos, 15)] == petir_header::_pionhitam))) {
                return return_value;
            }
            return_value = true;
            (newlevel = pas::increment(newlevel, 8));
            (cext = pas::decrement(cext, 8));
            return return_value;
        } else {
            pos = (moves & 127);
            if (((data.papan[pas::sub(pos, 16)] != 0) || (data.papan[pas::sub(pos, 24)] != 0))) {
                return return_value;
            }
            if ((((pos & 7) < 7) && (data.papan[pas::sub(pos, 15)] == petir_header::_pionputih))) {
                return return_value;
            }
            if ((((pos & 7) > 0) && (data.papan[pas::sub(pos, 17)] == petir_header::_pionputih))) {
                return return_value;
            }
            return_value = true;
            (newlevel = pas::increment(newlevel, 8));
            (cext = pas::decrement(cext, 8));
            return return_value;
        }
        return return_value;
    }
    if (((data.nilai_perwira_putih <= 12) && (data.nilai_perwira_hitam <= 12))) {
        (newlevel = pas::increment(newlevel, 8));
        (cext = pas::decrement(cext, 8));
        return_value = true;
        return return_value;
    } else {
        if (((data.nilai_perwira_putih <= 17) && (data.nilai_perwira_hitam <= 17))) {
            (newlevel = pas::increment(newlevel, 8));
            (cext = pas::decrement(cext, 8));
            return_value = true;
            return return_value;
        } else {
            (newlevel = pas::increment(newlevel, petir_header::pushed_pawn_extension_midgame));
            (cext = pas::decrement(cext, petir_header::pushed_pawn_extension_midgame));
            return_value = true;
            return return_value;
        }
    }
    return return_value;
}

// tools.pas:1179
bool winningmaterial(std::int32_t ms, std::uint8_t giliran) {
    bool return_value{};
    return_value = false;
    if (((giliran == petir_header::_sisihitam) && (petir_header::data.materialscore <= pas::negate(petir_usee::_nilai_benteng)))) {
        return_value = true;
    } else {
        if (((giliran == petir_header::_sisiputih) && (petir_header::data.materialscore >= petir_usee::_nilai_benteng))) {
            return_value = true;
        }
    }
    return return_value;
}

// tools.pas:1188
bool losingmaterial(std::int32_t ms, std::uint8_t giliran) {
    bool return_value{};
    return_value = false;
    if (((giliran == petir_header::_sisihitam) && (petir_header::data.materialscore >= petir_usee::_nilai_benteng))) {
        return_value = true;
    } else {
        if (((giliran == petir_header::_sisiputih) && (petir_header::data.materialscore <= pas::negate(petir_usee::_nilai_benteng)))) {
            return_value = true;
        }
    }
    return return_value;
}

// tools.pas:1160
bool menskak(petir_header::tdata& data, std::int32_t giliran) {
    bool return_value{};
    std::int32_t pas_temp_1{};
    return_value = false;
    pas_temp_1 = giliran;
    if (((pas_temp_1 == petir_header::_sisiputih))) {
        if (petir_tools::black_checked(data)) {
            return_value = true;
        }
    }
    else if (((pas_temp_1 == petir_header::_sisihitam))) {
        if (petir_tools::white_checked(data)) {
            return_value = true;
        }
    }
    return return_value;
}

// tools.pas:1169
bool menskak2(petir_header::tdata& data, std::uint16_t giliran, std::uint16_t moves) {
    bool return_value{};
    std::int32_t pas_temp_1{};
    return_value = false;
    pas_temp_1 = giliran;
    if (((pas_temp_1 == petir_header::_sisiputih))) {
        if ((petir_tools::white_menskak(data, moves) || petir_tools::black_open_check(data, moves))) {
            return_value = true;
        }
    }
    else if (((pas_temp_1 == petir_header::_sisihitam))) {
        if ((petir_tools::black_menskak(data, moves) || petir_tools::white_open_check(data, moves))) {
            return_value = true;
        }
    }
    return return_value;
}

// tools.pas:1197
bool pawnpush(petir_header::tdata& data, std::uint16_t giliran, std::uint16_t moves) {
    bool return_value{};
    std::uint8_t pp{};
    return return_value;
}

// tools.pas:280
bool capture(petir_header::tdata& data, std::uint16_t moves) {
    bool return_value{};
    std::uint8_t sto{};
    return_value = true;
    sto = (pas::shr(moves, 7) & 127);
    if ((sto == petir_header::_en_passant)) {
        return return_value;
    }
    if (((sto > 63) || (data.papan[sto] == 0))) {
        return_value = false;
    }
    return return_value;
}

// tools.pas:797
bool white_forkandpin(std::int32_t move, petir_header::tdata& data) {
    bool return_value{};
    std::uint64_t t64{};
    std::int32_t pos{};
    std::int32_t c{};
    std::int32_t pas_temp_1{};
    return_value = false;
    if ((move > 63)) {
        return return_value;
    }
    pas_temp_1 = data.papan[move];
    if (((pas_temp_1 == petir_header::_pionputih))) {
        c = 0;
        if ((((move & 7) > 0) && (data.papan[pas::add(move, 7)] < pas::negate(1)))) {
            (c = pas::increment(c, 1));
        } else {
            return return_value;
        }
        if ((((move & 7) < 7) && (data.papan[pas::add(move, 9)] < pas::negate(1)))) {
            (c = pas::increment(c, 1));
        }
        return_value = (c == 2);
    }
    else if (((pas_temp_1 == petir_header::_kudaputih))) {
        t64 = (petir_bitboard_mask::knightmask[move] & data.blackpieces);
        c = 0;
        while ((t64 != 0)) {
            pos = petir_bitboard_mask::firstbitp(pas::address(t64));
            t64 = (t64 & petir_bitboard_mask::bit2nmasknot[pos]);
            if ((data.papan[pos] <= petir_header::_bentenghitam)) {
                (c = pas::increment(c, 1));
            }
            if ((((data.papan[pos] == petir_header::_pionhitam) && (data.papan[pas::sub(pos, 8)] == petir_header::_pionputih)) && (petir_usee::see(petir_header::_sisiputih, data, move, pos) > 0))) {
                return_value = true;
                return return_value;
            }
        }
        return_value = (c >= 2);
    }
    else if (((pas_temp_1 == petir_header::_gajahputih))) {
        if ((data.nilai_perwira_hitam <= 17)) {
            t64 = petir_bitboard_mask::diaga8h1mask[move][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[move]) & 255)];
            t64 = (t64 | petir_bitboard_mask::diagh8a1mask[move][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[move]) & 255)]);
            t64 = (t64 & data.pawnblack);
            while ((t64 != 0)) {
                pos = petir_bitboard_mask::firstbitp(pas::address(t64));
                if (((data.papan[pas::sub(pos, 8)] == petir_header::_pionputih) && (petir_usee::see(petir_header::_sisiputih, data, move, pos) > 0))) {
                    return_value = true;
                    return return_value;
                }
                t64 = (t64 & petir_bitboard_mask::bit2nmasknot[pos]);
            }
        }
    }
    else if (((pas_temp_1 == petir_header::_bentengputih))) {
        if ((data.nilai_perwira_hitam <= 17)) {
            t64 = petir_bitboard_mask::horzmask2[move][(pas::shr(data.allpieces, pas::add((move & 56), 1)) & 63)];
            t64 = (t64 | petir_bitboard_mask::vertmask2[move][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[move]) & 63)]);
            t64 = (t64 & data.pawnblack);
            while ((t64 != 0)) {
                pos = petir_bitboard_mask::firstbitp(pas::address(t64));
                if (((data.papan[pas::sub(pos, 8)] == petir_header::_pionputih) && (petir_usee::see(petir_header::_sisiputih, data, move, pos) > 0))) {
                    return_value = true;
                    return return_value;
                }
                t64 = (t64 & petir_bitboard_mask::bit2nmasknot[pos]);
            }
        }
    }
    else if (((pas_temp_1 == petir_header::_rajaputih))) {
        if ((data.nilai_perwira_hitam <= 3)) {
            t64 = (petir_bitboard_mask::kingmask[move] & data.pawnblack);
            while ((t64 != 0)) {
                pos = petir_bitboard_mask::firstbitp(pas::address(t64));
                t64 = (t64 & petir_bitboard_mask::bit2nmasknot[pos]);
                if ((((petir_bitboard_mask::kingmask[pos] & data.kingblack) == 0) && ((petir_bitboard_mask::w_pawn_attack[pos] & data.pawnblack) == 0))) {
                    return_value = true;
                    return return_value;
                }
            }
        }
    }
    return return_value;
}

// tools.pas:890
bool black_forkandpin(std::int32_t move, petir_header::tdata& data) {
    bool return_value{};
    std::uint64_t t64{};
    std::int32_t pos{};
    std::int32_t c{};
    std::int32_t pas_temp_1{};
    return_value = false;
    if ((move > 63)) {
        return return_value;
    }
    pas_temp_1 = data.papan[move];
    if (((pas_temp_1 == petir_header::_pionhitam))) {
        c = 0;
        if ((((move & 7) > 0) && (data.papan[pas::sub(move, 9)] > 1))) {
            (c = pas::increment(c, 1));
        } else {
            return return_value;
        }
        if ((((move & 7) < 7) && (data.papan[pas::sub(move, 7)] > 1))) {
            (c = pas::increment(c, 1));
        } else {
            return return_value;
        }
        return_value = (c == 2);
    }
    else if (((pas_temp_1 == petir_header::_kudahitam))) {
        t64 = (petir_bitboard_mask::knightmask[move] & data.whitepieces);
        c = 0;
        while ((t64 != 0)) {
            pos = petir_bitboard_mask::firstbitp(pas::address(t64));
            t64 = (t64 & petir_bitboard_mask::bit2nmasknot[pos]);
            if ((data.papan[pos] >= petir_header::_bentengputih)) {
                (c = pas::increment(c, 1));
            }
            if ((((data.papan[pos] == petir_header::_pionputih) && (data.papan[pas::add(pos, 8)] == petir_header::_pionhitam)) && (petir_usee::see(petir_header::_sisihitam, data, move, pos) > 0))) {
                return_value = true;
                return return_value;
            }
        }
        return_value = (c >= 2);
    }
    else if (((pas_temp_1 == petir_header::_gajahhitam))) {
        if ((data.nilai_perwira_putih <= 17)) {
            t64 = petir_bitboard_mask::diaga8h1mask[move][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[move]) & 255)];
            t64 = (t64 | petir_bitboard_mask::diagh8a1mask[move][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[move]) & 255)]);
            t64 = (t64 & data.pawnwhite);
            while ((t64 != 0)) {
                pos = petir_bitboard_mask::firstbitp(pas::address(t64));
                if (((data.papan[pas::add(pos, 8)] == petir_header::_pionhitam) && (petir_usee::see(petir_header::_sisihitam, data, move, pos) > 0))) {
                    return_value = true;
                    return return_value;
                }
                t64 = (t64 & petir_bitboard_mask::bit2nmasknot[pos]);
            }
        }
    }
    else if (((pas_temp_1 == petir_header::_bentenghitam))) {
        if ((data.nilai_perwira_putih <= 17)) {
            t64 = petir_bitboard_mask::horzmask2[move][(pas::shr(data.allpieces, pas::add((move & 56), 1)) & 63)];
            t64 = (t64 | petir_bitboard_mask::vertmask2[move][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[move]) & 63)]);
            t64 = (t64 & data.pawnwhite);
            while ((t64 != 0)) {
                pos = petir_bitboard_mask::firstbitp(pas::address(t64));
                if (((data.papan[pas::add(pos, 8)] == petir_header::_pionhitam) && (petir_usee::see(petir_header::_sisihitam, data, move, pos) > 0))) {
                    return_value = true;
                    return return_value;
                }
                t64 = (t64 & petir_bitboard_mask::bit2nmasknot[pos]);
            }
        }
    }
    else if (((pas_temp_1 == petir_header::_rajahitam))) {
        if ((data.nilai_perwira_putih <= 3)) {
            t64 = (petir_bitboard_mask::kingmask[move] & data.pawnwhite);
            while ((t64 != 0)) {
                pos = petir_bitboard_mask::firstbitp(pas::address(t64));
                t64 = (t64 & petir_bitboard_mask::bit2nmasknot[pos]);
                if ((((petir_bitboard_mask::kingmask[pos] & data.kingwhite) == 0) && ((petir_bitboard_mask::b_pawn_attack[pos] & data.pawnwhite) == 0))) {
                    return_value = true;
                    return return_value;
                }
            }
        }
    }
    return return_value;
}

// tools.pas:268
std::int32_t captured_value(std::int32_t sto, petir_header::tdata& data) {
    std::int32_t return_value{};
    if ((data.cp == 0)) {
        return_value = 0;
    } else {
        if ((sto == petir_header::_en_passant)) {
            return_value = petir_usee::_nilai_pion;
        } else {
            if ((sto >= petir_header::_rokadependek)) {
                return_value = 0;
            } else {
                return_value = petir_usee::nilai_piece[data.papan[sto]];
            }
        }
        (return_value = pas::increment(return_value, 50));
    }
    return return_value;
}

// tools.pas:497
bool white_threat(petir_header::tdata& data, std::uint16_t moves, std::int32_t& threatpiece) {
    bool return_value{};
    std::uint64_t attack{};
    std::uint64_t raja{};
    std::uint64_t anu{};
    std::uint8_t sto{};
    std::uint8_t posraja{};
    std::int32_t temp{};
    std::int32_t x{};
    std::int32_t y{};
    std::int32_t pas_temp_1{};
    threatpiece = 0;
    posraja = petir_bitboard_mask::lastbitp(pas::address(data.kingblack));
    sto = (pas::shr(moves, 7) & 127);
    if ((sto > 63)) {
        if (((sto == petir_header::_rokadependek) || (sto == petir_header::_rokadepanjang))) {
            threatpiece = 50;
            return_value = false;
            return return_value;
        }
        return_value = true;
        return return_value;
    }
    return_value = false;
    attack = 0;
    raja = (petir_bitboard_mask::kingmask[posraja] | petir_bitboard_mask::bit2nmask[posraja]);
    pas_temp_1 = data.papan[sto];
    if (((pas_temp_1 == petir_header::_pionputih))) {
        if ((sto >= petir_header::a7)) {
            return_value = true;
            return return_value;
        }
        attack = petir_bitboard_mask::w_pawn_attack[sto];
    }
    else if (((pas_temp_1 == petir_header::_kudaputih))) {
        attack = petir_bitboard_mask::knightmask[sto];
    }
    else if (((pas_temp_1 == petir_header::_gajahputih))) {
        attack = 0;
        x = (sto & 7);
        y = pas::shr(sto, 3);
        (x = pas::increment(x, 1));
        (y = pas::increment(y, 1));
        while (((((x <= 7) && (y <= 7)) && (x >= 0)) && (y >= 0))) {
            temp = pas::add(pas::shl(y, 3), x);
            attack = (attack | petir_bitboard_mask::bit2nmask[temp]);
            if ((data.papan[temp] == petir_header::_pionputih)) {
                (x = pas::increment(x, 1));
                (y = pas::increment(y, 1));
                temp = pas::add(pas::shl(y, 3), x);
                attack = (attack | petir_bitboard_mask::bit2nmask[temp]);
                break;
            }
            if ((data.papan[temp] == petir_header::_mentriputih)) {
                (x = pas::increment(x, 1));
                (y = pas::increment(y, 1));
                continue;
            }
            if ((data.papan[temp] != 0)) {
                break;
            }
            (x = pas::increment(x, 1));
            (y = pas::increment(y, 1));
        }
        x = (sto & 7);
        y = pas::shr(sto, 3);
        (x = pas::decrement(x, 1));
        (y = pas::increment(y, 1));
        while (((((x <= 7) && (y <= 7)) && (x >= 0)) && (y >= 0))) {
            temp = pas::add(pas::shl(y, 3), x);
            attack = (attack | petir_bitboard_mask::bit2nmask[temp]);
            if ((data.papan[temp] == petir_header::_pionputih)) {
                (x = pas::decrement(x, 1));
                (y = pas::increment(y, 1));
                temp = pas::add(pas::shl(y, 3), x);
                attack = (attack | petir_bitboard_mask::bit2nmask[temp]);
                break;
            }
            if ((data.papan[temp] == petir_header::_mentriputih)) {
                (x = pas::decrement(x, 1));
                (y = pas::increment(y, 1));
                continue;
            }
            if ((data.papan[temp] != 0)) {
                break;
            }
            (x = pas::decrement(x, 1));
            (y = pas::increment(y, 1));
        }
        x = (sto & 7);
        y = pas::shr(sto, 3);
        (x = pas::increment(x, 1));
        (y = pas::decrement(y, 1));
        while (((((x <= 7) && (y <= 7)) && (x >= 0)) && (y >= 0))) {
            temp = pas::add(pas::shl(y, 3), x);
            attack = (attack | petir_bitboard_mask::bit2nmask[temp]);
            if ((data.papan[temp] == petir_header::_mentriputih)) {
                (x = pas::increment(x, 1));
                (y = pas::decrement(y, 1));
                continue;
            }
            if ((data.papan[temp] != 0)) {
                break;
            }
            (x = pas::increment(x, 1));
            (y = pas::decrement(y, 1));
        }
        x = (sto & 7);
        y = pas::shr(sto, 3);
        (x = pas::decrement(x, 1));
        (y = pas::decrement(y, 1));
        while (((((x <= 7) && (y <= 7)) && (x >= 0)) && (y >= 0))) {
            temp = pas::add(pas::shl(y, 3), x);
            attack = (attack | petir_bitboard_mask::bit2nmask[temp]);
            if ((data.papan[temp] == petir_header::_mentriputih)) {
                (x = pas::decrement(x, 1));
                (y = pas::decrement(y, 1));
                continue;
            }
            if ((data.papan[temp] != 0)) {
                break;
            }
            (x = pas::decrement(x, 1));
            (y = pas::decrement(y, 1));
        }
    }
    else if (((pas_temp_1 == petir_header::_bentengputih))) {
        anu = (data.allpieces & (~data.queenwhite));
        attack = petir_bitboard_mask::horzmask2[sto][(pas::shr(anu, pas::add((sto & 56), 1)) & 63)];
        temp = pas::add(sto, 8);
        while ((temp <= 63)) {
            attack = (attack | petir_bitboard_mask::bit2nmask[temp]);
            if (((data.papan[temp] == petir_header::_bentengputih) || (data.papan[temp] == petir_header::_mentriputih))) {
                temp = pas::add(temp, 8);
                continue;
            }
            if ((data.papan[temp] != 0)) {
                break;
            }
            temp = pas::add(temp, 8);
        }
        temp = pas::sub(sto, 8);
        while ((temp >= 0)) {
            attack = (attack | petir_bitboard_mask::bit2nmask[temp]);
            if (((data.papan[temp] == petir_header::_bentengputih) || (data.papan[temp] == petir_header::_mentriputih))) {
                temp = pas::sub(temp, 8);
                continue;
            }
            if ((data.papan[temp] != 0)) {
                break;
            }
            temp = pas::sub(temp, 8);
        }
    }
    else if (((pas_temp_1 == petir_header::_mentriputih))) {
        attack = petir_bitboard_mask::diaga8h1mask[sto][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[sto]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[sto][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[sto]) & 255)]);
        anu = (data.allpieces & (~data.rookwhite));
        attack = (attack | petir_bitboard_mask::horzmask2[sto][(pas::shr(anu, pas::add((sto & 56), 1)) & 63)]);
        temp = pas::add(sto, 8);
        while ((temp <= 63)) {
            attack = (attack | petir_bitboard_mask::bit2nmask[temp]);
            if (((data.papan[temp] == petir_header::_bentengputih) || (data.papan[temp] == petir_header::_mentriputih))) {
                temp = pas::add(temp, 8);
                continue;
            }
            if ((data.papan[temp] != 0)) {
                break;
            }
            temp = pas::add(temp, 8);
        }
        temp = pas::sub(sto, 8);
        while ((temp >= 0)) {
            attack = (attack | petir_bitboard_mask::bit2nmask[temp]);
            if (((data.papan[temp] == petir_header::_bentengputih) || (data.papan[temp] == petir_header::_mentriputih))) {
                temp = pas::sub(temp, 8);
                continue;
            }
            if ((data.papan[temp] != 0)) {
                break;
            }
            temp = pas::sub(temp, 8);
        }
    }
    else if (((pas_temp_1 == petir_header::_rajaputih))) {
        attack = petir_bitboard_mask::kingmask[sto];
    }
    threatpiece = 0;
    if (((attack & raja) != 0)) {
        return_value = true;
        return return_value;
    }
    if (((attack & data.queenblack) != 0)) {
        threatpiece = petir_usee::_nilai_mentri;
    } else {
        if (((attack & data.rookblack) != 0)) {
            threatpiece = petir_usee::_nilai_benteng;
        } else {
            if (((attack & data.bishopblack) != 0)) {
                threatpiece = petir_usee::_nilai_gajah;
            } else {
                if (((attack & data.knightblack) != 0)) {
                    threatpiece = petir_usee::_nilai_kuda;
                } else {
                    if (((attack & data.pawnblack) != 0)) {
                        threatpiece = petir_usee::_nilai_pion;
                    }
                }
            }
        }
    }
    return return_value;
}

// tools.pas:298
bool black_threat(petir_header::tdata& data, std::uint16_t moves, std::int32_t& threatpiece) {
    bool return_value{};
    std::uint64_t attack{};
    std::uint64_t raja{};
    std::uint64_t anu{};
    std::uint8_t sto{};
    std::uint8_t posraja{};
    std::int32_t temp{};
    std::int32_t x{};
    std::int32_t y{};
    std::int32_t pas_temp_1{};
    threatpiece = 0;
    posraja = petir_bitboard_mask::lastbitp(pas::address(data.kingwhite));
    sto = (pas::shr(moves, 7) & 127);
    if ((sto > 63)) {
        if (((sto == petir_header::_rokadependek) || (sto == petir_header::_rokadepanjang))) {
            threatpiece = 50;
            return_value = false;
            return return_value;
        }
        return_value = true;
        return return_value;
    }
    return_value = false;
    attack = 0;
    raja = (petir_bitboard_mask::kingmask[posraja] | petir_bitboard_mask::bit2nmask[posraja]);
    pas_temp_1 = data.papan[sto];
    if (((pas_temp_1 == petir_header::_pionhitam))) {
        if ((sto <= petir_header::h2)) {
            return_value = true;
            return return_value;
        }
        attack = petir_bitboard_mask::b_pawn_attack[sto];
    }
    else if (((pas_temp_1 == petir_header::_kudahitam))) {
        attack = petir_bitboard_mask::knightmask[sto];
    }
    else if (((pas_temp_1 == petir_header::_gajahhitam))) {
        attack = 0;
        x = (sto & 7);
        y = pas::shr(sto, 3);
        (x = pas::increment(x, 1));
        (y = pas::increment(y, 1));
        while (((((x <= 7) && (y <= 7)) && (x >= 0)) && (y >= 0))) {
            temp = pas::add(pas::shl(y, 3), x);
            attack = (attack | petir_bitboard_mask::bit2nmask[temp]);
            if ((data.papan[temp] == petir_header::_mentrihitam)) {
                (x = pas::increment(x, 1));
                (y = pas::increment(y, 1));
                continue;
            }
            if ((data.papan[temp] != 0)) {
                break;
            }
            (x = pas::increment(x, 1));
            (y = pas::increment(y, 1));
        }
        x = (sto & 7);
        y = pas::shr(sto, 3);
        (x = pas::decrement(x, 1));
        (y = pas::increment(y, 1));
        while (((((x <= 7) && (y <= 7)) && (x >= 0)) && (y >= 0))) {
            temp = pas::add(pas::shl(y, 3), x);
            attack = (attack | petir_bitboard_mask::bit2nmask[temp]);
            if ((data.papan[temp] == petir_header::_mentrihitam)) {
                (x = pas::decrement(x, 1));
                (y = pas::increment(y, 1));
                continue;
            }
            if ((data.papan[temp] != 0)) {
                break;
            }
            (x = pas::decrement(x, 1));
            (y = pas::increment(y, 1));
        }
        x = (sto & 7);
        y = pas::shr(sto, 3);
        (x = pas::increment(x, 1));
        (y = pas::decrement(y, 1));
        while (((((x <= 7) && (y <= 7)) && (x >= 0)) && (y >= 0))) {
            temp = pas::add(pas::shl(y, 3), x);
            attack = (attack | petir_bitboard_mask::bit2nmask[temp]);
            if ((data.papan[temp] == petir_header::_pionhitam)) {
                (x = pas::increment(x, 1));
                (y = pas::decrement(y, 1));
                temp = pas::add(pas::shl(y, 3), x);
                attack = (attack | petir_bitboard_mask::bit2nmask[temp]);
                break;
            }
            if ((data.papan[temp] == petir_header::_mentrihitam)) {
                (x = pas::increment(x, 1));
                (y = pas::decrement(y, 1));
                continue;
            }
            if ((data.papan[temp] != 0)) {
                break;
            }
            (x = pas::increment(x, 1));
            (y = pas::decrement(y, 1));
        }
        x = (sto & 7);
        y = pas::shr(sto, 3);
        (x = pas::decrement(x, 1));
        (y = pas::decrement(y, 1));
        while (((((x <= 7) && (y <= 7)) && (x >= 0)) && (y >= 0))) {
            temp = pas::add(pas::shl(y, 3), x);
            attack = (attack | petir_bitboard_mask::bit2nmask[temp]);
            if ((data.papan[temp] == petir_header::_pionhitam)) {
                (x = pas::decrement(x, 1));
                (y = pas::decrement(y, 1));
                temp = pas::add(pas::shl(y, 3), x);
                attack = (attack | petir_bitboard_mask::bit2nmask[temp]);
                break;
            }
            if ((data.papan[temp] == petir_header::_mentrihitam)) {
                (x = pas::decrement(x, 1));
                (y = pas::decrement(y, 1));
                continue;
            }
            if ((data.papan[temp] != 0)) {
                break;
            }
            (x = pas::decrement(x, 1));
            (y = pas::decrement(y, 1));
        }
        attack = petir_bitboard_mask::diaga8h1mask[sto][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[sto]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[sto][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[sto]) & 255)]);
    }
    else if (((pas_temp_1 == petir_header::_bentenghitam))) {
        anu = (data.allpieces & (~data.queenblack));
        attack = petir_bitboard_mask::horzmask2[sto][(pas::shr(anu, pas::add((sto & 56), 1)) & 63)];
        temp = pas::add(sto, 8);
        while ((temp <= 63)) {
            attack = (attack | petir_bitboard_mask::bit2nmask[temp]);
            if (((data.papan[temp] == petir_header::_bentenghitam) || (data.papan[temp] == petir_header::_mentrihitam))) {
                temp = pas::add(temp, 8);
                continue;
            }
            if ((data.papan[temp] != 0)) {
                break;
            }
            temp = pas::add(temp, 8);
        }
        temp = pas::sub(sto, 8);
        while ((temp >= 0)) {
            attack = (attack | petir_bitboard_mask::bit2nmask[temp]);
            if (((data.papan[temp] == petir_header::_bentenghitam) || (data.papan[temp] == petir_header::_mentrihitam))) {
                temp = pas::sub(temp, 8);
                continue;
            }
            if ((data.papan[temp] != 0)) {
                break;
            }
            temp = pas::sub(temp, 8);
        }
    }
    else if (((pas_temp_1 == petir_header::_mentrihitam))) {
        attack = petir_bitboard_mask::diaga8h1mask[sto][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[sto]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[sto][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[sto]) & 255)]);
        anu = (data.allpieces & (~data.rookblack));
        attack = (attack | petir_bitboard_mask::horzmask2[sto][(pas::shr(anu, pas::add((sto & 56), 1)) & 63)]);
        temp = pas::add(sto, 8);
        while ((temp <= 63)) {
            attack = (attack | petir_bitboard_mask::bit2nmask[temp]);
            if (((data.papan[temp] == petir_header::_bentenghitam) || (data.papan[temp] == petir_header::_mentrihitam))) {
                temp = pas::add(temp, 8);
                continue;
            }
            if ((data.papan[temp] != 0)) {
                break;
            }
            temp = pas::add(temp, 8);
        }
        temp = pas::sub(sto, 8);
        while ((temp >= 0)) {
            attack = (attack | petir_bitboard_mask::bit2nmask[temp]);
            if (((data.papan[temp] == petir_header::_bentenghitam) || (data.papan[temp] == petir_header::_mentrihitam))) {
                temp = pas::sub(temp, 8);
                continue;
            }
            if ((data.papan[temp] != 0)) {
                break;
            }
            temp = pas::sub(temp, 8);
        }
    }
    else if (((pas_temp_1 == petir_header::_rajahitam))) {
        attack = petir_bitboard_mask::knightmask[sto];
    }
    threatpiece = 0;
    if (((attack & raja) != 0)) {
        return_value = true;
        return return_value;
    }
    if (((attack & data.queenwhite) != 0)) {
        threatpiece = petir_usee::_nilai_mentri;
    } else {
        if (((attack & data.rookwhite) != 0)) {
            threatpiece = petir_usee::_nilai_benteng;
        } else {
            if (((attack & data.bishopwhite) != 0)) {
                threatpiece = petir_usee::_nilai_gajah;
            } else {
                if (((attack & data.knightwhite) != 0)) {
                    threatpiece = petir_usee::_nilai_kuda;
                } else {
                    if (((attack & data.pawnwhite) != 0)) {
                        threatpiece = petir_usee::_nilai_pion;
                    }
                }
            }
        }
    }
    return return_value;
}

// tools.pas:239
bool pawn_7_rank(std::int32_t giliran, petir_header::tdata& data) {
    bool return_value{};
    std::int32_t a{};
    if ((giliran == petir_header::_sisihitam)) {
        return_value = ((petir_bitboard_mask::rank_mask[1] & data.pawnblack) != 0);
    } else {
        return_value = ((petir_bitboard_mask::rank_mask[6] & data.pawnwhite) != 0);
    }
    return return_value;
}

// tools.pas:251
bool pawn_7_rankw(petir_header::tdata& data) {
    bool return_value{};
    std::int32_t a{};
    return_value = ((petir_bitboard_mask::rank_mask[6] & data.pawnwhite) != 0);
    return return_value;
}

// tools.pas:259
bool pawn_7_rankb(petir_header::tdata& data) {
    bool return_value{};
    std::int32_t a{};
    return_value = ((petir_bitboard_mask::rank_mask[1] & data.pawnblack) != 0);
    return return_value;
}

// tools.pas:143
bool pawn_6_rank(std::int32_t giliran, petir_header::tdata& data) {
    bool return_value{};
    std::int32_t a{};
    if ((giliran == petir_header::_sisihitam)) {
        return_value = false;
        if ((((data.papan[petir_header::d3] == petir_header::_pionhitam) && (data.papan[petir_header::d2] == 0)) && (data.papan[petir_header::d1] == 0))) {
            return_value = true;
            return return_value;
        }
        if ((((data.papan[petir_header::e3] == petir_header::_pionhitam) && (data.papan[petir_header::e2] == 0)) && (data.papan[petir_header::e1] == 0))) {
            return_value = true;
            return return_value;
        }
        if ((((data.papan[petir_header::a3] == petir_header::_pionhitam) && (data.papan[petir_header::a2] == 0)) && (data.papan[petir_header::a1] == 0))) {
            return_value = true;
            return return_value;
        }
        if ((((data.papan[petir_header::b3] == petir_header::_pionhitam) && (data.papan[petir_header::b2] == 0)) && (data.papan[petir_header::b1] == 0))) {
            return_value = true;
            return return_value;
        }
        if ((((data.papan[petir_header::c3] == petir_header::_pionhitam) && (data.papan[petir_header::c2] == 0)) && (data.papan[petir_header::c1] == 0))) {
            return_value = true;
            return return_value;
        }
        if ((((data.papan[petir_header::f3] == petir_header::_pionhitam) && (data.papan[petir_header::f2] == 0)) && (data.papan[petir_header::f1] == 0))) {
            return_value = true;
            return return_value;
        }
        if ((((data.papan[petir_header::g3] == petir_header::_pionhitam) && (data.papan[petir_header::g2] == 0)) && (data.papan[petir_header::g1] == 0))) {
            return_value = true;
            return return_value;
        }
        if ((((data.papan[petir_header::h3] == petir_header::_pionhitam) && (data.papan[petir_header::h2] == 0)) && (data.papan[petir_header::h1] == 0))) {
            return_value = true;
            return return_value;
        }
    } else {
        return_value = false;
        if ((((data.papan[petir_header::d6] == petir_header::_pionputih) && (data.papan[petir_header::d7] == 0)) && (data.papan[petir_header::d8] == 0))) {
            return_value = true;
            return return_value;
        }
        if ((((data.papan[petir_header::e6] == petir_header::_pionputih) && (data.papan[petir_header::e7] == 0)) && (data.papan[petir_header::e8] == 0))) {
            return_value = true;
            return return_value;
        }
        if ((((data.papan[petir_header::a6] == petir_header::_pionputih) && (data.papan[petir_header::a7] == 0)) && (data.papan[petir_header::a8] == 0))) {
            return_value = true;
            return return_value;
        }
        if ((((data.papan[petir_header::b6] == petir_header::_pionputih) && (data.papan[petir_header::b7] == 0)) && (data.papan[petir_header::b8] == 0))) {
            return_value = true;
            return return_value;
        }
        if ((((data.papan[petir_header::c6] == petir_header::_pionputih) && (data.papan[petir_header::c7] == 0)) && (data.papan[petir_header::c8] == 0))) {
            return_value = true;
            return return_value;
        }
        if ((((data.papan[petir_header::f6] == petir_header::_pionputih) && (data.papan[petir_header::f7] == 0)) && (data.papan[petir_header::f8] == 0))) {
            return_value = true;
            return return_value;
        }
        if ((((data.papan[petir_header::g6] == petir_header::_pionputih) && (data.papan[petir_header::g7] == 0)) && (data.papan[petir_header::g8] == 0))) {
            return_value = true;
            return return_value;
        }
        if ((((data.papan[petir_header::h6] == petir_header::_pionputih) && (data.papan[petir_header::h7] == 0)) && (data.papan[petir_header::h8] == 0))) {
            return_value = true;
            return return_value;
        }
    }
    return return_value;
}

// tools.pas:107
bool good_move_white(petir_header::tdata& data, std::int32_t move) {
    bool return_value{};
    return_value = false;
    if ((move > 63)) {
        return return_value;
    }
    if (((data.papan[move] == petir_header::_bentengputih) && (move >= petir_header::a7))) {
        return_value = true;
    } else {
        if (((data.papan[move] == petir_header::_pionputih) && ((petir_bitboard_mask::w_pion_bebas_mask[move] & data.pawnblack) == 0))) {
            return_value = true;
        }
    }
    return return_value;
}

// tools.pas:121
bool good_move_black(petir_header::tdata& data, std::int32_t move) {
    bool return_value{};
    return_value = false;
    if ((move > 63)) {
        return return_value;
    }
    if (((data.papan[move] == petir_header::_bentenghitam) && (move <= petir_header::h2))) {
        return_value = true;
    } else {
        if (((data.papan[move] == petir_header::_pionhitam) && ((petir_bitboard_mask::b_pion_bebas_mask[move] & data.pawnwhite) == 0))) {
            return_value = true;
        }
    }
    return return_value;
}

// tools.pas:290
bool wb_threat(std::uint8_t giliran, petir_header::tdata& data, std::uint16_t moves, std::int32_t& threatpiece) {
    bool return_value{};
    if ((giliran == petir_header::_sisiputih)) {
        return_value = petir_tools::white_threat(data, moves, threatpiece);
    } else {
        return_value = petir_tools::black_threat(data, moves, threatpiece);
    }
    return return_value;
}

// tools.pas:99
std::int32_t material(petir_header::tdata& data, std::int32_t giliran) {
    std::int32_t return_value{};
    if ((giliran == petir_header::_sisiputih)) {
        return_value = data.nilai_perwira_putih;
    } else {
        return_value = data.nilai_perwira_hitam;
    }
    return return_value;
}

// tools.pas:44
pas::String getfen(petir_header::tdata data, std::int32_t giliran) {
    pas::String return_value{};
    std::int32_t a{};
    std::int32_t b{};
    std::int32_t z{};
    std::int32_t m{};
    std::int32_t r{};
    pas::String temp{};
    std::int32_t lastempty{};
    std::int32_t pas_temp_1{};
    std::int32_t pas_temp_2{};
    b = 0;
    return_value = pas::String("");
    temp = pas::String("");
    lastempty = 0;
    pas_temp_1 = 0;
    for (a = 63; a >= pas_temp_1; --a) {
        z = pas::mul((a / 8), 8);
        m = pas::sub(7, (a % 8));
        r = pas::add(z, m);
        if ((data.papan[r] != 0)) {
            if ((lastempty != 0)) {
                temp = (temp + pas::inttostr(lastempty));
                lastempty = 0;
            }
        }
        pas_temp_2 = data.papan[r];
        if (((pas_temp_2 == petir_header::_pionhitam))) {
            temp = (temp + 'p');
        }
        else if (((pas_temp_2 == petir_header::_kudahitam))) {
            temp = (temp + 'n');
        }
        else if (((pas_temp_2 == petir_header::_gajahhitam))) {
            temp = (temp + 'b');
        }
        else if (((pas_temp_2 == petir_header::_bentenghitam))) {
            temp = (temp + 'r');
        }
        else if (((pas_temp_2 == petir_header::_mentrihitam))) {
            temp = (temp + 'q');
        }
        else if (((pas_temp_2 == petir_header::_rajahitam))) {
            temp = (temp + 'k');
        }
        else if (((pas_temp_2 == petir_header::_pionputih))) {
            temp = (temp + 'P');
        }
        else if (((pas_temp_2 == petir_header::_kudaputih))) {
            temp = (temp + 'N');
        }
        else if (((pas_temp_2 == petir_header::_gajahputih))) {
            temp = (temp + 'B');
        }
        else if (((pas_temp_2 == petir_header::_bentengputih))) {
            temp = (temp + 'R');
        }
        else if (((pas_temp_2 == petir_header::_mentriputih))) {
            temp = (temp + 'Q');
        }
        else if (((pas_temp_2 == petir_header::_rajaputih))) {
            temp = (temp + 'K');
        }
        else if (((pas_temp_2 == 0))) {
            lastempty = pas::add(lastempty, 1);
        }
        if (((a % 8) == 0)) {
            if ((lastempty != 0)) {
                temp = (temp + pas::inttostr(lastempty));
            }
            return_value = (return_value + temp);
            if (((a <= 56) && (a > 0))) {
                return_value = (return_value + '/');
            }
            temp = pas::String("");
            lastempty = 0;
        }
    }
    return_value = (return_value + ' ');
    if ((giliran == petir_header::_sisiputih)) {
        return_value = (return_value + 'w');
    } else {
        return_value = (return_value + 'b');
    }
    return_value = (return_value + pas::String(" - -"));
    return return_value;
}

// tools.pas:135
bool good_move(petir_header::tdata& data, std::int32_t move, std::int32_t giliran) {
    bool return_value{};
    if ((giliran == petir_header::_sisiputih)) {
        return_value = petir_tools::good_move_white(data, move);
    } else {
        return_value = petir_tools::good_move_black(data, move);
    }
    return return_value;
}

// tools.pas:691
bool white_menskak(petir_header::tdata& data, std::uint16_t moves) {
    bool return_value{};
    std::uint8_t sfrom{};
    std::uint8_t sto{};
    std::int32_t posraja{};
    std::int32_t promosi{};
    std::uint64_t attack{};
    std::int8_t piece{};
    std::int32_t pas_temp_1{};
    std::int32_t pas_temp_2{};
    std::int32_t pas_temp_3{};
    std::int32_t pas_temp_4{};
    return_value = false;
    posraja = petir_bitboard_mask::lastbitp(pas::address(data.kingblack));
    sfrom = (moves & 127);
    sto = (pas::shr(moves, 7) & 127);
    promosi = (pas::shr(moves, 14) & 7);
    piece = data.papan[sfrom];
    if ((promosi == petir_header::promosi_mentri)) {
        piece = petir_header::_mentriputih;
    } else {
        if ((promosi == petir_header::promosi_kuda)) {
            piece = petir_header::_kudaputih;
        }
    }
    if (((sto == 65) || (sto == 66))) {
        if ((sto == 65)) {
            sto = petir_header::f1;
        } else {
            sto = petir_header::d1;
        }
        piece = petir_header::_bentengputih;
    } else {
        if ((sto == 67)) {
            if ((data.ep > (sfrom & 7))) {
                sto = pas::add(sfrom, 9);
            } else {
                sto = pas::add(sfrom, 7);
            }
        }
    }
    pas_temp_1 = piece;
    if (((pas_temp_1 == petir_header::_pionputih))) {
        if (((petir_bitboard_mask::w_pawn_attack[sto] & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
            return_value = true;
        }
    }
    else if (((pas_temp_1 == petir_header::_kudaputih))) {
        if (((petir_bitboard_mask::knightmask[sto] & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
            return_value = true;
        }
    }
    else if (((pas_temp_1 == petir_header::_gajahputih))) {
        pas_temp_2 = pas::abs(petir_bitboard_mask::direction[sto][posraja]);
        if (((pas_temp_2 == 7))) {
            attack = petir_bitboard_mask::diaga8h1mask[sto][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[sto]) & 255)];
            if (((attack & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
                return_value = true;
            }
        }
        else if (((pas_temp_2 == 9))) {
            attack = petir_bitboard_mask::diagh8a1mask[sto][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[sto]) & 255)];
            if (((attack & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
                return_value = true;
            }
        }
    }
    else if (((pas_temp_1 == petir_header::_bentengputih))) {
        pas_temp_3 = pas::abs(petir_bitboard_mask::direction[sto][posraja]);
        if (((pas_temp_3 == 1))) {
            attack = petir_bitboard_mask::horzmask2[sto][(pas::shr(data.allpieces, pas::add((sto & 56), 1)) & 63)];
            if (((attack & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
                return_value = true;
            }
        }
        else if (((pas_temp_3 == 8))) {
            attack = petir_bitboard_mask::vertmask2[sto][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[sto]) & 63)];
            if (((attack & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
                return_value = true;
            }
        }
    }
    else if (((pas_temp_1 == petir_header::_mentriputih))) {
        pas_temp_4 = pas::abs(petir_bitboard_mask::direction[sto][posraja]);
        if (((pas_temp_4 == 1))) {
            attack = petir_bitboard_mask::horzmask2[sto][(pas::shr(data.allpieces, pas::add((sto & 56), 1)) & 63)];
            if (((attack & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
                return_value = true;
            }
        }
        else if (((pas_temp_4 == 8))) {
            attack = petir_bitboard_mask::vertmask2[sto][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[sto]) & 63)];
            if (((attack & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
                return_value = true;
            }
        }
        else if (((pas_temp_4 == 7))) {
            attack = petir_bitboard_mask::diaga8h1mask[sto][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[sto]) & 255)];
            if (((attack & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
                return_value = true;
            }
        }
        else if (((pas_temp_4 == 9))) {
            attack = petir_bitboard_mask::diagh8a1mask[sto][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[sto]) & 255)];
            if (((attack & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
                return_value = true;
            }
        }
    }
    return return_value;
}

// tools.pas:787
std::uint8_t ipopcount(std::uint64_t p) {
    std::uint8_t return_value{};
    return_value = 0;
    while ((p != 0)) {
        p = (p & (p - 1));
        (return_value = pas::increment(return_value, 1));
    }
    return return_value;
}

// tools.pas:984
bool black_menskak(petir_header::tdata& data, std::uint16_t moves) {
    bool return_value{};
    std::uint8_t sfrom{};
    std::uint8_t sto{};
    std::int32_t posraja{};
    std::int32_t promosi{};
    std::uint64_t attack{};
    std::int8_t piece{};
    std::int32_t pas_temp_1{};
    std::int32_t pas_temp_2{};
    std::int32_t pas_temp_3{};
    std::int32_t pas_temp_4{};
    return_value = false;
    posraja = petir_bitboard_mask::firstbitp(pas::address(data.kingwhite));
    sfrom = (moves & 127);
    sto = (pas::shr(moves, 7) & 127);
    promosi = (pas::shr(moves, 14) & 7);
    piece = data.papan[sfrom];
    if ((promosi == petir_header::promosi_mentri)) {
        piece = petir_header::_mentrihitam;
    } else {
        if ((promosi == petir_header::promosi_kuda)) {
            piece = petir_header::_kudahitam;
        }
    }
    if (((sto == 65) || (sto == 66))) {
        if ((sto == 65)) {
            sto = petir_header::f8;
        } else {
            sto = petir_header::d8;
        }
        piece = petir_header::_bentenghitam;
    } else {
        if ((sto == 67)) {
            if ((data.ep > (sfrom & 7))) {
                sto = pas::sub(sfrom, 7);
            } else {
                sto = pas::sub(sfrom, 9);
            }
        }
    }
    pas_temp_1 = piece;
    if (((pas_temp_1 == petir_header::_pionhitam))) {
        if (((petir_bitboard_mask::b_pawn_attack[sto] & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
            return_value = true;
        }
    }
    else if (((pas_temp_1 == petir_header::_kudahitam))) {
        if (((petir_bitboard_mask::knightmask[sto] & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
            return_value = true;
        }
    }
    else if (((pas_temp_1 == petir_header::_gajahhitam))) {
        pas_temp_2 = pas::abs(petir_bitboard_mask::direction[sto][posraja]);
        if (((pas_temp_2 == 7))) {
            attack = petir_bitboard_mask::diaga8h1mask[sto][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[sto]) & 255)];
            if (((attack & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
                return_value = true;
            }
        }
        else if (((pas_temp_2 == 9))) {
            attack = petir_bitboard_mask::diagh8a1mask[sto][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[sto]) & 255)];
            if (((attack & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
                return_value = true;
            }
        }
    }
    else if (((pas_temp_1 == petir_header::_bentenghitam))) {
        pas_temp_3 = pas::abs(petir_bitboard_mask::direction[sto][posraja]);
        if (((pas_temp_3 == 1))) {
            attack = petir_bitboard_mask::horzmask2[sto][(pas::shr(data.allpieces, pas::add((sto & 56), 1)) & 63)];
            if (((attack & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
                return_value = true;
            }
        }
        else if (((pas_temp_3 == 8))) {
            attack = petir_bitboard_mask::vertmask2[sto][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[sto]) & 63)];
            if (((attack & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
                return_value = true;
            }
        }
    }
    else if (((pas_temp_1 == petir_header::_mentrihitam))) {
        pas_temp_4 = pas::abs(petir_bitboard_mask::direction[sto][posraja]);
        if (((pas_temp_4 == 1))) {
            attack = petir_bitboard_mask::horzmask2[sto][(pas::shr(data.allpieces, pas::add((sto & 56), 1)) & 63)];
            if (((attack & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
                return_value = true;
            }
        }
        else if (((pas_temp_4 == 8))) {
            attack = petir_bitboard_mask::vertmask2[sto][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[sto]) & 63)];
            if (((attack & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
                return_value = true;
            }
        }
        else if (((pas_temp_4 == 7))) {
            attack = petir_bitboard_mask::diaga8h1mask[sto][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[sto]) & 255)];
            if (((attack & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
                return_value = true;
            }
        }
        else if (((pas_temp_4 == 9))) {
            attack = petir_bitboard_mask::diagh8a1mask[sto][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[sto]) & 255)];
            if (((attack & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
                return_value = true;
            }
        }
    }
    return return_value;
}

// tools.pas:1079
bool black_menskak3(petir_header::tdata& data, std::uint16_t moves, std::int32_t& threatpiece) {
    bool return_value{};
    std::uint8_t sfrom{};
    std::uint8_t sto{};
    std::int32_t posraja{};
    std::uint64_t attack{};
    std::uint64_t raja{};
    std::int32_t pas_temp_1{};
    std::int32_t pas_temp_2{};
    std::int32_t pas_temp_3{};
    std::int32_t pas_temp_4{};
    return_value = false;
    posraja = petir_bitboard_mask::firstbitp(pas::address(data.kingwhite));
    sfrom = (moves & 127);
    sto = (pas::shr(moves, 7) & 127);
    if ((sto > 63)) {
        return_value = true;
        return return_value;
    }
    raja = petir_bitboard_mask::bit2nmask[posraja];
    pas_temp_1 = data.papan[sfrom];
    if (((pas_temp_1 == petir_header::_pionhitam))) {
        if (((petir_bitboard_mask::b_pawn_attack[sto] & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
            return_value = true;
        }
    }
    else if (((pas_temp_1 == petir_header::_kudahitam))) {
        if (((petir_bitboard_mask::knightmask[sto] & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
            return_value = true;
        }
    }
    else if (((pas_temp_1 == petir_header::_gajahhitam))) {
        pas_temp_2 = pas::abs(petir_bitboard_mask::direction[sto][posraja]);
        if (((pas_temp_2 == 7))) {
            attack = petir_bitboard_mask::diaga8h1mask[sto][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[sto]) & 255)];
            if (((attack & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
                return_value = true;
            }
        }
        else if (((pas_temp_2 == 9))) {
            attack = petir_bitboard_mask::diagh8a1mask[sto][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[sto]) & 255)];
            if (((attack & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
                return_value = true;
            }
        }
    }
    else if (((pas_temp_1 == petir_header::_bentenghitam))) {
        pas_temp_3 = pas::abs(petir_bitboard_mask::direction[sto][posraja]);
        if (((pas_temp_3 == 1))) {
            attack = petir_bitboard_mask::horzmask2[sto][(pas::shr(data.allpieces, pas::add((sto & 56), 1)) & 63)];
            if (((attack & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
                return_value = true;
            }
        }
        else if (((pas_temp_3 == 8))) {
            attack = petir_bitboard_mask::vertmask2[sto][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[sto]) & 63)];
            if (((attack & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
                return_value = true;
            }
        }
    }
    else if (((pas_temp_1 == petir_header::_mentrihitam))) {
        pas_temp_4 = pas::abs(petir_bitboard_mask::direction[sto][posraja]);
        if (((pas_temp_4 == 1))) {
            attack = petir_bitboard_mask::horzmask2[sto][(pas::shr(data.allpieces, pas::add((sto & 56), 1)) & 63)];
            if (((attack & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
                return_value = true;
            }
        }
        else if (((pas_temp_4 == 8))) {
            attack = petir_bitboard_mask::vertmask2[sto][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[sto]) & 63)];
            if (((attack & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
                return_value = true;
            }
        }
        else if (((pas_temp_4 == 7))) {
            attack = petir_bitboard_mask::diaga8h1mask[sto][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[sto]) & 255)];
            if (((attack & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
                return_value = true;
            }
        }
        else if (((pas_temp_4 == 9))) {
            attack = petir_bitboard_mask::diagh8a1mask[sto][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[sto]) & 255)];
            if (((attack & petir_bitboard_mask::bit2nmask[posraja]) != 0)) {
                return_value = true;
            }
        }
    }
    return return_value;
}

}
