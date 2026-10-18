#pragma once
#include "petir.hpp"
#include <algorithm>

namespace petir_hashing {
// Stored depth is in quarter plies (search depth is divided by two on store).
// This margin is a heuristic allowance for extensions and quiescence, not a
// proof that descendants cannot reach the evaluation fade at counter 80.
inline bool counterCompatible(const petir_hashing_header::thashrecord& entry,
                              std::uint8_t counter) {
    if (entry.fiftyMoveCounter == counter) return true;
    constexpr int mateThreshold = petir_header::kingValue - 100;
    if (entry.score >= mateThreshold || entry.score <= -mateThreshold) return false;
    const int storedPlies = (int(entry.depth) + 3) / 4;
    return std::max<int>(entry.fiftyMoveCounter, counter) + storedPlies + 16 < 80;
}
}
