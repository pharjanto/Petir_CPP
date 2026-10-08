// Faithful translation of customboard.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_customboard {
std::int32_t a{};
std::int32_t b{};
std::int32_t pos{};
std::int32_t kar{};
// customboard.pas:83
void fillbitboard(petir_header::tdata& data) {
    std::int32_t a{};
    std::int32_t pas_temp_1{};
    data.whitepieces = 0;
    data.blackpieces = 0;
    data.allpiecesr90 = 0;
    data.allpiecesa8h1 = 0;
    data.allpiecesh8a1 = 0;
    data.allpieces = 0;
    data.kingblack = 0;
    data.kingwhite = 0;
    data.pawnblack = 0;
    data.pawnwhite = 0;
    data.knightblack = 0;
    data.knightwhite = 0;
    data.bishopblack = 0;
    data.bishopwhite = 0;
    data.rookblack = 0;
    data.rookwhite = 0;
    data.queenblack = 0;
    data.queenwhite = 0;
    data.materialscore = 0;
    data.hashkey = 0;
    data.ep = petir_header::_no_ep;
    data.nilai_perwira_putih = 0;
    data.nilai_perwira_hitam = 0;
    data.wc = 0;
    data.bc = 0;
    data.ngajahhitam = 0;
    data.ngajahputih = 0;
    pas_temp_1 = 63;
    for (a = 0; a <= pas_temp_1; ++a) {
        if ((data.papan[a] == petir_header::_pionputih)) {
            (data.materialscore = pas::increment(data.materialscore, petir_header::_nilai_pion));
            data.pawnwhite = (data.pawnwhite | petir_bitboard_mask::bit2nmask[a]);
            data.allpieces = (data.allpieces | petir_bitboard_mask::bit2nmask[a]);
            data.whitepieces = (data.whitepieces | petir_bitboard_mask::bit2nmask[a]);
            data.allpiecesr90 = (data.allpiecesr90 | petir_bitboard_mask::bit2nmask90[a]);
            data.allpiecesa8h1 = (data.allpiecesa8h1 | petir_bitboard_mask::bit2nmaska8h1[a]);
            data.allpiecesh8a1 = (data.allpiecesh8a1 | petir_bitboard_mask::bit2nmaskh8a1[a]);
        } else {
            if ((data.papan[a] == petir_header::_pionhitam)) {
                (data.materialscore = pas::decrement(data.materialscore, petir_header::_nilai_pion));
                data.pawnblack = (data.pawnblack | petir_bitboard_mask::bit2nmask[a]);
                data.allpieces = (data.allpieces | petir_bitboard_mask::bit2nmask[a]);
                data.blackpieces = (data.blackpieces | petir_bitboard_mask::bit2nmask[a]);
                data.allpiecesr90 = (data.allpiecesr90 | petir_bitboard_mask::bit2nmask90[a]);
                data.allpiecesa8h1 = (data.allpiecesa8h1 | petir_bitboard_mask::bit2nmaska8h1[a]);
                data.allpiecesh8a1 = (data.allpiecesh8a1 | petir_bitboard_mask::bit2nmaskh8a1[a]);
            }
        }
        if ((data.papan[a] == petir_header::_kudaputih)) {
            (data.materialscore = pas::increment(data.materialscore, petir_header::_nilai_kuda));
            data.knightwhite = (data.knightwhite | petir_bitboard_mask::bit2nmask[a]);
            data.allpieces = (data.allpieces | petir_bitboard_mask::bit2nmask[a]);
            data.whitepieces = (data.whitepieces | petir_bitboard_mask::bit2nmask[a]);
            data.allpiecesr90 = (data.allpiecesr90 | petir_bitboard_mask::bit2nmask90[a]);
            data.allpiecesa8h1 = (data.allpiecesa8h1 | petir_bitboard_mask::bit2nmaska8h1[a]);
            data.allpiecesh8a1 = (data.allpiecesh8a1 | petir_bitboard_mask::bit2nmaskh8a1[a]);
            (data.nilai_perwira_putih = pas::increment(data.nilai_perwira_putih, 3));
        } else {
            if ((data.papan[a] == petir_header::_gajahputih)) {
                (data.materialscore = pas::increment(data.materialscore, petir_header::_nilai_gajah));
                data.bishopwhite = (data.bishopwhite | petir_bitboard_mask::bit2nmask[a]);
                data.allpieces = (data.allpieces | petir_bitboard_mask::bit2nmask[a]);
                data.whitepieces = (data.whitepieces | petir_bitboard_mask::bit2nmask[a]);
                data.allpiecesr90 = (data.allpiecesr90 | petir_bitboard_mask::bit2nmask90[a]);
                data.allpiecesa8h1 = (data.allpiecesa8h1 | petir_bitboard_mask::bit2nmaska8h1[a]);
                data.allpiecesh8a1 = (data.allpiecesh8a1 | petir_bitboard_mask::bit2nmaskh8a1[a]);
                (data.ngajahputih = pas::increment(data.ngajahputih, 1));
                (data.nilai_perwira_putih = pas::increment(data.nilai_perwira_putih, 3));
            } else {
                if ((data.papan[a] == petir_header::_bentengputih)) {
                    (data.materialscore = pas::increment(data.materialscore, petir_header::_nilai_benteng));
                    data.rookwhite = (data.rookwhite | petir_bitboard_mask::bit2nmask[a]);
                    data.allpieces = (data.allpieces | petir_bitboard_mask::bit2nmask[a]);
                    data.whitepieces = (data.whitepieces | petir_bitboard_mask::bit2nmask[a]);
                    data.allpiecesr90 = (data.allpiecesr90 | petir_bitboard_mask::bit2nmask90[a]);
                    data.allpiecesa8h1 = (data.allpiecesa8h1 | petir_bitboard_mask::bit2nmaska8h1[a]);
                    data.allpiecesh8a1 = (data.allpiecesh8a1 | petir_bitboard_mask::bit2nmaskh8a1[a]);
                    (data.nilai_perwira_putih = pas::increment(data.nilai_perwira_putih, 5));
                } else {
                    if ((data.papan[a] == petir_header::_mentriputih)) {
                        (data.materialscore = pas::increment(data.materialscore, petir_header::_nilai_mentri));
                        data.queenwhite = (data.queenwhite | petir_bitboard_mask::bit2nmask[a]);
                        data.allpieces = (data.allpieces | petir_bitboard_mask::bit2nmask[a]);
                        data.whitepieces = (data.whitepieces | petir_bitboard_mask::bit2nmask[a]);
                        data.allpiecesr90 = (data.allpiecesr90 | petir_bitboard_mask::bit2nmask90[a]);
                        data.allpiecesa8h1 = (data.allpiecesa8h1 | petir_bitboard_mask::bit2nmaska8h1[a]);
                        data.allpiecesh8a1 = (data.allpiecesh8a1 | petir_bitboard_mask::bit2nmaskh8a1[a]);
                        (data.nilai_perwira_putih = pas::increment(data.nilai_perwira_putih, 9));
                    } else {
                        if ((data.papan[a] == petir_header::_rajaputih)) {
                            data.kingwhite = (data.kingwhite | petir_bitboard_mask::bit2nmask[a]);
                            data.allpieces = (data.allpieces | petir_bitboard_mask::bit2nmask[a]);
                            data.whitepieces = (data.whitepieces | petir_bitboard_mask::bit2nmask[a]);
                            data.allpiecesr90 = (data.allpiecesr90 | petir_bitboard_mask::bit2nmask90[a]);
                            data.allpiecesa8h1 = (data.allpiecesa8h1 | petir_bitboard_mask::bit2nmaska8h1[a]);
                            data.allpiecesh8a1 = (data.allpiecesh8a1 | petir_bitboard_mask::bit2nmaskh8a1[a]);
                        } else {
                            if ((data.papan[a] == petir_header::_kudahitam)) {
                                (data.materialscore = pas::decrement(data.materialscore, petir_header::_nilai_kuda));
                                data.knightblack = (data.knightblack | petir_bitboard_mask::bit2nmask[a]);
                                data.allpieces = (data.allpieces | petir_bitboard_mask::bit2nmask[a]);
                                data.blackpieces = (data.blackpieces | petir_bitboard_mask::bit2nmask[a]);
                                data.allpiecesr90 = (data.allpiecesr90 | petir_bitboard_mask::bit2nmask90[a]);
                                data.allpiecesa8h1 = (data.allpiecesa8h1 | petir_bitboard_mask::bit2nmaska8h1[a]);
                                data.allpiecesh8a1 = (data.allpiecesh8a1 | petir_bitboard_mask::bit2nmaskh8a1[a]);
                                (data.nilai_perwira_hitam = pas::increment(data.nilai_perwira_hitam, 3));
                            } else {
                                if ((data.papan[a] == petir_header::_gajahhitam)) {
                                    (data.materialscore = pas::decrement(data.materialscore, petir_header::_nilai_gajah));
                                    data.bishopblack = (data.bishopblack | petir_bitboard_mask::bit2nmask[a]);
                                    data.allpieces = (data.allpieces | petir_bitboard_mask::bit2nmask[a]);
                                    data.blackpieces = (data.blackpieces | petir_bitboard_mask::bit2nmask[a]);
                                    data.allpiecesr90 = (data.allpiecesr90 | petir_bitboard_mask::bit2nmask90[a]);
                                    data.allpiecesa8h1 = (data.allpiecesa8h1 | petir_bitboard_mask::bit2nmaska8h1[a]);
                                    data.allpiecesh8a1 = (data.allpiecesh8a1 | petir_bitboard_mask::bit2nmaskh8a1[a]);
                                    (data.nilai_perwira_hitam = pas::increment(data.nilai_perwira_hitam, 3));
                                    (data.ngajahhitam = pas::increment(data.ngajahhitam, 1));
                                } else {
                                    if ((data.papan[a] == petir_header::_bentenghitam)) {
                                        (data.materialscore = pas::decrement(data.materialscore, petir_header::_nilai_benteng));
                                        data.rookblack = (data.rookblack | petir_bitboard_mask::bit2nmask[a]);
                                        data.allpieces = (data.allpieces | petir_bitboard_mask::bit2nmask[a]);
                                        data.blackpieces = (data.blackpieces | petir_bitboard_mask::bit2nmask[a]);
                                        data.allpiecesr90 = (data.allpiecesr90 | petir_bitboard_mask::bit2nmask90[a]);
                                        data.allpiecesa8h1 = (data.allpiecesa8h1 | petir_bitboard_mask::bit2nmaska8h1[a]);
                                        data.allpiecesh8a1 = (data.allpiecesh8a1 | petir_bitboard_mask::bit2nmaskh8a1[a]);
                                        (data.nilai_perwira_hitam = pas::increment(data.nilai_perwira_hitam, 5));
                                    } else {
                                        if ((data.papan[a] == petir_header::_mentrihitam)) {
                                            (data.materialscore = pas::decrement(data.materialscore, petir_header::_nilai_mentri));
                                            data.queenblack = (data.queenblack | petir_bitboard_mask::bit2nmask[a]);
                                            data.allpieces = (data.allpieces | petir_bitboard_mask::bit2nmask[a]);
                                            data.blackpieces = (data.blackpieces | petir_bitboard_mask::bit2nmask[a]);
                                            data.allpiecesr90 = (data.allpiecesr90 | petir_bitboard_mask::bit2nmask90[a]);
                                            data.allpiecesa8h1 = (data.allpiecesa8h1 | petir_bitboard_mask::bit2nmaska8h1[a]);
                                            data.allpiecesh8a1 = (data.allpiecesh8a1 | petir_bitboard_mask::bit2nmaskh8a1[a]);
                                            (data.nilai_perwira_hitam = pas::increment(data.nilai_perwira_hitam, 9));
                                        } else {
                                            if ((data.papan[a] == petir_header::_rajahitam)) {
                                                data.kingblack = (data.kingblack | petir_bitboard_mask::bit2nmask[a]);
                                                data.allpieces = (data.allpieces | petir_bitboard_mask::bit2nmask[a]);
                                                data.blackpieces = (data.blackpieces | petir_bitboard_mask::bit2nmask[a]);
                                                data.allpiecesr90 = (data.allpiecesr90 | petir_bitboard_mask::bit2nmask90[a]);
                                                data.allpiecesa8h1 = (data.allpiecesa8h1 | petir_bitboard_mask::bit2nmaska8h1[a]);
                                                data.allpiecesh8a1 = (data.allpiecesh8a1 | petir_bitboard_mask::bit2nmaskh8a1[a]);
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
    }
    data.move50count = 0;
    data.rookqueen = (((data.queenblack | data.queenwhite) | data.rookblack) | data.rookwhite);
    data.bishopqueen = (((data.queenblack | data.queenwhite) | data.bishopblack) | data.bishopwhite);
}

// customboard.pas:14
void readnotation(pas::String n, petir_header::tdata& data, bool& wtm) {
    bool wq{};
    bool wk{};
    bool bq{};
    bool bk{};
    std::int32_t pas_temp_1{};
    std::int32_t pas_temp_2{};
    std::int32_t pas_temp_3{};
    petir_customboard::pos = 56;
    data.flagrokade = 0;
    pas_temp_1 = pas::length(n);
    for (petir_customboard::a = 1; petir_customboard::a <= pas_temp_1; ++petir_customboard::a) {
        if (((n[petir_customboard::a] >= '0' && n[petir_customboard::a] <= '9'))) {
            pas_temp_2 = pas::sub(pas::add(petir_customboard::pos, pas::strtoint(n[petir_customboard::a])), 1);
            for (petir_customboard::b = petir_customboard::pos; petir_customboard::b <= pas_temp_2; ++petir_customboard::b) {
                data.papan[petir_customboard::b] = 0;
            }
            petir_customboard::pos = pas::add(petir_customboard::pos, pas::strtoint(n[petir_customboard::a]));
        } else {
            if ((n[petir_customboard::a] == 'R')) {
                data.papan[petir_customboard::pos] = petir_header::_bentengputih;
            } else {
                if ((n[petir_customboard::a] == 'B')) {
                    data.papan[petir_customboard::pos] = petir_header::_gajahputih;
                } else {
                    if ((n[petir_customboard::a] == 'N')) {
                        data.papan[petir_customboard::pos] = petir_header::_kudaputih;
                    } else {
                        if ((n[petir_customboard::a] == 'Q')) {
                            data.papan[petir_customboard::pos] = petir_header::_mentriputih;
                        } else {
                            if ((n[petir_customboard::a] == 'P')) {
                                data.papan[petir_customboard::pos] = petir_header::_pionputih;
                            } else {
                                if ((n[petir_customboard::a] == 'K')) {
                                    data.papan[petir_customboard::pos] = petir_header::_rajaputih;
                                } else {
                                    if ((n[petir_customboard::a] == 'r')) {
                                        data.papan[petir_customboard::pos] = petir_header::_bentenghitam;
                                    } else {
                                        if ((n[petir_customboard::a] == 'b')) {
                                            data.papan[petir_customboard::pos] = petir_header::_gajahhitam;
                                        } else {
                                            if ((n[petir_customboard::a] == 'n')) {
                                                data.papan[petir_customboard::pos] = petir_header::_kudahitam;
                                            } else {
                                                if ((n[petir_customboard::a] == 'q')) {
                                                    data.papan[petir_customboard::pos] = petir_header::_mentrihitam;
                                                } else {
                                                    if ((n[petir_customboard::a] == 'p')) {
                                                        data.papan[petir_customboard::pos] = petir_header::_pionhitam;
                                                    } else {
                                                        if ((n[petir_customboard::a] == 'k')) {
                                                            data.papan[petir_customboard::pos] = petir_header::_rajahitam;
                                                        } else {
                                                            if ((n[petir_customboard::a] == '/')) {
                                                                petir_customboard::pos = pas::sub(petir_customboard::pos, 16);
                                                            } else {
                                                                if ((n[petir_customboard::a] == ' ')) {
                                                                    if ((n[pas::add(petir_customboard::a, 1)] == 'b')) {
                                                                        wtm = false;
                                                                    } else {
                                                                        if ((n[pas::add(petir_customboard::a, 1)] == 'w')) {
                                                                            wtm = true;
                                                                        }
                                                                    }
                                                                    petir_customboard::b = petir_customboard::a;
                                                                    (petir_customboard::b = pas::increment(petir_customboard::b, 3));
                                                                    wq = false;
                                                                    wk = false;
                                                                    bq = false;
                                                                    bk = false;
                                                                    while (((petir_customboard::b <= pas::length(n)) && ((n[petir_customboard::b] == 'K') || (n[petir_customboard::b] == 'Q') || (n[petir_customboard::b] == 'k') || (n[petir_customboard::b] == 'q')))) {
                                                                        pas_temp_3 = n[petir_customboard::b];
                                                                        if (((pas_temp_3 == 'K'))) {
                                                                            wk = true;
                                                                        }
                                                                        else if (((pas_temp_3 == 'Q'))) {
                                                                            wq = true;
                                                                        }
                                                                        else if (((pas_temp_3 == 'k'))) {
                                                                            bk = true;
                                                                        }
                                                                        else if (((pas_temp_3 == 'q'))) {
                                                                            bq = true;
                                                                        }
                                                                        (petir_customboard::b = pas::increment(petir_customboard::b, 1));
                                                                    }
                                                                    if (wk) {
                                                                        data.flagrokade = (data.flagrokade | petir_header::bitputihsc);
                                                                    }
                                                                    if (wq) {
                                                                        data.flagrokade = (data.flagrokade | petir_header::bitputihlc);
                                                                    }
                                                                    if (bk) {
                                                                        data.flagrokade = (data.flagrokade | petir_header::bithitamsc);
                                                                    }
                                                                    if (bq) {
                                                                        data.flagrokade = (data.flagrokade | petir_header::bithitamlc);
                                                                    }
                                                                    return;
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
                        }
                    }
                }
            }
        }
        if (((n[petir_customboard::a] >= 'a' && n[petir_customboard::a] <= 'z') || (n[petir_customboard::a] >= 'A' && n[petir_customboard::a] <= 'Z'))) {
            (petir_customboard::pos = pas::increment(petir_customboard::pos, 1));
        }
    }
}

}
