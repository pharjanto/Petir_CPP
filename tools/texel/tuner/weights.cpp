#include "weights.hpp"
#include <bit>
#include <cstdlib>
#include <iostream>
#include <initializer_list>
#include <limits>
#include <tuple>

namespace texel {
namespace {
using petir::Position;

[[noreturn]] void fail(const std::string& message) {
    std::cerr << "evalWeights: " << message << '\n';
    std::exit(2);
}

std::string squareName(int square) {
    return {static_cast<char>('a' + (square & 7)), static_cast<char>('1' + (square >> 3))};
}

std::string indexed(const std::string& name, int index) {
    return name + "[" + std::to_string(index) + "]";
}

// The values a stored weight can take. Byte tables hold their type's range;
// wider ones are limited to what the evaluation sensibly uses.
template<class T> std::pair<int, int> storageRange() {
    if constexpr (sizeof(T) == 1) return {std::numeric_limits<T>::min(), std::numeric_limits<T>::max()};
    else return {-2000, 2000};
}

// One weight for stored values that must stay equal. They must be equal now:
// this is the check that a Black table mirrors the White one.
template<class T> Weight tied(const std::string& name, std::vector<T*> values) {
    for (const T* value : values)
        if (*value != *values.front()) fail(name + ": the values it ties together differ");
    Weight weight;
    weight.name = name;
    std::tie(weight.minimum, weight.maximum) = storageRange<T>();
    weight.get = [first = values.front()] { return static_cast<int>(*first); };
    weight.set = [values](int value) { for (T* stored : values) *stored = static_cast<T>(value); };
    return weight;
}

template<class T> Weight tied(const std::string& name, std::initializer_list<T*> values) {
    return tied(name, std::vector<T*>(values));
}

Weight scalar(const std::string& name, int& value, int minimum = -1000) {
    Weight weight;
    weight.name = name;
    weight.minimum = minimum;
    weight.get = [&value] { return value; };
    weight.set = [&value](int changed) { value = changed; };
    return weight;
}

// Entries first..last of a table used for both colours alike.
template<class Table> void addTable(std::vector<Weight>& weights, const std::string& name, Table& table, int first, int last) {
    for (int index = first; index <= last; ++index) weights.push_back(tied(indexed(name, index), {&table[index]}));
}

// A White piece-square table and its Black mirror (square ^ 56), one weight
// per square from first to last (White's view). `onlyNonZero` keeps the
// squares whose entry is not zero: the outpost tables are only read there.
template<class Table> void addSquareTable(std::vector<Weight>& weights, const std::string& name, Table& white, Table& black,
                                          int piece, int first = 0, int last = 63, bool onlyNonZero = false) {
    for (int square = first; square <= last; ++square) {
        if (onlyNonZero && white[square] == 0) continue;
        auto weight = tied(name + "[" + squareName(square) + "]", {&white[square], &black[square ^ 56]});
        weight.mayCount = [square, piece](const Position& position) {
            return position.board[square] == piece || position.board[square ^ 56] == -piece;
        };
        weights.push_back(std::move(weight));
    }
}

int pieceCount(const Position& position, int piece, bool white) {
    switch (piece) {
    case 1: return std::popcount(white ? position.whitePawns : position.blackPawns);
    case 2: return std::popcount(white ? position.whiteKnights : position.blackKnights);
    case 3: return std::popcount(white ? position.whiteBishops : position.blackBishops);
    case 4: return std::popcount(white ? position.whiteRooks : position.blackRooks);
    case 5: return std::popcount(white ? position.whiteQueens : position.blackQueens);
    }
    return 0;
}

// The piece values as the tuner sees them; the evaluation reads the running
// material score instead.
int pieceValues[6] = {0, petir_header::pawnValue, petir_header::knightValue, petir_header::bishopValue,
                      petir_header::rookValue, petir_header::queenValue};
}

std::vector<Weight> evalWeights() {
    using namespace petir_ueval;
    std::vector<Weight> weights;
    const char* pieceNames[6] = {"", "pawnValue", "knightValue", "bishopValue", "rookValue", "queenValue"};
    for (int piece = 1; piece <= 5; ++piece) {
        auto weight = scalar(pieceNames[piece], pieceValues[piece], 0);
        weight.maximum = 2000;
        weight.piece = piece;
        weight.mayCount = [piece](const Position& position) {
            return pieceCount(position, piece, true) != pieceCount(position, piece, false);
        };
        weights.push_back(std::move(weight));
    }

    // Material balance, tempo and simple patterns.
    weights.push_back(scalar("sideToMoveBonus", sideToMoveBonus));
    weights.push_back(scalar("earlyEndgamePawnBonus", earlyEndgamePawnBonus));
    weights.push_back(scalar("realEndgamePawnBonus", realEndgamePawnBonus));
    weights.push_back(scalar("noPawnPenalty", noPawnPenalty));
    weights.push_back(scalar("queenKnightEndgameBonus", queenKnightEndgameBonus));
    weights.push_back(scalar("queenAgainstPawnsBonus", queenAgainstPawnsBonus));
    weights.push_back(scalar("blockedCenterPawnPenalty", blockedCenterPawnPenalty));
    weights.push_back(scalar("rookBlockedByKingPenalty", rookBlockedByKingPenalty));

    // Piece-square tables. Pawns stand on ranks 2-7 only. The endgame king
    // tables for one wing are mirrored (queenside = kingside ^ 7), so one
    // weight covers four stored entries.
    addSquareTable(weights, "pawnSquareBonus", whitePawnSquareBonus, blackPawnSquareBonus, petir_header::whitePawn, 8, 55);
    addSquareTable(weights, "knightSquareBonus", whiteKnightSquareBonus, blackKnightSquareBonus, petir_header::whiteKnight);
    addSquareTable(weights, "bishopSquareBonus", whiteBishopSquareBonus, blackBishopSquareBonus, petir_header::whiteBishop);
    addSquareTable(weights, "queenSquareBonus", whiteQueenSquareBonus, blackQueenSquareBonus, petir_header::whiteQueen);
    addSquareTable(weights, "kingSquarePenalty", whiteKingSquarePenalty, blackKingSquarePenalty, petir_header::whiteKing);
    addSquareTable(weights, "kingEndgameBothWingsBonus", whiteKingEndgameBothWingsBonus, blackKingEndgameBothWingsBonus,
                   petir_header::whiteKing);
    for (int square = 0; square < 64; ++square) {
        auto weight = tied("kingEndgameKingsideBonus[" + squareName(square) + "]",
                           {&whiteKingEndgameKingsideBonus[square], &blackKingEndgameKingsideBonus[square ^ 56],
                            &whiteKingEndgameQueensideBonus[square ^ 7], &blackKingEndgameQueensideBonus[square ^ 63]});
        weight.mayCount = [square](const Position& position) {
            return position.board[square] == petir_header::whiteKing || position.board[square ^ 7] == petir_header::whiteKing
                || position.board[square ^ 56] == petir_header::blackKing || position.board[square ^ 63] == petir_header::blackKing;
        };
        weights.push_back(std::move(weight));
    }
    // The rook table is the same on every rank and for both colours: one
    // weight per file.
    for (int file = 0; file < 8; ++file) {
        std::vector<std::int8_t*> squares;
        for (int rank = 0; rank < 8; ++rank) squares.push_back(&rookSquareBonus[rank * 8 + file]);
        auto weight = tied(std::string("rookFileBonus[") + static_cast<char>('a' + file) + "]", squares);
        weight.mayCount = [file](const Position& position) {
            return ((position.whiteRooks | position.blackRooks) & petir_bitboard_mask::fileMask[file]) != 0;
        };
        weights.push_back(std::move(weight));
    }
    addSquareTable(weights, "knightOutpostBonus", whiteKnightOutpostBonus, blackKnightOutpostBonus,
                   petir_header::whiteKnight, 0, 63, true);
    addSquareTable(weights, "bishopOutpostBonus", whiteBishopOutpostBonus, blackBishopOutpostBonus,
                   petir_header::whiteBishop, 0, 63, true);

    // Mobility, by the number of squares reached.
    addTable(weights, "knightMobilityBonus", knightMobilityBonus, 0, 8);
    addTable(weights, "bishopMobilityBonus", bishopMobilityBonus, 0, 15);
    addTable(weights, "rookMobilityBonus", rookMobilityBonus, 0, 15);
    addTable(weights, "queenMobilityBonus", queenMobilityBonus, 0, 31);

    // Pawn structure.
    addTable(weights, "doubledPawnPenalty", doubledPawnPenalty, 0, 6);
    addTable(weights, "isolatedPawnPenalty", isolatedPawnPenalty, 0, 7);
    addTable(weights, "isolatedPawnCountPenalty", isolatedPawnCountPenalty, 0, 8);
    weights.push_back(scalar("blockedPawnPenalty", blockedPawnPenalty));
    weights.push_back(scalar("potentialPasserBonus", potentialPasserBonus, 0));
    for (int rank = 1; rank <= 5; ++rank) {
        auto weight = scalar(indexed("candidateRankBonus", rank), candidateRankBonus[rank], 0);
        weight.maximum = 170; // the pawn cache keeps the bonus in a byte
        weights.push_back(std::move(weight));
    }
    weights.push_back(scalar("breakthroughBonus", breakthroughBonus, 0));
    weights.back().maximum = 255;

    // Passed pawns. The rank tables are indexed by White's rank, mirrored for Black.
    weights.push_back(scalar("passedPawnBonus", passedPawnBonus));
    for (int rank = 1; rank <= 6; ++rank) {
        weights.push_back(tied(indexed("passedPawnRankBonus", rank),
                               {&whitePassedPawnRankBonus[rank], &blackPassedPawnRankBonus[7 - rank]}));
        weights.push_back(tied(indexed("kingSupportedPasserBonus", rank),
                               {&whiteKingSupportedPasserBonus[rank], &blackKingSupportedPasserBonus[7 - rank]}));
    }
    weights.push_back(scalar("rookBehindPassedPawnBonus", rookBehindPassedPawnBonus, 0));
    weights.push_back(scalar("safeAdvanceBonus", safeAdvanceBonus));
    weights.push_back(scalar("safeAdvanceOnMoveBonus", safeAdvanceOnMoveBonus));
    weights.push_back(scalar("assuredPromotionBonus", assuredPromotionBonus));
    weights.push_back(scalar("connectedPassersBonus", connectedPassersBonus));
    weights.push_back(scalar("connectedPassersAdvancedBonus", connectedPassersAdvancedBonus));
    weights.push_back(scalar("connectedPassersFarAdvancedBonus", connectedPassersFarAdvancedBonus));
    weights.push_back(scalar("staggeredAdvancedPassersBonus", staggeredAdvancedPassersBonus));
    weights.push_back(scalar("staggeredFarAdvancedPassersBonus", staggeredFarAdvancedPassersBonus));
    weights.push_back(scalar("passerKingDistanceBonus", passerKingDistanceBonus));
    weights.push_back(scalar("passerStopBlockedPenalty", passerStopBlockedPenalty));
    weights.push_back(scalar("passerStopAttackedPenalty", passerStopAttackedPenalty));
    weights.push_back(scalar("passerSafeStopBonus", passerSafeStopBonus));
    weights.push_back(scalar("passerFreePathBonus", passerFreePathBonus));

    // Bishops and knights.
    addTable(weights, "bishopPairBonus", bishopPairBonus, 0, 8);
    addTable(weights, "badBishopPenalty", petir_evalmask::badBishopPenalty, 0, 8);
    weights.push_back(scalar("bishopOverKnightEndgameBonus", bishopOverKnightEndgameBonus, 0));
    weights.push_back(scalar("trappedBishopPenalty", trappedBishopPenalty));
    weights.push_back(scalar("trappedBishopPenalty2", trappedBishopPenalty2));
    weights.push_back(scalar("trappedKnightPenalty", trappedKnightPenalty));
    weights.push_back(scalar("knightTropismBonus", knightTropismBonus));
    weights.push_back(scalar("bishopTropismBonus", bishopTropismBonus));
    weights.push_back(scalar("queenTropismBonus", queenTropismBonus));
    weights.push_back(scalar("twoBishopsKingAttackBonus", twoBishopsKingAttackBonus));
    weights.push_back(scalar("knightBlockadeBonus", knightBlockadeBonus));

    // Rooks.
    addTable(weights, "semiOpenFileBonus", semiOpenFileBonus, 0, 7);
    addTable(weights, "openFileBonus", openFileBonus, 0, 7);
    weights.push_back(scalar("rookSemiOpenFileNearKingBonus", rookSemiOpenFileNearKingBonus));
    weights.push_back(scalar("rookOpenFileNearKingBonus", rookOpenFileNearKingBonus));
    weights.push_back(scalar("doubledRooksBonus", doubledRooksBonus));
    weights.push_back(scalar("doubledRooksSemiOpenNearKingBonus", doubledRooksSemiOpenNearKingBonus));
    weights.push_back(scalar("doubledRooksOpenFileBonus", doubledRooksOpenFileBonus));
    weights.push_back(scalar("doubledRooksNearKingBonus", doubledRooksNearKingBonus));
    weights.push_back(scalar("rookQueenSemiOpenFileBonus", rookQueenSemiOpenFileBonus));
    weights.push_back(scalar("rookQueenSemiOpenNearKingBonus", rookQueenSemiOpenNearKingBonus));
    weights.push_back(scalar("rookQueenOpenFileBonus", rookQueenOpenFileBonus));
    weights.push_back(scalar("rookOnSeventhBonus", rookOnSeventhBonus));
    weights.push_back(scalar("rookOnEighthBonus", rookOnEighthBonus));
    weights.push_back(scalar("doubledRooksSeventhBonus", doubledRooksSeventhBonus));
    weights.push_back(scalar("rookQueenSeventhBonus", rookQueenSeventhBonus));
    weights.push_back(scalar("blockedRookPenalty", blockedRookPenalty));
    weights.push_back(scalar("rookSafeSquareBonus", rookSafeSquareBonus));
    weights.push_back(scalar("rookPawnTargetBonus", rookPawnTargetBonus));
    weights.push_back(scalar("rookTiedDefenderPenalty", rookTiedDefenderPenalty));

    // King safety. The shelter and pressure tables are indexed by counts that
    // stay fixed; their entries are the weights.
    weights.push_back(scalar("lostCastlingPenalty", lostCastlingPenalty, 0));
    weights.push_back(scalar("castledBonus", castledBonus));
    weights.push_back(scalar("castledWrongSideBonus", castledWrongSideBonus));
    weights.push_back(scalar("missingFianchettoPenalty", missingFianchettoPenalty));
    addTable(weights, "pawnShieldPenalty", pawnShieldPenalty, 0, 14);
    addTable(weights, "kingPressurePenalty", kingPressurePenalty, 0, 63);
    return weights;
}

int whiteEval(const Position& position, int sideToMove) {
    constexpr int fullWindow = 20500;
    const int score = petir_ueval::evaluate(position, sideToMove, -fullWindow, fullWindow);
    return sideToMove == petir_header::whiteSide ? score : -score;
}

Position withMaterialChange(const Position& position, const int change[6]) {
    Position changed = position;
    for (int piece = 1; piece <= 5; ++piece)
        changed.materialScore += change[piece] * (pieceCount(position, piece, true) - pieceCount(position, piece, false));
    return changed;
}

std::vector<std::pair<int, double>> coefficients(std::vector<Weight>& weights, const Position& position,
                                                 int sideToMove, int baseEval, int step, bool everyWeight) {
    std::vector<std::pair<int, double>> result;
    for (int index = 0; index < static_cast<int>(weights.size()); ++index) {
        Weight& weight = weights[index];
        if (!everyWeight && weight.mayCount && !weight.mayCount(position)) continue;
        const int value = weight.get();
        const int change = value + step <= weight.maximum ? step : -step;
        int changedEval;
        if (weight.piece) {
            int material[6] = {};
            material[weight.piece] = change;
            changedEval = whiteEval(withMaterialChange(position, material), sideToMove);
        } else {
            weight.set(value + change);
            changedEval = whiteEval(position, sideToMove);
            weight.set(value);
        }
        if (changedEval != baseEval) result.emplace_back(index, static_cast<double>(changedEval - baseEval) / change);
    }
    return result;
}

}
