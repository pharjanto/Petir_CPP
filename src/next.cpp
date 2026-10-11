// Faithful translation of next.pas; Delphi source is in original/.
#include "petir.hpp"
#include <algorithm>
namespace petir_next {
std::int32_t t1 = 0;
// next.pas:54
void nextbest_max(petir_header::tmovelist& a, std::int32_t cur, std::int32_t tot) {
    auto* current = &a[cur];
    if(tot<=cur)return;
    // Checked endpoints establish a valid range without a capacity test for
    // every score. A branch-free maximum, then a scan for its first entry,
    // selects the same record as the first-maximum rule, preserving ties.
    auto* const end = &a[tot]+1;
    int best_score = current->score;
    for(auto* entry = current+1; entry != end; ++entry) best_score = std::max(best_score, entry->score);
    auto* best = current;
    while(best->score != best_score) ++best;
    if(best!=current)std::swap(*current,*best);
}

// next.pas:113
bool nextevasion(std::int32_t giliran, petir_header::tdata& data, std::int32_t& moves, std::int32_t& fase, std::int32_t& hmoves, std::int32_t& curmove, std::int32_t& jml, std::int32_t& skakcount, petir_header::tmovelist& ml) {
    bool return_value{};
    std::int32_t a{};
    std::int32_t pas_temp_1{};
    return_value = false;
    if ((fase == petir_header::_hashmove)) {
        if ((curmove == 0)) {
            if ((giliran == petir_header::_sisiputih)) {
                petir_movgen::white_evasion(data, ml, jml, skakcount);
            } else {
                petir_movgen::black_evasion(data, ml, jml, skakcount);
            }
            if ((jml == 0)) {
                return_value = true;
                return return_value;
            }
            pas_temp_1 = jml;
            for (a = 1; a <= pas_temp_1; ++a) {
                if ((ml[a].moves == hmoves)) {
                    (ml[a].score = pas::increment(ml[a].score, 87000000));
                    break;
                }
            }
        }
        (curmove = pas::increment(curmove, 1));
        if ((curmove > jml)) {
            moves = petir_header::_no_move;
            return return_value;
        } else {
            petir_next::nextbest_max(ml, curmove, jml);
            moves = ml[curmove].moves;
            return return_value;
        }
    }
    habis: ;
    if ((fase == pas::add(petir_header::_hashmove, 2))) {
        moves = petir_header::_no_move;
        return return_value;
    }
    return return_value;
}

// Capture scores from the generators are the expected gain in pawns, so many
// tie (pawn takes knight = knight takes rook). Keep the gain in the high bits
// and, among equal gains, take the bigger victim and then the cheaper attacker
// first. Knight underpromotions go to the bad captures. The gain is
// score >> cap_gain_shift.
static constexpr int cap_gain_shift = 6;
static void rescore_captures(const petir_header::tdata& data, petir_header::tmovelist& mlcap, std::int32_t jmlcap) {
    for (std::int32_t a = 1; a <= jmlcap; ++a) {
        const std::int32_t m = mlcap[a].moves;
        std::int32_t gain = mlcap[a].score;
        if (((pas::shr(m, 14) & 7) == petir_header::promosi_kuda)) {
            gain = -1;
        }
        const std::int32_t sto = (pas::shr(m, 7) & 127);
        const std::int32_t victim = (sto == petir_header::_en_passant) ? petir_header::_pionputih : pas::abs(data.papan[sto]);
        const std::int32_t attacker = pas::abs(data.papan[(m & 127)]);
        mlcap[a].score = (gain << cap_gain_shift) + (victim << 3) + (7 - attacker);
    }
}

// next.pas:155
bool nextmove(std::int32_t giliran, std::int32_t depth, petir_header::tdata& data, std::int32_t& moves, std::int32_t& fase, std::int32_t hmoves, std::int32_t& curmovecap, std::int32_t& curmovenoncap, std::int32_t& jmlcap, std::int32_t& jmlnoncap, petir_header::tmovelist& ml, petir_header::tmovelist& mlcap, std::int32_t& goodcap, petir_header::piece_to_history* const* cont) {
    bool return_value{};
    std::int32_t a{};
    std::int32_t killer0moves{};
    std::int32_t killer1moves{};
    std::int32_t nilai{};
    std::int32_t killer0moves2{};
    std::int32_t killer1moves2{};
    bool found{};
    bool sudahada{};
    std::int32_t pas_temp_1{};
    std::int32_t pas_temp_2{};
    std::int32_t pas_temp_3{};
    std::int32_t pas_temp_4{};
    return_value = false;
    if ((fase == petir_header::_hashmove)) {
        (fase = pas::increment(fase, 1));
        if ((hmoves != 0)) {
            moves = hmoves;
            return return_value;
        }
    }
    if ((fase == petir_header::_goodcaps)) {
        if ((curmovecap == 0)) {
            if ((giliran == petir_header::_sisiputih)) {
                petir_movgen::white_movgen_caps(data, mlcap, jmlcap);
            } else {
                petir_movgen::black_movgen_caps(data, mlcap, jmlcap);
            }
            rescore_captures(data, mlcap, jmlcap);
        }
        (curmovecap = pas::increment(curmovecap, 1));
        if ((curmovecap > jmlcap)) {
            (fase = pas::increment(fase, 1));
            curmovenoncap = 0;
        } else {
            petir_next::nextbest_max(mlcap, curmovecap, jmlcap);
            if ((mlcap[curmovecap].moves == hmoves)) {
                (curmovecap = pas::increment(curmovecap, 1));
                if ((curmovecap > jmlcap)) {
                    (fase = pas::increment(fase, 1));
                    curmovenoncap = 0;
                    goto other;
                } else {
                    petir_next::nextbest_max(mlcap, curmovecap, jmlcap);
                }
            }
            if ((mlcap[curmovecap].score < 0)) {
                badcap: ;
                (fase = pas::increment(fase, 1));
                curmovenoncap = 0;
                goto other;
            }
            moves = mlcap[curmovecap].moves;
            const std::int32_t gain = (mlcap[curmovecap].score >> cap_gain_shift);
            if ((gain > goodcap)) {
                goodcap = gain;
            }
            return return_value;
        }
    }
    other: ;
    if ((fase == petir_header::_killermoves1)) {
        (fase = pas::increment(fase, 1));
        if (((petir_header::killer0[depth] != hmoves) && petir_valid::isvalid(data, petir_header::killer0[depth], giliran))) {
            pas_temp_1 = pas::sub(curmovecap, 1);
            for (a = 1; a <= pas_temp_1; ++a) {
                if ((petir_header::killer0[depth] == mlcap[a].moves)) {
                    goto killer11;
                }
            }
            moves = petir_header::killer0[depth];
            return return_value;
        }
    }
    killer11: ;
    // One killer per ply: after killer0 the quiet moves follow, ordered by
    // history.
    if (((fase >= petir_header::_killermoves2) && (fase <= petir_header::_killermoves4))) {
        fase = petir_header::_othermoves;
    }
    if ((fase == petir_header::_othermoves)) {
        if ((curmovenoncap == 0)) {
            if ((giliran == petir_header::_sisiputih)) {
                petir_movgen::white_movgen_noncaps(data, ml, jmlnoncap);
            } else {
                petir_movgen::black_movgen_noncaps(data, ml, jmlnoncap);
            }
            // Add how each quiet move did after the previous move and after
            // our move before it.
            if ((cont != nullptr) && ((cont[0] != nullptr) || (cont[1] != nullptr))) {
                for (a = 1; a <= jmlnoncap; ++a) {
                    const std::int32_t piece = data.papan[(ml[a].moves & 127)];
                    const std::int32_t sto = (pas::shr(ml[a].moves, 7) & 127);
                    for (std::int32_t i = 0; i < 2; ++i) {
                        if ((cont[i] != nullptr)) {
                            (ml[a].score = pas::increment(ml[a].score, (*cont[i])[piece][sto]));
                        }
                    }
                }
            }
        }
        (curmovenoncap = pas::increment(curmovenoncap, 1));
        if ((curmovenoncap > jmlnoncap)) {
            (fase = pas::increment(fase, 1));
        } else {
            if ((curmovenoncap <= 20)) {
                petir_next::nextbest_max(ml, curmovenoncap, jmlnoncap);
            }
            while (((ml[curmovenoncap].moves == hmoves) || (ml[curmovenoncap].moves == petir_header::killer0[depth]))) {
                (curmovenoncap = pas::increment(curmovenoncap, 1));
                if ((curmovenoncap > jmlnoncap)) {
                    (fase = pas::increment(fase, 1));
                    goto fase3;
                } else {
                    if ((curmovenoncap <= 20)) {
                        petir_next::nextbest_max(ml, curmovenoncap, jmlnoncap);
                    }
                }
            }
            moves = ml[curmovenoncap].moves;
            return return_value;
        }
    }
    fase3: ;
    if ((fase == petir_header::_badcaps)) {
        if ((curmovecap <= jmlcap)) {
            petir_next::nextbest_max(mlcap, curmovecap, jmlcap);
            while (((mlcap[curmovecap].moves == hmoves) || (mlcap[curmovecap].moves == petir_header::killer0[depth]))) {
                (curmovecap = pas::increment(curmovecap, 1));
                if ((curmovecap > jmlcap)) {
                    (fase = pas::increment(fase, 1));
                    goto fase4;
                }
                petir_next::nextbest_max(mlcap, curmovecap, jmlcap);
            }
            moves = mlcap[curmovecap].moves;
            (curmovecap = pas::increment(curmovecap, 1));
            return return_value;
        } else {
            (fase = pas::increment(fase, 1));
        }
    }
    fase4: ;
    if ((fase == petir_header::_finish)) {
        moves = petir_header::_no_move;
    }
    return return_value;
}

// next.pas:77
bool nextroot(std::int32_t giliran, std::int32_t& moves, std::int32_t& fase, std::int32_t& hmoves, std::int32_t& curmove, std::int32_t& jml, petir_header::tmovelist ml) {
    bool return_value{};
    if ((fase == petir_header::_hashmove)) {
        (fase = pas::increment(fase, 1));
        if (petir_valid::isvalid(petir_header::data, hmoves, giliran)) {
            moves = hmoves;
            return return_value;
        }
    }
    if ((fase == pas::add(petir_header::_hashmove, 1))) {
        (curmove = pas::increment(curmove, 1));
        if ((curmove > jml)) {
            moves = petir_header::_no_move;
            return return_value;
        }
        petir_next::nextbest_max(ml, curmove, jml);
        if ((ml[curmove].moves == hmoves)) {
            (curmove = pas::increment(curmove, 1));
            if ((curmove > jml)) {
                moves = petir_header::_no_move;
                return return_value;
            } else {
                petir_next::nextbest_max(ml, curmove, jml);
            }
        }
        moves = ml[curmove].moves;
        return return_value;
    }
    return return_value;
}

// next.pas:15
bool nextq(std::int32_t giliran, petir_header::tdata& data, std::int32_t& moves, std::int32_t& fase, std::int32_t& curmove, std::int32_t& jml, std::int32_t& score, std::int32_t ply, petir_header::tmovelist& ml) {
    bool return_value{};
    return_value = false;
    if ((fase == 0)) {
        if ((giliran == petir_header::_sisiputih)) {
            if ((ply > 0)) {
                petir_movgen::q_white_movgen(data, ml, jml);
            } else {
                petir_movgen::white_movgen_caps(data, ml, jml);
            }
        } else {
            if ((ply > 0)) {
                petir_movgen::q_black_movgen(data, ml, jml);
            } else {
                petir_movgen::black_movgen_caps(data, ml, jml);
            }
        }
        (fase = pas::increment(fase, 1));
        if ((jml == 0)) {
            return_value = true;
            return return_value;
        }
    }
    if ((fase == 1)) {
        if ((curmove > jml)) {
            moves = petir_header::_no_move;
            return return_value;
        }
        petir_next::nextbest_max(ml, curmove, jml);
        moves = ml[curmove].moves;
        score = ml[curmove].score;
        (curmove = pas::increment(curmove, 1));
        return return_value;
    }
    return return_value;
}

}
