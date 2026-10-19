#pragma once
#include "petir.hpp"
namespace petir_hashing {
inline bool preserveExact(const petir_hashing_header::thashrecord& entry,
                          std::uint64_t key, int depth, int bound, std::uint8_t counter) {
    return (entry.flags & petir_hashing_header::usedFlag) && entry.key == key
        && entry.depth == depth && entry.boundType == petir_header::exactBound
        && (bound == petir_header::upperBound || bound == petir_header::lowerBound)
        && entry.fiftyMoveCounter == counter && !petir_hashing_header::isOld(entry);
}
}
