// Reads a lazystat run directory (hist-*.bin, rec-*.bin, sum-*.txt) and prints
//  1. the summed totals;
//  2. rest = full - partial quantiles per bucket, for the new and the second
//     (old) eval, next to the current margin;
//  3. a replay of the recorded calls under other margin schemes: exits and wrong
//     exits (the lazy result and the full score on different sides of alpha).
// "current" is the 4ce0793 scheme: 280 + trades, +200 early endgame, +600 endgame.
// Usage: analyze RUN_DIR [RECORD_SAMPLE [SPEC ...]]
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace fs = std::filesystem;
constexpr int restRange = 4000, phases = 3, tradeSlots = 13, bins = 2 * restRange + 1;
using Hist = unsigned long long[2][phases][tradeSlots][bins];
using PhaseHist = unsigned long long[2][25][3][bins];

#pragma pack(push, 1)
struct Record {
    std::int16_t alpha, partialNew, fullNew, partialOld, fullOld;
    std::uint8_t phaseNew, tradesNew, phaseOld, flags, whiteMaterial, blackMaterial, gamePhase, queens;
};
#pragma pack(pop)

const char* phaseName[phases] = {"middlegame", "early endgame", "endgame"};
int phaseAdd(int phase) { return phase == 2 ? 600 : phase == 1 ? 200 : 0; }
// Trade slot -> current adjustment: slot = -(adjustment)/10, adjustment in {0,-20,-30,-40,-50,...}.
int currentMargin(int phase, int slot) { return 280 + phaseAdd(phase) - slot * 10; }

struct Dist {
    std::vector<unsigned long long> h = std::vector<unsigned long long>(bins);
    unsigned long long n = 0;
    void add(const unsigned long long* src) { for (int i = 0; i < bins; ++i) { h[i] += src[i]; n += src[i]; } }
    // Smallest r >= 0 with P(rest > r) <= p (upper tail) or P(rest < -r) <= p (lower tail).
    int tail(double p, bool upper) const {
        unsigned long long beyond = 0;
        const unsigned long long limit = static_cast<unsigned long long>(p * n);
        for (int r = restRange; r >= 0; --r) {
            const unsigned long long next = beyond + h[upper ? restRange + r : restRange - r];
            if (next > limit) return r;
            beyond = next;
        }
        return 0;
    }
    int extreme(bool upper) const {
        for (int r = restRange; r >= 0; --r) if (h[upper ? restRange + r : restRange - r]) return r;
        return 0;
    }
    unsigned long long beyond(int m) const {  // |rest| > m
        unsigned long long c = 0;
        for (int r = m + 1; r <= restRange; ++r) c += h[restRange + r] + h[restRange - r];
        return c;
    }
};

int main(int argc, char** argv) {
    if (argc < 2) { std::fprintf(stderr, "usage: analyze RUN_DIR [RECORD_SAMPLE]\n"); return 2; }
    const fs::path dir = argv[1];
    const double sample = argc > 2 ? std::atof(argv[2]) : 1.0;
    static Hist hist{};
    static PhaseHist byPhase{};
    std::map<std::string, unsigned long long> totals;
    std::vector<Record> records;
    for (const auto& entry : fs::directory_iterator(dir)) {
        const std::string name = entry.path().filename().string();
        if (name.rfind("hist-", 0) == 0) {
            static Hist part;
            std::ifstream f(entry.path(), std::ios::binary);
            f.read(reinterpret_cast<char*>(&part), sizeof part);
            if (!f) { std::fprintf(stderr, "short %s\n", name.c_str()); return 1; }
            auto* d = &hist[0][0][0][0];
            const auto* s = &part[0][0][0][0];
            for (std::size_t i = 0; i < sizeof part / sizeof part[0][0][0][0]; ++i) d[i] += s[i];
            static PhaseHist partPhase;
            f.read(reinterpret_cast<char*>(&partPhase), sizeof partPhase);
            if (f) {
                auto* dp = &byPhase[0][0][0][0];
                const auto* sp = &partPhase[0][0][0][0];
                for (std::size_t i = 0; i < sizeof partPhase / sizeof partPhase[0][0][0][0]; ++i) dp[i] += sp[i];
            }
        } else if (name.rfind("sum-", 0) == 0) {
            std::ifstream f(entry.path());
            std::string key;
            unsigned long long value;
            while (f >> key >> value) totals[key] += value;
        } else if (name.rfind("rec-", 0) == 0) {
            const auto size = fs::file_size(entry.path());
            const std::size_t at = records.size();
            records.resize(at + size / sizeof(Record));
            std::ifstream f(entry.path(), std::ios::binary);
            f.read(reinterpret_cast<char*>(records.data() + at), (size / sizeof(Record)) * sizeof(Record));
        }
    }
    std::printf("== Totals\n");
    for (const auto& [k, v] : totals) std::printf("  %-18s %llu\n", k.c_str(), v);
    const double reached = static_cast<double>(totals["reachedNew"]);
    std::printf("  exit rate new %.2f%%, old %.2f%% of reached; wrong per exit new %.2e, old %.2e\n",
                100.0 * totals["exitsNew"] / reached, 100.0 * totals["exitsOld"] / totals["reachedOld"],
                double(totals["wrongNew"]) / totals["exitsNew"], double(totals["wrongOld"]) / totals["exitsOld"]);
    std::printf("  records loaded %zu (sample 1/%g)\n\n", records.size(), sample);

    // 2. Quantiles. Upper tail: rest > r (the full score rises; breaks a fail-low exit).
    // Lower tail: rest < -r (breaks a fail-high exit).
    std::printf("== rest = full - partial, side-relative. up/down = tail quantile of +rest / -rest\n");
    std::printf("%-6s %-14s %-6s %6s %11s | %5s %5s %5s %5s | %5s %5s %5s %5s | %5s %5s | %9s\n", "eval", "phase", "trades", "margin",
                "calls", "up99", "99.9", "99.99", "max", "dn99", "99.9", "99.99", "max", "p99.9", "p.999", ">margin");
    for (int v = 0; v < 2; ++v)
        for (int ph = 0; ph < phases; ++ph) {
            Dist all;
            for (int t = 0; t < tradeSlots; ++t) all.add(hist[v][ph][t]);
            auto row = [&](const Dist& d, const char* trades, int margin) {
                if (!d.n) return;
                std::printf("%-6s %-14s %-6s %6s %11llu | %5d %5d %5d %5d | %5d %5d %5d %5d | %5d %5d | %9llu\n", v ? "old" : "new",
                            phaseName[ph], trades, margin ? std::to_string(margin).c_str() : "-", d.n, d.tail(0.01, true),
                            d.tail(0.001, true), d.tail(0.0001, true), d.extreme(true), d.tail(0.01, false), d.tail(0.001, false),
                            d.tail(0.0001, false), d.extreme(false), std::max(d.tail(0.0005, true), d.tail(0.0005, false)),
                            std::max(d.tail(0.00005, true), d.tail(0.00005, false)), margin ? d.beyond(margin) : 0ULL);
            };
            row(all, "all", 0);
            for (int t = 0; t < tradeSlots; ++t) {
                Dist d;
                d.add(hist[v][ph][t]);
                if (d.n * 1000 < all.n) continue;  // skip tiny buckets
                row(d, std::to_string(-t * 10).c_str(), currentMargin(ph, t));
            }
        }

    // 2b. By gamePhase (0-24) and queens on the board.
    std::printf("\n== rest by gamePhase and queens (max of both tails)\n");
    std::printf("%-4s %-6s | %12s %6s %6s %6s | %12s %6s %6s %6s\n", "eval", "phase", "noQ calls", "99.9", "99.99", "max",
                "queen calls", "99.9", "99.99", "max");
    for (int v = 0; v < 2; ++v)
        for (int g = 0; g <= 24; ++g) {
            Dist none, queen;
            none.add(byPhase[v][g][0]);
            queen.add(byPhase[v][g][1]);
            queen.add(byPhase[v][g][2]);
            auto cells = [](const Dist& d) {
                char buf[64];
                if (!d.n) { std::snprintf(buf, sizeof buf, "%12s %6s %6s %6s", "-", "", "", ""); return std::string(buf); }
                std::snprintf(buf, sizeof buf, "%12llu %6d %6d %6d", d.n, std::max(d.tail(0.001, true), d.tail(0.001, false)),
                              std::max(d.tail(0.0001, true), d.tail(0.0001, false)), std::max(d.extreme(true), d.extreme(false)));
                return std::string(buf);
            };
            std::printf("%-4s %-6d | %s | %s\n", v ? "old" : "new", g, cells(none).c_str(), cells(queen).c_str());
        }

    // 3. Replay. A scheme maps a record to a margin. Specs (argv[3..]), all listed in README.md:
    //   current | mg:X (middlegame base X) | flat:X | flatT:X (X + trades) |
    //   taper:A:B (B + (A-B)*gamePhase/24) | taperT:A:B (+ trades) | taperQ:A:B:Q (+ Q with a queen on the board) |
    //   mgQ, mgQT, mgQE, base, baseE (comments below)
    using Scheme = std::function<int(const Record&)>;
    auto parse = [](const std::string& spec) -> Scheme {
        std::vector<int> n;
        std::string kind = spec.substr(0, spec.find(':'));
        for (std::size_t at = spec.find(':'); at != std::string::npos; at = spec.find(':', at + 1)) n.push_back(std::atoi(spec.c_str() + at + 1));
        auto trades = [](const Record& r) { return -10 * int(r.tradesNew); };
        auto taper = [](int a, int b, int g) { return b + (a - b) * g / 24; };
        if (kind == "current") return [](const Record& r) { return currentMargin(r.phaseNew, r.tradesNew); };
        if (kind == "mg") return [n](const Record& r) { return currentMargin(r.phaseNew, r.tradesNew) + (r.phaseNew == 0 ? n[0] - 280 : 0); };
        // mgQ:X:Q: middlegame class X (+ Q with a queen on the board), early endgame and endgame as current.
        if (kind == "mgQ") return [n](const Record& r) { return r.phaseNew == 0 ? n[0] + (r.queens ? n[1] : 0) : currentMargin(r.phaseNew, r.tradesNew); };
        // mgQT:X:Q: the same plus the trades term in the middlegame class.
        if (kind == "mgQT") return [n, trades](const Record& r) { return r.phaseNew == 0 ? n[0] + (r.queens ? n[1] : 0) + trades(r) : currentMargin(r.phaseNew, r.tradesNew); };
        // mgQE:X:Q:EE:EG: middlegame class X (+ Q with a queen), early endgame EE + trades, endgame EG + trades.
        if (kind == "mgQE") return [n, trades](const Record& r) {
            return r.phaseNew == 0 ? n[0] + (r.queens ? n[1] : 0) : (r.phaseNew == 1 ? n[2] : n[3]) + trades(r);
        };
        // base:B: the current scheme with its 280 base replaced by B in every phase class.
        if (kind == "base") return [n](const Record& r) { return currentMargin(r.phaseNew, r.tradesNew) - 280 + n[0]; };
        // baseE:B:EE:EG: base B + trades in every class, plus EE in the early endgame or EG in the endgame.
        if (kind == "baseE") return [n, trades](const Record& r) { return n[0] + trades(r) + (r.phaseNew == 2 ? n[2] : r.phaseNew == 1 ? n[1] : 0); };
        if (kind == "flat") return [n](const Record&) { return n[0]; };
        if (kind == "flatT") return [n, trades](const Record& r) { return n[0] + trades(r); };
        if (kind == "taper") return [n, taper](const Record& r) { return taper(n[0], n[1], r.gamePhase); };
        if (kind == "taperT") return [n, taper, trades](const Record& r) { return taper(n[0], n[1], r.gamePhase) + trades(r); };
        if (kind == "taperQ") return [n, taper](const Record& r) { return taper(n[0], n[1], r.gamePhase) + (r.queens ? n[2] : 0); };
        std::fprintf(stderr, "bad spec %s\n", spec.c_str());
        std::exit(2);
    };
    auto replay = [&](const std::string& name, const Scheme& margin, bool oldEval) {
        double exits = 0, wrong = 0, exitsByPhase[3] = {}, wrongByPhase[3] = {};
        std::vector<int> severity;
        for (const Record& r : records) {
            if (oldEval && !(r.flags & 1)) continue;
            const int partial = oldEval ? r.partialOld : r.partialNew, full = oldEval ? r.fullOld : r.fullNew;
            const int phase = oldEval ? r.phaseOld : r.phaseNew;
            const int m = margin(r);
            const int alpha = r.alpha, beta = r.alpha + 1;
            if (!(partial + m < alpha) && !(partial - m > beta)) continue;
            ++exits;
            ++exitsByPhase[phase];
            if ((partial > alpha) != (full > alpha)) {
                ++wrong;
                ++wrongByPhase[phase];
                severity.push_back(std::abs(full - alpha));
            }
        }
        std::sort(severity.begin(), severity.end());
        const double nullReached = oldEval ? totals["reachedOld"] : totals["reachedNew"];
        std::printf("%-22s %s exits %6.2f%% wrong %8.0f per-M %7.1f | mg %5.2f%% %8.0f | ee %5.2f%% %6.0f | eg %5.2f%% %6.0f | sevMed %3d\n",
                    name.c_str(), oldEval ? "old" : "new", 100.0 * exits * sample / nullReached, wrong * sample,
                    1e6 * wrong * sample / nullReached, 100.0 * exitsByPhase[0] * sample / nullReached, wrongByPhase[0] * sample,
                    100.0 * exitsByPhase[1] * sample / nullReached, wrongByPhase[1] * sample,
                    100.0 * exitsByPhase[2] * sample / nullReached, wrongByPhase[2] * sample,
                    severity.empty() ? 0 : severity[severity.size() / 2]);
    };
    std::printf("\n== Replay (exits as %% of null-window calls reaching the checkpoint; wrong scaled by the record sample;\n"
                "   per-M = wrong per million such calls; mg/ee/eg = exits and wrong by the current phase classes)\n");
    replay("current", parse("current"), true);
    std::vector<std::string> specs(argv + std::min(argc, 3), argv + argc);
    if (specs.empty()) specs = {"current", "mg:400", "mg:480"};
    // "o:SPEC" replays SPEC on the second eval.
    for (const auto& spec : specs) {
        if (spec.rfind("o:", 0) == 0) replay(spec, parse(spec.substr(2)), true);
        else replay(spec, parse(spec), false);
    }
    return 0;
}
