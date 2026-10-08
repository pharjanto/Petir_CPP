// Faithful translation of uiterative.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_uiterative {
std::int32_t retime{};
std::int32_t completed_move{},completed_depth{},completed_score{};
std::string completed_pv;
// uiterative.pas:145
std::int32_t iterative(std::int32_t level, std::int32_t giliran, std::int32_t tipe, petir_header::tdata data) {
    std::int32_t return_value{};
    std::int32_t l{};
    std::int32_t nilai{};
    std::int32_t nilai2{};
    std::int32_t nilai3{};
    std::int32_t a{};
    std::int32_t b{};
    std::uint8_t dummy{};
    std::int32_t lasttime{};
    bool anu{};
    std::int32_t lastmoves{};
    std::int32_t counter{};
    level=std::clamp(level,1,49);
    l = 1;
    completed_move=completed_depth=completed_score=0;completed_pv.clear();
    const auto interrupted=[] {return petir_header::stop_process.load() || petir_header::timepassed.load()>=petir_header::timelimit;};
    const auto complete=[&](int d,int score) {
        completed_move=petir_header::mmoves;completed_depth=d;completed_score=score;
        const int search_nodes=petir_header::total_node;
        try {completed_pv=petir_pv::getpv(data,giliran,d+(petir_header::use_mtdf?3:4)).value;}
        catch(...) {petir_header::total_node=search_nodes;throw;}
        petir_header::total_node=search_nodes;
        const auto best=petir_notation::movetonotation(completed_move,data.ep,giliran).value;
        // The root move is authoritative even if a TT child was replaced.
        const auto space=completed_pv.find(' ');
        completed_pv=best+(space==std::string::npos?"":completed_pv.substr(space));
    };
    const auto restore=[] {petir_header::mmoves=completed_move;return completed_score;};
    petir_root::rtl = petir_header::timelimit;
    petir_header::maxdepth = pas::round((l * 2.00));
    nilai3 = 0;
    petir_header::barisitr = 1;
    anu = false;
    petir_uiterative::retime = petir_header::timelimit;
    petir_root::ntimelimit = (petir_header::timelimit / 3);
    petir_header::maxext = 1.00;
    if (petir_uiterative::timemanage(giliran, data)) {
        completed_move=petir_header::mmoves;
        return return_value;
    }
    if ((!petir_header::use_mtdf)) {
        petir_header::toplevel = l;
        nilai = petir_root::searchroot(pas::mul(l, petir_header::fractional_ply), giliran, petir_header::_nodemax, pas::sub(pas::negate(petir_header::_infinity), 500), pas::add(petir_header::_infinity, 500), data, false, 0, pas::round(((petir_header::maxext * petir_header::toplevel) * petir_header::fractional_ply)));
    } else {
        petir_header::toplevel = l;
        nilai = petir_uiterative::mtdf(data, pas::mul(l, petir_header::fractional_ply), giliran, 0);
    }
    if(interrupted())return restore();
    complete(l,nilai);
    if (petir_header::testsolved) {
        return return_value;
    }
    lastmoves = petir_header::mmoves;
    counter = 0;
    while (l<level && !interrupted()) {
        (l = pas::increment(l, 1));
        petir_header::toplevel = l;
        petir_header::maxdepth = pas::round(pas::mul(l, 2));
        lasttime = petir_header::timepassed;
        petir_root::newpv = true;
        if ((!petir_header::use_mtdf)) {
            nilai3 = petir_uiterative::aspiration(pas::mul(l, petir_header::fractional_ply), giliran, tipe, nilai2, data);
        } else {
            nilai3 = petir_uiterative::mtdf(data, pas::mul(l, petir_header::fractional_ply), giliran, nilai2);
        }
        if ((petir_header::stop_process || (petir_header::timepassed >= petir_header::timelimit))) {
            return restore();
        }
        complete(l,nilai3);
        if ((((nilai3 < 0) && (nilai >= 10)) && (!anu))) {
            (petir_header::timelimit = pas::increment(petir_header::timelimit, (petir_root::rtl / 2)));
            anu = true;
        }
        if ((((lastmoves == petir_header::mmoves) && petir_root::newpv) && (l > 7))) {
            (counter = pas::increment(counter, 1));
            if ((((counter >= 2) && (counter <= 12)) && (nilai3 > pas::negate(20)))) {
                (petir_header::timelimit = pas::decrement(petir_header::timelimit, pas::round((petir_header::timelimit * (static_cast<double>(petir_uiterative::timereducement[counter]) / (100))))));
            }
        } else {
            if ((counter >= 4)) {
                (petir_header::timelimit = pas::increment(petir_header::timelimit, (petir_root::rtl / 8)));
            }
            counter = 0;
            lastmoves = petir_header::mmoves;
        }
        if (petir_header::testsolved) {
            return return_value;
        }
        lasttime = pas::sub(petir_header::timepassed, lasttime);
        nilai2 = nilai;
        nilai = nilai3;
    }
    petir_header::toplevel = level;
    return_value = nilai;
    return return_value;
}

// uiterative.pas:14
std::int32_t mtdf(petir_header::tdata data, std::uint8_t level, std::uint8_t giliran, std::int32_t firstguess) {
    std::int32_t return_value{};
    std::int32_t oldg{};
    std::int32_t beta{};
    std::int32_t g{};
    double momentum{};
    pas::String spv{};
    std::int32_t oldmoves{};
    std::int32_t globalupper{};
    std::int32_t globallower{};
    bool up{};
    bool down{};
    std::uint8_t dummy{};
    momentum = 1.5;
    g = firstguess;
    globalupper = pas::add(petir_header::_infinity, 100);
    globallower = pas::sub(pas::negate(petir_header::_infinity), 100);
    beta = g;
    oldmoves = 0;
    up = false;
    down = false;
    do {
        oldg = g;
        g = petir_root::searchroot(level, giliran, petir_header::_nodemax, pas::sub(beta, 1), beta, data, false, 0, pas::round((petir_header::maxext * level)));
        if(petir_header::stop_process || petir_header::timepassed>=petir_header::timelimit)return g;
        if ((g < beta)) {
            globalupper = g;
            beta = g;
            if ((!up)) {
                beta = pas::add(beta, pas::round((momentum * pas::sub(g, oldg))));
            }
            down = true;
        } else {
            globallower = g;
            beta = pas::add(g, 1);
            if ((!down)) {
                beta = pas::add(beta, pas::round((momentum * pas::sub(g, oldg))));
            }
            if ((petir_header::mmoves != oldmoves)) {
                spv = petir_pv::getpv(data, giliran, pas::add((level / petir_header::fractional_ply), 3));
                petir_pv::printpv(petir_header::barisitr, spv, (level / petir_header::fractional_ply), pas::floor((static_cast<double>(pas::sub(pas::gettickcount(), petir_header::startclock)) / (1000))), g);
            }
            up = true;
            oldmoves = petir_header::mmoves;
        }
    } while (!((globallower >= globalupper)));
    petir_header::mmoves = oldmoves;
    return_value = g;
    return return_value;
}

// uiterative.pas:68
std::int32_t aspiration(std::int32_t level, std::int32_t giliran, std::int32_t tipe, std::int32_t nilai, petir_header::tdata data) {
    std::int32_t return_value{};
    std::int32_t n{};
    std::int32_t n2{};
    std::int32_t d{};
    std::int32_t d2{};
    std::int32_t z{};
    d = 35;
    d2 = 100;
    z = 80;
    n = petir_root::searchroot(level, giliran, petir_header::_nodemax, pas::sub(nilai, d), pas::add(nilai, d), data, false, 0, z);
    if(petir_header::stop_process || petir_header::timepassed>=petir_header::timelimit)return n;
    n2 = n;
    if (((n >= pas::add(nilai, d)) && (n < petir_header::_infinity))) {
        n = petir_root::searchroot(level, giliran, petir_header::_nodemax, n2, pas::add(petir_header::_infinity, 500), data, false, 0, z);
        if ((n < n2)) {
            n = petir_root::searchroot(level, giliran, petir_header::_nodemax, pas::sub(pas::negate(petir_header::_infinity), 500), pas::add(petir_header::_infinity, 500), data, false, 0, z);
        }
    } else {
        if ((n <= pas::sub(nilai, d))) {
            (petir_uiterative::retime = pas::increment(petir_uiterative::retime, (petir_root::rtl / 2)));
            n = petir_root::searchroot(level, giliran, petir_header::_nodemax, pas::sub(pas::negate(petir_header::_infinity), 500), n, data, false, 0, z);
            if ((n > n2)) {
                n = petir_root::searchroot(level, giliran, petir_header::_nodemax, pas::sub(pas::negate(petir_header::_infinity), 500), pas::add(petir_header::_infinity, 500), data, false, 0, z);
            }
        }
    }
    return_value = n;
    return return_value;
}

// uiterative.pas:93
bool timemanage(std::int32_t giliran, petir_header::tdata data) {
    bool return_value{};
    bool incheck{};
    petir_header::tmovelist ml{};
    std::int32_t jml{};
    std::int32_t b{};
    std::int32_t a{};
    std::int32_t c{};
    bool cappion{};
    std::int32_t pas_temp_1{};
    incheck = false;
    petir_root::bmoves = petir_header::_no_move;
    cappion = false;
    return_value = false;
    if ((((giliran == petir_header::_sisiputih) && petir_tools::white_checked(data)) || ((giliran == petir_header::_sisihitam) && petir_tools::black_checked(data)))) {
        incheck = true;
    }
    if (incheck) {
        if ((giliran == petir_header::_sisiputih)) {
            petir_movgen::white_evasion(data, ml, jml, b);
        } else {
            petir_movgen::black_evasion(data, ml, jml, b);
        }
        if ((jml == 1)) {
            petir_header::mmoves = ml[1].moves;
            return_value = true;
            return return_value;
        }
    } else {
        if ((giliran == petir_header::_sisiputih)) {
            petir_movgen::white_movgen_caps(data, ml, jml);
        } else {
            petir_movgen::black_movgen_caps(data, ml, jml);
        }
        a = 0;
        pas_temp_1 = jml;
        for (b = 1; b <= pas_temp_1; ++b) {
            if ((ml[b].score > 1)) {
                (a = pas::increment(a, 1));
                c = a;
            } else {
                if ((ml[b].score > 0)) {
                    cappion = true;
                }
            }
        }
        ;
        if ((petir_header::timelimit < 2)) {
            petir_header::timelimit = 2;
        }
    }
    return return_value;
}

}
