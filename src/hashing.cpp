// Faithful translation of hashing.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_hashing {
std::int32_t anu = 0;
// hashing.pas:15
void addtable(std::uint64_t key, std::int32_t giliran, std::int32_t level, std::int32_t moves, std::int32_t nilai, std::int32_t tipe, std::uint8_t move50count) {
    std::int32_t hkey{};
    petir_hashing_header::thashrecord* kk{};
    if ((giliran == petir_header::_sisihitam)) {
        key = (~key);
    }
    hkey = (key & pas::sub(petir_header::maxhash, 1));
    level = pas::shr(level, 1);
    kk = pas::address(petir_hashing_header::hashtable[hkey]);
    if ((((*kk).flag & petir_hashing_header::bit_used) == 0)) {
        (*kk).flag = ((*kk).flag | petir_hashing_header::bit_used);
        (*kk).key = key;
        (*kk).nilai = nilai;
        (*kk).tipe = tipe;
        (*kk).level = level;
        (*kk).moves = moves;
        (*kk).move50count = move50count;
        (*kk).flag = petir_hashing_header::bit_used|petir_hashing_header::generation;
    } else {
        if (petir_hashing_header::is_old(*kk) || level >= (*kk).level) {
            (*kk).moves = moves;
            (*kk).move50count = move50count;
            (*kk).key = key;
            (*kk).nilai = nilai;
            (*kk).tipe = tipe;
            (*kk).level = level;
            (*kk).flag = petir_hashing_header::bit_used|petir_hashing_header::generation;
        }
    }
}

// hashing.pas:52
bool searchhash(std::uint64_t key, std::int32_t& alpha, std::int32_t& beta, std::int32_t level, std::int32_t giliran, std::int32_t& nilai, std::int32_t& moves, bool& avoidnm, std::uint8_t move50count) {
    bool return_value{};
    std::int32_t hkey{};
    petir_hashing_header::thashrecord* temp{};
    std::int32_t pas_temp_1{};
    if ((giliran == petir_header::_sisihitam)) {
        key = (~key);
    }
    hkey = (key & pas::sub(petir_header::maxhash, 1));
    temp = pas::address(petir_hashing_header::hashtable[hkey]);
    avoidnm = false;
    if ((((*temp).flag & petir_hashing_header::bit_used) != 0)) {
        if (((*temp).key == key)) {
            moves = (*temp).moves;
            // The board still supplies a move-ordering hint, but scores and
            // score-dependent pruning require the same halfmove counter.
            if (((*temp).move50count == move50count) && ((*temp).level >= pas::shr(level, 1))) {
                pas_temp_1 = (*temp).tipe;
                if (((pas_temp_1 == petir_header::pv_node))) {
                    nilai = (*temp).nilai;
                    return_value = true;
                    return return_value;
                }
                else if (((pas_temp_1 == petir_header::fail_low))) {
                    if (((*temp).nilai <= alpha)) {
                        nilai = (*temp).nilai;
                        return_value = true;
                        return return_value;
                    } else {
                        if (((*temp).nilai >= beta)) {
                            if ((pas::mul((*temp).level, 2) >= 16)) {
                                avoidnm = true;
                            }
                        }
                    }
                }
                else if (((pas_temp_1 == petir_header::fail_high))) {
                    if (((*temp).nilai >= beta)) {
                        nilai = (*temp).nilai;
                        return_value = true;
                        return return_value;
                    }
                }
            }
        }
    }
    return_value = false;
    return return_value;
}

}
