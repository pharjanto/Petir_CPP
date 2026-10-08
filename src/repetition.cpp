// Faithful translation of repetition.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_repetition {
petir_repetition::tarrreprecord repdata{};
// repetition.pas:22
void initrepdata() {
    std::int32_t a{};
    std::int32_t b{};
    std::int32_t pas_temp_1{};
    std::int32_t pas_temp_2{};
    pas_temp_1 = 1;
    for (b = 0; b <= pas_temp_1; ++b) {
        pas_temp_2 = pas::sub(petir_repetition::maxhashdraw, 1);
        for (a = 0; a <= pas_temp_2; ++a) {
            petir_repetition::repdata[b][a].used = false;
            petir_repetition::repdata[b][a].jumlah = 0;
        }
    }
}

// repetition.pas:53
std::int32_t addrephash(std::uint64_t key, std::int32_t giliran) {
    std::int32_t return_value{};
    std::uint64_t hkey{};
    petir_repetition::treprecord* p{};
    hkey = (key & pas::sub(petir_repetition::maxhashdraw, 1));
    return_value = pas::negate(1);
    while ((petir_repetition::repdata[giliran][hkey].used && (petir_repetition::repdata[giliran][hkey].key != key))) {
        if ((hkey == pas::sub(petir_repetition::maxhashdraw, 1))) {
            hkey = pas::negate(1);
        }
        (hkey = pas::increment(hkey, 1));
    }
    p = pas::address(petir_repetition::repdata[giliran][hkey]);
    if (((*p).used == false)) {
        (*p).used = true;
        (*p).key = key;
        (*p).jumlah = 0;
        return_value = 0;
    } else {
        ((*p).jumlah = pas::increment((*p).jumlah, 1));
        return_value = (*p).jumlah;
    }
    return return_value;
}

// repetition.pas:81
void delrephash(std::uint64_t key, std::int32_t giliran) {
    std::uint64_t hkey{};
    hkey = (key & pas::sub(petir_repetition::maxhashdraw, 1));
    while ((petir_repetition::repdata[giliran][hkey].key != key)) {
        if ((hkey == pas::sub(petir_repetition::maxhashdraw, 1))) {
            hkey = pas::negate(1);
        }
        (hkey = pas::increment(hkey, 1));
    }
    if ((petir_repetition::repdata[giliran][hkey].jumlah == 0)) {
        petir_repetition::repdata[giliran][hkey].used = false;
    } else {
        (petir_repetition::repdata[giliran][hkey].jumlah = pas::decrement(petir_repetition::repdata[giliran][hkey].jumlah, 1));
    }
}

// repetition.pas:34
bool isrep(std::uint64_t key, std::int32_t giliran) {
    bool return_value{};
    std::uint64_t hkey{};
    hkey = (key & pas::sub(petir_repetition::maxhashdraw, 1));
    while ((petir_repetition::repdata[giliran][hkey].key != key)) {
        if ((hkey == pas::sub(petir_repetition::maxhashdraw, 1))) {
            hkey = pas::negate(1);
        }
        (hkey = pas::increment(hkey, 1));
    }
    if ((petir_repetition::repdata[giliran][hkey].jumlah == 2)) {
        return_value = true;
        return return_value;
    }
    return_value = false;
    return return_value;
}

}
