#include "kpk.hpp"
#include "petir.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <vector>

namespace petir_kpk {
namespace {
// Normalize to a white pawn on files a-d, ranks 2-7.
constexpr int position_count = 24 * 64 * 64 * 2;
constexpr std::uint8_t unknown = 255, invalid = 254, draw = 253;
constexpr int terminal_win = -1, terminal_draw = -2;

constexpr std::uint64_t bit(int square) { return UINT64_C(1) << square; }
constexpr auto king_attacks = [] {
    std::array<std::uint64_t, 64> result{};
    for (int square = 0; square < 64; ++square)
        for (int rank = 0; rank < 8; ++rank)
            for (int file = 0; file < 8; ++file)
                if (rank >= square / 8 - 1 && rank <= square / 8 + 1
                    && file >= square % 8 - 1 && file <= square % 8 + 1 && rank * 8 + file != square)
                    result[square] |= bit(rank * 8 + file);
    return result;
}();

std::uint64_t pawn_attacks(int pawn) {
    return (pawn % 8 ? bit(pawn + 7) : 0) | (pawn % 8 < 7 ? bit(pawn + 9) : 0);
}

int index(int wk, int bk, int pawn, bool defender_to_move) {
    return (((pawn / 8 - 1) * 4 + pawn % 8) * 64 + wk) * 128 + bk * 2 + defender_to_move;
}

struct Position {
    int wk, bk, pawn;
    bool defender_to_move;

    explicit Position(int i) : wk((i / 128) % 64), bk((i / 2) % 64),
        pawn(((i / 8192) / 4 + 1) * 8 + (i / 8192) % 4), defender_to_move(i % 2 != 0) {}

    bool legal() const {
        return wk != bk && wk != pawn && bk != pawn
            && !(king_attacks[wk] & bit(bk))
            && (defender_to_move || !(pawn_attacks(pawn) & bit(bk)));
    }
};

// A safe promotion wins unless it stalemates. Consider the rook too: a queen
// can stalemate where rook underpromotion wins. K+B/K+N cannot win against K.
bool winning_promotion(const Position& p) {
    const int promotion = p.pawn + 8;
    if ((king_attacks[p.bk] & bit(promotion)) && !(king_attacks[p.wk] & bit(promotion)))
        return false; // The new piece can be captured immediately.
    for (bool queen : {true, false}) {
        std::uint64_t attacks = 0;
        for (int dr = -1; dr <= 1; ++dr) for (int df = -1; df <= 1; ++df) {
            if ((!dr && !df) || (!queen && dr && df)) continue;
            int rank = promotion / 8 + dr, file = promotion % 8 + df;
            while (rank >= 0 && rank < 8 && file >= 0 && file < 8) {
                const int square = rank * 8 + file;
                attacks |= bit(square);
                if (square == p.wk) break;
                rank += dr;
                file += df;
            }
        }
        if ((attacks & bit(p.bk)) // Checkmate or a won, non-stalemate KQK/KRK.
            || (king_attacks[p.bk] & ~(attacks | king_attacks[p.wk] | bit(p.wk) | bit(promotion))))
            return true;
    }
    return false;
}

struct Moves {
    std::array<int, 10> next{};
    unsigned zeroing = 0;
    int count = 0;

    void add(int destination, bool resets_clock = false) {
        if (resets_clock) zeroing |= 1U << count;
        next[count++] = destination;
    }
};

Moves moves(const Position& p) {
    Moves result;
    auto targets = king_attacks[p.defender_to_move ? p.bk : p.wk]
        & ~(king_attacks[p.defender_to_move ? p.wk : p.bk] | bit(p.defender_to_move ? p.wk : p.bk));
    if (p.defender_to_move) {
        targets &= ~pawn_attacks(p.pawn);
        while (targets) {
            const int to = std::countr_zero(targets);
            targets &= targets - 1;
            result.add(to == p.pawn ? terminal_draw : index(p.wk, to, p.pawn, false));
        }
    } else {
        targets &= ~bit(p.pawn);
        while (targets) {
            const int to = std::countr_zero(targets);
            targets &= targets - 1;
            result.add(index(to, p.bk, p.pawn, true));
        }
        const int to = p.pawn + 8;
        if (to != p.wk && to != p.bk) {
            if (p.pawn / 8 == 6)
                result.add(winning_promotion(p) ? terminal_win : terminal_draw, true);
            else {
                result.add(index(p.wk, p.bk, to, true), true);
                if (p.pawn / 8 == 1 && to + 8 != p.wk && to + 8 != p.bk)
                    result.add(index(p.wk, p.bk, to + 8, true), true);
            }
        }
    }
    return result;
}

struct Table {
    // Each winning entry stores exact distance to the next winning pawn move.
    // Other entries are draw/invalid. Persistent storage: 192 KiB.
    std::array<std::uint8_t, position_count> distance{};

    Table() {
        // Cache the graph only during construction to keep initialization fast.
        std::vector<Moves> graph(position_count);
        for (int i = 0; i < position_count; ++i) {
            const Position p(i);
            distance[i] = p.legal() ? unknown : invalid;
            if (distance[i] == invalid) continue;
            graph[i] = moves(p);
            if (!graph[i].count)
                distance[i] = p.defender_to_move && (pawn_attacks(p.pawn) & bit(p.bk)) ? 0 : draw;
        }
        // Winning attractor: the pawn side needs one winning successor; the
        // defender must lose after every legal move. Unresolved cycles draw.
        bool changed;
        do {
            changed = false;
            for (int i = 0; i < position_count; ++i) {
                if (distance[i] != unknown) continue;
                const auto& successors = graph[i];
                bool any_win = false, all_win = true;
                for (int j = 0; j < successors.count; ++j) {
                    const int next = successors.next[j];
                    const bool win = next == terminal_win || (next >= 0 && distance[next] == 0);
                    any_win |= win;
                    all_win &= win;
                }
                if ((i % 2) ? all_win : any_win) {
                    distance[i] = 0;
                    changed = true;
                }
            }
        } while (changed);

        auto outcomes = distance;
        for (int i = 0; i < position_count; ++i) {
            if (outcomes[i] == unknown) outcomes[i] = draw;
            distance[i] = outcomes[i] == 0 ? unknown : outcomes[i];
        }
        // DTZ minimax over winning continuations. The pawn side minimizes the
        // time to reset; the defender maximizes it. This also prevents cycling
        // among equally winning king moves and handles an existing rule-50 clock.
        do {
            changed = false;
            for (int i = 0; i < position_count; ++i) {
                if (outcomes[i] != 0) continue;
                const auto& successors = graph[i];
                int best = i % 2 ? 0 : unknown;
                for (int j = 0; j < successors.count; ++j) {
                    const int next = successors.next[j];
                    if (next == terminal_draw || (next >= 0 && outcomes[next] != 0)) continue;
                    const int candidate = (successors.zeroing & (1U << j)) ? 1
                        : next == terminal_win ? 1 : std::min<int>(unknown, distance[next] + 1);
                    best = i % 2 ? std::max(best, candidate) : std::min(best, candidate);
                }
                if (best < distance[i]) {
                    distance[i] = static_cast<std::uint8_t>(best);
                    changed = true;
                }
            }
        } while (changed);
    }
};

const Table& table() {
    static const Table value;
    return value;
}
}

void initialize() { (void)table(); }

bool probe(const petir_header::tdata& p, int stm, int& score) {
    const auto pawns = p.pawnwhite | p.pawnblack;
    if (p.nilai_perwira_putih || p.nilai_perwira_hitam || !std::has_single_bit(pawns)
        || !std::has_single_bit(p.kingwhite) || !std::has_single_bit(p.kingblack)
        || p.allpieces != (pawns | p.kingwhite | p.kingblack)) return false;
    const bool white_pawn = p.pawnwhite != 0;
    int pawn = std::countr_zero(pawns);
    int wk = std::countr_zero(white_pawn ? p.kingwhite : p.kingblack);
    int bk = std::countr_zero(white_pawn ? p.kingblack : p.kingwhite);
    if (!white_pawn) { pawn ^= 56; wk ^= 56; bk ^= 56; }
    if (pawn % 8 >= 4) { pawn ^= 7; wk ^= 7; bk ^= 7; }
    if (pawn / 8 < 1 || pawn / 8 > 6) return false;
    const bool defender_to_move = stm != (white_pawn ? petir_header::_sisiputih : petir_header::_sisihitam);
    const auto dtz = table().distance[index(wk, bk, pawn, defender_to_move)];
    if (dtz == invalid) return false;
    if (dtz == draw || p.move50count >= 100 || p.move50count + dtz > 100)
        score = 0;
    else {
        // A proven win below queen value, with a progress incentive. Never a
        // mate score: the bitbase proves the outcome, not distance to checkmate.
        score = 700 + 8 * (pawn / 8) - dtz;
        if (defender_to_move) score = -score;
    }
    return true;
}
}
