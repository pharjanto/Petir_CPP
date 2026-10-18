// Read-only TT diagnostics, compiled only with PETIR_HASH_STATS. Each root
// search resets the counters; the command "hashstats" reports and clears them.
#pragma once
#ifdef PETIR_HASH_STATS
#define PETIR_HASH_STAT(...) __VA_ARGS__
#include "petir.hpp"
#include <algorithm>
#include <string>
namespace petir_hash_stats {
using u64 = std::uint64_t;
// probeHash copies the entry it actually selected. Root, PV extraction, and
// fallback probes omit this output; only mainSearch passes it to record().
struct Probe {
    bool keyHit = false;
    petir_hashing_header::thashrecord entry{};
};
struct Counters {
    u64 probes{}, pvProbesExcluded{}, keyHits{}, counterMatches{}, counterMismatches{};
    u64 depthEligible{}, depthRejectedMismatches{}, boundRejectedMismatches{};
    u64 acceptedCutoffs{}, counterBlockedCutoffs{}, decisionMismatches{};
    // Both counters <80; either >=80 but neither >=100; either >=100.
    u64 mismatchBands[3]{}, blockedBands[3]{};
    // Absolute difference 1, 2, 3+; bound exact, upper, lower.
    u64 blockedDifferences[3]{}, blockedBounds[3]{}, blockedBandsByDifference[3][3]{};
    // Requested whole plies: 0-1, 2, 3, 4-5, 6-8, 9+.
    u64 blockedDepths[6]{};
    // Selected entry from current generation, previous generation, older.
    u64 blockedAges[3]{};
};
inline Counters counters;
inline void reset() { counters = {}; }
inline void record(const Probe& probe, int depth, int alpha, int beta,
                   std::uint8_t counter, int nodeType, bool actualHit) {
    if (nodeType == petir_search::pvNode) { ++counters.pvProbesExcluded; return; }
    ++counters.probes;
    if (!probe.keyHit) { counters.decisionMismatches += actualHit; return; }
    ++counters.keyHits;
    const auto& entry = probe.entry;
    const bool sameCounter = entry.fiftyMoveCounter == counter;
    const bool enoughDepth = entry.depth >= pas::shiftRight(depth, 1);
    const bool boundCuts = entry.boundType == petir_header::exactBound
        || (entry.boundType == petir_header::upperBound && entry.score <= alpha)
        || (entry.boundType == petir_header::lowerBound && entry.score >= beta);
    counters.decisionMismatches += actualHit != (sameCounter && enoughDepth && boundCuts);
    counters.counterMatches += sameCounter;
    counters.counterMismatches += !sameCounter;
    counters.depthEligible += enoughDepth;
    counters.acceptedCutoffs += actualHit;
    if (sameCounter) return;
    const int highCounter = std::max<int>(entry.fiftyMoveCounter, counter);
    const int band = highCounter < 80 ? 0 : highCounter < 100 ? 1 : 2;
    ++counters.mismatchBands[band];
    if (!enoughDepth) { ++counters.depthRejectedMismatches; return; }
    if (!boundCuts) { ++counters.boundRejectedMismatches; return; }
    ++counters.counterBlockedCutoffs;
    ++counters.blockedBands[band];
    const int difference = std::abs(int(entry.fiftyMoveCounter) - int(counter));
    const int differenceIndex = std::min(difference, 3) - 1;
    ++counters.blockedDifferences[differenceIndex];
    ++counters.blockedBandsByDifference[band][differenceIndex];
    ++counters.blockedBounds[entry.boundType];
    const int plies = depth / petir_header::plyUnits;
    const int depthIndex = plies <= 1 ? 0 : plies == 2 ? 1 : plies == 3 ? 2 : plies <= 5 ? 3 : plies <= 8 ? 4 : 5;
    ++counters.blockedDepths[depthIndex];
    const int ageIndex = !petir_hashing_header::isOld(entry) ? 0 : petir_hashing_header::isPreviousGeneration(entry) ? 1 : 2;
    ++counters.blockedAges[ageIndex];
}
inline void put(std::string& text, u64 value) { text += std::to_string(value); }
template<class T, std::size_t N> void put(std::string& text, const T (&values)[N]) {
    text += '[';
    for (std::size_t i = 0; i < N; ++i) { if (i) text += ','; put(text, values[i]); }
    text += ']';
}
inline std::string report() {
    std::string text = "hashstats {";
    bool first = true;
    auto field = [&](const char* name, const auto& value) {
        if (!first) text += ',';
        first = false; text += '"'; text += name; text += "\":"; put(text, value);
    };
    field("probes", counters.probes); field("pv_probes_excluded", counters.pvProbesExcluded);
    field("key_hits", counters.keyHits); field("counter_matches", counters.counterMatches);
    field("counter_mismatches", counters.counterMismatches); field("depth_eligible", counters.depthEligible);
    field("depth_rejected_mismatches", counters.depthRejectedMismatches);
    field("bound_rejected_mismatches", counters.boundRejectedMismatches);
    field("accepted_cutoffs", counters.acceptedCutoffs); field("counter_blocked_cutoffs", counters.counterBlockedCutoffs);
    field("decision_mismatches", counters.decisionMismatches); field("mismatch_bands", counters.mismatchBands);
    field("blocked_bands", counters.blockedBands); field("blocked_differences", counters.blockedDifferences);
    field("blocked_bounds", counters.blockedBounds); field("blocked_bands_by_difference", counters.blockedBandsByDifference);
    field("blocked_depths", counters.blockedDepths); field("blocked_ages", counters.blockedAges);
    text += '}'; reset(); return text;
}
}
#else
#define PETIR_HASH_STAT(...)
#endif
