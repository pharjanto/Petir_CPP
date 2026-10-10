// Faithful translation of hashing.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_hashing {
std::int32_t anu = 0;
static void store(petir_hashing_header::thashrecord& e, std::uint64_t key, std::int32_t level, std::int32_t moves, std::int32_t nilai, std::int32_t tipe, std::uint8_t move50count) {
    e.moves = moves;
    e.move50count = move50count;
    e.key = key;
    e.nilai = nilai;
    e.tipe = tipe;
    e.level = level;
    e.flag = petir_hashing_header::bit_used|petir_hashing_header::generation;
}
// hashing.pas:15
// Each bucket has two entries: the first keeps the deeper or newer result, the
// second takes whatever the first turns away, so shallow moves are not lost.
// A current first entry pushed out by another position moves to the second.
// A first entry from the previous search that is deeper than the new result
// stays, and the new result goes to the second; older entries give way.
void addtable(std::uint64_t key, std::int32_t giliran, std::int32_t level, std::int32_t moves, std::int32_t nilai, std::int32_t tipe, std::uint8_t move50count) {
    if ((giliran == petir_header::_sisihitam)) {
        key = (~key);
    }
    level = pas::shr(level, 1);
    auto* bucket = petir_hashing_header::bucket(key);
    auto& deep = bucket[0];
    const bool aged = petir_hashing_header::is_old(deep) && !(petir_hashing_header::is_previous(deep) && (level < deep.level) && (deep.key != key));
    if (((deep.flag & petir_hashing_header::bit_used) == 0) || aged || level >= deep.level) {
        if (((deep.flag & petir_hashing_header::bit_used) != 0) && !petir_hashing_header::is_old(deep) && (deep.key != key)) {
            bucket[1] = deep;
        }
        store(deep, key, level, moves, nilai, tipe, move50count);
    } else if (deep.key != key) {
        store(bucket[1], key, level, moves, nilai, tipe, move50count);
    }
}

// hashing.pas:52
bool searchhash(std::uint64_t key, std::int32_t& alpha, std::int32_t& beta, std::int32_t level, std::int32_t giliran, std::int32_t& nilai, std::int32_t& moves, bool& avoidnm, std::uint8_t move50count) {
    bool return_value{};
    petir_hashing_header::thashrecord* temp{};
    std::int32_t pas_temp_1{};
    if ((giliran == petir_header::_sisihitam)) {
        key = (~key);
    }
    auto* bucket = petir_hashing_header::bucket(key);
    temp = (((bucket[0].flag & petir_hashing_header::bit_used) != 0) && (bucket[0].key == key)) ? &bucket[0] : &bucket[1];
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
