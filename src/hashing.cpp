// Faithful translation of hashing.pas; Delphi source is in original/.
#include "petir.hpp"
#include "hash_stats.hpp"
#include "hash_counter.hpp"
#include "hash_update.hpp"
namespace petir_hashing {
static void storeEntry(petir_hashing_header::thashrecord& entry, std::uint64_t key, std::int32_t depth, std::int32_t move, std::int32_t score, std::int32_t boundType, std::uint8_t fiftyMoveCounter, int slot) {
    PETIR_HASH_STAT(petir_hash_stats::recordStore(entry,key,depth,boundType,fiftyMoveCounter,slot);)
#ifdef PETIR_PRESERVE_EXACT
    if (preserveExact(entry,key,depth,boundType,fiftyMoveCounter)) {
        PETIR_HASH_STAT(++petir_hash_stats::counters.storeProtected;)
        return;
    }
#endif
    PETIR_HASH_STAT(++petir_hash_stats::counters.storeWrites;)
    entry.move = move;
    entry.fiftyMoveCounter = fiftyMoveCounter;
    entry.key = key;
    entry.score = score;
    entry.boundType = boundType;
    entry.depth = depth;
    entry.flags = petir_hashing_header::usedFlag|petir_hashing_header::generation;
}
// hashing.pas:15
// Each bucket has two entries: the first keeps the deeper or newer result, the
// second takes whatever the first turns away, so shallow moves are not lost.
// A current first entry pushed out by another position moves to the second.
// A first entry from the previous search that is deeper than the new result
// stays, and the new result goes to the second; older entries give way.
void storeHash(std::uint64_t key, std::int32_t sideToMove, std::int32_t depth, std::int32_t move, std::int32_t score, std::int32_t boundType, std::uint8_t fiftyMoveCounter) {
    PETIR_HASH_STAT(++petir_hash_stats::counters.storeCalls;)
    if ((sideToMove == petir_header::blackSide)) {
        key = (~key);
    }
    depth = pas::shiftRight(depth, 1);
    auto* bucketEntries = petir_hashing_header::bucket(key);
    auto& depthEntry = bucketEntries[0];
    const bool replaceable = petir_hashing_header::isOld(depthEntry) && !(petir_hashing_header::isPreviousGeneration(depthEntry) && (depth < depthEntry.depth) && (depthEntry.key != key));
    if (((depthEntry.flags & petir_hashing_header::usedFlag) == 0) || replaceable || depth >= depthEntry.depth) {
        if (((depthEntry.flags & petir_hashing_header::usedFlag) != 0) && !petir_hashing_header::isOld(depthEntry) && (depthEntry.key != key)) {
            bucketEntries[1] = depthEntry;
        }
        storeEntry(depthEntry, key, depth, move, score, boundType, fiftyMoveCounter, 0);
    } else if (depthEntry.key != key) {
        storeEntry(bucketEntries[1], key, depth, move, score, boundType, fiftyMoveCounter, 1);
    }
}

// hashing.pas:52
bool probeHash(std::uint64_t key, std::int32_t& alpha, std::int32_t& beta, std::int32_t depth, std::int32_t sideToMove, std::int32_t& score, std::int32_t& move, bool& avoidNullMove, std::uint8_t fiftyMoveCounter
    PETIR_HASH_STAT(, petir_hash_stats::Probe* diagnostic)) {
    PETIR_HASH_STAT(if (diagnostic) *diagnostic = {};)
    bool result{};
    petir_hashing_header::thashrecord* entry{};
    std::int32_t boundType{};
    if ((sideToMove == petir_header::blackSide)) {
        key = (~key);
    }
    auto* bucketEntries = petir_hashing_header::bucket(key);
    entry = (((bucketEntries[0].flags & petir_hashing_header::usedFlag) != 0) && (bucketEntries[0].key == key)) ? &bucketEntries[0] : &bucketEntries[1];
    avoidNullMove = false;
    if ((((*entry).flags & petir_hashing_header::usedFlag) != 0)) {
        if (((*entry).key == key)) {
            PETIR_HASH_STAT(if (diagnostic) { diagnostic->keyHit = true; diagnostic->entry = *entry; })
            move = (*entry).move;
            // Reuse non-mate scores across counters only well below the fade.
            // Null-move restrictions still require exact counter agreement.
            if (counterCompatible(*entry, fiftyMoveCounter) && ((*entry).depth >= pas::shiftRight(depth, 1))) {
                boundType = (*entry).boundType;
                if (((boundType == petir_header::exactBound))) {
                    score = (*entry).score;
                    result = true;
                    return result;
                }
                else if (((boundType == petir_header::upperBound))) {
                    if (((*entry).score <= alpha)) {
                        score = (*entry).score;
                        result = true;
                        return result;
                    } else {
                        if (((*entry).score >= beta)) {
                            if (entry->fiftyMoveCounter == fiftyMoveCounter && (pas::mul((*entry).depth, 2) >= 16)) {
                                avoidNullMove = true;
                            }
                        }
                    }
                }
                else if (((boundType == petir_header::lowerBound))) {
                    if (((*entry).score >= beta)) {
                        score = (*entry).score;
                        result = true;
                        return result;
                    }
                }
            }
        }
    }
    result = false;
    return result;
}

bool readHashEntry(std::uint64_t key, std::int32_t sideToMove, std::uint8_t fiftyMoveCounter, std::int32_t& score, std::int32_t& boundType, std::int32_t& depth) {
    if (sideToMove == petir_header::blackSide) {
        key = ~key;
    }
    auto* bucketEntries = petir_hashing_header::bucket(key);
    for (int slot = 0; slot < 2; ++slot) {
        const auto& entry = bucketEntries[slot];
        if (((entry.flags & petir_hashing_header::usedFlag) == 0) || (entry.key != key)) continue;
        if (!counterCompatible(entry, fiftyMoveCounter)) return false;
        score = entry.score;
        boundType = entry.boundType;
        depth = entry.depth * 2;
        return true;
    }
    return false;
}

}
