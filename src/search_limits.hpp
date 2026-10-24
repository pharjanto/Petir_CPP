#pragma once
#include <cstdint>
#include <vector>

namespace petir {
// Set before starting the worker, reset by prepareSearch, then read only.
struct SearchLimits {
    bool restrictRootMoves=false;
    std::vector<int> rootMoves;
    std::int64_t nodes=-1; // -1 means unlimited; zero returns a legal fallback.
    int mate=0; // Full moves, with a search horizon of at most 2 * mate plies.
};
extern SearchLimits searchLimits;
}
