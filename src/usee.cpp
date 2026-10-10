// Faithful translation of usee.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_usee {
// usee.pas:83
std::int32_t see(std::int32_t giliran, const petir_header::tdata& data, std::int32_t sfrom, std::int32_t sto) {
    std::int32_t return_value{};
    // Every entry used by the backward pass is assigned during the forward pass.
    pas::Array<std::int8_t, 0, 32> list;
    std::uint64_t attack{};
    std::uint64_t temp{};
    std::int32_t piece{};
    std::int32_t square{};
    std::int32_t n{};
    attack = (petir_bitboard_mask::knightmask[sto] & (data.knightblack | data.knightwhite));
    attack = (attack | ((petir_bitboard_mask::horzmask2[sto][(pas::shr(data.allpieces, pas::add((sto & 56), 1)) & 63)] | petir_bitboard_mask::vertmask2[sto][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[sto]) & 63)]) & data.rookqueen));
    attack = (attack | ((petir_bitboard_mask::diaga8h1mask[sto][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[sto]) & 255)] | petir_bitboard_mask::diagh8a1mask[sto][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[sto]) & 255)]) & data.bishopqueen));
    attack = (attack | (petir_bitboard_mask::w_pawn_attack[sto] & data.pawnblack));
    attack = (attack | (petir_bitboard_mask::b_pawn_attack[sto] & data.pawnwhite));
    attack = (attack | (petir_bitboard_mask::kingmask[sto] & (data.kingblack | data.kingwhite)));
    list[0] = petir_usee::nilai_piece[data.papan[sto]];
    petir_usee::xray(attack, sfrom, sto, data);
    attack = (attack & (~petir_bitboard_mask::bit(sfrom)));
    giliran = pas::sub(3, giliran);
    piece = petir_usee::nilai_piece[data.papan[sfrom]];
    n = 1;
    while ((attack != 0)) {
        if ((giliran == petir_header::_sisiputih)) {
            temp = (attack & data.pawnwhite);
            if ((temp != 0)) {
                square = petir_bitboard_mask::lastbitp(pas::address(temp));
            } else {
                temp = (attack & data.knightwhite);
                if ((temp != 0)) {
                    square = petir_bitboard_mask::lastbitp(pas::address(temp));
                } else {
                    temp = (attack & data.bishopwhite);
                    if ((temp != 0)) {
                        square = petir_bitboard_mask::lastbitp(pas::address(temp));
                    } else {
                        temp = (attack & data.rookwhite);
                        if ((temp != 0)) {
                            square = petir_bitboard_mask::lastbitp(pas::address(temp));
                        } else {
                            temp = (attack & data.queenwhite);
                            if ((temp != 0)) {
                                square = petir_bitboard_mask::lastbitp(pas::address(temp));
                            } else {
                                temp = (attack & data.kingwhite);
                                if ((temp != 0)) {
                                    square = petir_bitboard_mask::firstbitp(pas::address(temp));
                                } else {
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        } else {
            temp = (attack & data.pawnblack);
            if ((temp != 0)) {
                square = petir_bitboard_mask::firstbitp(pas::address(temp));
            } else {
                temp = (attack & data.knightblack);
                if ((temp != 0)) {
                    square = petir_bitboard_mask::firstbitp(pas::address(temp));
                } else {
                    temp = (attack & data.bishopblack);
                    if ((temp != 0)) {
                        square = petir_bitboard_mask::firstbitp(pas::address(temp));
                    } else {
                        temp = (attack & data.rookblack);
                        if ((temp != 0)) {
                            square = petir_bitboard_mask::firstbitp(pas::address(temp));
                        } else {
                            temp = (attack & data.queenblack);
                            if ((temp != 0)) {
                                square = petir_bitboard_mask::firstbitp(pas::address(temp));
                            } else {
                                temp = (attack & data.kingblack);
                                if ((temp != 0)) {
                                    square = petir_bitboard_mask::lastbitp(pas::address(temp));
                                } else {
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        list[n] = pas::add(pas::negate(list[pas::sub(n, 1)]), piece);
        piece = petir_usee::nilai_piece[data.papan[square]];
        petir_usee::xray(attack, square, sto, data);
        attack = (attack & petir_bitboard_mask::bitnot(square));
        (n = pas::increment(n, 1));
        giliran = pas::sub(3, giliran);
    }
    (n = pas::decrement(n, 1));
    while ((n > 0)) {
        if ((list[n] > pas::negate(list[pas::sub(n, 1)]))) {
            list[pas::sub(n, 1)] = pas::negate(list[n]);
        }
        (n = pas::decrement(n, 1));
    }
    return_value = list[0];
    return return_value;
}

// usee.pas:21
void xray(std::uint64_t& attack, std::int32_t square, std::int32_t target, const petir_header::tdata& data) {
    std::int32_t dn{};
    std::int32_t a{};
    std::uint64_t temp{};
    std::int32_t pas_temp_1{};
    dn = petir_bitboard_mask::direction2[target][square];
    if ((dn == 0)) {
        return;
    }
    pas_temp_1 = dn;
    if (((pas_temp_1 == 8))) {
        temp = (data.bishopqueen & petir_bitboard_mask::bishop_attack_plus7_mask[square]);
        if ((temp != 0)) {
            attack = (attack | (petir_bitboard_mask::diaga8h1mask[square][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[square]) & 255)] & temp));
        }
    }
    else if (((pas_temp_1 == 7))) {
        temp = (data.bishopqueen & petir_bitboard_mask::bishop_attack_min7_mask[square]);
        if ((temp != 0)) {
            attack = (attack | (petir_bitboard_mask::diaga8h1mask[square][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[square]) & 255)] & temp));
        }
    }
    else if (((pas_temp_1 == 6))) {
        temp = (data.bishopqueen & petir_bitboard_mask::bishop_attack_plus9_mask[square]);
        if ((temp != 0)) {
            attack = (attack | (petir_bitboard_mask::diagh8a1mask[square][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[square]) & 255)] & temp));
        }
    }
    else if (((pas_temp_1 == 5))) {
        temp = (data.bishopqueen & petir_bitboard_mask::bishop_attack_min9_mask[square]);
        if ((temp != 0)) {
            attack = (attack | (petir_bitboard_mask::diagh8a1mask[square][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[square]) & 255)] & temp));
        }
    }
    else if (((pas_temp_1 == 4))) {
        temp = (data.rookqueen & petir_bitboard_mask::rook_attack_right_mask[square]);
        if ((temp != 0)) {
            attack = (attack | (petir_bitboard_mask::horzmask2[square][(pas::shr(data.allpieces, pas::add((square & 56), 1)) & 63)] & temp));
        }
    }
    else if (((pas_temp_1 == 3))) {
        temp = (data.rookqueen & petir_bitboard_mask::rook_attack_left_mask[square]);
        if ((temp != 0)) {
            attack = (attack | (petir_bitboard_mask::horzmask2[square][(pas::shr(data.allpieces, pas::add((square & 56), 1)) & 63)] & temp));
        }
    }
    else if (((pas_temp_1 == 2))) {
        temp = (data.rookqueen & petir_bitboard_mask::rook_attack_up_mask[square]);
        if ((temp != 0)) {
            attack = (attack | (petir_bitboard_mask::vertmask2[square][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[square]) & 63)] & temp));
        }
    }
    else {
        temp = (data.rookqueen & petir_bitboard_mask::rook_attack_down_mask[square]);
        if ((temp != 0)) {
            attack = (attack | (petir_bitboard_mask::vertmask2[square][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[square]) & 63)] & temp));
        }
    }
}

}
