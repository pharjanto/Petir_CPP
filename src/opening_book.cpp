// Faithful translation of opening_book.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_opening_book {
petir_opening_book::hashbook book{};
std::int32_t no = 0;
std::int32_t a{};
// opening_book.pas:441
void learn_pgn(pas::String a) {
    pas::File f{};
    pas::String s{};
    pas::String notasi{};
    pas::String s2{};
    std::uint8_t ma{};
    std::uint8_t mt{};
    std::uint8_t p{};
    std::uint8_t jml{};
    std::uint8_t spasi{};
    std::uint8_t lastspasi{};
    petir_header::tdata data{};
    bool err{};
    bool stop{};
    pas::String oldnotasi{};
    std::uint64_t k{};
    std::int32_t giliran{};
    std::int32_t whiteelo{};
    std::int32_t blackelo{};
    std::int32_t code{};
    std::uint8_t hasil{};
    pas::assignfile(f, a);
    pas::reset(f);
    giliran = petir_header::_sisiputih;
    jml = 0;
    stop = false;
    while ((!pas::eof(f))) {
        pas::readln(f, s);
        if ((s != pas::String(""))) {
            s = (s + ' ');
        }
        lastspasi = 1;
        if ((pas::uppercase(pas::copy(s, 1, 7)) == pas::uppercase(pas::String("[Result")))) {
            whiteelo = 0;
            blackelo = 0;
            if ((s[11] == '/')) {
                hasil = petir_header::result_draw;
            } else {
                if ((s[10] == '1')) {
                    hasil = petir_header::result_white_win;
                } else {
                    if ((s[10] == '0')) {
                        hasil = petir_header::result_black_win;
                    } else {
                        if ((s[10] == '*')) {
                            hasil = petir_header::result_draw;
                        } else {
                            pas::showmessage(pas::String("result error"));
                            return;
                        }
                    }
                }
            }
        } else {
            if ((pas::uppercase(pas::copy(s, 1, 9)) == pas::String("[WHITEELO"))) {
                s2 = pas::copy(s, 12, 4);
                pas::val(s2, whiteelo, code);
            } else {
                if ((pas::uppercase(pas::copy(s, 1, 9)) == pas::String("[BLACKELO"))) {
                    s2 = pas::copy(s, 12, 4);
                    pas::val(s2, blackelo, code);
                }
            }
        }
        if (((pas::length(s) > 0) && (s[1] != '['))) {
            if ((pas::copy(s, 1, 2) == pas::String("1."))) {
                petir_udata::initstate(data);
                giliran = petir_header::_sisiputih;
                jml = 0;
                stop = false;
                (petir_opening_book::no = pas::increment(petir_opening_book::no, 1));
            }
            while (((pas::pos(' ', s) != 0) && (!stop))) {
                spasi = pas::pos(' ', s);
                notasi = pas::copy(s, lastspasi, pas::sub(spasi, lastspasi));
                pas::p_delete(s, lastspasi, pas::add(pas::sub(spasi, lastspasi), 1));
                if ((!((notasi[1] >= '1' && notasi[1] <= '9')))) {
                    k = petir_hashing_header::hashvalue(data, giliran);
                    oldnotasi = notasi;
                    if ((notasi[pas::length(notasi)] == '+')) {
                        notasi = pas::copy(notasi, 1, pas::sub(pas::length(notasi), 1));
                    }
                    try {
                        petir_opening_book::make(notasi, data, giliran);
                        if ((((whiteelo >= petir_opening_book::minimum_elo) && (blackelo >= petir_opening_book::minimum_elo)) && (jml >= 20))) {
                            if ((!petir_opening_book::addhashbook(data.hashkey, hasil, giliran, whiteelo, blackelo))) {
                                return;
                            }
                        }
                        (jml = pas::increment(jml, 1));
                        if ((jml >= 34)) {
                            stop = true;
                        }
                    } catch (...) {
                        stop = true;
                    }
                    giliran = pas::sub(3, giliran);
                }
            }
        }
    }
    pas::closefile(f);
    pas::showmessage(pas::String("finish"));
    petir_opening_book::savebook();
}

// opening_book.pas:212
void initbook() {
    std::int32_t a{};
    std::int32_t b{};
    std::int32_t pas_temp_1{};
    std::int32_t pas_temp_2{};
    pas_temp_1 = 1;
    for (a = 0; a <= pas_temp_1; ++a) {
        pas_temp_2 = pas::sub(petir_opening_book::maxhashbook, 1);
        for (b = 0; b <= pas_temp_2; ++b) {
            petir_opening_book::book[a][b].used = false;
        }
    }
}

// opening_book.pas:373
void loadbook() {
    pas::File f{};
    pas::Array<petir_opening_book::tbook, 1, 10000> buf{};
    std::int32_t a{};
    std::int32_t numread{};
    std::int32_t pas_temp_1{};
    std::int32_t pas_temp_2{};
    if (pas::fileexists((pas::form1.defaultdir + pas::String("\\white.book")))) {
        pas::assignfile(f, (pas::form1.defaultdir + pas::String("\\white.book")));
        pas::reset(f, 1);
        if ((pas::ioresult() != 0)) {
            return;
        }
        do {
            pas::blockread(f, buf, sizeof(buf), numread);
            numread = (numread / sizeof(petir_opening_book::tbook));
            pas_temp_1 = numread;
            for (a = 1; a <= pas_temp_1; ++a) {
                petir_opening_book::no = numread;
                petir_opening_book::storebook(buf[a], petir_header::_sisiputih);
            }
        } while (!((numread == 0)));
        pas::closefile(f);
    }
    if (pas::fileexists((pas::form1.defaultdir + pas::String("\\black.book")))) {
        pas::assignfile(f, (pas::form1.defaultdir + pas::String("\\black.book")));
        pas::reset(f, 1);
        if ((pas::ioresult() != 0)) {
            return;
        }
        do {
            pas::blockread(f, buf, sizeof(buf), numread);
            numread = (numread / sizeof(petir_opening_book::tbook));
            pas_temp_2 = numread;
            for (a = 1; a <= pas_temp_2; ++a) {
                petir_opening_book::storebook(buf[a], petir_header::_sisihitam);
            }
        } while (!((numread == 0)));
        pas::closefile(f);
    }
}

// opening_book.pas:114
std::int32_t searchbook(petir_header::tdata data, std::int32_t giliran) {
    std::int32_t return_value{};
    petir_header::tdata temp{};
    petir_header::tmovelist ml{};
    std::int32_t jml{};
    std::int32_t n{};
    std::int32_t n2{};
    bool pp{};
    std::uint8_t s{};
    pas::Array<petir_opening_book::anonymous_record_1, 1, 20> m{};
    std::int32_t totalw{};
    std::int32_t w{};
    std::int32_t r{};
    std::int32_t pas_temp_1{};
    std::int32_t pas_temp_2{};
    std::int32_t pas_temp_3{};
    std::int32_t pas_temp_4{};
    std::int32_t pas_temp_5{};
    temp = data;
    n = 0;
    totalw = 0;
    return_value = petir_header::_no_move;
    pas::fillchar(m, sizeof(m), 0);
    if ((giliran == petir_header::_sisiputih)) {
        petir_movgen::white_movgen_noncaps(data, ml, jml);
        pas_temp_1 = jml;
        for (petir_opening_book::a = 1; petir_opening_book::a <= pas_temp_1; ++petir_opening_book::a) {
            data = temp;
            petir_makemove::makewhitemove(ml[petir_opening_book::a].moves, data);
            if (petir_tools::white_checked(data)) {
                continue;
            }
            if (petir_opening_book::searchhashbook(data.hashkey, giliran, w)) {
                (n = pas::increment(n, 1));
                m[n].moves = ml[petir_opening_book::a].moves;
                m[n].weight = w;
                (totalw = pas::increment(totalw, w));
            }
        }
        data = temp;
        petir_movgen::white_movgen_caps(data, ml, jml);
        pas_temp_2 = jml;
        for (petir_opening_book::a = 1; petir_opening_book::a <= pas_temp_2; ++petir_opening_book::a) {
            data = temp;
            petir_makemove::makewhitemove(ml[petir_opening_book::a].moves, data);
            if (petir_tools::white_checked(data)) {
                continue;
            }
            if (petir_opening_book::searchhashbook(data.hashkey, giliran, w)) {
                (n = pas::increment(n, 1));
                m[n].moves = ml[petir_opening_book::a].moves;
                m[n].weight = w;
                (totalw = pas::increment(totalw, w));
            }
        }
    } else {
        petir_movgen::black_movgen_noncaps(data, ml, jml);
        pas_temp_3 = jml;
        for (petir_opening_book::a = 1; petir_opening_book::a <= pas_temp_3; ++petir_opening_book::a) {
            data = temp;
            petir_makemove::makeblackmove(ml[petir_opening_book::a].moves, data);
            if (petir_tools::black_checked(data)) {
                continue;
            }
            if (petir_opening_book::searchhashbook(data.hashkey, giliran, w)) {
                (n = pas::increment(n, 1));
                m[n].moves = ml[petir_opening_book::a].moves;
                m[n].weight = w;
                (totalw = pas::increment(totalw, w));
            }
        }
        data = temp;
        petir_movgen::black_movgen_caps(data, ml, jml);
        pas_temp_4 = jml;
        for (petir_opening_book::a = 1; petir_opening_book::a <= pas_temp_4; ++petir_opening_book::a) {
            data = temp;
            petir_makemove::makeblackmove(ml[petir_opening_book::a].moves, data);
            if (petir_tools::black_checked(data)) {
                continue;
            }
            if (petir_opening_book::searchhashbook(data.hashkey, giliran, w)) {
                (n = pas::increment(n, 1));
                m[n].moves = ml[petir_opening_book::a].moves;
                m[n].weight = w;
                (totalw = pas::increment(totalw, w));
            }
        }
    }
    if ((n == 0)) {
        return_value = petir_header::_no_move;
        return return_value;
    } else {
        r = pas::add(pas::random(totalw), 1);
        w = 0;
        pas_temp_5 = n;
        for (petir_opening_book::a = 1; petir_opening_book::a <= pas_temp_5; ++petir_opening_book::a) {
            (w = pas::increment(w, m[petir_opening_book::a].weight));
            if ((w >= r)) {
                return_value = m[petir_opening_book::a].moves;
                return return_value;
            }
        }
    }
    return return_value;
}

// opening_book.pas:40
bool searchhashbook(std::uint64_t key, std::uint8_t giliran, std::int32_t& weight) {
    bool return_value{};
    std::uint64_t hkey{};
    std::uint64_t temp{};
    petir_opening_book::thashbook* p{};
    std::int32_t blackwin{};
    hkey = (key & pas::sub(petir_opening_book::maxhashbook, 1));
    temp = hkey;
    (giliran = pas::decrement(giliran, 1));
    while ((petir_opening_book::book[giliran][hkey].used && (petir_opening_book::book[giliran][hkey].book.key != key))) {
        (hkey = pas::increment(hkey, 1));
        if ((hkey > pas::sub(petir_opening_book::maxhashbook, 1))) {
            hkey = 0;
        }
        if ((hkey == temp)) {
            return_value = false;
            return return_value;
        }
    }
    p = pas::address(petir_opening_book::book[giliran][hkey]);
    if ((((*p).used == false) || ((*p).book.bobot < 4))) {
        return_value = false;
        return return_value;
    } else {
        return_value = true;
        (giliran = pas::increment(giliran, 1));
        if ((giliran == petir_header::_sisiputih)) {
            weight = (*p).book.bobot;
        } else {
            weight = (*p).book.bobot;
        }
        if ((weight <= 0)) {
            return_value = false;
        }
    }
    return return_value;
}

// opening_book.pas:77
bool searchhashbook2(std::uint64_t key, std::uint8_t giliran, std::int32_t& weight) {
    bool return_value{};
    std::uint64_t hkey{};
    std::uint64_t temp{};
    petir_opening_book::thashbook* p{};
    std::int32_t blackwin{};
    hkey = (key & pas::sub(petir_opening_book::maxhashbook, 1));
    temp = hkey;
    (giliran = pas::decrement(giliran, 1));
    while ((petir_opening_book::book[giliran][hkey].used && (petir_opening_book::book[giliran][hkey].book.key != key))) {
        (hkey = pas::increment(hkey, 1));
        if ((hkey > pas::sub(petir_opening_book::maxhashbook, 1))) {
            hkey = 0;
        }
        if ((hkey == temp)) {
            return_value = false;
            return return_value;
        }
    }
    p = pas::address(petir_opening_book::book[giliran][hkey]);
    if ((((*p).used == false) || ((*p).book.bobot < 4))) {
        return_value = false;
        return return_value;
    } else {
        return_value = true;
        (giliran = pas::increment(giliran, 1));
        if ((giliran == petir_header::_sisiputih)) {
            weight = (*p).book.bobot;
        } else {
            weight = (*p).book.bobot;
        }
    }
    return return_value;
}

// opening_book.pas:307
bool edithashbook(std::uint64_t key, std::uint8_t giliran, std::int32_t newbobot) {
    bool return_value{};
    std::uint64_t hkey{};
    std::uint64_t temp{};
    petir_opening_book::thashbook* p{};
    return_value = true;
    hkey = (key & pas::sub(petir_opening_book::maxhashbook, 1));
    temp = hkey;
    (giliran = pas::decrement(giliran, 1));
    while ((petir_opening_book::book[giliran][hkey].used && (petir_opening_book::book[giliran][hkey].book.key != key))) {
        (hkey = pas::increment(hkey, 1));
        if ((hkey > pas::sub(petir_opening_book::maxhashbook, 1))) {
            hkey = 0;
        }
        if ((hkey == temp)) {
            return_value = false;
            pas::showmessage(pas::String("hash penuh"));
            return return_value;
        }
    }
    p = pas::address(petir_opening_book::book[giliran][hkey]);
    (*p).book.bobot = newbobot;
    return return_value;
}

// opening_book.pas:414
void savebook() {
    std::int32_t a{};
    std::int32_t b{};
    pas::File f{};
    std::int32_t pas_temp_1{};
    std::int32_t pas_temp_2{};
    pas::assignfile(f, (pas::form1.defaultdir + pas::String("\\white.book")));
    pas::rewrite(f);
    pas_temp_1 = pas::sub(petir_opening_book::maxhashbook, 1);
    for (b = 0; b <= pas_temp_1; ++b) {
        if (petir_opening_book::book[pas::sub(petir_header::_sisiputih, 1)][b].used) {
            pas::write(f, petir_opening_book::book[pas::sub(petir_header::_sisiputih, 1)][b].book, sizeof(petir_opening_book::book[pas::sub(petir_header::_sisiputih, 1)][b].book));
        }
    }
    pas::closefile(f);
    pas::assignfile(f, (pas::form1.defaultdir + pas::String("\\black.book")));
    pas::rewrite(f);
    pas_temp_2 = pas::sub(petir_opening_book::maxhashbook, 1);
    for (b = 0; b <= pas_temp_2; ++b) {
        if (petir_opening_book::book[pas::sub(petir_header::_sisihitam, 1)][b].used) {
            pas::write(f, petir_opening_book::book[pas::sub(petir_header::_sisihitam, 1)][b].book, sizeof(petir_opening_book::book[pas::sub(petir_header::_sisihitam, 1)][b].book));
        }
    }
    pas::closefile(f);
}

// opening_book.pas:36
void learn() {
}

// opening_book.pas:222
bool addhashbook(std::uint64_t key, std::uint8_t hasil, std::uint8_t giliran, std::int32_t whiteelo, std::int32_t blackelo) {
    bool return_value{};
    std::uint64_t hkey{};
    std::uint64_t temp{};
    petir_opening_book::thashbook* p{};
    return_value = true;
    hkey = (key & pas::sub(petir_opening_book::maxhashbook, 1));
    temp = hkey;
    (giliran = pas::decrement(giliran, 1));
    while ((petir_opening_book::book[giliran][hkey].used && (petir_opening_book::book[giliran][hkey].book.key != key))) {
        (hkey = pas::increment(hkey, 1));
        if ((hkey > pas::sub(petir_opening_book::maxhashbook, 1))) {
            hkey = 0;
        }
        if ((hkey == temp)) {
            return_value = false;
            pas::showmessage(pas::String("hash penuh"));
            return return_value;
        }
    }
    p = pas::address(petir_opening_book::book[giliran][hkey]);
    (giliran = pas::increment(giliran, 1));
    if (((*p).used == false)) {
        (*p).used = true;
        (*p).book.key = key;
        (*p).book.bobot = 0;
        if ((hasil == petir_header::result_draw)) {
            ((*p).book.bobot = pas::increment((*p).book.bobot, 1));
        } else {
            if (((giliran == petir_header::_sisiputih) && (hasil == petir_header::result_white_win))) {
                ((*p).book.bobot = pas::increment((*p).book.bobot, 2));
            } else {
                if (((giliran == petir_header::_sisihitam) && (hasil == petir_header::result_black_win))) {
                    ((*p).book.bobot = pas::increment((*p).book.bobot, 2));
                }
            }
        }
    } else {
        if ((hasil == petir_header::result_draw)) {
            ((*p).book.bobot = pas::increment((*p).book.bobot, 1));
        } else {
            if ((giliran == petir_header::_sisiputih)) {
                if ((hasil == petir_header::result_white_win)) {
                    ((*p).book.bobot = pas::increment((*p).book.bobot, 2));
                    if ((pas::sub(blackelo, whiteelo) >= 200)) {
                        ((*p).book.bobot = pas::increment((*p).book.bobot, 3));
                    } else {
                        if ((pas::sub(blackelo, whiteelo) >= 100)) {
                            ((*p).book.bobot = pas::increment((*p).book.bobot, 1));
                        }
                    }
                } else {
                    if (((*p).book.bobot >= 2)) {
                        ((*p).book.bobot = pas::decrement((*p).book.bobot, 2));
                        if (((pas::sub(whiteelo, blackelo) >= 200) && ((*p).book.bobot >= 2))) {
                            ((*p).book.bobot = pas::decrement((*p).book.bobot, 2));
                        } else {
                            if (((pas::sub(whiteelo, blackelo) >= 100) && ((*p).book.bobot >= 1))) {
                                ((*p).book.bobot = pas::decrement((*p).book.bobot, 1));
                            }
                        }
                    }
                }
            } else {
                if ((giliran == petir_header::_sisihitam)) {
                    if ((hasil == petir_header::result_black_win)) {
                        ((*p).book.bobot = pas::increment((*p).book.bobot, 2));
                        if ((pas::sub(whiteelo, blackelo) >= 200)) {
                            ((*p).book.bobot = pas::increment((*p).book.bobot, 4));
                        } else {
                            if ((pas::sub(whiteelo, blackelo) >= 100)) {
                                ((*p).book.bobot = pas::increment((*p).book.bobot, 2));
                            } else {
                                if ((pas::sub(whiteelo, blackelo) >= 50)) {
                                    ((*p).book.bobot = pas::increment((*p).book.bobot, 1));
                                }
                            }
                        }
                    } else {
                        if (((*p).book.bobot >= 2)) {
                            ((*p).book.bobot = pas::decrement((*p).book.bobot, 2));
                            if (((pas::sub(blackelo, whiteelo) >= 250) && ((*p).book.bobot >= 2))) {
                                ((*p).book.bobot = pas::decrement((*p).book.bobot, 2));
                            } else {
                                if (((pas::sub(blackelo, whiteelo) >= 150) && ((*p).book.bobot >= 1))) {
                                    ((*p).book.bobot = pas::decrement((*p).book.bobot, 1));
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return return_value;
}

// opening_book.pas:335
bool storebook(petir_opening_book::tbook b, std::int32_t giliran) {
    bool return_value{};
    std::uint64_t hkey{};
    std::uint64_t temp{};
    petir_opening_book::thashbook* p{};
    return_value = true;
    hkey = (b.key & pas::sub(petir_opening_book::maxhashbook, 1));
    temp = hkey;
    (giliran = pas::decrement(giliran, 1));
    while ((petir_opening_book::book[giliran][hkey].used && (petir_opening_book::book[giliran][hkey].book.key != b.key))) {
        (hkey = pas::increment(hkey, 1));
        if ((hkey > pas::sub(petir_opening_book::maxhashbook, 1))) {
            hkey = 0;
        }
        if ((hkey == temp)) {
            return_value = false;
            pas::showmessage(pas::String("hash penuh"));
            return return_value;
        }
    }
    p = pas::address(petir_opening_book::book[giliran][hkey]);
    (*p).book = b;
    (*p).used = true;
    return return_value;
}

// opening_book.pas:361
void make(pas::String notasi, petir_header::tdata& data, std::uint8_t giliran) {
    std::int32_t move{};
    bool pp{};
    if ((notasi[1] != 'O')) {
        notasi = petir_notation::shortnotationtolong(notasi, data, giliran);
    }
    move = petir_notation::notationtomove(notasi, data, giliran);
    if ((giliran == petir_header::_sisiputih)) {
        petir_makemove::makewhitemove(move, data);
    } else {
        petir_makemove::makeblackmove(move, data);
    }
}

}
