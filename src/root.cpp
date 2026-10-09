// Faithful translation of root.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_root {
std::int32_t bmoves{};
std::int32_t ntimelimit{};
std::int32_t rtl{};
bool newpv{};
// root.pas:16
std::int32_t searchroot(std::int32_t level, std::int32_t giliran, std::int32_t tipe, std::int32_t alpha, std::int32_t beta, petir_header::tdata data, bool nullmove, std::int32_t depth, std::int32_t cext) {
    std::int32_t return_value{};
    std::int32_t g{};
    std::int32_t fase{};
    std::int32_t t_alpha{};
    std::int32_t t_beta{};
    std::int32_t nilai{};
    std::int32_t tempi{};
    bool anu{};
    bool ext{};
    bool incheck{};
    bool first{};
    bool finish{};
    bool pp{};
    bool draw{};
    bool mbadcap{};
    petir_header::tdata temp{};
    std::int8_t cp{};
    std::int32_t goodcap{};
    std::int32_t tnilai{};
    petir_header::tmovelist ml{};
    petir_header::tmovelist mlcap{};
    pas::String spv{};
    std::int32_t lmoves{};
    std::int32_t legalmove{};
    std::int32_t nl{};
    std::int32_t moves{};
    std::int32_t hmoves{};
    std::int32_t curmovecap{};
    std::int32_t curmovenoncap{};
    std::int32_t jmlcap{};
    std::int32_t jmlnoncap{};
    std::int32_t newlevel{};
    std::int32_t nlev{};
    std::uint64_t h2{};
    std::uint64_t h{};
    std::int32_t sc{};
    std::int32_t cnode{};
    std::int32_t tcext{};
    std::int32_t rlevel{};
    std::int32_t sto{};
    std::int32_t eext{};
    std::uint8_t stat{};
    std::int32_t pas_temp_1{};
    std::int32_t pas_temp_2{};
    rlevel = (level / petir_header::fractional_ply);
    if(petir::search_should_stop(true))return 0;
    pas::fillchar(petir_search::curr_move, sizeof(petir_search::curr_move), 0);
    cnode = petir_header::total_node;
    hmoves = petir_header::_no_move;
    // Retain the hash move for ordering, but search root moves at every depth.
    // Cached score cutoffs remain available below the root.
    (void)petir_hashing::searchhash(data.hashkey, alpha, beta, level, giliran, nilai, hmoves, mbadcap, data.move50count);
    h = data.hashkey;
    (petir_header::total_node = pas::increment(petir_header::total_node, 1));
    finish = false;
    fase = 0;
    temp = data;
    curmovecap = 0;
    t_alpha = alpha;
    t_beta = beta;
    if ((tipe == petir_header::_nodemax)) {
        g = pas::sub(pas::negate(petir_header::_infinity), 100);
    } else {
        g = pas::add(petir_header::_infinity, 100);
    }
    first = true;
    legalmove = 0;
    nlev = pas::sub(level, petir_header::fractional_ply);
    eext = 0;
    if ((giliran == petir_header::_sisiputih)) {
        if (petir_tools::white_checked(data)) {
            incheck = true;
        } else {
            incheck = false;
        }
    } else {
        if (petir_tools::black_checked(data)) {
            incheck = true;
        } else {
            incheck = false;
        }
    }
    tcext = cext;
    petir_search::empty_historytable();
    do {
        if(petir::search_should_stop(true))return 0;
        petir_search::stack[depth].ext = false;
        pas::fillchar(petir_search::stack, sizeof(petir_search::stack), 0);
        cext = tcext;
        ext = false;
        draw = false;
        newlevel = nlev;
        if ((!incheck)) {
            petir_next::nextmove(giliran, depth, data, moves, fase, hmoves, curmovecap, curmovenoncap, jmlcap, jmlnoncap, ml, mlcap, goodcap);
        } else {
            if (petir_next::nextevasion(giliran, data, moves, fase, hmoves, curmovecap, jmlcap, sc, mlcap)) {
                return_value = pas::sub(pas::negate(petir_header::_nilai_raja), rlevel);
                return return_value;
            }
            if ((cext > 0)) {
                petir_search::stack[depth].ext = true;
                if ((jmlcap == 1)) {
                    (newlevel = pas::increment(newlevel, petir_header::single_reply_extension));
                    (cext = pas::decrement(cext, petir_header::single_reply_extension));
                } else {
                    if ((jmlcap == 2)) {
                        (newlevel = pas::increment(newlevel, petir_header::two_reply_extension));
                        (cext = pas::decrement(cext, petir_header::two_reply_extension));
                    }
                }
                if ((sc == 2)) {
                    (newlevel = pas::increment(newlevel, petir_header::double_skak_extension));
                    (cext = pas::decrement(cext, petir_header::double_skak_extension));
                }
                (newlevel = pas::increment(newlevel, petir_header::skak_extension));
                (cext = pas::decrement(cext, petir_header::skak_extension));
                ext = true;
            }
        }
        if ((moves == petir_header::_no_move)) {
            break;
        }
        sto = (pas::shr(moves, 7) & 127);
        if ((sto <= 63)) {
            cp = data.papan[sto];
        } else {
            if ((sto == petir_header::_en_passant)) {
                cp = 1;
            } else {
                cp = 0;
            }
        }
        if (((cp == 0) && ((pas::shr(moves, 14) & 7) != 0))) {
            cp = 1;
        }
        if ((giliran == petir_header::_sisiputih)) {
            petir_makemove::makewhitemove(moves, data);
            if (petir_tools::white_checked(data)) {
                data = temp;
                continue;
            }
        } else {
            petir_makemove::makeblackmove(moves, data);
            if (petir_tools::black_checked(data)) {
                data = temp;
                continue;
            }
        }
        (legalmove = pas::increment(legalmove, 1));
        petir_search::stack[depth].fase = fase;
        petir_search::stack[depth].legalmove = legalmove;
        petir_search::stack[depth].mte = false;
        h2 = data.hashkey;
        if ((petir_repetition::addrephash(data.hashkey, pas::sub(2, giliran)) == 2)) {
            nilai = 0;
            draw = true;
            goto search2;
        }
        petir_search::curr_move[depth] = moves;
        if (((((!ext) && (sto <= 63)) && (pas::abs(data.papan[sto]) == petir_header::_pionputih)) && petir_tools::isextension(data, newlevel, giliran, moves, cext))) {
            petir_search::stack[depth].ext = true;
            goto search1;
        }
        search1: ;
        if ((legalmove == 1)) {
            nilai = pas::negate(petir_search::mainsearch(newlevel, pas::sub(3, giliran), pas::negate(t_beta), pas::negate(t_alpha), data, nullmove, pas::add(depth, 1), cext, false, 0, petir_search::node_pv));
        } else {
            nilai = pas::negate(petir_search::mainsearch(newlevel, pas::sub(3, giliran), pas::negate(t_beta), pas::negate(t_alpha), data, nullmove, pas::add(depth, 1), cext, false, 0, petir_search::node_cut));
        }
        tnilai = nilai;
        search2: ;
        if (petir::search_should_stop(true)) {
            petir_repetition::delrephash(h2, pas::sub(2, giliran));
            data = temp;
            return return_value;
        }
        lmoves = petir_header::mmoves;
        if ((tipe == petir_header::_nodemax)) {
            if ((((((!petir_header::use_mtdf) && (nilai > t_alpha)) && (nilai < beta)) && (!first)) && (!draw))) {
                petir_header::mmoves = moves;
                if (petir_header::runningtest) {
                    pas_temp_1 = 5;
                    for (sc = 1; sc <= pas_temp_1; ++sc) {
                        if ((moves == petir_header::testanswer[sc])) {
                            petir_header::testsolved = true;
                        }
                    }
                }
                nilai = pas::negate(petir_search::mainsearch(newlevel, pas::sub(3, giliran), pas::negate(beta), pas::negate(t_alpha), data, nullmove, pas::add(depth, 1), cext, false, 0, petir_search::node_pv));
                if (petir::search_should_stop(true)) {
                    petir_repetition::delrephash(h2, pas::sub(2, giliran));
                    data = temp;
                    return return_value;
                }
                if ((nilai < tnilai)) {
                    petir_header::mmoves = lmoves;
                }
            }
            if ((nilai > g)) {
                g = nilai;
                if ((nilai > t_alpha)) {
                    if ((!petir_header::use_mtdf)) {
                        spv = petir_pv::getpv(data, pas::sub(3, giliran), pas::add(rlevel, 3));
                        spv = ((petir_notation::movetonotation(moves, temp.ep, giliran) + ' ') + spv);
                        petir_pv::printpv(petir_header::barisitr, spv, rlevel, pas::floor((static_cast<double>(pas::sub(pas::gettickcount(), petir_header::startclock)) / (1000))), nilai);
                    }
                    if (petir_header::runningtest) {
                        pas_temp_2 = 5;
                        for (sc = 1; sc <= pas_temp_2; ++sc) {
                            if ((moves == petir_header::testanswer[sc])) {
                                petir_header::testsolved = true;
                            }
                        }
                    }
                    if ((lmoves != moves)) {
                        if ((((petir_header::modetime == petir_header::moves_minutes) || (petir_header::modetime == petir_header::minutes_game_increment)) && (eext <= 6))) {
                            petir::search_time.adjust_soft(0.1);
                            (eext = pas::increment(eext, 1));
                        }
                    }
                    petir_header::mmoves = moves;
                    if ((petir_header::mmoves != petir_root::bmoves)) {
                        petir_root::bmoves = petir_header::_no_move;
                    }
                    t_alpha = nilai;
                    if ((g >= beta)) {
                        if ((cp == 0)) {
                            // Castle codes 65/66 are valid history indices, but
                            // not board squares. Read the piece from its source.
                            petir_search::addhistory(rlevel, depth, moves, giliran, temp.papan[moves & 127]);
                        }
                        (petir_header::totalco = pas::increment(petir_header::totalco, 1));
                        if (first) {
                            (petir_header::cofirst = pas::increment(petir_header::cofirst, 1));
                        }
                        petir_repetition::delrephash(h2, pas::sub(2, giliran));
                        data = temp;
                        return_value = g;
                        break;
                    }
                }
            }
            if ((!petir_header::use_mtdf)) {
                t_beta = pas::add(t_alpha, 1);
            }
        }
        first = false;
        petir_repetition::delrephash(h2, pas::sub(2, giliran));
        data = temp;
    } while (!(finish));
    if ((legalmove == 0)) {
        return_value = petir_header::contemp_draw;
        g = return_value;
        petir_header::stalemate = true;
    }
    return_value = g;
    if ((petir_header::mmoves == petir_header::_no_move)) {
        petir_header::mmoves = hmoves;
    }
    if (petir::search_should_stop(true)) {
        return return_value;
    }
    if ((g <= alpha)) {
        petir_hashing::addtable(h, giliran, level, petir_header::mmoves, g, petir_header::fail_low, data.move50count);
    } else {
        if ((g >= beta)) {
            petir_hashing::addtable(h, giliran, level, petir_header::mmoves, g, petir_header::fail_high, data.move50count);
        } else {
            petir_hashing::addtable(h, giliran, level, petir_header::mmoves, g, petir_header::pv_node, data.move50count);
        }
    }
    return return_value;
}

}
