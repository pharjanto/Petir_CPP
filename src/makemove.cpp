// Faithful translation of makemove.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_makemove {
// makemove.pas:413
void makewhitemove(std::int32_t moves, petir_header::tdata& data) {
    std::int32_t captured_piece{};
    std::int32_t sfrom{};
    std::int32_t sto{};
    std::int32_t promosi{};
    std::int32_t pas_temp_1{};
    (petir_header::total_node = pas::increment(petir_header::total_node, 1));
    sfrom = (moves & 127);
    sto = (pas::shr(moves, 7) & 127);
    (data.move50count = pas::increment(data.move50count, 1));
    pas_temp_1 = sto;
    if (((pas_temp_1 == petir_header::_rokadependek))) {
        data.cp = 0;
        petir_makemove::whiterokadependek(data);
    }
    else if (((pas_temp_1 == petir_header::_rokadepanjang))) {
        data.cp = 0;
        petir_makemove::whiterokadepanjang(data);
    }
    else if (((pas_temp_1 == petir_header::_en_passant))) {
        petir_makemove::white_ep(data, sfrom, sto);
        data.cp = petir_header::_pionputih;
    }
    else {
        if ((data.ep != petir_header::_no_ep)) {
            data.hashkey = ((data.hashkey ^ petir_header::enpassanthashkey[data.ep]) ^ petir_header::enpassanthashkey[petir_header::_no_ep]);
            data.ep = petir_header::_no_ep;
        }
        captured_piece = data.papan[sto];
        if ((captured_piece != 0)) {
            data.move50count = 0;
            data.blackpieces = (data.blackpieces & petir_bitboard_mask::bitnot(sto));
            if ((captured_piece == petir_header::_pionhitam)) {
                data.pawnblack = (data.pawnblack & petir_bitboard_mask::bitnot(sto));
                data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_pionhitam][sto]);
                data.cp = petir_header::_pionputih;
                (data.materialscore = pas::increment(data.materialscore, petir_header::_nilai_pion));
            } else {
                if ((captured_piece == petir_header::_kudahitam)) {
                    (data.materialscore = pas::increment(data.materialscore, petir_header::_nilai_kuda));
                    data.knightblack = (data.knightblack & petir_bitboard_mask::bitnot(sto));
                    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_kudahitam][sto]);
                    data.cp = petir_header::_kudaputih;
                    (data.nilai_perwira_hitam = pas::decrement(data.nilai_perwira_hitam, 3));
                } else {
                    if ((captured_piece == petir_header::_gajahhitam)) {
                        (data.materialscore = pas::increment(data.materialscore, petir_header::_nilai_gajah));
                        data.bishopblack = (data.bishopblack & petir_bitboard_mask::bitnot(sto));
                        data.bishopqueen = (data.bishopqueen & petir_bitboard_mask::bitnot(sto));
                        data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_gajahhitam][sto]);
                        data.cp = petir_header::_kudaputih;
                        (data.ngajahhitam = pas::decrement(data.ngajahhitam, 1));
                        (data.nilai_perwira_hitam = pas::decrement(data.nilai_perwira_hitam, 3));
                    } else {
                        if ((captured_piece == petir_header::_bentenghitam)) {
                            (data.materialscore = pas::increment(data.materialscore, petir_header::_nilai_benteng));
                            data.rookblack = (data.rookblack & petir_bitboard_mask::bitnot(sto));
                            data.rookqueen = (data.rookqueen & petir_bitboard_mask::bitnot(sto));
                            data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_bentenghitam][sto]);
                            data.cp = petir_header::_bentengputih;
                            (data.nilai_perwira_hitam = pas::decrement(data.nilai_perwira_hitam, 5));
                        } else {
                            (data.materialscore = pas::increment(data.materialscore, petir_header::_nilai_mentri));
                            data.queenblack = (data.queenblack & petir_bitboard_mask::bitnot(sto));
                            data.rookqueen = (data.rookqueen & petir_bitboard_mask::bitnot(sto));
                            data.bishopqueen = (data.bishopqueen & petir_bitboard_mask::bitnot(sto));
                            data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_mentrihitam][sto]);
                            data.cp = petir_header::_mentriputih;
                            (data.nilai_perwira_hitam = pas::decrement(data.nilai_perwira_hitam, 9));
                        }
                    }
                }
            }
        } else {
            data.cp = 0;
            if ((((data.papan[sfrom] == petir_header::_pionputih) && (pas::sub(sto, sfrom) == 16)) && (((data.papan[pas::sub(sto, 1)] == petir_header::_pionhitam) && ((sto & 7) >= 1)) || ((data.papan[pas::add(sto, 1)] == petir_header::_pionhitam) && ((sto & 7) <= 6))))) {
                data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[data.ep]);
                data.ep = (sfrom & 7);
                data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[data.ep]);
            }
        }
        data.papan[sto] = data.papan[sfrom];
        data.papan[sfrom] = 0;
        data.whitepieces = (data.whitepieces & petir_bitboard_mask::bitnot(sfrom));
        data.whitepieces = (data.whitepieces | petir_bitboard_mask::bit(sto));
        data.allpieces = (data.allpieces & petir_bitboard_mask::bitnot(sfrom));
        data.allpieces = (data.allpieces | petir_bitboard_mask::bit(sto));
        data.allpiecesr90 = (data.allpiecesr90 & (~petir_bitboard_mask::bit2nmask90[sfrom]));
        data.allpiecesr90 = (data.allpiecesr90 | petir_bitboard_mask::bit2nmask90[sto]);
        data.allpiecesa8h1 = (data.allpiecesa8h1 & (~petir_bitboard_mask::bit2nmaska8h1[sfrom]));
        data.allpiecesa8h1 = (data.allpiecesa8h1 | petir_bitboard_mask::bit2nmaska8h1[sto]);
        data.allpiecesh8a1 = (data.allpiecesh8a1 & (~petir_bitboard_mask::bit2nmaskh8a1[sfrom]));
        data.allpiecesh8a1 = (data.allpiecesh8a1 | petir_bitboard_mask::bit2nmaskh8a1[sto]);
        if ((data.papan[sto] == petir_header::_pionputih)) {
            data.move50count = 0;
            data.pawnwhite = (data.pawnwhite | petir_bitboard_mask::bit(sto));
            data.pawnwhite = (data.pawnwhite & petir_bitboard_mask::bitnot(sfrom));
            data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_pionputih][sfrom]);
            data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_pionputih][sto]);
            promosi = (pas::shr(moves, 14) & 7);
            if (((sto >= petir_header::a8) && (promosi == 0))) {
                promosi = petir_header::promosi_mentri;
            }
            if ((promosi != 0)) {
                petir_makemove::promosi_putih(data, sto, promosi);
                data.cp = 10;
            }
        } else {
            if ((data.papan[sto] == petir_header::_kudaputih)) {
                data.knightwhite = (data.knightwhite | petir_bitboard_mask::bit(sto));
                data.knightwhite = (data.knightwhite & petir_bitboard_mask::bitnot(sfrom));
                data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_kudaputih][sfrom]);
                data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_kudaputih][sto]);
            } else {
                if ((data.papan[sto] == petir_header::_gajahputih)) {
                    data.bishopwhite = (data.bishopwhite | petir_bitboard_mask::bit(sto));
                    data.bishopqueen = (data.bishopqueen | petir_bitboard_mask::bit(sto));
                    data.bishopwhite = (data.bishopwhite & petir_bitboard_mask::bitnot(sfrom));
                    data.bishopqueen = (data.bishopqueen & petir_bitboard_mask::bitnot(sfrom));
                    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_gajahputih][sfrom]);
                    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_gajahputih][sto]);
                } else {
                    if ((data.papan[sto] == petir_header::_bentengputih)) {
                        data.rookwhite = (data.rookwhite | petir_bitboard_mask::bit(sto));
                        data.rookqueen = (data.rookqueen | petir_bitboard_mask::bit(sto));
                        data.rookwhite = (data.rookwhite & petir_bitboard_mask::bitnot(sfrom));
                        data.rookqueen = (data.rookqueen & petir_bitboard_mask::bitnot(sfrom));
                        data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_bentengputih][sfrom]);
                        data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_bentengputih][sto]);
                        if (((data.flagrokade & petir_header::bitputihnoc) != 0)) {
                            if ((sfrom == petir_header::h1)) {
                                data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
                                data.flagrokade = (data.flagrokade & (~petir_header::bitputihsc));
                                data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
                            } else {
                                if ((sfrom == petir_header::a1)) {
                                    data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
                                    data.flagrokade = (data.flagrokade & (~petir_header::bitputihlc));
                                    data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
                                }
                            }
                        }
                    } else {
                        if ((data.papan[sto] == petir_header::_mentriputih)) {
                            data.queenwhite = (data.queenwhite | petir_bitboard_mask::bit(sto));
                            data.bishopqueen = (data.bishopqueen | petir_bitboard_mask::bit(sto));
                            data.rookqueen = (data.rookqueen | petir_bitboard_mask::bit(sto));
                            data.queenwhite = (data.queenwhite & petir_bitboard_mask::bitnot(sfrom));
                            data.rookqueen = (data.rookqueen & petir_bitboard_mask::bitnot(sfrom));
                            data.bishopqueen = (data.bishopqueen & petir_bitboard_mask::bitnot(sfrom));
                            data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_mentriputih][sfrom]);
                            data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_mentriputih][sto]);
                        } else {
                            if ((data.papan[sto] == petir_header::_rajaputih)) {
                                data.kingwhite = (data.kingwhite | petir_bitboard_mask::bit(sto));
                                data.kingwhite = (data.kingwhite & petir_bitboard_mask::bitnot(sfrom));
                                data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_rajaputih][sfrom]);
                                data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_rajaputih][sto]);
                                if (((data.flagrokade & petir_header::bitputihnoc) != 0)) {
                                    data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
                                    data.flagrokade = (data.flagrokade & (~petir_header::bitputihnoc));
                                    data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// makemove.pas:627
void makeblackmove(std::int32_t moves, petir_header::tdata& data) {
    std::int32_t captured_piece{};
    std::int32_t piece{};
    std::int32_t sfrom{};
    std::int32_t sto{};
    std::int32_t promosi{};
    std::int32_t pas_temp_1{};
    (petir_header::total_node = pas::increment(petir_header::total_node, 1));
    sfrom = (moves & 127);
    sto = (pas::shr(moves, 7) & 127);
    (data.move50count = pas::increment(data.move50count, 1));
    data.cp = 0;
    pas_temp_1 = sto;
    if (((pas_temp_1 == petir_header::_rokadependek))) {
        petir_makemove::blackrokadependek(data);
    }
    else if (((pas_temp_1 == petir_header::_rokadepanjang))) {
        petir_makemove::blackrokadepanjang(data);
    }
    else if (((pas_temp_1 == petir_header::_en_passant))) {
        petir_makemove::black_ep(data, sfrom, sto);
        data.cp = petir_header::_pionputih;
    }
    else {
        piece = data.papan[sfrom];
        if ((data.ep != petir_header::_no_ep)) {
            data.hashkey = ((data.hashkey ^ petir_header::enpassanthashkey[data.ep]) ^ petir_header::enpassanthashkey[petir_header::_no_ep]);
            data.ep = petir_header::_no_ep;
        }
        captured_piece = data.papan[sto];
        data.hashkey = (data.hashkey ^ petir_header::hashkey[piece][sfrom]);
        data.hashkey = (data.hashkey ^ petir_header::hashkey[piece][sto]);
        if ((captured_piece != 0)) {
            data.move50count = 0;
            data.hashkey = (data.hashkey ^ petir_header::hashkey[captured_piece][sto]);
            data.whitepieces = (data.whitepieces & petir_bitboard_mask::bitnot(sto));
            if ((captured_piece == petir_header::_pionputih)) {
                data.pawnwhite = (data.pawnwhite & petir_bitboard_mask::bitnot(sto));
                (data.materialscore = pas::decrement(data.materialscore, petir_header::_nilai_pion));
                data.cp = petir_header::_pionputih;
            } else {
                if ((captured_piece == petir_header::_kudaputih)) {
                    (data.materialscore = pas::decrement(data.materialscore, petir_header::_nilai_kuda));
                    data.knightwhite = (data.knightwhite & petir_bitboard_mask::bitnot(sto));
                    (data.nilai_perwira_putih = pas::decrement(data.nilai_perwira_putih, 3));
                    data.cp = petir_header::_kudaputih;
                } else {
                    if ((captured_piece == petir_header::_gajahputih)) {
                        (data.materialscore = pas::decrement(data.materialscore, petir_header::_nilai_gajah));
                        data.bishopwhite = (data.bishopwhite & petir_bitboard_mask::bitnot(sto));
                        data.bishopqueen = (data.bishopqueen & petir_bitboard_mask::bitnot(sto));
                        (data.nilai_perwira_putih = pas::decrement(data.nilai_perwira_putih, 3));
                        data.cp = petir_header::_kudaputih;
                        (data.ngajahputih = pas::decrement(data.ngajahputih, 1));
                    } else {
                        if ((captured_piece == petir_header::_bentengputih)) {
                            (data.materialscore = pas::decrement(data.materialscore, petir_header::_nilai_benteng));
                            data.rookwhite = (data.rookwhite & petir_bitboard_mask::bitnot(sto));
                            data.rookqueen = (data.rookqueen & petir_bitboard_mask::bitnot(sto));
                            (data.nilai_perwira_putih = pas::decrement(data.nilai_perwira_putih, 5));
                            data.cp = petir_header::_bentengputih;
                        } else {
                            (data.materialscore = pas::decrement(data.materialscore, petir_header::_nilai_mentri));
                            data.queenwhite = (data.queenwhite & petir_bitboard_mask::bitnot(sto));
                            data.rookqueen = (data.rookqueen & petir_bitboard_mask::bitnot(sto));
                            data.bishopqueen = (data.bishopqueen & petir_bitboard_mask::bitnot(sto));
                            (data.nilai_perwira_putih = pas::decrement(data.nilai_perwira_putih, 9));
                            data.cp = petir_header::_mentriputih;
                        }
                    }
                }
            }
        } else {
            if ((((data.papan[sfrom] == petir_header::_pionhitam) && (pas::sub(sfrom, sto) == 16)) && (((data.papan[pas::sub(sto, 1)] == petir_header::_pionputih) && ((sto & 7) >= 1)) || ((data.papan[pas::add(sto, 1)] == petir_header::_pionputih) && ((sto & 7) <= 6))))) {
                data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[data.ep]);
                data.ep = (sfrom & 7);
                data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[data.ep]);
            }
        }
        data.papan[sto] = piece;
        data.papan[sfrom] = 0;
        data.blackpieces = (data.blackpieces & petir_bitboard_mask::bitnot(sfrom));
        data.blackpieces = (data.blackpieces | petir_bitboard_mask::bit(sto));
        data.allpieces = (data.allpieces & petir_bitboard_mask::bitnot(sfrom));
        data.allpieces = (data.allpieces | petir_bitboard_mask::bit(sto));
        data.allpiecesr90 = (data.allpiecesr90 & (~petir_bitboard_mask::bit2nmask90[sfrom]));
        data.allpiecesr90 = (data.allpiecesr90 | petir_bitboard_mask::bit2nmask90[sto]);
        data.allpiecesa8h1 = (data.allpiecesa8h1 & (~petir_bitboard_mask::bit2nmaska8h1[sfrom]));
        data.allpiecesa8h1 = (data.allpiecesa8h1 | petir_bitboard_mask::bit2nmaska8h1[sto]);
        data.allpiecesh8a1 = (data.allpiecesh8a1 & (~petir_bitboard_mask::bit2nmaskh8a1[sfrom]));
        data.allpiecesh8a1 = (data.allpiecesh8a1 | petir_bitboard_mask::bit2nmaskh8a1[sto]);
        if ((piece == petir_header::_pionhitam)) {
            data.move50count = 0;
            data.pawnblack = (data.pawnblack | petir_bitboard_mask::bit(sto));
            data.pawnblack = (data.pawnblack & petir_bitboard_mask::bitnot(sfrom));
            promosi = (pas::shr(moves, 14) & 7);
            if (((sto <= petir_header::h1) && (promosi == 0))) {
                promosi = petir_header::promosi_mentri;
            }
            if ((promosi != 0)) {
                petir_makemove::promosi_hitam(data, sto, promosi);
                data.cp = 11;
            }
        } else {
            if ((piece == petir_header::_kudahitam)) {
                data.knightblack = (data.knightblack | petir_bitboard_mask::bit(sto));
                data.knightblack = (data.knightblack & petir_bitboard_mask::bitnot(sfrom));
            } else {
                if ((piece == petir_header::_gajahhitam)) {
                    data.bishopblack = (data.bishopblack | petir_bitboard_mask::bit(sto));
                    data.bishopqueen = (data.bishopqueen | petir_bitboard_mask::bit(sto));
                    data.bishopblack = (data.bishopblack & petir_bitboard_mask::bitnot(sfrom));
                    data.bishopqueen = (data.bishopqueen & petir_bitboard_mask::bitnot(sfrom));
                } else {
                    if ((piece == petir_header::_bentenghitam)) {
                        data.rookblack = (data.rookblack | petir_bitboard_mask::bit(sto));
                        data.rookqueen = (data.rookqueen | petir_bitboard_mask::bit(sto));
                        data.rookblack = (data.rookblack & petir_bitboard_mask::bitnot(sfrom));
                        data.rookqueen = (data.rookqueen & petir_bitboard_mask::bitnot(sfrom));
                        if (((data.flagrokade & petir_header::bithitamnoc) != 0)) {
                            if ((sfrom == petir_header::h8)) {
                                data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
                                data.flagrokade = (data.flagrokade & (~petir_header::bithitamsc));
                                data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
                            } else {
                                if ((sfrom == petir_header::a8)) {
                                    data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
                                    data.flagrokade = (data.flagrokade & (~petir_header::bithitamlc));
                                    data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
                                }
                            }
                        }
                    } else {
                        if ((piece == petir_header::_mentrihitam)) {
                            data.queenblack = (data.queenblack | petir_bitboard_mask::bit(sto));
                            data.rookqueen = (data.rookqueen | petir_bitboard_mask::bit(sto));
                            data.bishopqueen = (data.bishopqueen | petir_bitboard_mask::bit(sto));
                            data.queenblack = (data.queenblack & petir_bitboard_mask::bitnot(sfrom));
                            data.rookqueen = (data.rookqueen & petir_bitboard_mask::bitnot(sfrom));
                            data.bishopqueen = (data.bishopqueen & petir_bitboard_mask::bitnot(sfrom));
                        } else {
                            if ((piece == petir_header::_rajahitam)) {
                                data.kingblack = (data.kingblack | petir_bitboard_mask::bit(sto));
                                data.kingblack = (data.kingblack & petir_bitboard_mask::bitnot(sfrom));
                                if (((data.flagrokade & petir_header::bithitamnoc) != 0)) {
                                    data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
                                    data.flagrokade = (data.flagrokade & (~petir_header::bithitamnoc));
                                    data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// makemove.pas:817
void cekerror(petir_header::tdata data) {
    std::uint64_t piecepos{};
    std::int32_t a{};
    std::int32_t pos{};
    std::int32_t pas_temp_1{};
    pas_temp_1 = 63;
    for (a = 0; a <= pas_temp_1; ++a) {
        if (((data.papan[a] == petir_header::_pionputih) && ((petir_bitboard_mask::bit(a) & data.pawnwhite) == 0))) {
            pas::form1.caption = pas::String("error 1");
        }
        if (((data.papan[a] == petir_header::_pionhitam) && ((petir_bitboard_mask::bit(a) & data.pawnblack) == 0))) {
            pas::form1.caption = pas::String("error 2");
        }
        if (((data.papan[a] == petir_header::_kudaputih) && ((petir_bitboard_mask::bit(a) & data.knightwhite) == 0))) {
            pas::form1.caption = pas::String("error 30");
        }
        if (((data.papan[a] == petir_header::_gajahputih) && ((petir_bitboard_mask::bit(a) & data.bishopwhite) == 0))) {
            pas::form1.caption = pas::String("error 4");
        }
        if (((data.papan[a] == petir_header::_bentengputih) && ((petir_bitboard_mask::bit(a) & data.rookwhite) == 0))) {
            pas::form1.caption = pas::String("error 5");
        }
        if (((data.papan[a] == petir_header::_mentriputih) && ((petir_bitboard_mask::bit(a) & data.queenwhite) == 0))) {
            pas::form1.caption = pas::String("error 6");
        }
        if (((data.papan[a] == petir_header::_rajaputih) && ((petir_bitboard_mask::bit(a) & data.kingwhite) == 0))) {
            pas::form1.caption = pas::String("error 7");
        }
        if (((data.papan[a] == petir_header::_rajahitam) && ((petir_bitboard_mask::bit(a) & data.kingblack) == 0))) {
            pas::form1.caption = pas::String("error 8");
        }
        if (((data.papan[a] == petir_header::_bentenghitam) && ((petir_bitboard_mask::bit(a) & data.rookblack) == 0))) {
            pas::form1.caption = pas::String("error 8");
        }
        if (((data.papan[a] == petir_header::_mentrihitam) && ((petir_bitboard_mask::bit(a) & data.queenblack) == 0))) {
            pas::form1.caption = pas::String("error 8");
        }
        if (((data.papan[a] == petir_header::_gajahhitam) && ((petir_bitboard_mask::bit(a) & data.bishopblack) == 0))) {
            pas::form1.caption = pas::String("error 8");
        }
        if (((data.papan[a] == petir_header::_kudahitam) && ((petir_bitboard_mask::bit(a) & data.knightblack) == 0))) {
            pas::form1.caption = pas::String("error 8");
        }
        if (((data.papan[a] != 0) && ((petir_bitboard_mask::bit2nmaska8h1[a] & data.allpiecesa8h1) == 0))) {
            pas::form1.caption = pas::String("error 5");
            break;
        }
        if (((data.papan[a] != 0) && ((petir_bitboard_mask::bit2nmask90[a] & data.allpiecesr90) == 0))) {
            pas::form1.caption = pas::String("error 5");
            break;
        }
        if (((data.papan[a] != 0) && ((petir_bitboard_mask::bit2nmaskh8a1[a] & data.allpiecesh8a1) == 0))) {
            pas::form1.caption = pas::String("error 5");
            break;
        }
    }
    if (((data.whitepieces | data.blackpieces) != data.allpieces)) {
        pas::form1.caption = pas::String("error 1");
    }
    if (((((((data.pawnwhite | data.knightwhite) | data.bishopwhite) | data.rookwhite) | data.queenwhite) | data.kingwhite) != data.whitepieces)) {
        pas::form1.caption = pas::String("error 2");
    }
    if (((((((data.pawnblack | data.knightblack) | data.bishopblack) | data.rookblack) | data.queenblack) | data.kingblack) != data.blackpieces)) {
        pas::form1.caption = pas::String("error 3000");
    }
    piecepos = data.allpieces;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bitnot(pos));
        if ((data.papan[pos] == 0)) {
            pas::form1.caption = pas::String("error4");
        }
        if (((data.allpiecesr90 & petir_bitboard_mask::bit2nmask90[pos]) == 0)) {
            pas::form1.caption = pas::String("error4");
        }
        if (((data.allpiecesa8h1 & petir_bitboard_mask::bit2nmaska8h1[pos]) == 0)) {
            pas::form1.caption = pas::String("error4");
        }
        if (((data.allpiecesh8a1 & petir_bitboard_mask::bit2nmaskh8a1[pos]) == 0)) {
            pas::form1.caption = pas::String("error4");
        }
    }
    piecepos = data.whitepieces;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bitnot(pos));
        if ((data.papan[pos] <= 0)) {
            pas::form1.caption = pas::String("error4");
        }
        if (((data.allpiecesr90 & petir_bitboard_mask::bit2nmask90[pos]) == 0)) {
            pas::form1.caption = pas::String("error4");
        }
        if (((data.allpiecesa8h1 & petir_bitboard_mask::bit2nmaska8h1[pos]) == 0)) {
            pas::form1.caption = pas::String("error4");
        }
        if (((data.allpiecesh8a1 & petir_bitboard_mask::bit2nmaskh8a1[pos]) == 0)) {
            pas::form1.caption = pas::String("error4");
        }
    }
    piecepos = data.blackpieces;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bitnot(pos));
        if ((data.papan[pos] >= 0)) {
            pas::form1.caption = pas::String("error5");
        }
        if (((data.allpiecesr90 & petir_bitboard_mask::bit2nmask90[pos]) == 0)) {
            pas::form1.caption = pas::String("error4");
        }
        if (((data.allpiecesa8h1 & petir_bitboard_mask::bit2nmaska8h1[pos]) == 0)) {
            pas::form1.caption = pas::String("error4");
        }
        if (((data.allpiecesh8a1 & petir_bitboard_mask::bit2nmaskh8a1[pos]) == 0)) {
            pas::form1.caption = pas::String("error4");
        }
    }
    piecepos = data.pawnblack;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bitnot(pos));
        if ((data.papan[pos] != petir_header::_pionhitam)) {
            pas::form1.caption = pas::String("error6");
        }
    }
    piecepos = data.knightblack;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bitnot(pos));
        if ((data.papan[pos] != petir_header::_kudahitam)) {
            pas::form1.caption = pas::String("error7");
        }
    }
    piecepos = data.bishopblack;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bitnot(pos));
        if ((data.papan[pos] != petir_header::_gajahhitam)) {
            pas::form1.caption = pas::String("error8");
        }
    }
    piecepos = data.rookblack;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bitnot(pos));
        if ((data.papan[pos] != petir_header::_bentenghitam)) {
            pas::form1.caption = pas::String("error9");
        }
    }
    piecepos = data.queenblack;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bitnot(pos));
        if ((data.papan[pos] != petir_header::_mentrihitam)) {
            pas::form1.caption = pas::String("error100");
            return;
        }
    }
    piecepos = data.kingblack;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bitnot(pos));
        if ((data.papan[pos] != petir_header::_rajahitam)) {
            pas::form1.caption = pas::String("error 11");
        }
    }
    piecepos = data.pawnwhite;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bitnot(pos));
        if ((data.papan[pos] != petir_header::_pionputih)) {
            pas::form1.caption = pas::String("error 12");
        }
    }
    piecepos = data.knightwhite;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bitnot(pos));
        if ((data.papan[pos] != petir_header::_kudaputih)) {
            pas::form1.caption = pas::String("error 13");
        }
    }
    piecepos = data.bishopwhite;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bitnot(pos));
        if ((data.papan[pos] != petir_header::_gajahputih)) {
            pas::form1.caption = pas::String("error 14");
        }
    }
    piecepos = data.rookwhite;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bitnot(pos));
        if ((data.papan[pos] != petir_header::_bentengputih)) {
            pas::form1.caption = pas::String("error 15");
        }
    }
    piecepos = data.queenwhite;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bitnot(pos));
        if ((data.papan[pos] != petir_header::_mentriputih)) {
            pas::form1.caption = pas::String("error 16");
        }
    }
    piecepos = data.kingwhite;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bitnot(pos));
        if ((data.papan[pos] != petir_header::_rajaputih)) {
            pas::form1.caption = pas::String("error 17");
        }
    }
}

// makemove.pas:14
void whiterokadependek(petir_header::tdata& data) {
    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_bentengputih][petir_header::h1]);
    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_rajaputih][petir_header::e1]);
    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_bentengputih][petir_header::f1]);
    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_rajaputih][petir_header::g1]);
    data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
    data.wc = petir_header::rokadependek;
    data.flagrokade = (data.flagrokade & (~petir_header::bitputihnoc));
    data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
    data.papan[petir_header::e1] = 0;
    data.papan[petir_header::h1] = 0;
    data.papan[petir_header::f1] = petir_header::_bentengputih;
    data.papan[petir_header::g1] = petir_header::_rajaputih;
    data.kingwhite = (data.kingwhite & petir_bitboard_mask::bitnot(petir_header::e1));
    data.rookwhite = (data.rookwhite & petir_bitboard_mask::bitnot(petir_header::h1));
    data.rookqueen = (data.rookqueen & petir_bitboard_mask::bitnot(petir_header::h1));
    data.rookwhite = (data.rookwhite | petir_bitboard_mask::bit(petir_header::f1));
    data.rookqueen = (data.rookqueen | petir_bitboard_mask::bit(petir_header::f1));
    data.kingwhite = (data.kingwhite | petir_bitboard_mask::bit(petir_header::g1));
    data.whitepieces = (data.whitepieces & petir_bitboard_mask::bitnot(petir_header::e1));
    data.whitepieces = (data.whitepieces & petir_bitboard_mask::bitnot(petir_header::h1));
    data.whitepieces = (data.whitepieces | petir_bitboard_mask::bit(petir_header::f1));
    data.whitepieces = (data.whitepieces | petir_bitboard_mask::bit(petir_header::g1));
    data.allpiecesr90 = (data.allpiecesr90 & (~petir_bitboard_mask::bit2nmask90[petir_header::e1]));
    data.allpiecesr90 = (data.allpiecesr90 & (~petir_bitboard_mask::bit2nmask90[petir_header::h1]));
    data.allpiecesr90 = (data.allpiecesr90 | petir_bitboard_mask::bit2nmask90[petir_header::f1]);
    data.allpiecesr90 = (data.allpiecesr90 | petir_bitboard_mask::bit2nmask90[petir_header::g1]);
    data.allpiecesa8h1 = (data.allpiecesa8h1 & (~petir_bitboard_mask::bit2nmaska8h1[petir_header::e1]));
    data.allpiecesa8h1 = (data.allpiecesa8h1 & (~petir_bitboard_mask::bit2nmaska8h1[petir_header::h1]));
    data.allpiecesa8h1 = (data.allpiecesa8h1 | petir_bitboard_mask::bit2nmaska8h1[petir_header::f1]);
    data.allpiecesa8h1 = (data.allpiecesa8h1 | petir_bitboard_mask::bit2nmaska8h1[petir_header::g1]);
    data.allpiecesh8a1 = (data.allpiecesh8a1 & (~petir_bitboard_mask::bit2nmaskh8a1[petir_header::e1]));
    data.allpiecesh8a1 = (data.allpiecesh8a1 & (~petir_bitboard_mask::bit2nmaskh8a1[petir_header::h1]));
    data.allpiecesh8a1 = (data.allpiecesh8a1 | petir_bitboard_mask::bit2nmaskh8a1[petir_header::f1]);
    data.allpiecesh8a1 = (data.allpiecesh8a1 | petir_bitboard_mask::bit2nmaskh8a1[petir_header::g1]);
    data.allpieces = (data.blackpieces | data.whitepieces);
    data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[data.ep]);
    data.ep = petir_header::_no_ep;
    data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[petir_header::_no_ep]);
}

// makemove.pas:68
void whiterokadepanjang(petir_header::tdata& data) {
    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_bentengputih][petir_header::a1]);
    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_rajaputih][petir_header::e1]);
    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_bentengputih][petir_header::d1]);
    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_rajaputih][petir_header::c1]);
    data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
    data.flagrokade = (data.flagrokade & (~petir_header::bitputihnoc));
    data.wc = petir_header::rokadepanjang;
    data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
    data.papan[petir_header::e1] = 0;
    data.papan[petir_header::a1] = 0;
    data.papan[petir_header::d1] = petir_header::_bentengputih;
    data.papan[petir_header::c1] = petir_header::_rajaputih;
    data.kingwhite = (data.kingwhite & petir_bitboard_mask::bitnot(petir_header::e1));
    data.rookwhite = (data.rookwhite & petir_bitboard_mask::bitnot(petir_header::a1));
    data.rookqueen = (data.rookqueen & petir_bitboard_mask::bitnot(petir_header::a1));
    data.rookwhite = (data.rookwhite | petir_bitboard_mask::bit(petir_header::d1));
    data.rookqueen = (data.rookqueen | petir_bitboard_mask::bit(petir_header::d1));
    data.kingwhite = (data.kingwhite | petir_bitboard_mask::bit(petir_header::c1));
    data.whitepieces = (data.whitepieces & petir_bitboard_mask::bitnot(petir_header::e1));
    data.whitepieces = (data.whitepieces & petir_bitboard_mask::bitnot(petir_header::a1));
    data.whitepieces = (data.whitepieces | petir_bitboard_mask::bit(petir_header::d1));
    data.whitepieces = (data.whitepieces | petir_bitboard_mask::bit(petir_header::c1));
    data.allpiecesr90 = (data.allpiecesr90 & (~petir_bitboard_mask::bit2nmask90[petir_header::e1]));
    data.allpiecesr90 = (data.allpiecesr90 & (~petir_bitboard_mask::bit2nmask90[petir_header::a1]));
    data.allpiecesr90 = (data.allpiecesr90 | petir_bitboard_mask::bit2nmask90[petir_header::d1]);
    data.allpiecesr90 = (data.allpiecesr90 | petir_bitboard_mask::bit2nmask90[petir_header::c1]);
    data.allpiecesa8h1 = (data.allpiecesa8h1 & (~petir_bitboard_mask::bit2nmaska8h1[petir_header::e1]));
    data.allpiecesa8h1 = (data.allpiecesa8h1 & (~petir_bitboard_mask::bit2nmaska8h1[petir_header::a1]));
    data.allpiecesa8h1 = (data.allpiecesa8h1 | petir_bitboard_mask::bit2nmaska8h1[petir_header::d1]);
    data.allpiecesa8h1 = (data.allpiecesa8h1 | petir_bitboard_mask::bit2nmaska8h1[petir_header::c1]);
    data.allpiecesh8a1 = (data.allpiecesh8a1 & (~petir_bitboard_mask::bit2nmaskh8a1[petir_header::e1]));
    data.allpiecesh8a1 = (data.allpiecesh8a1 & (~petir_bitboard_mask::bit2nmaskh8a1[petir_header::a1]));
    data.allpiecesh8a1 = (data.allpiecesh8a1 | petir_bitboard_mask::bit2nmaskh8a1[petir_header::d1]);
    data.allpiecesh8a1 = (data.allpiecesh8a1 | petir_bitboard_mask::bit2nmaskh8a1[petir_header::c1]);
    data.allpieces = (data.blackpieces | data.whitepieces);
    data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[data.ep]);
    data.ep = petir_header::_no_ep;
    data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[data.ep]);
}

// makemove.pas:125
void blackrokadependek(petir_header::tdata& data) {
    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_bentenghitam][petir_header::h8]);
    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_rajahitam][petir_header::e8]);
    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_bentenghitam][petir_header::f8]);
    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_rajahitam][petir_header::g8]);
    data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
    data.flagrokade = (data.flagrokade & (~petir_header::bithitamnoc));
    data.bc = petir_header::rokadependek;
    data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
    data.papan[petir_header::e8] = 0;
    data.papan[petir_header::h8] = 0;
    data.papan[petir_header::f8] = petir_header::_bentenghitam;
    data.papan[petir_header::g8] = petir_header::_rajahitam;
    data.blackpieces = (data.blackpieces & petir_bitboard_mask::bitnot(petir_header::e8));
    data.blackpieces = (data.blackpieces & petir_bitboard_mask::bitnot(petir_header::h8));
    data.blackpieces = (data.blackpieces | petir_bitboard_mask::bit(petir_header::f8));
    data.blackpieces = (data.blackpieces | petir_bitboard_mask::bit(petir_header::g8));
    data.kingblack = (data.kingblack & petir_bitboard_mask::bitnot(petir_header::e8));
    data.rookblack = (data.rookblack & petir_bitboard_mask::bitnot(petir_header::h8));
    data.rookqueen = (data.rookqueen & petir_bitboard_mask::bitnot(petir_header::h8));
    data.rookblack = (data.rookblack | petir_bitboard_mask::bit(petir_header::f8));
    data.rookqueen = (data.rookqueen | petir_bitboard_mask::bit(petir_header::f8));
    data.kingblack = (data.kingblack | petir_bitboard_mask::bit(petir_header::g8));
    data.allpiecesr90 = (data.allpiecesr90 & (~petir_bitboard_mask::bit2nmask90[petir_header::e8]));
    data.allpiecesr90 = (data.allpiecesr90 & (~petir_bitboard_mask::bit2nmask90[petir_header::h8]));
    data.allpiecesr90 = (data.allpiecesr90 | petir_bitboard_mask::bit2nmask90[petir_header::f8]);
    data.allpiecesr90 = (data.allpiecesr90 | petir_bitboard_mask::bit2nmask90[petir_header::g8]);
    data.allpiecesa8h1 = (data.allpiecesa8h1 & (~petir_bitboard_mask::bit2nmaska8h1[petir_header::e8]));
    data.allpiecesa8h1 = (data.allpiecesa8h1 & (~petir_bitboard_mask::bit2nmaska8h1[petir_header::h8]));
    data.allpiecesa8h1 = (data.allpiecesa8h1 | petir_bitboard_mask::bit2nmaska8h1[petir_header::f8]);
    data.allpiecesa8h1 = (data.allpiecesa8h1 | petir_bitboard_mask::bit2nmaska8h1[petir_header::g8]);
    data.allpiecesh8a1 = (data.allpiecesh8a1 & (~petir_bitboard_mask::bit2nmaskh8a1[petir_header::e8]));
    data.allpiecesh8a1 = (data.allpiecesh8a1 & (~petir_bitboard_mask::bit2nmaskh8a1[petir_header::h8]));
    data.allpiecesh8a1 = (data.allpiecesh8a1 | petir_bitboard_mask::bit2nmaskh8a1[petir_header::f8]);
    data.allpiecesh8a1 = (data.allpiecesh8a1 | petir_bitboard_mask::bit2nmaskh8a1[petir_header::g8]);
    data.allpieces = (data.blackpieces | data.whitepieces);
    data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[data.ep]);
    data.ep = petir_header::_no_ep;
    data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[data.ep]);
}

// makemove.pas:177
void blackrokadepanjang(petir_header::tdata& data) {
    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_bentenghitam][petir_header::a8]);
    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_rajahitam][petir_header::e8]);
    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_bentenghitam][petir_header::d8]);
    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_rajahitam][petir_header::c8]);
    data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
    data.flagrokade = (data.flagrokade & (~petir_header::bithitamnoc));
    data.bc = petir_header::rokadepanjang;
    data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
    data.papan[petir_header::e8] = 0;
    data.papan[petir_header::a8] = 0;
    data.papan[petir_header::d8] = petir_header::_bentenghitam;
    data.papan[petir_header::c8] = petir_header::_rajahitam;
    data.blackpieces = (data.blackpieces & petir_bitboard_mask::bitnot(petir_header::e8));
    data.blackpieces = (data.blackpieces & petir_bitboard_mask::bitnot(petir_header::a8));
    data.blackpieces = (data.blackpieces | petir_bitboard_mask::bit(petir_header::d8));
    data.blackpieces = (data.blackpieces | petir_bitboard_mask::bit(petir_header::c8));
    data.kingblack = (data.kingblack & petir_bitboard_mask::bitnot(petir_header::e8));
    data.rookblack = (data.rookblack & petir_bitboard_mask::bitnot(petir_header::a8));
    data.rookqueen = (data.rookqueen & petir_bitboard_mask::bitnot(petir_header::a8));
    data.rookblack = (data.rookblack | petir_bitboard_mask::bit(petir_header::d8));
    data.rookqueen = (data.rookqueen | petir_bitboard_mask::bit(petir_header::d8));
    data.kingblack = (data.kingblack | petir_bitboard_mask::bit(petir_header::c8));
    data.allpiecesr90 = (data.allpiecesr90 & (~petir_bitboard_mask::bit2nmask90[petir_header::e8]));
    data.allpiecesr90 = (data.allpiecesr90 & (~petir_bitboard_mask::bit2nmask90[petir_header::a8]));
    data.allpiecesr90 = (data.allpiecesr90 | petir_bitboard_mask::bit2nmask90[petir_header::d8]);
    data.allpiecesr90 = (data.allpiecesr90 | petir_bitboard_mask::bit2nmask90[petir_header::c8]);
    data.allpiecesa8h1 = (data.allpiecesa8h1 & (~petir_bitboard_mask::bit2nmaska8h1[petir_header::e8]));
    data.allpiecesa8h1 = (data.allpiecesa8h1 & (~petir_bitboard_mask::bit2nmaska8h1[petir_header::a8]));
    data.allpiecesa8h1 = (data.allpiecesa8h1 | petir_bitboard_mask::bit2nmaska8h1[petir_header::d8]);
    data.allpiecesa8h1 = (data.allpiecesa8h1 | petir_bitboard_mask::bit2nmaska8h1[petir_header::c8]);
    data.allpiecesh8a1 = (data.allpiecesh8a1 & (~petir_bitboard_mask::bit2nmaskh8a1[petir_header::e8]));
    data.allpiecesh8a1 = (data.allpiecesh8a1 & (~petir_bitboard_mask::bit2nmaskh8a1[petir_header::a8]));
    data.allpiecesh8a1 = (data.allpiecesh8a1 | petir_bitboard_mask::bit2nmaskh8a1[petir_header::d8]);
    data.allpiecesh8a1 = (data.allpiecesh8a1 | petir_bitboard_mask::bit2nmaskh8a1[petir_header::c8]);
    data.allpieces = (data.blackpieces | data.whitepieces);
    data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[data.ep]);
    data.ep = petir_header::_no_ep;
    data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[data.ep]);
}

// makemove.pas:230
void black_ep(petir_header::tdata& data, std::int32_t sfrom, std::int32_t sto) {
    data.move50count = 0;
    (data.materialscore = pas::decrement(data.materialscore, petir_header::_nilai_pion));
    if ((data.ep > (sfrom & 7))) {
        sto = pas::sub(sfrom, 7);
    } else {
        sto = pas::sub(sfrom, 9);
    }
    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_pionhitam][sfrom]);
    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_pionhitam][sto]);
    data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[data.ep]);
    data.ep = petir_header::_no_ep;
    data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[petir_header::_no_ep]);
    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_pionputih][pas::add(sto, 8)]);
    data.papan[sto] = petir_header::_pionhitam;
    data.papan[sfrom] = 0;
    data.papan[pas::add(sto, 8)] = 0;
    data.whitepieces = (data.whitepieces & petir_bitboard_mask::bitnot(pas::add(sto, 8)));
    data.blackpieces = (data.blackpieces & petir_bitboard_mask::bitnot(sfrom));
    data.blackpieces = (data.blackpieces | petir_bitboard_mask::bit(sto));
    data.allpieces = (data.allpieces & petir_bitboard_mask::bitnot(pas::add(sto, 8)));
    data.allpieces = (data.allpieces & petir_bitboard_mask::bitnot(sfrom));
    data.allpieces = (data.allpieces | petir_bitboard_mask::bit(sto));
    data.allpiecesr90 = (data.allpiecesr90 & (~petir_bitboard_mask::bit2nmask90[pas::add(sto, 8)]));
    data.allpiecesr90 = (data.allpiecesr90 & (~petir_bitboard_mask::bit2nmask90[sfrom]));
    data.allpiecesr90 = (data.allpiecesr90 | petir_bitboard_mask::bit2nmask90[sto]);
    data.allpiecesa8h1 = (data.allpiecesa8h1 & (~petir_bitboard_mask::bit2nmaska8h1[pas::add(sto, 8)]));
    data.allpiecesa8h1 = (data.allpiecesa8h1 & (~petir_bitboard_mask::bit2nmaska8h1[sfrom]));
    data.allpiecesa8h1 = (data.allpiecesa8h1 | petir_bitboard_mask::bit2nmaska8h1[sto]);
    data.allpiecesh8a1 = (data.allpiecesh8a1 & (~petir_bitboard_mask::bit2nmaskh8a1[pas::add(sto, 8)]));
    data.allpiecesh8a1 = (data.allpiecesh8a1 & (~petir_bitboard_mask::bit2nmaskh8a1[sfrom]));
    data.allpiecesh8a1 = (data.allpiecesh8a1 | petir_bitboard_mask::bit2nmaskh8a1[sto]);
    data.pawnblack = (data.pawnblack & petir_bitboard_mask::bitnot(sfrom));
    data.pawnwhite = (data.pawnwhite & petir_bitboard_mask::bitnot(pas::add(sto, 8)));
    data.pawnblack = (data.pawnblack | petir_bitboard_mask::bit(sto));
}

// makemove.pas:279
void white_ep(petir_header::tdata& data, std::int32_t sfrom, std::int32_t sto) {
    data.move50count = 0;
    (data.materialscore = pas::increment(data.materialscore, petir_header::_nilai_pion));
    if ((data.ep > (sfrom & 7))) {
        sto = pas::add(sfrom, 9);
    } else {
        sto = pas::add(sfrom, 7);
    }
    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_pionputih][sfrom]);
    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_pionputih][sto]);
    data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[data.ep]);
    data.ep = petir_header::_no_ep;
    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_pionhitam][pas::sub(sto, 8)]);
    data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[petir_header::_no_ep]);
    data.papan[sto] = petir_header::_pionputih;
    data.papan[sfrom] = 0;
    data.papan[pas::sub(sto, 8)] = 0;
    data.blackpieces = (data.blackpieces & petir_bitboard_mask::bitnot(pas::sub(sto, 8)));
    data.whitepieces = (data.whitepieces & petir_bitboard_mask::bitnot(sfrom));
    data.whitepieces = (data.whitepieces | petir_bitboard_mask::bit(sto));
    data.allpieces = (data.allpieces & petir_bitboard_mask::bitnot(pas::sub(sto, 8)));
    data.allpieces = (data.allpieces & petir_bitboard_mask::bitnot(sfrom));
    data.allpieces = (data.allpieces | petir_bitboard_mask::bit(sto));
    data.allpiecesr90 = (data.allpiecesr90 & (~petir_bitboard_mask::bit2nmask90[pas::sub(sto, 8)]));
    data.allpiecesr90 = (data.allpiecesr90 & (~petir_bitboard_mask::bit2nmask90[sfrom]));
    data.allpiecesr90 = (data.allpiecesr90 | petir_bitboard_mask::bit2nmask90[sto]);
    data.allpiecesa8h1 = (data.allpiecesa8h1 & (~petir_bitboard_mask::bit2nmaska8h1[pas::sub(sto, 8)]));
    data.allpiecesa8h1 = (data.allpiecesa8h1 & (~petir_bitboard_mask::bit2nmaska8h1[sfrom]));
    data.allpiecesa8h1 = (data.allpiecesa8h1 | petir_bitboard_mask::bit2nmaska8h1[sto]);
    data.allpiecesh8a1 = (data.allpiecesh8a1 & (~petir_bitboard_mask::bit2nmaskh8a1[pas::sub(sto, 8)]));
    data.allpiecesh8a1 = (data.allpiecesh8a1 & (~petir_bitboard_mask::bit2nmaskh8a1[sfrom]));
    data.allpiecesh8a1 = (data.allpiecesh8a1 | petir_bitboard_mask::bit2nmaskh8a1[sto]);
    data.pawnwhite = (data.pawnwhite & petir_bitboard_mask::bitnot(sfrom));
    data.pawnblack = (data.pawnblack & petir_bitboard_mask::bitnot(pas::sub(sto, 8)));
    data.pawnwhite = (data.pawnwhite | petir_bitboard_mask::bit(sto));
}

// makemove.pas:326
void promosi_putih(petir_header::tdata& data, std::int32_t sto, std::int32_t promosi) {
    data.pawnwhite = (data.pawnwhite & (~petir_bitboard_mask::bit(sto)));
    (data.materialscore = pas::decrement(data.materialscore, petir_header::_nilai_pion));
    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_pionputih][sto]);
    if ((promosi == petir_header::promosi_mentri)) {
        data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_mentriputih][sto]);
        data.papan[sto] = petir_header::_mentriputih;
        data.queenwhite = (data.queenwhite | petir_bitboard_mask::bit(sto));
        data.bishopqueen = (data.bishopqueen | petir_bitboard_mask::bit(sto));
        data.rookqueen = (data.rookqueen | petir_bitboard_mask::bit(sto));
        (data.materialscore = pas::increment(data.materialscore, petir_header::_nilai_mentri));
        (data.nilai_perwira_putih = pas::increment(data.nilai_perwira_putih, 9));
    } else {
        if ((promosi == petir_header::promosi_kuda)) {
            data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_kudaputih][sto]);
            data.papan[sto] = petir_header::_kudaputih;
            data.knightwhite = (data.knightwhite | petir_bitboard_mask::bit(sto));
            (data.materialscore = pas::increment(data.materialscore, petir_header::_nilai_kuda));
            (data.nilai_perwira_putih = pas::increment(data.nilai_perwira_putih, 3));
        } else {
            if ((promosi == petir_header::promosi_benteng)) {
                data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_bentengputih][sto]);
                data.papan[sto] = petir_header::_bentengputih;
                data.rookwhite = (data.rookwhite | petir_bitboard_mask::bit(sto));
                data.rookqueen = (data.rookqueen | petir_bitboard_mask::bit(sto));
                (data.materialscore = pas::increment(data.materialscore, petir_header::_nilai_benteng));
                (data.nilai_perwira_putih = pas::increment(data.nilai_perwira_putih, 5));
            } else {
                if ((promosi == petir_header::promosi_gajah)) {
                    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_gajahputih][sto]);
                    data.papan[sto] = petir_header::_gajahputih;
                    data.bishopwhite = (data.bishopwhite | petir_bitboard_mask::bit(sto));
                    data.bishopqueen = (data.bishopqueen | petir_bitboard_mask::bit(sto));
                    (data.ngajahputih = pas::increment(data.ngajahputih, 1));
                    (data.materialscore = pas::increment(data.materialscore, petir_header::_nilai_gajah));
                    (data.nilai_perwira_putih = pas::increment(data.nilai_perwira_putih, 3));
                }
            }
        }
    }
}

// makemove.pas:369
void promosi_hitam(petir_header::tdata& data, std::int32_t sto, std::int32_t promosi) {
    data.pawnblack = (data.pawnblack & (~petir_bitboard_mask::bit(sto)));
    (data.materialscore = pas::increment(data.materialscore, petir_header::_nilai_pion));
    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_pionhitam][sto]);
    if ((promosi == petir_header::promosi_mentri)) {
        data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_mentrihitam][sto]);
        data.papan[sto] = petir_header::_mentrihitam;
        data.queenblack = (data.queenblack | petir_bitboard_mask::bit(sto));
        data.rookqueen = (data.rookqueen | petir_bitboard_mask::bit(sto));
        data.bishopqueen = (data.bishopqueen | petir_bitboard_mask::bit(sto));
        (data.materialscore = pas::decrement(data.materialscore, petir_header::_nilai_mentri));
        (data.nilai_perwira_hitam = pas::increment(data.nilai_perwira_hitam, 9));
    } else {
        if ((promosi == petir_header::promosi_kuda)) {
            data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_kudahitam][sto]);
            data.papan[sto] = petir_header::_kudahitam;
            data.knightblack = (data.knightblack | petir_bitboard_mask::bit(sto));
            (data.materialscore = pas::decrement(data.materialscore, petir_header::_nilai_kuda));
            (data.nilai_perwira_hitam = pas::increment(data.nilai_perwira_hitam, 3));
        } else {
            if ((promosi == petir_header::promosi_benteng)) {
                data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_bentenghitam][sto]);
                data.papan[sto] = petir_header::_bentenghitam;
                data.rookblack = (data.rookblack | petir_bitboard_mask::bit(sto));
                data.rookqueen = (data.rookqueen | petir_bitboard_mask::bit(sto));
                (data.materialscore = pas::decrement(data.materialscore, petir_header::_nilai_benteng));
                (data.nilai_perwira_hitam = pas::increment(data.nilai_perwira_hitam, 5));
            } else {
                if ((promosi == petir_header::promosi_gajah)) {
                    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_gajahhitam][sto]);
                    data.papan[sto] = petir_header::_gajahhitam;
                    data.bishopblack = (data.bishopblack | petir_bitboard_mask::bit(sto));
                    data.bishopqueen = (data.bishopqueen | petir_bitboard_mask::bit(sto));
                    (data.ngajahhitam = pas::increment(data.ngajahhitam, 1));
                    (data.materialscore = pas::decrement(data.materialscore, petir_header::_nilai_gajah));
                    (data.nilai_perwira_hitam = pas::increment(data.nilai_perwira_hitam, 3));
                }
            }
        }
    }
}

}
