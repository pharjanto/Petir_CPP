// Lazy-eval statistics in search (tools/lazy_margin, see README.md). The engine
// is the new tree (prepare_trees.py) with evaluate() renamed to evaluateCandidate;
// this wrapper gives search exactly that result, so the search tree is unchanged.
// For every null-window call that reaches the lazy checkpoint it also computes
//   new: the new tree's eval              partial at the checkpoint, full score
//   old: the second tree's eval           the same; by default (08c0f11, before
//                                         Texel) with materialScore recomputed at
//                                         its piece values (LAZY_SAME_MATERIAL: not)
// and records rest = full - partial in histograms, actual exits and wrong exits,
// and (alpha, partial, full) of every call where the partial score is at least
// 100 outside the window for either eval (only those can exit at any margin
// worth testing).
//
// LAZYSTAT_SAMPLE=K keeps every K-th record (histograms and totals count every call).
// LAZYSTAT_GAMELIKE=1 counts only searches with no ucinewgame since the previous
// search: the second search of order_stats.py --warmup (game-like mode).
// LAZYSTAT_OUT=dir: writes dir/hist-<pid>.bin, dir/rec-<pid>.bin, dir/sum-<pid>.txt.
#include "petir.hpp"
#include "lazy_probe.hpp"
#include <algorithm>
#include <bit>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <process.h>

namespace petir_ueval {
std::int32_t evaluateCandidate(const petir_header::tdata& position, std::int32_t sideToMove, std::int32_t alpha, std::int32_t beta, bool opponentPerspective);
extern LazyProbe lazyProbe;
}
namespace petir_ueval_old {
std::int32_t evaluate(const petir_header::tdata& position, std::int32_t sideToMove, std::int32_t alpha, std::int32_t beta, bool opponentPerspective);
extern LazyProbe lazyProbe;
}

namespace {
constexpr int restRange = 4000;  // histogram covers rest in [-4000, 4000], clamped
constexpr int phases = 3, tradeSlots = 13;
// hist[version][phase][-trades / 10][rest + restRange]
unsigned long long hist[2][phases][tradeSlots][2 * restRange + 1];
// byPhase[version][gamePhase 0-24][sides with a queen 0-2][rest + restRange]
unsigned long long byPhase[2][25][3][2 * restRange + 1];

int gamePhase(const petir_header::tdata& p) {
    return std::min(24, std::popcount(p.whiteKnights | p.blackKnights | p.whiteBishops | p.blackBishops)
                        + 2 * std::popcount(p.whiteRooks | p.blackRooks) + 4 * std::popcount(p.whiteQueens | p.blackQueens));
}
int queenSides(const petir_header::tdata& p) { return (p.whiteQueens != 0) + (p.blackQueens != 0); }

#pragma pack(push, 1)
struct Record {
    std::int16_t alpha;
    std::int16_t partialNew, fullNew, partialOld, fullOld;
    std::uint8_t phaseNew, tradesNew;  // tradesNew = -trades / 10
    std::uint8_t phaseOld, flags;      // flags: 1 old reached, 2 stale king board, 4 material mismatch
    std::uint8_t whiteMaterial, blackMaterial;  // non-pawn material (N/B 3, R 5, Q 9)
    std::uint8_t gamePhase, queens;             // gamePhase (N/B 1, R 2, Q 4, 0-24), number of sides with a queen
};
#pragma pack(pop)

struct Totals {
    unsigned long long calls = 0, nullCalls = 0, reachedNew = 0, reachedOld = 0, exitsNew = 0, wrongNew = 0,
        exitsOld = 0, wrongOld = 0, records = 0, materialMismatch = 0, staleBoards = 0, partialMismatch = 0;
} totals;

std::FILE* records = nullptr;
std::string outDir;
unsigned long long recordSample = 1, qualifying = 0;  // LAZYSTAT_SAMPLE=K keeps every K-th record

std::int16_t clamp16(int value) { return static_cast<std::int16_t>(std::clamp(value, -32000, 32000)); }

int material(const petir_header::tdata& p, int pawn, int knight, int bishop, int rook, int queen) {
    return pawn * (std::popcount(p.whitePawns) - std::popcount(p.blackPawns))
         + knight * (std::popcount(p.whiteKnights) - std::popcount(p.blackKnights))
         + bishop * (std::popcount(p.whiteBishops) - std::popcount(p.blackBishops))
         + rook * (std::popcount(p.whiteRooks) - std::popcount(p.blackRooks))
         + queen * (std::popcount(p.whiteQueens) - std::popcount(p.blackQueens));
}

bool staleKingBoard(const petir_header::tdata& p) {
    return std::popcount(p.whiteKingBoard) != 1 || std::popcount(p.blackKingBoard) != 1;
}

bool exits(const LazyProbe& probe, int alpha, int beta) {
    return probe.partial + probe.margin < alpha || probe.partial - probe.margin > beta;
}

void addHist(int version, const LazyProbe& probe, int full, int phase, int queens) {
    const int rest = std::clamp(full - probe.partial, -restRange, restRange);
    ++hist[version][probe.phase][std::clamp(-probe.trades / 10, 0, tradeSlots - 1)][rest + restRange];
    ++byPhase[version][phase][queens][rest + restRange];
}

struct Report {
    ~Report() {
        if (outDir.empty()) return;
        const std::string pid = std::to_string(_getpid());
        if (records) std::fclose(records);
        if (std::FILE* f = std::fopen((outDir + "/hist-" + pid + ".bin").c_str(), "wb")) {
            std::fwrite(hist, sizeof hist, 1, f);
            std::fwrite(byPhase, sizeof byPhase, 1, f);
            std::fclose(f);
        }
        if (std::FILE* f = std::fopen((outDir + "/sum-" + pid + ".txt").c_str(), "w")) {
            std::fprintf(f, "calls %llu\nnullCalls %llu\nreachedNew %llu\nreachedOld %llu\nexitsNew %llu\nwrongNew %llu\n"
                            "exitsOld %llu\nwrongOld %llu\nrecords %llu\nmaterialMismatch %llu\nstaleBoards %llu\npartialMismatch %llu\n",
                         totals.calls, totals.nullCalls, totals.reachedNew, totals.reachedOld, totals.exitsNew, totals.wrongNew,
                         totals.exitsOld, totals.wrongOld, totals.records, totals.materialMismatch, totals.staleBoards,
                         totals.partialMismatch);
            std::fclose(f);
        }
    }
} report;

bool newGameSinceGo = true, counting = true;
}

void lazyStatNewGame() { newGameSinceGo = true; }
void lazyStatGo() {
    static const bool gameLike = std::getenv("LAZYSTAT_GAMELIKE") != nullptr;
    counting = !gameLike || !newGameSinceGo;
    newGameSinceGo = false;
}

std::int32_t petir_ueval::evaluate(const petir_header::tdata& position, std::int32_t sideToMove, std::int32_t alpha, std::int32_t beta, bool opponentPerspective) {
    static const bool init = [] {
        if (const char* dir = std::getenv("LAZYSTAT_OUT")) {
            outDir = dir;
            records = std::fopen((outDir + "/rec-" + std::to_string(_getpid()) + ".bin").c_str(), "wb");
        }
        if (const char* k = std::getenv("LAZYSTAT_SAMPLE")) recordSample = std::max(1ULL, std::strtoull(k, nullptr, 10));
        return true;
    }();
    (void)init;
    if (!counting) return evaluateCandidate(position, sideToMove, alpha, beta, opponentPerspective);
    ++totals.calls;
    lazyProbe.reached = false;
    const std::int32_t result = evaluateCandidate(position, sideToMove, alpha, beta, opponentPerspective);
    if (alpha != beta - 1) return result;
    ++totals.nullCalls;
    const LazyProbe probeNew = lazyProbe;
    if (!probeNew.reached) return result;
    ++totals.reachedNew;
    const bool stale = staleKingBoard(position);
    totals.staleBoards += stale;

    const int phase = gamePhase(position), queens = queenSides(position);
    const int fullNew = evaluateCandidate(position, sideToMove, -20500, 20500, opponentPerspective);
    const bool exitNew = exits(probeNew, alpha, beta);
    if (exitNew) {
        ++totals.exitsNew;
        totals.partialMismatch += result != probeNew.partial;
        totals.wrongNew += (result > alpha) != (fullNew > alpha);
    }
    addHist(0, probeNew, fullNew, phase, queens);

    petir_header::tdata old = position;
    const bool materialMismatch = material(position, petir_header::pawnValue, petir_header::knightValue, petir_header::bishopValue,
                                           petir_header::rookValue, petir_header::queenValue) != position.materialScore;
    totals.materialMismatch += materialMismatch;
#ifdef LAZY_SAME_MATERIAL
    // The second eval is a variant of the same tree: same piece values.
    if (false)
#endif
    old.materialScore = position.materialScore - material(position, petir_header::pawnValue, petir_header::knightValue,
                                                          petir_header::bishopValue, petir_header::rookValue, petir_header::queenValue)
                      + material(position, 90, 312, 319, 500, 975);  // 08c0f11 piece values
    petir_ueval_old::lazyProbe.reached = false;
    const int resultOld = petir_ueval_old::evaluate(old, sideToMove, alpha, beta, opponentPerspective);
    const LazyProbe probeOld = petir_ueval_old::lazyProbe;
    int fullOld = 0;
    if (probeOld.reached) {
        ++totals.reachedOld;
        fullOld = petir_ueval_old::evaluate(old, sideToMove, -20500, 20500, opponentPerspective);
        if (exits(probeOld, alpha, beta)) {
            ++totals.exitsOld;
            totals.wrongOld += (resultOld > alpha) != (fullOld > alpha);
        }
        addHist(1, probeOld, fullOld, phase, queens);
    }

    const auto outside = [&](int partial) { return std::max(alpha - partial, partial - beta) >= 100; };
    if (records && (outside(probeNew.partial) || (probeOld.reached && outside(probeOld.partial))) && qualifying++ % recordSample == 0) {
        Record r{};
        r.alpha = clamp16(alpha);
        r.partialNew = clamp16(probeNew.partial);
        r.fullNew = clamp16(fullNew);
        r.partialOld = clamp16(probeOld.reached ? probeOld.partial : 0);
        r.fullOld = clamp16(fullOld);
        r.phaseNew = static_cast<std::uint8_t>(probeNew.phase);
        r.tradesNew = static_cast<std::uint8_t>(-probeNew.trades / 10);
        r.phaseOld = static_cast<std::uint8_t>(probeOld.reached ? probeOld.phase : 0);
        r.flags = static_cast<std::uint8_t>((probeOld.reached ? 1 : 0) | (stale ? 2 : 0) | (materialMismatch ? 4 : 0));
        r.whiteMaterial = position.whiteNonPawnMaterial;
        r.blackMaterial = position.blackNonPawnMaterial;
        r.gamePhase = static_cast<std::uint8_t>(phase);
        r.queens = static_cast<std::uint8_t>(queens);
        std::fwrite(&r, sizeof r, 1, records);
        ++totals.records;
    }
    return result;
}
