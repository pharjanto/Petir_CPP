// Faithful translation of etc.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_etc {
// etc.pas:130
void updateblackkey(std::int32_t moves, petir_header::tdata& data) {
    std::int32_t captured_piece{};
    std::int32_t piece{};
    std::int32_t sfrom{};
    std::int32_t sto{};
    std::int32_t promosi{};
    sfrom = (moves & 127);
    sto = (pas::shr(moves, 7) & 127);
    promosi = (pas::shr(moves, 14) & 7);
    if ((sto == petir_header::_rokadependek)) {
        data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_bentenghitam][petir_header::h8]);
        data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_rajahitam][petir_header::e8]);
        data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_bentenghitam][petir_header::f8]);
        data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_rajahitam][petir_header::g8]);
        data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
        data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[(data.flagrokade & (~12))]);
        data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[data.ep]);
        data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[petir_header::_no_ep]);
    } else {
        if ((sto == petir_header::_rokadepanjang)) {
            data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_bentenghitam][petir_header::a8]);
            data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_rajahitam][petir_header::e8]);
            data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_bentenghitam][petir_header::d8]);
            data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_rajahitam][petir_header::c8]);
            data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
            data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[(data.flagrokade & (~12))]);
            data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[data.ep]);
            data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[petir_header::_no_ep]);
        } else {
            if ((sto == petir_header::_en_passant)) {
                data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_pionhitam][sfrom]);
                data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_pionhitam][sto]);
                data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[data.ep]);
                data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[petir_header::_no_ep]);
                data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_pionputih][pas::add(sto, 8)]);
            } else {
                piece = data.papan[sfrom];
                data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[data.ep]);
                data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[petir_header::_no_ep]);
                captured_piece = data.papan[sto];
                data.hashkey = (data.hashkey ^ petir_header::hashkey[piece][sfrom]);
                data.hashkey = (data.hashkey ^ petir_header::hashkey[piece][sto]);
                if ((captured_piece != 0)) {
                    data.hashkey = (data.hashkey ^ petir_header::hashkey[captured_piece][sto]);
                } else {
                    if ((((data.papan[sfrom] == petir_header::_pionhitam) && (pas::sub(sfrom, sto) == 16)) && (((data.papan[pas::sub(sto, 1)] == petir_header::_pionputih) && ((sto % 8) >= 1)) || ((data.papan[pas::add(sto, 1)] == petir_header::_pionputih) && ((sto % 8) <= 6))))) {
                        data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[data.ep]);
                        data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[(sfrom % 8)]);
                    }
                }
                if ((data.papan[sfrom] == petir_header::_bentenghitam)) {
                    if ((sfrom == petir_header::h8)) {
                        data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
                        data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[(data.flagrokade & (~petir_header::bithitamsc))]);
                    } else {
                        if ((sfrom == petir_header::a8)) {
                            data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
                            data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[(data.flagrokade & (~petir_header::bithitamlc))]);
                        }
                    }
                } else {
                    if ((data.papan[sfrom] == petir_header::_rajahitam)) {
                        data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
                        data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[(data.flagrokade & (~12))]);
                    }
                }
                if ((promosi != 0)) {
                    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_pionhitam][sto]);
                    if ((promosi == petir_header::promosi_mentri)) {
                        data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_mentrihitam][sto]);
                    } else {
                        if ((promosi == petir_header::promosi_kuda)) {
                            data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_kudahitam][sto]);
                        } else {
                            if ((promosi == petir_header::promosi_benteng)) {
                                data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_bentenghitam][sto]);
                            } else {
                                if ((promosi == petir_header::promosi_gajah)) {
                                    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_gajahhitam][sto]);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// etc.pas:21
void updatewhitekey(std::int32_t moves, petir_header::tdata& data) {
    std::int32_t captured_piece{};
    std::int32_t piece{};
    std::int32_t sfrom{};
    std::int32_t sto{};
    std::int32_t promosi{};
    sfrom = (moves & 127);
    sto = (pas::shr(moves, 7) & 127);
    promosi = (pas::shr(moves, 14) & 7);
    if ((sto == petir_header::_rokadependek)) {
        data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_bentengputih][petir_header::h1]);
        data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_rajaputih][petir_header::e1]);
        data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_bentengputih][petir_header::f1]);
        data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_rajaputih][petir_header::g1]);
        data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
        data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[(data.flagrokade & (~3))]);
        data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[data.ep]);
        data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[petir_header::_no_ep]);
    } else {
        if ((sto == petir_header::_rokadepanjang)) {
            data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_bentengputih][petir_header::a1]);
            data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_rajaputih][petir_header::e1]);
            data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_bentengputih][petir_header::d1]);
            data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_rajaputih][petir_header::c1]);
            data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
            data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[(data.flagrokade & (~3))]);
            data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[data.ep]);
            data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[petir_header::_no_ep]);
        } else {
            if ((sto == petir_header::_en_passant)) {
                data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_pionputih][sfrom]);
                data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_pionputih][sto]);
                data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[data.ep]);
                data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_pionhitam][pas::sub(sto, 8)]);
                data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[petir_header::_no_ep]);
            } else {
                data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[data.ep]);
                data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[petir_header::_no_ep]);
                piece = data.papan[sfrom];
                captured_piece = data.papan[sto];
                data.hashkey = (data.hashkey ^ petir_header::hashkey[piece][sfrom]);
                data.hashkey = (data.hashkey ^ petir_header::hashkey[piece][sto]);
                if ((captured_piece != 0)) {
                    data.hashkey = (data.hashkey ^ petir_header::hashkey[captured_piece][sto]);
                } else {
                    if ((((data.papan[sfrom] == petir_header::_pionputih) && (pas::sub(sto, sfrom) == 16)) && (((data.papan[pas::sub(sto, 1)] == petir_header::_pionhitam) && ((sto % 8) >= 1)) || ((data.papan[pas::add(sto, 1)] == petir_header::_pionhitam) && ((sto % 8) <= 6))))) {
                        data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[data.ep]);
                        data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[(sfrom % 8)]);
                    }
                }
                if ((data.papan[sfrom] == petir_header::_bentengputih)) {
                    if ((sfrom == petir_header::h1)) {
                        data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
                        data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[(data.flagrokade & (~petir_header::bitputihsc))]);
                    } else {
                        if ((sfrom == petir_header::a1)) {
                            data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
                            data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[(data.flagrokade & (~petir_header::bitputihlc))]);
                        }
                    }
                } else {
                    if ((data.papan[sfrom] == petir_header::_rajaputih)) {
                        data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[data.flagrokade]);
                        data.hashkey = (data.hashkey ^ petir_header::rokadehashkey[(data.flagrokade & (~3))]);
                    }
                }
                if ((promosi != 0)) {
                    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_pionputih][sto]);
                    if ((promosi == petir_header::promosi_mentri)) {
                        data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_mentriputih][sto]);
                    } else {
                        if ((promosi == petir_header::promosi_kuda)) {
                            data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_kudaputih][sto]);
                        } else {
                            if ((promosi == petir_header::promosi_benteng)) {
                                data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_bentengputih][sto]);
                            } else {
                                if ((promosi == petir_header::promosi_gajah)) {
                                    data.hashkey = (data.hashkey ^ petir_header::hashkey[petir_header::_gajahputih][sto]);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// etc.pas:13
void updatekey(petir_header::tdata& data, std::int32_t moves, std::int32_t giliran) {
    if ((giliran == petir_header::_sisiputih)) {
        petir_etc::updatewhitekey(moves, data);
    } else {
        petir_etc::updateblackkey(moves, data);
    }
}

}
