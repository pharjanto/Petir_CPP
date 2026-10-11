// Faithful translation of valid.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_valid {
// valid.pas:10
bool isvalid(petir_header::tdata& data, std::int32_t moves, std::int32_t giliran) {
    bool return_value{};
    std::int32_t sfrom{};
    std::int32_t sto{};
    std::int32_t piece{};
    std::int32_t piece2{};
    std::int32_t d{};
    std::uint64_t bitto{};
    std::uint64_t attack{};
    sfrom = (moves & 127);
    sto = (pas::shr(moves, 7) & 127);
    return_value = false;
    piece = data.papan[sfrom];
    if ((sto <= 63)) {
        if (((piece > 0) && (data.papan[sto] > 0))) {
            return return_value;
        } else {
            if (((piece < 0) && (data.papan[sto] < 0))) {
                return return_value;
            }
        }
    } else {
        if (((sto == 65) || (sto == 66))) {
            if (((giliran == petir_header::_sisiputih) && (data.papan[petir_header::e1] != petir_header::_rajaputih))) {
                return return_value;
            }
            if (((giliran == petir_header::_sisihitam) && (data.papan[petir_header::e8] != petir_header::_rajahitam))) {
                return return_value;
            }
        }
    }
    if (((giliran == petir_header::_sisiputih) && (data.papan[sfrom] <= 0))) {
        return return_value;
    }
    if (((giliran == petir_header::_sisihitam) && (data.papan[sfrom] >= 0))) {
        return return_value;
    }
    piece2 = pas::abs(piece);
    if ((sto <= 63)) {
        bitto = petir_bitboard_mask::bit(sto);
    }
    if ((piece == petir_header::_pionputih)) {
        d = pas::sub(sto, sfrom);
        if ((sto == petir_header::_en_passant)) {
            if (((pas::abs(pas::sub((sfrom & 7), data.ep)) == 1) && (pas::shr(sfrom, 3) == 4))) {
                return_value = true;
                return return_value;
            }
        }
        // Not in valid.pas: a capture must change file by one, or an edge pawn
        // would "capture" around the board (a3 to h3 is d == 7).
        if ((((d == 7) || (d == 9)) && (pas::abs(pas::sub((sto & 7), (sfrom & 7))) == 1) && (data.papan[sto] < 0))) {
            return_value = true;
            return return_value;
        }
        if (((d == 8) && (data.papan[sto] == 0))) {
            return_value = true;
            return return_value;
        }
        if (((((d == 16) && (data.papan[sto] == 0)) && (data.papan[pas::add(sfrom, 8)] == 0)) && (sfrom <= petir_header::h2))) {
            return_value = true;
            return return_value;
        }
    }
    if ((piece == petir_header::_pionhitam)) {
        d = pas::sub(sfrom, sto);
        if ((sto == petir_header::_en_passant)) {
            if (((pas::abs(pas::sub((sfrom & 7), data.ep)) == 1) && (pas::shr(sfrom, 3) == 3))) {
                return_value = true;
                return return_value;
            }
        }
        if ((((d == 7) || (d == 9)) && (pas::abs(pas::sub((sto & 7), (sfrom & 7))) == 1) && (data.papan[sto] > 0))) {
            return_value = true;
            return return_value;
        }
        if (((d == 8) && (data.papan[sto] == 0))) {
            return_value = true;
            return return_value;
        }
        if (((((d == 16) && (data.papan[sto] == 0)) && (data.papan[pas::sub(sfrom, 8)] == 0)) && (sfrom >= petir_header::a7))) {
            return_value = true;
            return return_value;
        }
    }
    if ((piece2 == petir_header::_kudaputih)) {
        if (((petir_bitboard_mask::knightmask[sfrom] & bitto) != 0)) {
            return_value = true;
            return return_value;
        }
    }
    if (((piece2 == petir_header::_bentengputih) || (piece2 == petir_header::_mentriputih))) {
        attack = petir_bitboard_mask::horzmask2[sfrom][(pas::shr(data.allpieces, pas::add((sfrom & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[sfrom][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[sfrom]) & 63)]);
        if ((((attack & bitto) != 0) && ((pas::shr(moves, 14) & 7) == 0))) {
            return_value = true;
            return return_value;
        }
    }
    if (((piece2 == petir_header::_gajahputih) || (piece2 == petir_header::_mentriputih))) {
        attack = petir_bitboard_mask::diaga8h1mask[sfrom][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[sfrom]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[sfrom][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[sfrom]) & 255)]);
        if ((((attack & bitto) != 0) && ((pas::shr(moves, 14) & 7) == 0))) {
            return_value = true;
            return return_value;
        }
    }
    if ((piece == petir_header::_rajaputih)) {
        if ((sto == petir_header::_rokadependek)) {
            if ((((((((((data.flagrokade & petir_header::bitputihsc) != 0) && (data.papan[petir_header::e1] == petir_header::_rajaputih)) && (data.papan[petir_header::f1] == 0)) && (data.papan[petir_header::g1] == 0)) && (!petir_tools::white_attacked(data, petir_header::e1))) && (!petir_tools::white_attacked(data, petir_header::f1))) && (!petir_tools::white_attacked(data, petir_header::g1))) && (data.papan[petir_header::h1] == petir_header::_bentengputih))) {
                return_value = true;
                return return_value;
            }
        } else {
            if ((sto == petir_header::_rokadepanjang)) {
                if (((((((((((data.flagrokade & petir_header::bitputihlc) != 0) && (data.papan[petir_header::e1] == petir_header::_rajaputih)) && (data.papan[petir_header::d1] == 0)) && (data.papan[petir_header::c1] == 0)) && (data.papan[petir_header::b1] == 0)) && (!petir_tools::white_attacked(data, petir_header::e1))) && (!petir_tools::white_attacked(data, petir_header::d1))) && (!petir_tools::white_attacked(data, petir_header::c1))) && (data.papan[petir_header::a1] == petir_header::_bentengputih))) {
                    return_value = true;
                    return return_value;
                }
            } else {
                if (((petir_bitboard_mask::kingmask[sfrom] & bitto) != 0)) {
                    return_value = true;
                    return return_value;
                }
            }
        }
    }
    if ((piece == petir_header::_rajahitam)) {
        if ((sto == petir_header::_rokadependek)) {
            if ((((((((((data.flagrokade & petir_header::bithitamsc) != 0) && (!petir_tools::black_attacked(data, petir_header::e8))) && (data.papan[petir_header::e8] == petir_header::_rajahitam)) && (data.papan[petir_header::f8] == 0)) && (data.papan[petir_header::g8] == 0)) && (!petir_tools::black_attacked(data, petir_header::f8))) && (!petir_tools::black_attacked(data, petir_header::g8))) && (data.papan[petir_header::h8] == petir_header::_bentenghitam))) {
                return_value = true;
                return return_value;
            }
        } else {
            if ((sto == petir_header::_rokadepanjang)) {
                if (((((((((((data.flagrokade & petir_header::bithitamlc) != 0) && (data.papan[petir_header::e8] == petir_header::_rajahitam)) && (!petir_tools::black_attacked(data, petir_header::e8))) && (data.papan[petir_header::d8] == 0)) && (data.papan[petir_header::c8] == 0)) && (data.papan[petir_header::b8] == 0)) && (!petir_tools::black_attacked(data, petir_header::d8))) && (!petir_tools::black_attacked(data, petir_header::c8))) && (data.papan[petir_header::a8] == petir_header::_bentenghitam))) {
                    return_value = true;
                    return return_value;
                }
            } else {
                if (((petir_bitboard_mask::kingmask[sfrom] & bitto) != 0)) {
                    return_value = true;
                    return return_value;
                }
            }
        }
    }
    return return_value;
}

}
