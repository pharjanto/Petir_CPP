// Faithful translation of hashing_header.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_hashing_header {
petir_hashing_header::thashdata hashtable{};
std::uint8_t generation{};
std::int32_t anu{};
// hashing_header.pas:89
void inithashkey() {
    std::int32_t a{};
    std::int32_t b{};
    std::int32_t pas_temp_1{};
    std::int32_t pas_temp_2{};
    std::int32_t pas_temp_3{};
    std::int32_t pas_temp_4{};
    pas::randseed = 99999;
    for (a = -6; a <= 6; ++a) {
        for (b = 0; b <= 127; ++b) {
            const std::uint64_t key = pas::delphi_hash_random();
            if (b <= 63) petir_header::hashkey[a][b] = key;
            else petir_hashing_header::anu = static_cast<std::int32_t>(key);
        }
    }
    for (a = 0; a <= 255; ++a) petir_header::rokadehashkey[a] = pas::delphi_hash_random();
    for (a = 0; a <= petir_header::_no_ep; ++a) petir_header::enpassanthashkey[a] = pas::delphi_hash_random();
    petir_header::ephashkey = petir_header::enpassanthashkey[10];
}

// hashing_header.pas:74
std::uint64_t hashvalue(petir_header::tdata& data, std::int32_t giliran) {
    std::uint64_t return_value{};
    std::uint64_t t{};
    std::int32_t a{};
    std::int32_t pas_temp_1{};
    t = 0;
    pas_temp_1 = 63;
    for (a = 0; a <= pas_temp_1; ++a) {
        if ((data.papan[a] != 0)) {
            t = (t ^ petir_header::hashkey[data.papan[a]][a]);
        }
    }
    t = (t ^ petir_header::rokadehashkey[data.flagrokade]);
    t = (t ^ petir_header::enpassanthashkey[data.ep]);
    return_value = t;
    return return_value;
}

// hashing_header.pas:44
void cleartable() {
    std::int32_t a{};
    std::int32_t pas_temp_1{};
    pas_temp_1 = pas::sub(petir_header::maxhash, 1);
    for (a = 0; a <= pas_temp_1; ++a) {
        petir_hashing_header::hashtable[a].flag = (petir_hashing_header::hashtable[a].flag & (~petir_hashing_header::bit_used));
        petir_hashing_header::hashtable[a].flag = (petir_hashing_header::hashtable[a].flag & (~petir_hashing_header::bit_old));
    }
}

// hashing_header.pas:64
void cleartable2() {
    // The tag changes replacement priority only; key and rule-50 checks remain.
    generation = (generation+1)&generation_mask;
}

// hashing_header.pas:37
void inittable() {
    pas::setlength(petir_hashing_header::hashtable, petir_header::maxhash);
}

// hashing_header.pas:59
void clearreversetable() {
}

// hashing_header.pas:55
void reversetable() {
}

// hashing_header.pas:113
void coba() {
}

}
