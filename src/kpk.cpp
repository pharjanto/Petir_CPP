#include "kpk.hpp"
#include "petir.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <vector>

namespace petir_kpk {
namespace {
// Normalize to a white pawn on files a-d, ranks 2-7.
constexpr int positionCount = 24 * 64 * 64 * 2;
constexpr std::uint8_t unknownEntry = 255, invalidEntry = 254, drawEntry = 253;
constexpr int terminalWin = -1, terminalDraw = -2;

constexpr std::uint64_t bitOf(int square) { return UINT64_C(1) << square; }
constexpr auto kingAttackTable = [] {
    std::array<std::uint64_t, 64> result{};
    for (int square = 0; square < 64; ++square)
        for (int rank = 0; rank < 8; ++rank)
            for (int file = 0; file < 8; ++file)
                if (rank >= square / 8 - 1 && rank <= square / 8 + 1
                    && file >= square % 8 - 1 && file <= square % 8 + 1 && rank * 8 + file != square)
                    result[square] |= bitOf(rank * 8 + file);
    return result;
}();

std::uint64_t pawnAttacksFrom(int pawnSquare) {
    return (pawnSquare % 8 ? bitOf(pawnSquare + 7) : 0) | (pawnSquare % 8 < 7 ? bitOf(pawnSquare + 9) : 0);
}

int positionIndex(int strongKing, int weakKing, int pawnSquare, bool defenderToMove) {
    return (((pawnSquare / 8 - 1) * 4 + pawnSquare % 8) * 64 + strongKing) * 128 + weakKing * 2 + defenderToMove;
}

struct Position {
    int strongKing, weakKing, pawnSquare;
    bool defenderToMove;

    explicit Position(int entryIndex) : strongKing((entryIndex / 128) % 64), weakKing((entryIndex / 2) % 64),
        pawnSquare(((entryIndex / 8192) / 4 + 1) * 8 + (entryIndex / 8192) % 4), defenderToMove(entryIndex % 2 != 0) {}

    bool isLegal() const {
        return strongKing != weakKing && strongKing != pawnSquare && weakKing != pawnSquare
            && !(kingAttackTable[strongKing] & bitOf(weakKing))
            && (defenderToMove || !(pawnAttacksFrom(pawnSquare) & bitOf(weakKing)));
    }
};

// A safe promotion wins unless it stalemates. Consider the rook too: a queen
// can stalemate where rook underpromotion wins. K+B/K+N cannot win against K.
bool winningPromotion(const Position& position) {
    const int promotionSquare = position.pawnSquare + 8;
    if ((kingAttackTable[position.weakKing] & bitOf(promotionSquare)) && !(kingAttackTable[position.strongKing] & bitOf(promotionSquare)))
        return false; // The new piece can be captured immediately.
    for (bool asQueen : {true, false}) {
        std::uint64_t attacks = 0;
        for (int rankStep = -1; rankStep <= 1; ++rankStep) for (int fileStep = -1; fileStep <= 1; ++fileStep) {
            if ((!rankStep && !fileStep) || (!asQueen && rankStep && fileStep)) continue;
            int rank = promotionSquare / 8 + rankStep, file = promotionSquare % 8 + fileStep;
            while (rank >= 0 && rank < 8 && file >= 0 && file < 8) {
                const int square = rank * 8 + file;
                attacks |= bitOf(square);
                if (square == position.strongKing) break;
                rank += rankStep;
                file += fileStep;
            }
        }
        if ((attacks & bitOf(position.weakKing)) // Checkmate or a won, non-stalemate KQK/KRK.
            || (kingAttackTable[position.weakKing] & ~(attacks | kingAttackTable[position.strongKing] | bitOf(position.strongKing) | bitOf(promotionSquare))))
            return true;
    }
    return false;
}

struct Moves {
    std::array<int, 10> destinations{};
    unsigned zeroingMask = 0;
    int count = 0;

    void add(int destination, bool resetsClock = false) {
        if (resetsClock) zeroingMask |= 1U << count;
        destinations[count++] = destination;
    }
};

Moves generateMoves(const Position& position) {
    Moves result;
    auto kingTargets = kingAttackTable[position.defenderToMove ? position.weakKing : position.strongKing]
        & ~(kingAttackTable[position.defenderToMove ? position.strongKing : position.weakKing] | bitOf(position.defenderToMove ? position.strongKing : position.weakKing));
    if (position.defenderToMove) {
        kingTargets &= ~pawnAttacksFrom(position.pawnSquare);
        while (kingTargets) {
            const int toSquare = std::countr_zero(kingTargets);
            kingTargets &= kingTargets - 1;
            result.add(toSquare == position.pawnSquare ? terminalDraw : positionIndex(position.strongKing, toSquare, position.pawnSquare, false));
        }
    } else {
        kingTargets &= ~bitOf(position.pawnSquare);
        while (kingTargets) {
            const int toSquare = std::countr_zero(kingTargets);
            kingTargets &= kingTargets - 1;
            result.add(positionIndex(toSquare, position.weakKing, position.pawnSquare, true));
        }
        const int toSquare = position.pawnSquare + 8;
        if (toSquare != position.strongKing && toSquare != position.weakKing) {
            if (position.pawnSquare / 8 == 6)
                result.add(winningPromotion(position) ? terminalWin : terminalDraw, true);
            else {
                result.add(positionIndex(position.strongKing, position.weakKing, toSquare, true), true);
                if (position.pawnSquare / 8 == 1 && toSquare + 8 != position.strongKing && toSquare + 8 != position.weakKing)
                    result.add(positionIndex(position.strongKing, position.weakKing, toSquare + 8, true), true);
            }
        }
    }
    return result;
}

struct Table {
    // Each winning entry stores exact distance to the next winning pawn move.
    // Other entries are draw/invalid. Persistent storage: 192 KiB.
    std::array<std::uint8_t, positionCount> distance{};

    Table() {
        // Cache the graph only during construction to keep initialization fast.
        std::vector<Moves> graph(positionCount);
        for (int index = 0; index < positionCount; ++index) {
            const Position position(index);
            distance[index] = position.isLegal() ? unknownEntry : invalidEntry;
            if (distance[index] == invalidEntry) continue;
            graph[index] = generateMoves(position);
            if (!graph[index].count)
                distance[index] = position.defenderToMove && (pawnAttacksFrom(position.pawnSquare) & bitOf(position.weakKing)) ? 0 : drawEntry;
        }
        // Winning attractor: the pawn side needs one winning successor; the
        // defender must lose after every legal move. Unresolved cycles draw.
        bool changed;
        do {
            changed = false;
            for (int index = 0; index < positionCount; ++index) {
                if (distance[index] != unknownEntry) continue;
                const auto& successors = graph[index];
                bool anyWin = false, allWin = true;
                for (int successorIndex = 0; successorIndex < successors.count; ++successorIndex) {
                    const int successor = successors.destinations[successorIndex];
                    const bool isWin = successor == terminalWin || (successor >= 0 && distance[successor] == 0);
                    anyWin |= isWin;
                    allWin &= isWin;
                }
                if ((index % 2) ? allWin : anyWin) {
                    distance[index] = 0;
                    changed = true;
                }
            }
        } while (changed);

        auto outcomes = distance;
        for (int index = 0; index < positionCount; ++index) {
            if (outcomes[index] == unknownEntry) outcomes[index] = drawEntry;
            distance[index] = outcomes[index] == 0 ? unknownEntry : outcomes[index];
        }
        // DTZ minimax over winning continuations. The pawn side minimizes the
        // time to reset; the defender maximizes it. This also prevents cycling
        // among equally winning king moves and handles an existing rule-50 clock.
        do {
            changed = false;
            for (int index = 0; index < positionCount; ++index) {
                if (outcomes[index] != 0) continue;
                const auto& successors = graph[index];
                int best = index % 2 ? 0 : unknownEntry;
                for (int successorIndex = 0; successorIndex < successors.count; ++successorIndex) {
                    const int successor = successors.destinations[successorIndex];
                    if (successor == terminalDraw || (successor >= 0 && outcomes[successor] != 0)) continue;
                    const int candidate = (successors.zeroingMask & (1U << successorIndex)) ? 1
                        : successor == terminalWin ? 1 : std::min<int>(unknownEntry, distance[successor] + 1);
                    best = index % 2 ? std::max(best, candidate) : std::min(best, candidate);
                }
                if (best < distance[index]) {
                    distance[index] = static_cast<std::uint8_t>(best);
                    changed = true;
                }
            }
        } while (changed);
    }
};

const Table& table() {
    static const Table instance;
    return instance;
}
}

void initialize() { (void)table(); }

bool probe(const petir_header::tdata& position, int sideToMove, int& score) {
    const auto pawns = position.whitePawns | position.blackPawns;
    if (position.whiteNonPawnMaterial || position.blackNonPawnMaterial || !std::has_single_bit(pawns)
        || !std::has_single_bit(position.whiteKingBoard) || !std::has_single_bit(position.blackKingBoard)
        || position.allPieces != (pawns | position.whiteKingBoard | position.blackKingBoard)) return false;
    const bool pawnIsWhite = position.whitePawns != 0;
    int pawnSquare = std::countr_zero(pawns);
    int strongKing = std::countr_zero(pawnIsWhite ? position.whiteKingBoard : position.blackKingBoard);
    int weakKing = std::countr_zero(pawnIsWhite ? position.blackKingBoard : position.whiteKingBoard);
    if (!pawnIsWhite) { pawnSquare ^= 56; strongKing ^= 56; weakKing ^= 56; }
    if (pawnSquare % 8 >= 4) { pawnSquare ^= 7; strongKing ^= 7; weakKing ^= 7; }
    if (pawnSquare / 8 < 1 || pawnSquare / 8 > 6) return false;
    const bool defenderToMove = sideToMove != (pawnIsWhite ? petir_header::whiteSide : petir_header::blackSide);
    const auto distanceToZero = table().distance[positionIndex(strongKing, weakKing, pawnSquare, defenderToMove)];
    if (distanceToZero == invalidEntry) return false;
    if (distanceToZero == drawEntry || position.fiftyMoveCounter >= 100 || position.fiftyMoveCounter + distanceToZero > 100)
        score = 0;
    else {
        // A proven win below queen value, with a progress incentive. Never a
        // mate score: the bitbase proves the outcome, not distance to checkmate.
        score = 700 + 8 * (pawnSquare / 8) - distanceToZero;
        if (defenderToMove) score = -score;
    }
    return true;
}
}
