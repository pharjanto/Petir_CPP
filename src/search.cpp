// Faithful translation of search.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_search {
pas::Array<std::int32_t, 0, 63> curr_move{};
pas::Array<pas::Array<std::int32_t, 0, 63>, pas::negate(6), 6> hist_tried{};
pas::Array<pas::Array<std::int32_t, 0, 63>, pas::negate(6), 6> hist_succ{};
pas::Array<petir_search::anonymous_record_1, 0, 63> stack{};
std::int32_t rs{};
// search.pas:44
void empty_historytable() {
    pas::fillchar(petir_search::hist_tried, sizeof(petir_search::hist_tried), 0);
    pas::fillchar(petir_search::hist_succ, sizeof(petir_search::hist_succ), 0);
}

// search.pas:72
std::int32_t mainsearch(std::int32_t level, std::int32_t giliran, std::int32_t alpha, std::int32_t beta, petir_header::tdata& data, bool nullmove, std::int32_t depth, std::int32_t cext, bool nmtree, std::int32_t reduced, std::int32_t nodetype) {
    std::int32_t return_value{};
    std::int32_t g{};
    std::int32_t fase{};
    petir_header::tdata temp{};
    // Generators write both fields in entries 1..count before move ordering.
    // Avoid clearing two 2 KiB buffers on every node; capacity checks remain.
    petir_header::tmovelist ml;
    petir_header::tmovelist mlcap;
    std::int32_t curmovecap{};
    std::int32_t curmovenoncap{};
    std::int32_t jmlcap{};
    std::int32_t jmlnoncap{};
    std::int32_t newlevel{};
    std::int32_t nlev{};
    std::int32_t jml{};
    std::int32_t tcext{};
    std::int32_t t_alpha{};
    std::int32_t t_beta{};
    std::int32_t nilai{};
    std::int32_t tempi{};
    std::int32_t areduce{};
    std::uint8_t sto{};
    bool nullmoveok{};
    bool incheck{};
    bool mte{};
    std::uint64_t h{};
    std::int32_t goodcap{};
    std::int32_t tempep{};
    std::uint8_t r{};
    std::uint8_t stat{};
    std::int32_t rlevel{};
    std::int32_t legalmove{};
    std::int32_t moves{};
    std::int32_t hmoves{};
    std::int32_t tmoves{};
    std::int32_t neval{};
    bool futflag{};
    bool scoredraw{};
    bool isscoredraw{};
    bool donotreduce{};
    std::int32_t pas_temp_1{};
    std::int32_t pas_temp_2{};
// search.pas:91
    auto get_moving_piece = [&]() -> std::int8_t {
    std::int8_t return_value{};
    return_value = temp.papan[(moves & 127)];
    return return_value;
};

// search.pas:97
    auto get_dest = [&](std::int32_t sto) -> std::int8_t {
    std::int8_t return_value{};
    std::int32_t pas_temp_1{};
    return_value = pas::negate(1);
    pas_temp_1 = sto;
    if (((pas_temp_1 == petir_header::_rokadepanjang))) {
        if ((giliran == petir_header::_sisiputih)) {
            return_value = petir_header::c1;
        } else {
            return_value = petir_header::c8;
        }
    }
    else if (((pas_temp_1 == petir_header::_rokadependek))) {
        if ((giliran == petir_header::_sisiputih)) {
            return_value = petir_header::g1;
        } else {
            return_value = petir_header::g8;
        }
    }
    else {
        return_value = sto;
    }
    return return_value;
};

// search.pas:107
    auto seval = [&]() -> std::int32_t {
    std::int32_t return_value{};
    if (((neval == petir_search::noval) && (depth >= 2))) {
        return_value = petir_ueval::eval(data, giliran, pas::negate(petir_header::_infinity), petir_header::_infinity);
    } else {
        return_value = neval;
    }
    return return_value;
};

    // Portability guards: stop safely and keep the original 64-ply arrays in bounds.
    if (petir::search_should_stop())return 0;
    if (depth >= 63) return petir_ueval::eval(data, giliran, alpha, beta);
    petir_search::stack[depth].ext = false;
    if ((data.move50count >= 101)) {
        return_value = petir_header::contemp_draw;
        return return_value;
    }
    hmoves = petir_header::_no_move;
    futflag = false;
    if ((petir_hashing::searchhash(data.hashkey, alpha, beta, level, giliran, nilai, hmoves, futflag, data.move50count) && (nodetype != petir_search::node_pv))) {
        return_value = nilai;
        return return_value;
    }
    h = data.hashkey;
    rlevel = pas::shr(level, 3);
    mte = false;
    if ((giliran == petir_header::_sisiputih)) {
        if (petir_tools::white_checked(data)) {
            incheck = true;
            petir_search::stack[depth].ext = true;
            if ((cext > 0)) {
                if (((pas::add(data.nilai_perwira_putih, data.nilai_perwira_hitam) > 24) || (alpha != pas::sub(beta, 1)))) {
                    (level = pas::increment(level, 6));
                } else {
                    (level = pas::increment(level, 2));
                }
            }
            goto nonullmove;
        } else {
            incheck = false;
        }
    } else {
        if (petir_tools::black_checked(data)) {
            incheck = true;
            petir_search::stack[depth].ext = true;
            if ((cext > 0)) {
                if (((pas::add(data.nilai_perwira_putih, data.nilai_perwira_hitam) > 24) || (alpha != pas::sub(beta, 1)))) {
                    (level = pas::increment(level, 6));
                } else {
                    (level = pas::increment(level, 2));
                }
            }
            goto nonullmove;
        } else {
            incheck = false;
        }
    }
    petir_search::stack[depth].mte = false;
    if (((((((rlevel >= 2) && (beta < pas::sub(petir_header::_infinity, 100))) && (!nullmove)) && (petir_ueval::materialvalue(giliran, data.materialscore) >= pas::negate(500))) && (alpha == pas::sub(beta, 1))) && (!futflag))) {
        if ((giliran == petir_header::_sisiputih)) {
            tempep = data.nilai_perwira_putih;
        } else {
            tempep = data.nilai_perwira_hitam;
        }
        if ((tempep >= 3)) {
            tempep = data.ep;
            pas_temp_1 = rlevel;
            if (((pas_temp_1 == 4))) {
                if ((petir_ueval::materialvalue(giliran, data.materialscore) >= pas::add(alpha, 500))) {
                    newlevel = pas::sub(level, 32);
                } else {
                    if ((nodetype == petir_search::node_pv)) {
                        newlevel = pas::sub(level, 24);
                    } else {
                        newlevel = pas::sub(level, 32);
                    }
                }
            }
            else {
                newlevel = pas::sub(level, 32);
            }
            if ((data.ep != petir_header::_no_ep)) {
                data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[data.ep]);
                data.ep = petir_header::_no_ep;
                data.hashkey = (data.hashkey ^ petir_header::enpassanthashkey[petir_header::_no_ep]);
            }
            areduce = 0;
            petir_search::stack[depth].evaluated = false;
            if ((newlevel < petir_header::fractional_ply)) {
                if (((data.nilai_perwira_putih <= 9) && (data.nilai_perwira_hitam <= 9))) {
                    nilai = pas::negate(petir_search::q_search(pas::sub(3, giliran), pas::negate(beta), pas::negate(pas::sub(beta, 1)), data, 0, pas::add(depth, 1), cext, petir_search::noval, pas::negate(nodetype)));
                } else {
                    nilai = pas::negate(petir_search::q_search(pas::sub(3, giliran), pas::negate(beta), pas::negate(pas::sub(beta, 1)), data, 1, pas::add(depth, 1), cext, petir_search::noval, pas::negate(nodetype)));
                }
            } else {
                nilai = pas::negate(petir_search::mainsearch(newlevel, pas::sub(3, giliran), pas::negate(beta), pas::negate(pas::sub(beta, 1)), data, true, pas::add(depth, 1), cext, true, 0, pas::negate(nodetype)));
            }
            if(petir::search_should_stop()) {data.hashkey=h;data.ep=tempep;return 0;}
            if ((nilai >= beta)) {
                data.hashkey = h;
                data.ep = tempep;
                return_value = nilai;
                return return_value;
            }
            if (((nilai == pas::add(pas::add(pas::negate(petir_header::_nilai_raja), depth), 2)) && (cext > 0))) {
                petir_search::stack[depth].mte = true;
                (level = pas::increment(level, 6));
                (cext = pas::decrement(cext, 6));
                mte = true;
            }
            data.hashkey = h;
            data.ep = tempep;
        }
    }
    nonullmove: ;
    donotreduce = false;
    if ((petir_search::stack[pas::sub(depth, 1)].evaluated && ((pas::sub(petir_search::stack[pas::sub(depth, 1)].evalvalue, petir_ueval::materialvalue(giliran, data.materialscore)) >= 90) || (petir_search::stack[pas::sub(depth, 1)].evalvalue >= beta)))) {
        donotreduce = true;
    }
    fase = 0;
    temp = data;
    curmovecap = 0;
    tmoves = petir_header::_no_move;
    t_alpha = alpha;
    t_beta = beta;
    g = pas::sub(pas::negate(petir_header::_infinity), 100);
    legalmove = 0;
    nlev = pas::sub(level, petir_header::fractional_ply);
    tcext = cext;
    goodcap = 0;
    rlevel = pas::shr(level, 3);
    isscoredraw = false;
    do {
        if(petir::search_should_stop())return 0;
        petir_search::stack[depth].evaluated = false;
        scoredraw = false;
        futflag = false;
        neval = petir_search::noval;
        cext = tcext;
        newlevel = nlev;
        areduce = 0;
        if ((!incheck)) {
            petir_next::nextmove(giliran, depth, data, moves, fase, hmoves, curmovecap, curmovenoncap, jmlcap, jmlnoncap, ml, mlcap, goodcap);
            if ((moves == petir_header::_no_move)) {
                break;
            }
            sto = (pas::shr(moves, 7) & 127);
            if ((giliran == petir_header::_sisiputih)) {
                if ((data.papan[(moves & 127)] == petir_header::_rajaputih)) {
                    if (petir_tools::white_attacked(data, sto)) {
                        continue;
                    }
                } else {
                    if (petir_tools::white_open_check(data, moves)) {
                        continue;
                    }
                }
            } else {
                if ((data.papan[(moves & 127)] == petir_header::_rajahitam)) {
                    if (petir_tools::black_attacked(data, sto)) {
                        continue;
                    }
                } else {
                    if (petir_tools::black_open_check(data, moves)) {
                        continue;
                    }
                }
            }
        } else {
            if (petir_next::nextevasion(giliran, data, moves, fase, hmoves, curmovecap, jml, jmlcap, ml)) {
                return_value = pas::add(pas::negate(petir_header::_nilai_raja), depth);
                return return_value;
            }
            if ((moves == petir_header::_no_move)) {
                break;
            }
            sto = (pas::shr(moves, 7) & 127);
            if (((legalmove == 0) && (cext > 0))) {
                if ((jml == 1)) {
                    (newlevel = pas::increment(newlevel, petir_header::single_reply_extension));
                    (nlev = pas::increment(nlev, petir_header::single_reply_extension));
                    (cext = pas::increment(cext, 8));
                    (tcext = pas::increment(tcext, 8));
                    (newlevel = pas::decrement(newlevel, (newlevel & 7)));
                }
                if ((jmlcap == 2)) {
                    (newlevel = pas::increment(newlevel, petir_header::double_skak_extension));
                    (nlev = pas::increment(nlev, petir_header::double_skak_extension));
                    (cext = pas::decrement(cext, petir_header::double_skak_extension));
                    (tcext = pas::decrement(tcext, petir_header::double_skak_extension));
                }
                (cext = pas::decrement(cext, petir_header::skak_extension));
                (tcext = pas::decrement(tcext, petir_header::skak_extension));
            }
        }
        if ((giliran == petir_header::_sisiputih)) {
            petir_makemove::makewhitemove(moves, data);
        } else {
            petir_makemove::makeblackmove(moves, data);
        }
        if ((sto == petir_header::_en_passant)) {
            if ((((giliran == petir_header::_sisiputih) && petir_tools::white_checked(data)) || ((giliran == petir_header::_sisihitam) && petir_tools::black_checked(data)))) {
                data = temp;
                continue;
            }
        }
        if ((data.cp == 0)) {
            (petir_search::hist_tried[get_moving_piece()][get_dest(sto)] = pas::increment(petir_search::hist_tried[get_moving_piece()][get_dest(sto)], pas::mul(rlevel, rlevel)));
        }
        petir_search::curr_move[depth] = moves;
        (legalmove = pas::increment(legalmove, 1));
        if ((petir_repetition::addrephash(data.hashkey, pas::sub(2, giliran)) == 1)) {
            nilai = petir_header::contemp_draw;
            scoredraw = true;
            goto search3;
        }
        petir_search::stack[depth].fase = fase;
        petir_search::stack[depth].legalmove = legalmove;
        if ((((data.nilai_perwira_putih == 0) && (data.nilai_perwira_hitam == 0)) && (nodetype == petir_search::node_pv))) {
            if (((data.cp == 1) && (cext > 0))) {
                (newlevel = pas::increment(newlevel, 4));
                (cext = pas::decrement(cext, 4));
            }
            if ((((((data.cp >= petir_header::_kudaputih) && (data.cp <= petir_header::_mentriputih)) && (data.materialscore >= pas::negate(385))) && (data.materialscore <= 385)) && (cext > 0))) {
                (cext = pas::decrement(cext, 16));
                (newlevel = pas::increment(newlevel, 16));
                goto search1;
            }
        }
        if ((((((((sto <= 63) && (pas::abs(data.papan[sto]) == petir_header::_pionputih)) && (nodetype == petir_search::node_pv)) && (cext > 0)) && (((giliran == petir_header::_sisiputih) && (sto >= petir_header::a7)) || ((giliran == petir_header::_sisihitam) && (sto <= petir_header::h2)))) && (!incheck)) && (petir_usee::see(giliran, temp, (moves & 127), sto) >= 0))) {
            (newlevel = pas::increment(newlevel, 8));
            (cext = pas::decrement(cext, 8));
            goto search1;
        }
        if ((((((((alpha != pas::sub(beta, 1)) && (data.cp > 1)) && (data.cp < 9)) && (data.cp == temp.cp)) && (!incheck)) && (cext > 0)) && (!mte))) {
            (newlevel = pas::increment(newlevel, 8));
            (cext = pas::decrement(cext, 8));
            goto search1;
        }
        if (((((((((rlevel <= 2) && (!incheck)) && (data.cp == 0)) && (pas::sub(beta, alpha) == 1)) && (!petir_search::stack[pas::sub(depth, 1)].mte)) && (!petir_search::stack[pas::sub(depth, 1)].ext)) && (nodetype != petir_search::node_pv)) && (moves != hmoves))) {
            pas_temp_2 = rlevel;
            if (((pas_temp_2 == 1))) {
                tempi = petir_ueval::materialvalue(giliran, data.materialscore);
                if ((pas::add(data.nilai_perwira_putih, data.nilai_perwira_hitam) <= 24)) {
                    (tempi = pas::increment(tempi, 300));
                }
                if ((((((((pas::add(tempi, 250) < t_alpha) && (!petir_tools::menskak2(temp, giliran, moves))) && (data.nilai_perwira_putih > 3)) && (data.nilai_perwira_hitam > 3)) && (!petir_tools::pawn_7_rank(giliran, data))) && (data.pawnwhite != 0)) && (data.pawnblack != 0))) {
                    nilai = tempi;
                    goto search3;
                }
            }
            else if (((pas_temp_2 == 2))) {
                tempi = petir_ueval::materialvalue(giliran, data.materialscore);
                if ((pas::add(data.nilai_perwira_putih, data.nilai_perwira_hitam) <= 24)) {
                    (tempi = pas::increment(tempi, 300));
                }
                if (((((((((pas::add(tempi, 300) < t_alpha) && (!petir_tools::menskak2(temp, giliran, moves))) && (data.nilai_perwira_putih > 5)) && (data.nilai_perwira_hitam > 5)) && (!petir_tools::pawn_7_rank(giliran, data))) && (!petir_tools::pawn_6_rank(giliran, data))) && (data.pawnwhite != 0)) && (data.pawnblack != 0))) {
                    nilai = tempi;
                    goto search3;
                }
            }
        }
        if ((giliran == petir_header::_sisiputih)) {
            if ((((((((((((((!incheck) && (rlevel >= 2)) && (depth >= 2)) && (data.cp == 0)) && (fase > petir_header::_killermoves3)) && (alpha == t_alpha)) && (pas::sub(beta, alpha) == 1)) && (!petir_tools::pawn_7_rank(petir_header::_sisiputih, data))) && (!mte)) && (!petir_tools::good_move_white(data, sto))) && (((data.nilai_perwira_putih >= 6) && (data.nilai_perwira_hitam >= 6)) || (((data.nilai_perwira_putih >= 3) && (data.nilai_perwira_hitam >= 3)) && (rlevel == 2)))) && (!petir_tools::white_threat(data, moves, tempi))) && (!petir_tools::black_open_check(temp, moves)))) {
                if ((((((tempi == 0) || ((((legalmove > 5) && (petir_search::stack[pas::sub(depth, 2)].fase > petir_header::_killermoves4)) && (petir_search::stack[pas::sub(depth, 1)].legalmove <= 3)) && (!petir_search::stack[pas::sub(depth, 1)].mte))) && (legalmove > 3)) && (petir_search::hist_tried[get_moving_piece()][get_dest(sto)] >= petir_search::min_try)) && (petir_search::hist_succ[get_moving_piece()][get_dest(sto)] <= pas::shr(petir_search::hist_tried[get_moving_piece()][get_dest(sto)], 3)))) {
                    (newlevel = pas::decrement(newlevel, 8));
                    areduce = 8;
                    if ((petir_search::hist_succ[get_moving_piece()][get_dest(sto)] <= pas::shr(petir_search::hist_tried[get_moving_piece()][get_dest(sto)], 4))) {
                        (newlevel = pas::decrement(newlevel, 8));
                        areduce = 16;
                    }
                    goto search1;
                }
                if ((rlevel <= 8)) {
                    nilai = petir_ueval::eval(data, giliran, pas::negate(petir_header::_infinity), petir_header::_infinity, true);
                    neval = pas::negate(nilai);
                    petir_search::stack[depth].evaluated = true;
                    petir_search::stack[depth].evalvalue = neval;
                    if (((pas::add(pas::add(pas::add(nilai, tempi), petir_search::margin[rlevel]), 30) < t_alpha) && (!petir_tools::pawn_6_rank(giliran, data)))) {
                        (newlevel = pas::decrement(newlevel, 24));
                        areduce = 24;
                    } else {
                        if ((pas::add(pas::add(nilai, tempi), petir_search::margin[rlevel]) < t_alpha)) {
                            (newlevel = pas::decrement(newlevel, 16));
                            areduce = 16;
                        } else {
                            if ((pas::sub(pas::add(pas::add(nilai, tempi), petir_search::margin[rlevel]), 5) < t_alpha)) {
                                (newlevel = pas::decrement(newlevel, 8));
                                areduce = 8;
                            } else {
                                if (((tempi > 0) && (pas::add(pas::add(pas::add(nilai, petir_search::margin[rlevel]), 40), pas::shr(tempi, 1)) < t_alpha))) {
                                    newlevel = 0;
                                } else {
                                    if ((((goodcap > 1) && (tempi == 0)) && (nilai < t_alpha))) {
                                        if ((goodcap >= 5)) {
                                            (newlevel = pas::decrement(newlevel, 16));
                                            areduce = 16;
                                        } else {
                                            (newlevel = pas::decrement(newlevel, 8));
                                            areduce = 8;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                ;
            }
        } else {
            if ((((((((((((((!incheck) && (rlevel >= 2)) && (depth >= 2)) && (data.cp == 0)) && (fase > petir_header::_killermoves3)) && (alpha == t_alpha)) && (pas::sub(beta, alpha) == 1)) && (!petir_tools::pawn_7_rankb(data))) && (!mte)) && (!petir_tools::good_move_black(data, sto))) && (((data.nilai_perwira_putih >= 6) && (data.nilai_perwira_hitam >= 6)) || (((data.nilai_perwira_putih >= 3) && (data.nilai_perwira_hitam >= 3)) && (rlevel == 2)))) && (!petir_tools::black_threat(data, moves, tempi))) && (!petir_tools::white_open_check(temp, moves)))) {
                if ((((((tempi == 0) || ((((legalmove > 5) && (petir_search::stack[pas::sub(depth, 2)].fase > petir_header::_killermoves4)) && (petir_search::stack[pas::sub(depth, 1)].legalmove <= 3)) && (!petir_search::stack[pas::sub(depth, 1)].mte))) && (legalmove > 3)) && (petir_search::hist_tried[get_moving_piece()][get_dest(sto)] >= petir_search::min_try)) && (petir_search::hist_succ[get_moving_piece()][get_dest(sto)] <= pas::shr(petir_search::hist_tried[get_moving_piece()][get_dest(sto)], 3)))) {
                    (newlevel = pas::decrement(newlevel, 8));
                    areduce = 8;
                    if ((petir_search::hist_succ[get_moving_piece()][get_dest(sto)] <= pas::shr(petir_search::hist_tried[get_moving_piece()][get_dest(sto)], 4))) {
                        (newlevel = pas::decrement(newlevel, 8));
                        areduce = 16;
                    }
                    goto search1;
                }
                if ((rlevel <= 8)) {
                    nilai = petir_ueval::eval(data, giliran, pas::negate(petir_header::_infinity), petir_header::_infinity, true);
                    neval = pas::negate(nilai);
                    petir_search::stack[depth].evaluated = true;
                    petir_search::stack[depth].evalvalue = neval;
                    if (((pas::add(pas::add(pas::add(nilai, tempi), petir_search::margin[rlevel]), 30) < t_alpha) && (!petir_tools::pawn_6_rank(giliran, data)))) {
                        (newlevel = pas::decrement(newlevel, 24));
                        areduce = 24;
                    } else {
                        if ((pas::add(pas::add(nilai, tempi), petir_search::margin[rlevel]) < t_alpha)) {
                            (newlevel = pas::decrement(newlevel, 16));
                            areduce = 16;
                        } else {
                            if ((pas::sub(pas::add(pas::add(nilai, tempi), petir_search::margin[rlevel]), 5) < t_alpha)) {
                                (newlevel = pas::decrement(newlevel, 8));
                                areduce = 8;
                            } else {
                                if (((tempi > 0) && (pas::add(pas::add(pas::add(nilai, petir_search::margin[rlevel]), 40), pas::shr(tempi, 1)) < t_alpha))) {
                                    newlevel = 0;
                                } else {
                                    if ((((goodcap > 1) && (tempi == 0)) && (nilai < t_alpha))) {
                                        if ((goodcap >= 5)) {
                                            (newlevel = pas::decrement(newlevel, 16));
                                            areduce = 16;
                                        } else {
                                            (newlevel = pas::decrement(newlevel, 8));
                                            areduce = 8;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                ;
            }
        }
        if (((((((((((((((!incheck) && (depth <= 10)) && (data.cp == 0)) && (depth >= 2)) && (legalmove > 6)) && (!mte)) && (areduce == 0)) && (!petir_search::stack[pas::sub(depth, 1)].mte)) && (!petir_search::stack[pas::sub(depth, 2)].mte)) && (!petir_tools::pawn_7_rank(1, data))) && (!petir_tools::pawn_7_rank(2, data))) && (!donotreduce)) && (!petir_tools::good_move(data, sto, giliran))) && (!petir_tools::menskak2(temp, giliran, moves)))) {
            (newlevel = pas::decrement(newlevel, 8));
            areduce = 8;
            goto search1;
        }
        if ((((((((((((((!incheck) && (rlevel >= 4)) && (depth >= 2)) && (nodetype != petir_search::node_pv)) && (((nodetype == petir_search::node_cut) && (legalmove > 3)) || ((nodetype == petir_search::node_all) && (legalmove > 6)))) && (!mte)) && (fase > petir_header::_killermoves3)) && (areduce == 0)) && (data.cp == 0)) && (!petir_tools::menskak2(temp, giliran, moves))) && (!petir_tools::pawn_7_rank(giliran, data))) && (!petir_tools::good_move(data, sto, giliran))) && (!donotreduce))) {
            (newlevel = pas::decrement(newlevel, 8));
            areduce = 8;
        }
        search1: ;
        if ((newlevel < 8)) {
            if ((giliran == petir_header::_sisiputih)) {
                if ((petir_tools::white_forkandpin(sto, data) && (cext > 0))) {
                    newlevel = 8;
                    cext = 0;
                }
            } else {
                if ((petir_tools::black_forkandpin(sto, data) && (cext > 0))) {
                    newlevel = 8;
                    cext = 0;
                }
            }
        }
        if ((newlevel < petir_header::fractional_ply)) {
            if ((legalmove == 1)) {
                nilai = pas::negate(petir_search::q_search(pas::sub(3, giliran), pas::negate(t_beta), pas::negate(t_alpha), data, 1, pas::add(depth, 1), cext, neval, pas::negate(nodetype)));
            } else {
                nilai = pas::negate(petir_search::q_search(pas::sub(3, giliran), pas::negate(t_beta), pas::negate(t_alpha), data, 1, pas::add(depth, 1), cext, neval, petir_search::node_cut));
            }
        } else {
            if ((legalmove == 1)) {
                nilai = pas::negate(petir_search::mainsearch(newlevel, pas::sub(3, giliran), pas::negate(t_beta), pas::negate(t_alpha), data, false, pas::add(depth, 1), cext, nmtree, areduce, pas::negate(nodetype)));
            } else {
                nilai = pas::negate(petir_search::mainsearch(newlevel, pas::sub(3, giliran), pas::negate(t_beta), pas::negate(t_alpha), data, false, pas::add(depth, 1), cext, nmtree, areduce, petir_search::node_cut));
            }
        }
        search2: ;
        if(petir::search_should_stop()) {
            petir_repetition::delrephash(data.hashkey,pas::sub(2,giliran));data=temp;return 0;
        }
        if ((((nilai > t_alpha) && (nilai < beta)) && (legalmove > 1))) {
            if ((areduce != 0)) {
                (newlevel = pas::increment(newlevel, areduce));
            }
            if ((newlevel < petir_header::fractional_ply)) {
                nilai = pas::negate(petir_search::q_search(pas::sub(3, giliran), pas::negate(beta), pas::negate(t_alpha), data, 1, pas::add(depth, 1), cext, petir_search::noval, petir_search::node_pv));
            } else {
                nilai = pas::negate(petir_search::mainsearch(newlevel, pas::sub(3, giliran), pas::negate(beta), pas::negate(t_alpha), data, false, pas::add(depth, 1), cext, nmtree, areduce, petir_search::node_pv));
            }
        } else {
            if ((nilai >= beta)) {
                if ((areduce != 0)) {
                    (newlevel = pas::increment(newlevel, areduce));
                    areduce = 0;
                    if ((newlevel < petir_header::fractional_ply)) {
                        nilai = pas::negate(petir_search::q_search(pas::sub(3, giliran), pas::negate(nilai), pas::negate(t_alpha), data, 1, pas::add(depth, 1), cext, petir_search::noval, petir_search::node_pv));
                    } else {
                        nilai = pas::negate(petir_search::mainsearch(newlevel, pas::sub(3, giliran), pas::negate(nilai), pas::negate(t_alpha), data, false, pas::add(depth, 1), cext, nmtree, areduce, petir_search::node_pv));
                    }
                }
            }
        }
        search3: ;
        if(petir::search_should_stop()) {
            petir_repetition::delrephash(data.hashkey,pas::sub(2,giliran));data=temp;return 0;
        }
        if ((nilai > g)) {
            g = nilai;
            isscoredraw = scoredraw;
            if ((nilai > t_alpha)) {
                tmoves = moves;
                t_alpha = nilai;
                if ((g >= beta)) {
                    if ((data.cp == 0)) {
                        (petir_search::hist_succ[get_moving_piece()][get_dest(sto)] = pas::increment(petir_search::hist_succ[get_moving_piece()][get_dest(sto)], pas::mul(rlevel, rlevel)));
                    }
                    if (((data.cp == 0) && (!incheck))) {
                        petir_search::addhistory(rlevel, depth, moves, giliran, get_moving_piece());
                    }
                    petir_repetition::delrephash(data.hashkey, pas::sub(2, giliran));
                    data = temp;
                    break;
                }
            }
        }
        t_beta = pas::add(t_alpha, 1);
        petir_repetition::delrephash(data.hashkey, pas::sub(2, giliran));
        data = temp;
    } while (!((1 == 2)));
    if (petir::search_should_stop()) {
        return return_value;
    }
    if ((legalmove == 0)) {
        tmoves = petir_header::_no_move;
        return_value = petir_header::contemp_draw;
        g = return_value;
    } else {
        return_value = g;
        if ((tmoves == petir_header::_no_move)) {
            tmoves = hmoves;
        }
    }
    if ((!isscoredraw)) {
        if ((g <= alpha)) {
            petir_hashing::addtable(h, giliran, level, tmoves, g, petir_header::fail_low, data.move50count);
        } else {
            if ((g >= beta)) {
                petir_hashing::addtable(h, giliran, level, tmoves, g, petir_header::fail_high, data.move50count);
            } else {
                petir_hashing::addtable(h, giliran, level, tmoves, g, petir_header::pv_node, data.move50count);
                sto = (pas::shr(tmoves, 7) & 127);
                if ((((tmoves != petir_header::_no_move) && (sto != petir_header::_en_passant)) && ((sto > 63) || (data.papan[sto] == 0)))) {
                    if ((giliran == petir_header::_sisiputih)) {
                        (petir_header::historyw[data.papan[(tmoves & 127)]][(pas::shr(tmoves, 7) & 127)] = pas::increment(petir_header::historyw[data.papan[(tmoves & 127)]][(pas::shr(tmoves, 7) & 127)], pas::mul(rlevel, rlevel)));
                    } else {
                        (petir_header::historyb[data.papan[(tmoves & 127)]][(pas::shr(tmoves, 7) & 127)] = pas::increment(petir_header::historyb[data.papan[(tmoves & 127)]][(pas::shr(tmoves, 7) & 127)], pas::mul(rlevel, rlevel)));
                    }
                }
            }
        }
    }
    return return_value;
}

// search.pas:51
void addhistory(std::int32_t rlevel, std::int32_t depth, std::int32_t moves, std::int32_t giliran, std::int32_t piece) {
    if ((giliran == petir_header::_sisiputih)) {
        (petir_header::historyw[piece][(pas::shr(moves, 7) & 127)] = pas::increment(petir_header::historyw[piece][(pas::shr(moves, 7) & 127)], pas::mul(rlevel, rlevel)));
    } else {
        (petir_header::historyb[piece][(pas::shr(moves, 7) & 127)] = pas::increment(petir_header::historyb[piece][(pas::shr(moves, 7) & 127)], pas::mul(rlevel, rlevel)));
    }
    if ((petir_header::killer0[depth] != moves)) {
        petir_header::killer1[depth] = petir_header::killer0[depth];
        petir_header::killer0[depth] = moves;
    }
}

}
