// Faithful translation of pv.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_pv {
// pv.pas:52
pas::String getpv(petir_header::tdata data, std::int32_t giliran, std::int32_t maxdepth) {
    pas::String return_value{};
    std::int32_t nilai{};
    std::int32_t hmoves{};
    std::int32_t l{};
    bool ab{};
    bool pp{};
    bool found{};
    std::int32_t a{};
    std::int32_t b{};
    hmoves = petir_header::_no_move;
    return_value = pas::String("");
    a = pas::add(petir_header::_infinity, 50000);
    b = 0;
    if ((!petir_hashing::searchhash(data.hashkey, a, b, 0, giliran, nilai, hmoves, found, data.move50count))) {
        hmoves = petir_header::_no_move;
    }
    l = 0;
    while ((hmoves != petir_header::_no_move)) {
        (l = pas::increment(l, 1));
        return_value = (return_value + petir_notation::movetonotation(hmoves, data.ep, giliran));
        if ((l >= pas::add(maxdepth, 1))) {
            return return_value;
        }
        if ((giliran == petir_header::_sisihitam)) {
            petir_makemove::makeblackmove(hmoves, data);
        } else {
            petir_makemove::makewhitemove(hmoves, data);
        }
        giliran = pas::sub(3, giliran);
        if ((!petir_hashing::searchhash(data.hashkey, a, b, 0, giliran, nilai, hmoves, found, data.move50count))) {
            hmoves = petir_header::_no_move;
        }
        return_value = (return_value + ' ');
    }
    return return_value;
}

// pv.pas:24
void printpv(std::int32_t& n, pas::String spv, std::int32_t l, std::int32_t t, std::int32_t nilai3) {
    pas::String s{};
    if ((!petir_header::usewinboard)) {
    } else {
        if (petir_header::usewinboard) {
            s = pas::inttostr(l) + ' ' + pas::inttostr(nilai3) + ' ' + pas::inttostr(static_cast<std::int32_t>(petir::search_time.elapsed_ms()/10)) + ' ' + pas::inttostr(petir_header::total_node) + ' ' + spv;
            ;
            pas::send_winboard(s);
        }
    }
}

// pv.pas:12
pas::String strtime(std::int32_t time) {
    pas::String return_value{};
    pas::String st1{};
    pas::String st2{};
    pas::String st3{};
    st1 = pas::inttostr((time / 3600));
    if ((pas::strtoint(st1) < 10)) {
        st1 = ('0' + st1);
    }
    st2 = pas::inttostr(((time % 3600) / 60));
    if ((pas::strtoint(st2) < 10)) {
        st2 = ('0' + st2);
    }
    st3 = pas::inttostr((time % 60));
    if ((pas::strtoint(st3) < 10)) {
        st3 = ('0' + st3);
    }
    return_value = ((((st1 + ':') + st2) + ':') + st3);
    return return_value;
}

}
