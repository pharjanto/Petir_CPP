// Quiescence search from search.pas; Delphi source is in original/.
#include "petir.hpp"
#include "order_stats.hpp"
namespace petir_search {
namespace {
// Called only when the side to move is not in check. Prove a legal move exists
// before stand pat, without generating every quiet move in the common case.
bool has_legal_noncheck_move(petir_header::tdata& data, int side, petir_header::tmovelist& moves) {
    const bool white=side==petir_header::_sisiputih;
    auto pushes=(white?data.pawnwhite<<8:data.pawnblack>>8)&~data.allpieces;
    while(pushes) {
        const int to=petir_bitboard_mask::firstbitp(pas::address(pushes));
        pushes&=pushes-1;
        const int from=to+(white?-8:8);
        const auto move=static_cast<std::uint16_t>(from|(to<<7));
        if(!(white?petir_tools::white_open_check(data,move):petir_tools::black_open_check(data,move)))return true;
    }
    auto legal=[&](int move) {
        auto child=data;
        const auto nodes=petir_header::total_node;
        if(white)petir_makemove::makewhitemove(move,child);else petir_makemove::makeblackmove(move,child);
        petir_header::total_node=nodes;
        return !(white?petir_tools::white_checked(child):petir_tools::black_checked(child));
    };
    auto king=white?data.kingwhite:data.kingblack;
    const int from=petir_bitboard_mask::firstbitp(pas::address(king));
    auto targets=petir_bitboard_mask::kingmask[from]&~(white?data.whitepieces:data.blackpieces);
    while(targets) {
        const int to=petir_bitboard_mask::firstbitp(pas::address(targets));
        targets&=targets-1;
        // In a nonchecked position the old king square cannot shield an attack
        // on an adjacent destination, so an attack test suffices here.
        if(!(white?petir_tools::white_attacked(data,to):petir_tools::black_attacked(data,to)))return true;
    }
    int count=0;
    if(white)petir_movgen::white_movgen_caps(data,moves,count);else petir_movgen::black_movgen_caps(data,moves,count);
    for(int i=1;i<=count;++i)if(legal(moves[i].moves))return true;
    if(white)petir_movgen::white_movgen_noncaps(data,moves,count,0);else petir_movgen::black_movgen_noncaps(data,moves,count);
    for(int i=1;i<=count;++i)if(legal(moves[i].moves))return true;
    return false;
}
}
// search.pas:982
std::int32_t q_search(std::int32_t giliran, std::int32_t alpha, std::int32_t beta, petir_header::tdata& data, std::int32_t ply, std::int32_t depth, std::int32_t cext, std::int32_t neval, std::int32_t nodetype) {
    std::int32_t return_value{};
    petir_header::tdata temp{};
    petir_header::tmovelist ml;
    std::uint8_t tstat{};
    bool incheck{};
    std::uint64_t pin_mask{};
    std::int32_t v{};
    std::int32_t tempnm{};
    std::int32_t sc{};
    std::int32_t t_alpha{};
    std::int32_t t_beta{};
    std::int32_t g{};
    std::int32_t a{};
    std::int32_t jml{};
    std::int32_t nilai{};
    std::int32_t e{};
    std::int32_t sto{};
    std::int32_t sfrom{};
    bool abcd{};
    std::int32_t pas_temp_1{};
    std::int32_t pas_temp_2{};
    std::int32_t pas_temp_4{};
// search.pas:991
    auto cap_is_danger = [&]() -> bool {
    bool return_value{};
    if ((giliran == petir_header::_sisiputih)) {
        if ((data.papan[sto] == petir_header::_mentrihitam)) {
            return_value = true;
            return return_value;
        }
        if ((((ml[a].score > 0) && ((petir_bitboard_mask::kingmask[sto] & data.kingblack) != 0)) && (data.queenwhite != 0))) {
            return_value = true;
            return return_value;
        }
        if (((data.papan[(ml[a].moves & 127)] == 1) && (sto >= petir_header::a7))) {
            return_value = true;
            return return_value;
        }
        if (((data.papan[sto] == pas::negate(1)) && (sto <= petir_header::h2))) {
            return_value = true;
            return return_value;
        }
    } else {
        if ((data.papan[sto] == petir_header::_mentriputih)) {
            return_value = true;
            return return_value;
        }
        if ((((ml[a].score > 0) && ((petir_bitboard_mask::kingmask[sto] & data.kingwhite) != 0)) && (data.queenblack != 0))) {
            return_value = true;
            return return_value;
        }
        if (((data.papan[(ml[a].moves & 127)] == pas::negate(1)) && (sto <= petir_header::h2))) {
            return_value = true;
            return return_value;
        }
        if (((data.papan[sto] == 1) && (sto >= petir_header::a7))) {
            return_value = true;
            return return_value;
        }
    }
    return_value = false;
    return return_value;
};

    // Portability guards: stop safely and keep the original 64-ply arrays in bounds.
    if (petir::search_should_stop()) return 0;
    if ((giliran == petir_header::_sisiputih)) {
        incheck = petir_tools::white_checked(data);
    } else {
        incheck = petir_tools::black_checked(data);
    }
    if ((!incheck)) {
        // Stalemate is terminal even when a cached evaluation would fail high.
        if(!has_legal_noncheck_move(data,giliran,ml))return petir_header::contemp_draw;
        if(depth>=63)return petir_ueval::eval(data,giliran,alpha,beta);
        if ((neval == petir_search::noval)) {
            e = petir_ueval::eval(data, giliran, alpha, beta);
        } else {
            e = neval;
        }
        if ((e >= beta)) {
            return_value = e;
            return return_value;
        }
        if ((e > alpha)) {
            alpha = e;
        }
    }
    if ((giliran == petir_header::_sisiputih)) {
        if (incheck) {
            petir_movgen::white_evasion(data, ml, jml, sc);
            pas_temp_1 = jml;
            if (((pas_temp_1 == 1))) {
                (ply = pas::increment(ply, 1));
            }
            else if (((pas_temp_1 == 0))) {
                return_value = pas::add(pas::negate(petir_header::_nilai_raja), depth);
                return return_value;
            }
        } else {
            if ((ply > 0)) {
                petir_movgen::q_white_movgen(data, ml, jml);
            } else {
                petir_movgen::white_movgen_caps2(data, ml, jml);
            }
        }
    } else {
        if (incheck) {
            petir_movgen::black_evasion(data, ml, jml, sc);
            pas_temp_2 = jml;
            if (((pas_temp_2 == 1))) {
                (ply = pas::increment(ply, 1));
            }
            else if (((pas_temp_2 == 0))) {
                return_value = pas::add(pas::negate(petir_header::_nilai_raja), depth);
                return return_value;
            }
        } else {
            if ((ply > 0)) {
                petir_movgen::q_black_movgen(data, ml, jml);
            } else {
                petir_movgen::black_movgen_caps2(data, ml, jml);
            }
        }
    }
    if(depth>=63)return petir_ueval::eval(data,giliran,alpha,beta);
    temp = data;
    if (!incheck && jml == 0) {
        return e;
    }
    if ((!incheck)) {
        g = e;
    } else {
        g = pas::sub(pas::negate(petir_header::_infinity), 100);
    }
    t_alpha = alpha;
    t_beta = beta;
    pin_mask = incheck?0:petir_tools::pinned_pieces(data,giliran);
    pas_temp_4 = jml;
    PETIR_ORDER_STAT(int ostat_searched = 0;)
    for (a = 1; a <= pas_temp_4; ++a) {
        if(petir::search_should_stop())return 0;
        if ((a != jml)) {
            petir_next::nextbest_max(ml, a, jml);
        }
        abcd = false;
        sto = (pas::shr(ml[a].moves, 7) & 127);
        if ((!incheck)) {
            if ((ml[a].score < 0)) {
                break;
            }
            if ((((alpha == pas::sub(beta, 1)) && (sto <= 63)) && (data.papan[sto] != 0))) {
                v = pas::add(pas::add(e, petir_header::nilai_piece[data.papan[sto]]), 70);
                if (((((v < t_alpha) && (!petir_tools::menskak2(temp, giliran, ml[a].moves))) && (!petir_tools::pawn_7_rank(giliran, data))) && ((pas::shr(ml[a].moves, 14) & 7) == 0))) {
                    if ((!cap_is_danger())) {
                        if ((v > g)) {
                            g = v;
                        }
                        continue;
                    }
                }
            }
            sfrom = (ml[a].moves & 127);
            if ((giliran == petir_header::_sisiputih)) {
                if (((data.papan[sfrom] == petir_header::_rajaputih) && petir_tools::white_attacked(data, sto))) {
                    continue;
                } else {
                    if ((pin_mask & petir_bitboard_mask::bit(sfrom)) && petir_tools::white_open_check(data, ml[a].moves)) {
                        continue;
                    }
                }
            } else {
                if (((data.papan[sfrom] == petir_header::_rajahitam) && petir_tools::black_attacked(data, sto))) {
                    continue;
                } else {
                    if ((pin_mask & petir_bitboard_mask::bit(sfrom)) && petir_tools::black_open_check(data, ml[a].moves)) {
                        continue;
                    }
                }
            }
        }
        if ((giliran == petir_header::_sisiputih)) {
            petir_makemove::makewhitemove(ml[a].moves, data);
        } else {
            petir_makemove::makeblackmove(ml[a].moves, data);
        }
        if ((sto == petir_header::_en_passant)) {
            if ((((giliran == petir_header::_sisiputih) && petir_tools::white_checked(data)) || ((giliran == petir_header::_sisihitam) && petir_tools::black_checked(data)))) {
                data = temp;
                continue;
            }
        }
        PETIR_ORDER_STAT(++ostat_searched;)
        nilai = pas::negate(petir_search::q_search(pas::sub(3, giliran), pas::negate(t_beta), pas::negate(t_alpha), data, pas::sub(ply, 1), pas::add(depth, 1), cext, petir_search::noval, pas::negate(nodetype)));
        if(petir::search_should_stop()) {data=temp;return 0;}
        if ((nilai >= beta)) {
            PETIR_ORDER_STAT(petir_order_stats::q_cutoff(incheck, ostat_searched);)
            data = temp;
            return_value = nilai;
            return return_value;
        }
        if ((nilai > g)) {
            g = nilai;
            if ((nilai > t_alpha)) {
                t_alpha = nilai;
            }
        }
        data = temp;
    }
    PETIR_ORDER_STAT(petir_order_stats::q_done(incheck, ostat_searched);)
    return_value = g;
    return return_value;
}

}
