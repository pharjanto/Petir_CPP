// Faithful translation of ueval.pas; Delphi source is in original/.
#include "petir.hpp"
#include "kpk.hpp"
#include <algorithm>
#include <bit>
#include <cstring>
#if defined(_MSC_VER) && defined(_M_X64)
#include <intrin.h>
#endif
namespace petir_ueval {
int passedPawnActivity(const petir_header::tdata& position, int pawnSquare, bool isWhite) {
    using namespace petir_bitboard_mask;
    // Keep this refinement in endgames and leave KPK to its existing evaluator.
    if (position.whiteNonPawnMaterial > 12 || position.blackNonPawnMaterial > 12) return 0;
    const auto allPawns = position.whitePawns | position.blackPawns;
    if (position.whiteNonPawnMaterial == 0 && position.blackNonPawnMaterial == 0
        && !(allPawns & (allPawns - 1))) return 0;
    const int rank = isWhite ? pawnSquare / 8 : 7 - pawnSquare / 8;
    if (rank < 4 || rank > 6) return 0;
    const int step = isWhite ? 8 : -8;
    const int blockSquare = pawnSquare + step;
    const int weight = rank - 3; // Fifth/sixth/seventh rank: 1/2/3.
    const int ownKing = kingSquare(isWhite ? position.whiteKingBoard : position.blackKingBoard);
    const int enemyKing = kingSquare(isWhite ? position.blackKingBoard : position.whiteKingBoard);
    const auto kingDistance = [blockSquare](int king) {
        return std::min(5, std::max(std::abs((king & 7) - (blockSquare & 7)),
                                    std::abs(king / 8 - blockSquare / 8)));
    };
    int bonus = 2 * weight * (kingDistance(enemyKing) - kingDistance(ownKing));
    if (position.board[blockSquare] != 0) return bonus - 8 * weight;

    const auto enemyRooks = isWhite ? position.blackRooks | position.blackQueens
                                  : position.whiteRooks | position.whiteQueens;
    const auto enemyBishops = isWhite ? position.blackBishops | position.blackQueens
                                    : position.whiteBishops | position.whiteQueens;
    const auto enemyKnights = isWhite ? position.blackKnights : position.whiteKnights;
    const auto enemyPawns = isWhite ? position.blackPawns : position.whitePawns;
    const auto enemyKingBoard = isWhite ? position.blackKingBoard : position.whiteKingBoard;
    // A rook/queen behind the pawn attacks its future squares once it advances.
    const auto rearRay = isWhite ? rookRayDownMask[pawnSquare] : rookRayUpMask[pawnSquare];
    const bool rearAttack = (verticalAttacks[pawnSquare][(position.allPiecesRotated90 >> verticalShift[pawnSquare]) & 63]
                              & rearRay & enemyRooks) != 0;
    const auto isAttacked = [&](int square) {
        if (rearAttack || (knightAttacks[square] & enemyKnights)
            || (kingAttacks[square] & enemyKingBoard)
            || ((isWhite ? whitePawnAttacks[square] : blackPawnAttacks[square]) & enemyPawns)) return true;
        const auto straightAttacks = horizontalAttacks[square][(position.allPieces >> ((square & 56) + 1)) & 63]
                            | verticalAttacks[square][(position.allPiecesRotated90 >> verticalShift[square]) & 63];
        if (straightAttacks & enemyRooks) return true;
        const auto diagonalAttacks = diagonalA8H1Attacks[square][(position.allPiecesA8H1 >> diagonalA8H1Shift[square]) & 255]
                            | diagonalH8A1Attacks[square][(position.allPiecesH8A1 >> diagonalH8A1Shift[square]) & 255];
        return (diagonalAttacks & enemyBishops) != 0;
    };
    // Geometric safety is deliberately conservative: defended but attacked
    // squares receive no safe-path bonus. This is not a promotion proof.
    if (isAttacked(blockSquare)) return bonus - 6 * weight;
    bonus += 4 * weight;
    for (int square = blockSquare + step; square >= 0 && square < 64; square += step)
        if (position.board[square] != 0 || isAttacked(square)) return bonus;
    return bonus + 4 * weight;
}

// Count each neighboring pair of foremost passers once, before promotion
// shortcuts. The trailing pawn's rank determines the existing pair bonus.
int connectedPassedPawns(const petir_header::tdata& position, bool isWhite) {
    using namespace petir_bitboard_mask;
    const auto ownPawns = isWhite ? position.whitePawns : position.blackPawns;
    const auto enemyPawns = isWhite ? position.blackPawns : position.whitePawns;
    int passerRanks[8];
    for (int file = 0; file < 8; ++file) {
        passerRanks[file] = -1;
        const auto filePawns = ownPawns & fileMask[file];
        if (!filePawns) continue;
        const int square = isWhite ? 63 - std::countl_zero(filePawns) : std::countr_zero(filePawns);
        if ((isWhite ? whitePassedPawnMask[square] : blackPassedPawnMask[square]) & enemyPawns) continue;
        passerRanks[file] = isWhite ? square / 8 : 7 - square / 8;
    }
    int bonus = 0;
    for (int file = 1; file < 8; ++file) {
        if (passerRanks[file - 1] < 0 || passerRanks[file] < 0) continue;
        const int rank = std::min(passerRanks[file - 1], passerRanks[file]);
        const int separation = std::abs(passerRanks[file - 1] - passerRanks[file]);
        bonus += connectedPassersBonus;
        if (rank >= 5)
            bonus += separation == 0 ? connectedPassersFarAdvancedBonus
                   : separation == 1 ? connectedPassersFarAdvancedBonus + 30
                                     : connectedPassersAdvancedBonus;
        else if (rank >= 4) {
            if (separation == 0) bonus += connectedPassersAdvancedBonus;
            else if (separation == 1) bonus += connectedPassersAdvancedBonus + 20;
        }
    }
    return bonus;
}

// Piece-owner perspective. Small additions to existing mobility/PST terms.
int endgamePieceActivity(const petir_header::tdata& position, int pieceSquare, bool isWhite) {
    using namespace petir_bitboard_mask;
    if (position.whiteNonPawnMaterial > 12 || position.blackNonPawnMaterial > 12) return 0;
    if (!(position.whitePawns | position.blackPawns)) return 0;
    const int pieceType = std::abs(position.board[pieceSquare]);
    if (pieceType != 2 && pieceType != 4) return 0;
    const auto ownPieces = isWhite ? position.whitePieces : position.blackPieces;
    const auto enemyPieces = isWhite ? position.blackPieces : position.whitePieces;
    const auto ownPawns = isWhite ? position.whitePawns : position.blackPawns;
    const auto enemyPawns = isWhite ? position.blackPawns : position.whitePawns;
    const auto attacksFrom = [&](int square, int piece) {
        const int pieceKind = std::abs(piece);
        if (pieceKind == 1) return piece > 0 ? whitePawnAttacks[square] : blackPawnAttacks[square];
        if (pieceKind == 2) return knightAttacks[square];
        if (pieceKind == 6) return kingAttacks[square];
        std::uint64_t reach = 0;
        if (pieceKind == 4 || pieceKind == 5)
            reach = horizontalAttacks[square][(position.allPieces >> ((square & 56) + 1)) & 63]
                  | verticalAttacks[square][(position.allPiecesRotated90 >> verticalShift[square]) & 63];
        if (pieceKind == 3 || pieceKind == 5)
            reach |= diagonalA8H1Attacks[square][(position.allPiecesA8H1 >> diagonalA8H1Shift[square]) & 255]
                   | diagonalH8A1Attacks[square][(position.allPiecesH8A1 >> diagonalH8A1Shift[square]) & 255];
        return reach;
    };
    const auto attackMap = [&](std::uint64_t pieces) {
        std::uint64_t result = 0;
        while (pieces) {
            const int square = std::countr_zero(pieces);
            pieces &= pieces - 1;
            result |= attacksFrom(square, position.board[square]);
        }
        return result;
    };
    const auto enemyAttacks = attackMap(enemyPieces);
    const auto reach = attacksFrom(pieceSquare, position.board[pieceSquare]);
    const auto pieceBit = UINT64_C(1) << pieceSquare;
    // No extra activity reward while the piece itself is under attack.
    // This conservative geometric test does not resolve pins or exchanges.
    if (enemyAttacks & pieceBit) return 0;
    if (pieceType == 2) {
        const int outpostBonus = isWhite ? whiteKnightOutpostBonus[pieceSquare] : blackKnightOutpostBonus[pieceSquare];
        if (!outpostBonus || !((isWhite ? blackPawnAttacks[pieceSquare] : whitePawnAttacks[pieceSquare]) & ownPawns)) return 0;
        if ((isWhite ? petir_evalmask::whiteOutpostMask[pieceSquare] : petir_evalmask::blackOutpostMask[pieceSquare])
            & enemyPawns) return 0;
        const int frontSquare = pieceSquare + (isWhite ? 8 : -8);
        const bool isBlockade = frontSquare >= 0 && frontSquare < 64
                           && (enemyPawns & (UINT64_C(1) << frontSquare));
        // Reward a supported, unchallengeable outpost only with a concrete
        // pawn target or an enemy pawn immediately blocked by this knight.
        if (!(reach & enemyPawns & ~enemyAttacks) && !isBlockade) return 0;
        return outpostBonus / 2 + (isBlockade ? 4 : 0);
    }
    const int safeSquares = std::popcount(reach & ~ownPieces & ~enemyAttacks);
    int bonus = 2 * std::min(6, safeSquares) - 6; // -6..+6, on top of raw mobility.
    bonus += 4 * std::min(2, std::popcount(reach & enemyPawns & ~enemyAttacks));
    if (safeSquares <= 3) {
        const auto otherDefenders = attackMap(ownPieces & ~pieceBit);
        const auto tiedPawns = reach & ownPawns & enemyAttacks & ~otherDefenders;
        bonus -= 3 * std::min(2, std::popcount(tiedPawns));
    }
    return bonus;
}

// Percent of the evaluation retained in exactly KRP versus KR. This is
// soft positional scaling, never an exact draw claim or a tablebase probe.
int rookPawnScale(const petir_header::tdata& position, int sideToMove) {
    using namespace petir_bitboard_mask;
    const auto pawns = position.whitePawns | position.blackPawns;
    if (std::popcount(pawns) != 1 || std::popcount(position.whiteRooks) != 1
        || std::popcount(position.blackRooks) != 1
        || (position.whiteQueens | position.blackQueens | position.whiteBishops | position.blackBishops
            | position.whiteKnights | position.blackKnights)) return 100;
    const bool pawnIsWhite = position.whitePawns != 0;
    const int pawnSquare = kingSquare(pawns);
    const int strongKing = kingSquare(pawnIsWhite ? position.whiteKingBoard : position.blackKingBoard);
    const int weakKing = kingSquare(pawnIsWhite ? position.blackKingBoard : position.whiteKingBoard);
    const int strongRook = kingSquare(pawnIsWhite ? position.whiteRooks : position.blackRooks);
    const int weakRook = kingSquare(pawnIsWhite ? position.blackRooks : position.whiteRooks);
    const auto rookAttacksFrom = [&](int square) {
        return horizontalAttacks[square][(position.allPieces >> ((square & 56) + 1)) & 63]
             | verticalAttacks[square][(position.allPiecesRotated90 >> verticalShift[square]) & 63];
    };
    const auto strongRookAttacks = rookAttacksFrom(strongRook);
    const auto weakRookAttacks = rookAttacksFrom(weakRook);
    const auto pawnAttacks = pawnIsWhite ? whitePawnAttacks[pawnSquare] : blackPawnAttacks[pawnSquare];
    const auto bit = [](int square) { return UINT64_C(1) << square; };
    // Avoid tactical exceptions: checks or either rook under direct attack.
    if ((weakRookAttacks & (bit(strongKing) | bit(strongRook)))
        || (strongRookAttacks & (bit(weakKing) | bit(weakRook)))
        || (pawnAttacks & (bit(weakKing) | bit(weakRook)))
        || (kingAttacks[weakKing] & bit(strongRook))
        || (kingAttacks[strongKing] & bit(weakRook))) return 50;
    // Normalize the pawn owner to White and the pawn to files a-d.
    const int flip = (pawnIsWhite ? 0 : 56) ^ ((pawnSquare & 7) >= 4 ? 7 : 0);
    const int pawn = pawnSquare ^ flip, strongKingSquare = strongKing ^ flip, weakKingSquare = weakKing ^ flip;
    const int strongRookSquare = strongRook ^ flip, weakRookSquare = weakRook ^ flip;
    const int file = pawn & 7, rank = pawn / 8, promotionSquare = 56 + file;
    const auto distance = [](int first, int second) {
        return std::max(std::abs((first & 7) - (second & 7)), std::abs(first / 8 - second / 8));
    };
    // Established third-rank defense, with lateral room for the rook.
    if (rank <= 4 && strongKingSquare / 8 <= 4 && distance(weakKingSquare, promotionSquare) <= 1 && weakRookSquare / 8 == 5
        && std::abs((weakRookSquare & 7) - file) >= 2
        && std::abs((weakRookSquare & 7) - (weakKingSquare & 7)) >= 2) return 25;
    // King on the promotion square, rear checks, attacking king too far away.
    if (rank >= 5 && weakKingSquare == promotionSquare && weakRookSquare / 8 == 0 && distance(strongKingSquare, pawn) >= 2) return 25;
    // Rook pawn with its own rook trapped in front and a distant defending king.
    if (pawn == 48 && strongRookSquare == 56 && (weakKingSquare == 54 || weakKingSquare == 55) && (weakRookSquare & 7) == 0
        && weakRookSquare / 8 <= 2 && ((strongKingSquare & 7) >= 3 || strongKingSquare / 8 <= 4)) return 25;
    // Favorable seventh-rank non-rook pawn: king on promotion square,
    // distant defending king, usable rook and the pawn owner's move.
    if (rank == 6 && file != 0 && strongKingSquare == promotionSquare && std::abs((weakKingSquare & 7) - file) >= 2
        && weakKingSquare / 8 >= 5 && strongRookSquare / 8 <= 3 && (strongRookSquare & 7) != file && distance(weakKingSquare, strongRookSquare) >= 3
        && sideToMove == (pawnIsWhite ? petir_header::whiteSide : petir_header::blackSide)
        && !(weakRookAttacks & bit(pawnSquare)) && !(kingAttacks[weakKing] & bit(pawnSquare))) return 100;
    return 50;
}

namespace {
// These helpers are only used in loops that have already checked bits != 0.
int popLowestBit(std::uint64_t& bits) {
#if defined(_MSC_VER) && defined(_M_X64)
    unsigned long square;
    _BitScanForward64(&square, bits);
#else
    const auto square = std::countr_zero(bits);
#endif
    bits &= bits - 1;
    return static_cast<int>(square);
}

int popHighestBit(std::uint64_t& bits) {
#if defined(_MSC_VER) && defined(_M_X64)
    unsigned long square;
    _BitScanReverse64(&square, bits);
#else
    const auto square = 63 - std::countl_zero(bits);
#endif
    bits ^= UINT64_C(1) << square;
    return static_cast<int>(square);
}

std::uint8_t pawnFiles(std::uint64_t pawns) {
    pawns |= pawns >> 32;
    pawns |= pawns >> 16;
    pawns |= pawns >> 8;
    return static_cast<std::uint8_t>(pawns);
}

#if defined(_MSC_VER) && defined(_M_X64)
const bool hasNativePopcount = [] {
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);
    return (cpuInfo[2] & (1 << 23)) != 0;
}();
#endif

template<bool NativePopcount>
std::uint8_t countBits(std::uint64_t bits) {
#if defined(_MSC_VER) && defined(_M_X64)
    if constexpr (NativePopcount) {
        return static_cast<std::uint8_t>(__popcnt64(bits));
    } else {
        // Keep a hardware-independent path for CPUs without POPCNT.
        return static_cast<std::uint8_t>(
            petir_evalmask::bitCountTable[bits & 0xffff]
            + petir_evalmask::bitCountTable[(bits >> 16) & 0xffff]
            + petir_evalmask::bitCountTable[(bits >> 32) & 0xffff]
            + petir_evalmask::bitCountTable[bits >> 48]);
    }
#else
    return static_cast<std::uint8_t>(std::popcount(bits));
#endif
}

// ueval.pas:850
template<bool NativePopcount>
std::int32_t evaluateImpl(const petir_header::tdata& position, std::int32_t sideToMove, std::int32_t alpha, std::int32_t beta, bool opponentPerspective) {
    std::int32_t result{};
    bool kingsOnOppositeWings{};
    bool likelyDraw{};
    bool stackedOnFile{};
    bool nearEnemyKingWing{};
    bool realEndgame{};
    bool earlyEndgame{};
    bool endgame{};
    std::int32_t score{};
    std::uint64_t attacks{};
    std::uint64_t pieces{};
    std::int32_t square{};
    std::int32_t rookFile{};
    std::int32_t termScore{};
    std::int32_t file{};
    std::int32_t weightedTerm{};
    std::int32_t pawnsOnBishopColor{};
    std::int32_t whiteKingSquare{};
    std::int32_t blackKingSquare{};
    std::int32_t whiteKingFile{};
    std::int32_t blackKingFile{};
    std::int32_t whitePassedFiles{};
    std::int32_t blackPassedFiles{};
    std::int32_t blackKingPressure{};
    std::int32_t whiteKingPressure{};
    std::int32_t lazyMargin{};
    std::int32_t bishopSquare{};
    std::int32_t pawnTally{};
    std::int32_t scratchValue{};
    std::uint64_t rankAttacks{};
    std::uint64_t fileAttacks{};
    std::uint64_t whitePawnAttackMap{};
    std::uint64_t blackPawnAttackMap{};
    std::uint64_t scratchBits{};
    std::uint64_t kingZoneHits{};
    std::uint64_t queenMobilitySquares{};
    std::uint64_t whiteKingZone{};
    std::uint64_t blackKingZone{};
    bool evaluateWhiteKingSafety{};
    bool evaluateBlackKingSafety{};
    std::int32_t whitePassedCount{};
    std::int32_t blackPassedCount{};
    std::int32_t whitePawnCount{};
    std::int32_t blackPawnCount{};
    std::int32_t blackAttackerCount{};
    std::int32_t whiteAttackerCount{};
    std::int32_t realSideToMove{};
    std::uint64_t whiteRookAttackMap{};
    std::uint64_t blackRookAttackMap{};
    std::uint64_t whiteBishopAttackMap{};
    std::uint64_t blackBishopAttackMap{};
    std::uint64_t blackQueenAttackMap{};
    std::uint64_t whiteQueenAttackMap{};
    std::uint64_t whiteKnightAttackMap{};
    std::uint64_t blackKnightAttackMap{};
    std::int32_t blackBishopCase{};
    std::int32_t whiteBishopCase{};
    std::int32_t whiteSupporterCase{};
    std::int32_t blackSupporterCase{};
    std::int32_t whiteAttackerCase{};
    std::int32_t blackAttackerCase{};
    likelyDraw = false;
    if (((petir_ueval::isMaterialDraw(position, likelyDraw) || ((opponentPerspective == false) && petir_ueval::isTheoreticalDraw(position, sideToMove))) || ((opponentPerspective == true) && petir_ueval::isTheoreticalDraw(position, pas::sub(3, sideToMove))))) {
        result = petir_header::drawScore;
        return result;
    }
    blackPawnCount = 0;
    whitePawnCount = 0;
    whitePassedCount = 0;
    blackPassedCount = 0;
    realSideToMove = sideToMove;
    if (opponentPerspective) {
        realSideToMove = pas::sub(3, sideToMove);
    }
    whiteAttackerCount = 0;
    blackAttackerCount = 0;
    score = position.materialScore;
    if ((((position.board[petir_header::h6] == petir_header::whiteBishop) && (position.board[petir_header::g5] == petir_header::blackPawn)) && ((position.board[petir_header::f6] == petir_header::blackPawn) || (petir_usee::staticExchange(petir_header::whiteSide, position, petir_header::h6, petir_header::g5) < 0)))) {
        (score = pas::decrement(score, petir_ueval::trappedBishopPenalty2));
    }
    if ((((position.board[petir_header::a6] == petir_header::whiteBishop) && (position.board[petir_header::b5] == petir_header::blackPawn)) && ((position.board[petir_header::c6] == petir_header::blackPawn) || (petir_usee::staticExchange(petir_header::whiteSide, position, petir_header::a6, petir_header::b5) < 0)))) {
        (score = pas::decrement(score, petir_ueval::trappedBishopPenalty2));
    }
    if ((((position.board[petir_header::h3] == petir_header::blackBishop) && (position.board[petir_header::g4] == petir_header::whitePawn)) && ((position.board[petir_header::f3] == petir_header::whitePawn) || (petir_usee::staticExchange(petir_header::blackSide, position, petir_header::h3, petir_header::g4) < 0)))) {
        (score = pas::increment(score, petir_ueval::trappedBishopPenalty2));
    }
    if ((((position.board[petir_header::a3] == petir_header::blackBishop) && (position.board[petir_header::b4] == petir_header::whitePawn)) && ((position.board[petir_header::c3] == petir_header::whitePawn) || (petir_usee::staticExchange(petir_header::blackSide, position, petir_header::a3, petir_header::b4) < 0)))) {
        (score = pas::increment(score, petir_ueval::trappedBishopPenalty2));
    }
    if ((((position.board[petir_header::h7] == petir_header::whiteBishop) && (((position.board[petir_header::g7] == petir_header::blackPawn) && (realSideToMove == petir_header::blackSide)) || (position.board[petir_header::g6] == petir_header::blackPawn))) && ((position.board[petir_header::f7] == petir_header::blackPawn) || (petir_usee::staticExchange(petir_header::whiteSide, position, petir_header::h7, petir_header::g6) < 0)))) {
        (score = pas::decrement(score, petir_ueval::trappedBishopPenalty));
    }
    if ((((position.board[petir_header::a7] == petir_header::whiteBishop) && (((position.board[petir_header::b7] == petir_header::blackPawn) && (realSideToMove == petir_header::blackSide)) || (position.board[petir_header::b6] == petir_header::blackPawn))) && ((position.board[petir_header::c7] == petir_header::blackPawn) || (petir_usee::staticExchange(petir_header::whiteSide, position, petir_header::a7, petir_header::b6) < 0)))) {
        (score = pas::decrement(score, petir_ueval::trappedBishopPenalty));
    }
    if ((((position.board[petir_header::h2] == petir_header::blackBishop) && (((position.board[petir_header::g2] == petir_header::whitePawn) && (realSideToMove == petir_header::whiteSide)) || (position.board[petir_header::g3] == petir_header::whitePawn))) && ((position.board[petir_header::f2] == petir_header::whitePawn) || (petir_usee::staticExchange(petir_header::blackSide, position, petir_header::h2, petir_header::g3) < 0)))) {
        (score = pas::increment(score, petir_ueval::trappedBishopPenalty));
    }
    if ((((position.board[petir_header::a2] == petir_header::blackBishop) && (((position.board[petir_header::b2] == petir_header::whitePawn) && (realSideToMove == petir_header::whiteSide)) || (position.board[petir_header::b3] == petir_header::whitePawn))) && ((position.board[petir_header::c2] == petir_header::whitePawn) || (petir_usee::staticExchange(petir_header::blackSide, position, petir_header::a2, petir_header::b3) < 0)))) {
        (score = pas::increment(score, petir_ueval::trappedBishopPenalty));
    }
    if ((((position.board[petir_header::a8] == petir_header::whiteKnight) && (petir_usee::staticExchange(petir_header::whiteSide, position, petir_header::a8, petir_header::c7) < 0)) && (petir_usee::staticExchange(petir_header::whiteSide, position, petir_header::a8, petir_header::b6) < 0))) {
        (score = pas::decrement(score, petir_ueval::trappedKnightPenalty));
    }
    if ((((position.board[petir_header::h8] == petir_header::whiteKnight) && (petir_usee::staticExchange(petir_header::whiteSide, position, petir_header::h8, petir_header::f7) < 0)) && (petir_usee::staticExchange(petir_header::whiteSide, position, petir_header::h8, petir_header::g6) < 0))) {
        (score = pas::decrement(score, petir_ueval::trappedKnightPenalty));
    }
    if ((((position.board[petir_header::a1] == petir_header::blackKnight) && (petir_usee::staticExchange(petir_header::blackSide, position, petir_header::a1, petir_header::c2) < 0)) && (petir_usee::staticExchange(petir_header::blackSide, position, petir_header::a1, petir_header::b3) < 0))) {
        (score = pas::increment(score, petir_ueval::trappedKnightPenalty));
    }
    if ((((position.board[petir_header::h1] == petir_header::blackKnight) && (petir_usee::staticExchange(petir_header::blackSide, position, petir_header::h1, petir_header::f2) < 0)) && (petir_usee::staticExchange(petir_header::blackSide, position, petir_header::h1, petir_header::g3) < 0))) {
        (score = pas::increment(score, petir_ueval::trappedKnightPenalty));
    }
    if (((position.whiteNonPawnMaterial < 16) || (position.blackNonPawnMaterial < 16))) {
        realEndgame = ((position.whiteNonPawnMaterial <= 10) && (position.blackNonPawnMaterial <= 10));
        endgame = ((position.whiteNonPawnMaterial <= 12) && (position.blackNonPawnMaterial <= 12));
        if (realEndgame) {
            (score = pas::increment(score, pas::shiftLeft(countBits<NativePopcount>(position.whitePawns), 3)));
            (score = pas::decrement(score, pas::shiftLeft(countBits<NativePopcount>(position.blackPawns), 3)));
        } else {
            (score = pas::increment(score, pas::shiftLeft(countBits<NativePopcount>(position.whitePawns), 2)));
            (score = pas::decrement(score, pas::shiftLeft(countBits<NativePopcount>(position.blackPawns), 2)));
        }
        if (((((position.whiteNonPawnMaterial == 12) && (position.blackNonPawnMaterial == 12)) && (position.blackQueens != 0)) && (position.whiteQueens != 0))) {
            if ((position.whiteKnights != 0)) {
                (score = pas::increment(score, petir_ueval::queenKnightEndgameBonus));
            }
            if ((position.blackKnights != 0)) {
                (score = pas::decrement(score, petir_ueval::queenKnightEndgameBonus));
            }
        }
        earlyEndgame = true;
    } else {
        endgame = false;
        earlyEndgame = false;
        realEndgame = false;
    }
    if ((realSideToMove == petir_header::whiteSide)) {
        (score = pas::increment(score, 10));
    } else {
        (score = pas::decrement(score, 10));
    }
    if (((position.whiteNonPawnMaterial > 17) && (position.blackNonPawnMaterial > 17))) {
        if ((((position.board[petir_header::f1] == petir_header::whiteKing) || (position.board[petir_header::g1] == petir_header::whiteKing)) && (((position.board[petir_header::h1] == petir_header::whiteRook) || (position.board[petir_header::g1] == petir_header::whiteRook)) || (position.board[petir_header::h2] == petir_header::whiteRook)))) {
            (score = pas::decrement(score, petir_ueval::rookBlockedByKingPenalty));
        }
        if ((((position.board[petir_header::c1] == petir_header::whiteKing) || (position.board[petir_header::b1] == petir_header::whiteKing)) && (((position.board[petir_header::a1] == petir_header::whiteRook) || (position.board[petir_header::a2] == petir_header::whiteRook)) || (position.board[petir_header::b1] == petir_header::whiteRook)))) {
            (score = pas::decrement(score, petir_ueval::rookBlockedByKingPenalty));
        }
        if ((((position.board[petir_header::f8] == petir_header::blackKing) || (position.board[petir_header::g8] == petir_header::blackKing)) && (((position.board[petir_header::h8] == petir_header::blackRook) || (position.board[petir_header::g8] == petir_header::blackRook)) || (position.board[petir_header::h7] == petir_header::blackRook)))) {
            (score = pas::increment(score, petir_ueval::rookBlockedByKingPenalty));
        }
        if ((((position.board[petir_header::c8] == petir_header::blackKing) || (position.board[petir_header::b8] == petir_header::blackKing)) && (((position.board[petir_header::a8] == petir_header::blackRook) || (position.board[petir_header::b8] == petir_header::blackRook)) || (position.board[petir_header::a7] == petir_header::blackRook)))) {
            (score = pas::increment(score, petir_ueval::rookBlockedByKingPenalty));
        }
    }
    // Complete evaluation when its score needs the later draw scaling.
    if (!likelyDraw && position.fiftyMoveCounter < 80 && alpha == pas::sub(beta, 1)) {
        lazyMargin = petir_ueval::lazyMarginAdjustment(position);
        if (endgame) {
            (lazyMargin = pas::increment(lazyMargin, 600));
        } else {
            if (earlyEndgame) {
                (lazyMargin = pas::increment(lazyMargin, 200));
            }
        }
        if ((sideToMove == petir_header::whiteSide)) {
            termScore = score;
        } else {
            termScore = pas::negate(score);
        }
        if ((pas::add(pas::add(termScore, 280), lazyMargin) < alpha)) {
            result = termScore;
            return result;
        }
        if ((pas::sub(termScore, pas::add(280, lazyMargin)) > beta)) {
            result = termScore;
            return result;
        }
    }
    whiteKingPressure = 0;
    blackKingPressure = 0;
    evaluateWhiteKingSafety = true;
    evaluateBlackKingSafety = true;
    if (endgame) {
        evaluateWhiteKingSafety = false;
        evaluateBlackKingSafety = false;
    } else {
        if ((position.blackQueens == 0)) {
            evaluateWhiteKingSafety = false;
        }
        if ((position.whiteQueens == 0)) {
            evaluateBlackKingSafety = false;
        }
    }
    whitePawnAttackMap = 0;
    blackPawnAttackMap = 0;
    whitePassedFiles = 0;
    blackPassedFiles = 0;
    blackKingSquare = petir_bitboard_mask::kingSquare(position.blackKingBoard);
    whiteKingSquare = petir_bitboard_mask::kingSquare(position.whiteKingBoard);
    whiteKingFile = (whiteKingSquare & 7);
    blackKingFile = (blackKingSquare & 7);
    if (earlyEndgame) {
        kingsOnOppositeWings = false;
    } else {
        kingsOnOppositeWings = ((((position.whiteCastleType == petir_header::castledShort) && (position.blackCastleType == petir_header::castledLong)) || ((position.blackCastleType == petir_header::castledShort) && (position.whiteCastleType == petir_header::castledLong))) || (pas::abs(pas::sub(whiteKingFile, blackKingFile)) >= 3));
    }
    whiteKingZone = (petir_bitboard_mask::kingAttacks[whiteKingSquare] | position.whiteKingBoard);
    blackKingZone = (petir_bitboard_mask::kingAttacks[blackKingSquare] | position.blackKingBoard);
    whiteRookAttackMap = 0;
    blackRookAttackMap = 0;
    whiteQueenAttackMap = 0;
    blackQueenAttackMap = 0;
    whiteKnightAttackMap = 0;
    blackKnightAttackMap = 0;
    whiteBishopAttackMap = 0;
    blackBishopAttackMap = 0;
    if (((position.blackCastleType == petir_header::notCastled) && ((position.castleRights & petir_header::blackCastleRights) == 0))) {
        if ((position.whiteQueens != 0)) {
            (score = pas::increment(score, petir_ueval::lostCastlingPenalty));
        } else {
            (score = pas::increment(score, pas::shiftRight(petir_ueval::lostCastlingPenalty, 1)));
        }
    } else {
        if ((position.blackCastleType != petir_header::notCastled)) {
            if (((position.blackCastleType == petir_header::castledLong) && (position.whiteCastleType == petir_header::castledShort))) {
                (score = pas::increment(score, petir_ueval::castledWrongSideBonus));
            } else {
                (score = pas::decrement(score, petir_ueval::castledBonus));
            }
        }
    }
    if (((position.whiteCastleType == petir_header::notCastled) && ((position.castleRights & petir_header::whiteCastleRights) == 0))) {
        if ((position.blackQueens != 0)) {
            (score = pas::decrement(score, petir_ueval::lostCastlingPenalty));
        } else {
            (score = pas::decrement(score, pas::shiftRight(petir_ueval::lostCastlingPenalty, 1)));
        }
    } else {
        if ((position.whiteCastleType != petir_header::notCastled)) {
            if (((position.whiteCastleType == petir_header::castledLong) && (position.blackCastleType == petir_header::castledShort))) {
                (score = pas::decrement(score, petir_ueval::castledWrongSideBonus));
            } else {
                (score = pas::increment(score, petir_ueval::castledBonus));
            }
        }
    }
    if ((((position.blackQueens != 0) && (position.blackNonPawnMaterial >= 16)) || (position.blackNonPawnMaterial >= 19))) {
        termScore = 0;
        if ((whiteKingFile >= 5)) {
            if (((petir_bitboard_mask::fileMask[6] & position.whitePawns) == 0)) {
                (termScore = pas::increment(termScore, 4));
                if (((petir_bitboard_mask::fileMask[6] & position.blackPawns) == 0)) {
                    (termScore = pas::increment(termScore, 1));
                }
            } else {
                if (((position.board[14] != petir_header::whitePawn) && (position.board[22] != petir_header::whitePawn))) {
                    (termScore = pas::increment(termScore, 2));
                }
            }
            if (((petir_bitboard_mask::fileMask[7] & position.whitePawns) == 0)) {
                (termScore = pas::increment(termScore, 2));
                if (((petir_bitboard_mask::fileMask[7] & position.blackPawns) == 0)) {
                    (termScore = pas::increment(termScore, 1));
                }
            } else {
                if (((position.board[15] != petir_header::whitePawn) && (position.board[23] != petir_header::whitePawn))) {
                    (termScore = pas::increment(termScore, 2));
                }
            }
            if (((petir_bitboard_mask::fileMask[5] & position.whitePawns) == 0)) {
                (termScore = pas::increment(termScore, 1));
            }
            if (((position.board[petir_header::g2] != petir_header::whiteBishop) && (position.board[petir_header::g2] != petir_header::whitePawn))) {
                (score = pas::decrement(score, petir_ueval::missingFianchettoPenalty));
            }
            if ((position.whiteCastleType != position.blackCastleType)) {
                (termScore = pas::increment(termScore, 1));
            }
            if ((position.blackQueens == 0)) {
                termScore = (termScore / 2);
            }
            (score = pas::decrement(score, petir_ueval::pawnShieldPenalty[termScore]));
        } else {
            if ((whiteKingFile <= 2)) {
                if (((petir_bitboard_mask::fileMask[0] & position.whitePawns) == 0)) {
                    (termScore = pas::increment(termScore, 3));
                    if (((petir_bitboard_mask::fileMask[0] & position.blackPawns) == 0)) {
                        (termScore = pas::increment(termScore, 1));
                    }
                } else {
                    if (((position.board[8] != petir_header::whitePawn) && (position.board[16] != petir_header::whitePawn))) {
                        (termScore = pas::increment(termScore, 2));
                    }
                }
                if (((petir_bitboard_mask::fileMask[1] & position.whitePawns) == 0)) {
                    (termScore = pas::increment(termScore, 4));
                    if (((petir_bitboard_mask::fileMask[1] & position.blackPawns) == 0)) {
                        (termScore = pas::increment(termScore, 1));
                    }
                } else {
                    if (((position.board[9] != petir_header::whitePawn) && (position.board[17] != petir_header::whitePawn))) {
                        (termScore = pas::increment(termScore, 2));
                    }
                }
                if (((petir_bitboard_mask::fileMask[2] & position.whitePawns) == 0)) {
                    (termScore = pas::increment(termScore, 1));
                }
                if (((position.board[petir_header::b2] != petir_header::whiteBishop) && (position.board[petir_header::b2] != petir_header::whitePawn))) {
                    (score = pas::decrement(score, petir_ueval::missingFianchettoPenalty));
                }
                if ((position.whiteCastleType != position.blackCastleType)) {
                    (termScore = pas::increment(termScore, 1));
                }
                if ((position.blackQueens == 0)) {
                    termScore = (termScore / 2);
                }
                (score = pas::decrement(score, petir_ueval::pawnShieldPenalty[termScore]));
            } else {
                file = whiteKingFile;
                if ((file > 0)) {
                    if (((petir_bitboard_mask::fileMask[pas::sub(file, 1)] & position.whitePawns) == 0)) {
                        (termScore = pas::increment(termScore, 2));
                    }
                }
                if (((petir_bitboard_mask::fileMask[file] & position.whitePawns) == 0)) {
                    (termScore = pas::increment(termScore, 2));
                }
                if ((file < 7)) {
                    if (((petir_bitboard_mask::fileMask[pas::add(file, 1)] & position.whitePawns) == 0)) {
                        (termScore = pas::increment(termScore, 2));
                    }
                }
                if ((position.blackQueens == 0)) {
                    termScore = (termScore / 2);
                }
                (score = pas::decrement(score, petir_ueval::pawnShieldPenalty[termScore]));
            }
        }
    }
    if ((((position.whiteQueens != 0) && (position.whiteNonPawnMaterial >= 16)) || (position.whiteNonPawnMaterial >= 19))) {
        termScore = 0;
        if ((blackKingFile >= 5)) {
            if (((petir_bitboard_mask::fileMask[6] & position.blackPawns) == 0)) {
                (termScore = pas::increment(termScore, 4));
                if (((petir_bitboard_mask::fileMask[6] & position.whitePawns) == 0)) {
                    (termScore = pas::increment(termScore, 1));
                }
            } else {
                if (((position.board[54] != petir_header::blackPawn) && (position.board[46] != petir_header::blackPawn))) {
                    (termScore = pas::increment(termScore, 2));
                }
            }
            if (((petir_bitboard_mask::fileMask[7] & position.blackPawns) == 0)) {
                (termScore = pas::increment(termScore, 2));
                if (((petir_bitboard_mask::fileMask[7] & position.whitePawns) == 0)) {
                    (termScore = pas::increment(termScore, 1));
                }
            } else {
                if (((position.board[55] != petir_header::blackPawn) && (position.board[47] != petir_header::blackPawn))) {
                    (termScore = pas::increment(termScore, 2));
                }
            }
            if (((petir_bitboard_mask::fileMask[5] & position.blackPawns) == 0)) {
                (termScore = pas::increment(termScore, 1));
            }
            if (((position.board[petir_header::g7] != petir_header::blackPawn) && (position.board[petir_header::g7] != petir_header::blackBishop))) {
                (score = pas::increment(score, petir_ueval::missingFianchettoPenalty));
            }
            if ((position.whiteCastleType != position.blackCastleType)) {
                (termScore = pas::increment(termScore, 1));
            }
            if ((position.whiteQueens == 0)) {
                termScore = (termScore / 2);
            }
            (score = pas::increment(score, petir_ueval::pawnShieldPenalty[termScore]));
        } else {
            if ((blackKingFile <= 2)) {
                if (((petir_bitboard_mask::fileMask[0] & position.blackPawns) == 0)) {
                    (termScore = pas::increment(termScore, 3));
                    if (((petir_bitboard_mask::fileMask[0] & position.whitePawns) == 0)) {
                        (termScore = pas::increment(termScore, 1));
                    }
                } else {
                    if (((position.board[48] != petir_header::blackPawn) && (position.board[40] != petir_header::blackPawn))) {
                        (termScore = pas::increment(termScore, 2));
                    }
                }
                if (((petir_bitboard_mask::fileMask[1] & position.blackPawns) == 0)) {
                    (termScore = pas::increment(termScore, 4));
                    if (((petir_bitboard_mask::fileMask[1] & position.whitePawns) == 0)) {
                        (termScore = pas::increment(termScore, 1));
                    }
                } else {
                    if (((position.board[49] != petir_header::blackPawn) && (position.board[41] != petir_header::blackPawn))) {
                        (termScore = pas::increment(termScore, 2));
                    }
                }
                if (((petir_bitboard_mask::fileMask[2] & position.blackPawns) == 0)) {
                    (termScore = pas::increment(termScore, 1));
                }
                if (((position.board[petir_header::b7] != petir_header::blackPawn) && (position.board[petir_header::b7] != petir_header::blackBishop))) {
                    (score = pas::increment(score, petir_ueval::missingFianchettoPenalty));
                }
                if ((position.whiteCastleType != position.blackCastleType)) {
                    (termScore = pas::increment(termScore, 1));
                }
                if ((position.whiteQueens == 0)) {
                    termScore = (termScore / 2);
                }
                (score = pas::increment(score, petir_ueval::pawnShieldPenalty[termScore]));
            } else {
                if ((blackKingFile > 0)) {
                    if (((petir_bitboard_mask::fileMask[pas::sub(blackKingFile, 1)] & position.blackPawns) == 0)) {
                        (termScore = pas::increment(termScore, 2));
                    }
                }
                if (((petir_bitboard_mask::fileMask[blackKingFile] & position.blackPawns) == 0)) {
                    (termScore = pas::increment(termScore, 2));
                }
                if ((blackKingFile < 7)) {
                    if (((petir_bitboard_mask::fileMask[pas::add(blackKingFile, 1)] & position.blackPawns) == 0)) {
                        (termScore = pas::increment(termScore, 2));
                    }
                }
                if ((position.whiteQueens == 0)) {
                    termScore = (termScore / 2);
                }
                (score = pas::increment(score, petir_ueval::pawnShieldPenalty[termScore]));
            }
        }
    }
    blackBishopCase = position.blackBishopCount;
    if (((blackBishopCase == 1))) {
        bishopSquare = petir_bitboard_mask::lastBit(pas::address(position.blackBishops));
        if ((endgame && (position.whiteBishopCount == 0))) {
            scratchBits = (position.blackPawns | position.whitePawns);
            if ((((scratchBits & petir_evalmask::filesABMask) != 0) && ((scratchBits & petir_evalmask::filesGHMask) != 0))) {
                if (realEndgame) {
                    (score = pas::decrement(score, petir_ueval::bishopOverKnightEndgameBonus));
                } else {
                    (score = pas::decrement(score, pas::shiftRight(petir_ueval::bishopOverKnightEndgameBonus, 1)));
                }
            }
        }
    }
    else if (((blackBishopCase == 2))) {
        bishopSquare = 64;
        if ((position.whiteBishopCount == 0)) {
            (score = pas::decrement(score, petir_ueval::bishopPairBonus[pas::shiftRight(countBits<NativePopcount>((position.blackPawns | position.whitePawns)), 1)]));
        }
        if ((endgame && (position.whiteBishopCount == 1))) {
            scratchBits = (position.blackPawns | position.whitePawns);
            if ((((scratchBits & petir_evalmask::filesABMask) != 0) && ((scratchBits & petir_evalmask::filesGHMask) != 0))) {
                if (realEndgame) {
                    (score = pas::decrement(score, petir_ueval::bishopOverKnightEndgameBonus));
                } else {
                    (score = pas::decrement(score, pas::shiftRight(petir_ueval::bishopOverKnightEndgameBonus, 1)));
                }
            }
        }
    }
    else {
        bishopSquare = 64;
    }
    // Connection scoring is independent of handled-file masks and early exits.
    score += connectedPassedPawns(position, true)
        * endgameScale[std::min<int>(position.blackNonPawnMaterial, 41)] / 100;
    score -= connectedPassedPawns(position, false)
        * endgameScale[std::min<int>(position.whiteNonPawnMaterial, 41)] / 100;
    const auto blackPawnFiles = pawnFiles(position.blackPawns);
    const auto blackIsolatedFiles = blackPawnFiles & ~((blackPawnFiles << 1) | (blackPawnFiles >> 1));
    blackPawnCount = countBits<NativePopcount>(position.blackPawns);
    pawnsOnBishopColor = bishopSquare < 64 ? countBits<NativePopcount>(position.blackPawns & UINT64_C(0x0000ffffffffffff)
        & (petir_ueval::squareColor[bishopSquare] == 0 ? UINT64_C(0xaa55aa55aa55aa55) : UINT64_C(0x55aa55aa55aa55aa))) : 0;
    // Each occupied file has exactly one foremost pawn; all others are doubled.
    pawnTally = blackPawnCount - countBits<NativePopcount>(std::uint64_t(blackPawnFiles));
    scratchValue = 0;
    pieces = position.blackPawns;
    while ((pieces != 0)) {
        square = popLowestBit(pieces);
        (score = pas::decrement(score, petir_ueval::blackPawnSquareBonus[square]));
        if ((kingsOnOppositeWings && (petir_evalmask::squareWing[square] == petir_evalmask::squareWing[whiteKingSquare]))) {
            (score = pas::decrement(score, petir_ueval::blackPawnSquareBonus[square]));
        }
        if (((square >= petir_header::a7) && (position.board[pas::sub(square, 8)] == petir_header::whitePawn))) {
            (score = pas::increment(score, petir_ueval::blockedPawnPenalty));
        }
        if ((blackIsolatedFiles & (1 << (square & 7))) != 0) {
            if ((position.whiteNonPawnMaterial != 0)) {
                (scratchValue = pas::increment(scratchValue, 1));
                (score = pas::increment(score, pas::shiftLeft(petir_ueval::isolatedPawnPenalty[(square & 7)], 1)));
                stackedOnFile = false;
                if (((petir_bitboard_mask::rookRayUpMask[square] & position.blackPawns) != 0)) {
                    (score = pas::increment(score, pas::shiftLeft(petir_ueval::isolatedPawnPenalty[(square & 7)], 1)));
                    stackedOnFile = true;
                }
                if (((petir_bitboard_mask::rookRayDownMask[square] & (position.whitePawns | position.blackPawns)) == 0)) {
                    (score = pas::increment(score, petir_ueval::isolatedPawnPenalty[(square & 7)]));
                    if (stackedOnFile) {
                        (score = pas::increment(score, petir_ueval::isolatedPawnPenalty[(square & 7)]));
                    }
                }
                if (((square == petir_header::a7) || (square == petir_header::b7))) {
                    if (((((position.board[petir_header::a6] == petir_header::whitePawn) && (position.board[petir_header::b5] == petir_header::whitePawn)) || ((position.board[petir_header::b6] == petir_header::whitePawn) && (position.board[petir_header::a5] == petir_header::whitePawn))) || ((position.board[petir_header::b6] == petir_header::whitePawn) && (position.board[petir_header::a6] == petir_header::whitePawn)))) {
                        (score = pas::increment(score, petir_ueval::potentialPasserBonus));
                    } else {
                        if (((position.board[petir_header::a5] == petir_header::whitePawn) && (position.board[petir_header::b5] == petir_header::whitePawn))) {
                            (score = pas::increment(score, pas::shiftRight(petir_ueval::potentialPasserBonus, 1)));
                        }
                    }
                } else {
                    if (((square == petir_header::g7) || (square == petir_header::h7))) {
                        if (((((position.board[petir_header::g6] == petir_header::whitePawn) && (position.board[petir_header::h5] == petir_header::whitePawn)) || ((position.board[petir_header::h6] == petir_header::whitePawn) && (position.board[petir_header::g5] == petir_header::whitePawn))) || ((position.board[petir_header::h6] == petir_header::whitePawn) && (position.board[petir_header::g6] == petir_header::whitePawn)))) {
                            (score = pas::increment(score, petir_ueval::potentialPasserBonus));
                        } else {
                            if (((position.board[petir_header::g5] == petir_header::whitePawn) && (position.board[petir_header::h5] == petir_header::whitePawn))) {
                                (score = pas::increment(score, pas::shiftRight(petir_ueval::potentialPasserBonus, 1)));
                            }
                        }
                    }
                }
            }
        }
        if ((((petir_bitboard_mask::blackPassedPawnMask[square] & position.whitePawns) == 0) && ((blackPassedFiles & petir_header::powerOfTwo((square & 7))) == 0))) {
            (blackPassedCount = pas::increment(blackPassedCount, 1));
            file = (square & 7);
            (score = pas::decrement(score, petir_ueval::passedPawnBonus));
            score -= passedPawnActivity(position, square, false);
            termScore = 0;
            blackPassedFiles = (blackPassedFiles | petir_header::powerOfTwo(file));
            if ((position.whiteNonPawnMaterial == 0)) {
                if (((square <= petir_header::h3) && ((petir_bitboard_mask::blackPawnAttacks[square] & position.blackKingBoard) != 0))) {
                    (score = pas::decrement(score, petir_ueval::assuredPromotionBonus));
                    (score = pas::decrement(score, pas::shiftLeft(petir_ueval::blackPassedPawnRankBonus[pas::shiftRight(square, 3)], 1)));
                    continue;
                }
                if (((((square <= petir_header::g2) && (square >= petir_header::b2)) && (position.board[pas::sub(square, 8)] != petir_header::whiteKing)) && ((petir_bitboard_mask::kingAttacks[square] & position.blackKingBoard) != 0))) {
                    if ((((((blackKingSquare >= petir_header::a3) && ((petir_bitboard_mask::kingAttacks[square] & position.whiteKingBoard) != 0)) && (realSideToMove == petir_header::whiteSide)) && (position.blackNonPawnMaterial == 0)) && ((whiteKingSquare == pas::sub(square, 1)) || (whiteKingSquare == pas::add(square, 1))))) {
                    } else {
                        (score = pas::decrement(score, petir_ueval::assuredPromotionBonus));
                        (score = pas::decrement(score, pas::shiftLeft(petir_ueval::blackPassedPawnRankBonus[pas::shiftRight(square, 3)], 1)));
                        continue;
                    }
                }
                if (((pas::add(pas::shiftRight(square, 3), 1) < pas::shiftRight(whiteKingSquare, 3)) || ((pas::shiftRight(square, 3) < pas::shiftRight(whiteKingSquare, 3)) && (realSideToMove == petir_header::blackSide)))) {
                    (score = pas::decrement(score, petir_ueval::assuredPromotionBonus));
                    (score = pas::decrement(score, petir_ueval::blackPassedPawnRankBonus[pas::shiftRight(square, 3)]));
                    continue;
                }
                if (((pas::shiftRight(square, 3) < pas::sub(pas::abs(pas::sub((square & 7), whiteKingFile)), 1)) || ((pas::shiftRight(square, 3) == pas::sub(pas::abs(pas::sub((square & 7), whiteKingFile)), 1)) && (realSideToMove == petir_header::blackSide)))) {
                    (score = pas::decrement(score, petir_ueval::assuredPromotionBonus));
                    (score = pas::decrement(score, petir_ueval::blackPassedPawnRankBonus[pas::shiftRight(square, 3)]));
                    continue;
                }
                if (((pas::shiftRight(square, 3) == 2) && ((petir_bitboard_mask::blackPassedPawnMask[square] & position.blackKingBoard) != 0))) {
                    (score = pas::decrement(score, petir_ueval::assuredPromotionBonus));
                    (score = pas::decrement(score, petir_ueval::blackPassedPawnRankBonus[pas::shiftRight(square, 3)]));
                    continue;
                }
            }
            (termScore = pas::increment(termScore, petir_ueval::blackPassedPawnRankBonus[pas::shiftRight(square, 3)]));
            fileAttacks = petir_bitboard_mask::rookRayUpMask[square];
            if (((fileAttacks & position.blackRooks) != 0)) {
                if ((((petir_bitboard_mask::verticalAttacks[square][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[square]) & 63)] & position.blackRooks) != 0) && (!(position.board[pas::sub(square, 8)] > 0)))) {
                    (termScore = pas::increment(termScore, petir_ueval::rookBehindPassedPawnBonus));
                }
            } else {
                if (((fileAttacks & position.whiteRooks) != 0)) {
                    if (((petir_bitboard_mask::verticalAttacks[square][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[square]) & 63)] & position.whiteRooks) != 0)) {
                        (termScore = pas::decrement(termScore, pas::shiftRight(petir_ueval::rookBehindPassedPawnBonus, 1)));
                        if ((((position.whiteNonPawnMaterial == 5) && (position.blackNonPawnMaterial == 5)) && (petir_evalmask::closenessTable[square][blackKingSquare] <= 5))) {
                            (termScore = pas::decrement(termScore, pas::shiftRight(petir_ueval::rookBehindPassedPawnBonus, 2)));
                        }
                    }
                }
            }
            if ((position.board[pas::sub(square, 8)] > 0)) {
                (termScore = pas::decrement(termScore, pas::shiftRight(petir_ueval::blackPassedPawnRankBonus[pas::shiftRight(square, 3)], 1)));
                (termScore = pas::increment(termScore, pas::shiftRight(petir_ueval::blackPassedPawnRankBonus[pas::shiftRight(square, 3)], 3)));
            } else {
                if ((((square <= petir_header::h2) && (position.board[pas::sub(square, 8)] == 0)) && (petir_usee::staticExchange(petir_header::blackSide, position, square, pas::sub(square, 8)) == 0))) {
                    if ((realSideToMove == petir_header::blackSide)) {
                        (score = pas::decrement(score, pas::add(petir_ueval::safeAdvanceBonus, 40)));
                    } else {
                        (score = pas::decrement(score, petir_ueval::safeAdvanceBonus));
                    }
                }
            }
            if (((pas::sub(blackKingSquare, square) <= 9) && ((petir_bitboard_mask::kingAttacks[square] & position.blackKingBoard) != 0))) {
                (termScore = pas::increment(termScore, petir_ueval::blackKingSupportedPasserBonus[pas::shiftRight(square, 3)]));
            } else {
                if ((((petir_evalmask::closenessTable[square][whiteKingSquare] >= 7) && (pas::shiftRight(whiteKingSquare, 3) <= pas::shiftRight(square, 3))) && (pas::shiftRight(square, 3) >= pas::abs(pas::sub(whiteKingFile, (square & 7)))))) {
                    if ((petir_evalmask::closenessTable[square][whiteKingSquare] >= 8)) {
                        (termScore = pas::decrement(termScore, pas::shiftRight(petir_ueval::blackPassedPawnRankBonus[pas::shiftRight(square, 3)], 1)));
                    } else {
                        (termScore = pas::decrement(termScore, pas::shiftRight(petir_ueval::blackPassedPawnRankBonus[pas::shiftRight(square, 3)], 2)));
                    }
                }
            }
            if ((termScore > 0)) {
                (score = pas::decrement(score, (pas::mul(termScore, petir_ueval::endgameScale[std::min<int>(position.whiteNonPawnMaterial, 41)]) / 100)));
            }
        }
    }
    blackPawnAttackMap = pas::shiftRight((position.blackPawns & petir_bitboard_mask::notFileAMask), 9);
    blackPawnAttackMap = (blackPawnAttackMap | pas::shiftRight((position.blackPawns & petir_bitboard_mask::notFileHMask), 7));
    (score = pas::increment(score, petir_ueval::doubledPawnPenalty[pawnTally]));
    (score = pas::increment(score, petir_ueval::isolatedPawnCountPenalty[scratchValue]));
    if (petir_ueval::hasSingleBit(position.blackBishops)) {
        (score = pas::increment(score, petir_evalmask::badBishopPenalty[pawnsOnBishopColor]));
    }
    kingZoneHits = (blackPawnAttackMap & whiteKingZone);
    (whiteKingPressure = pas::increment(whiteKingPressure, countBits<NativePopcount>(kingZoneHits)));
    whiteBishopCase = position.whiteBishopCount;
    if (((whiteBishopCase == 1))) {
        bishopSquare = petir_bitboard_mask::firstBit(pas::address(position.whiteBishops));
        if ((endgame && (position.blackBishopCount == 0))) {
            scratchBits = (position.blackPawns | position.whitePawns);
            if ((((scratchBits & petir_evalmask::filesABMask) != 0) && ((scratchBits & petir_evalmask::filesGHMask) != 0))) {
                if (realEndgame) {
                    (score = pas::increment(score, petir_ueval::bishopOverKnightEndgameBonus));
                } else {
                    (score = pas::increment(score, pas::shiftRight(petir_ueval::bishopOverKnightEndgameBonus, 1)));
                }
            }
        }
    }
    else if (((whiteBishopCase == 2))) {
        bishopSquare = 64;
        if ((position.blackBishopCount == 0)) {
            (score = pas::increment(score, petir_ueval::bishopPairBonus[pas::shiftRight(countBits<NativePopcount>((position.blackPawns | position.whitePawns)), 1)]));
        }
        if ((endgame && (position.blackBishopCount == 1))) {
            scratchBits = (position.blackPawns | position.whitePawns);
            if ((((scratchBits & petir_evalmask::filesABMask) != 0) && ((scratchBits & petir_evalmask::filesGHMask) != 0))) {
                if (realEndgame) {
                    (score = pas::increment(score, petir_ueval::bishopOverKnightEndgameBonus));
                } else {
                    (score = pas::increment(score, pas::shiftRight(petir_ueval::bishopOverKnightEndgameBonus, 1)));
                }
            }
        }
    }
    else {
        bishopSquare = 64;
    }
    pieces = position.whitePawns;
    const auto whitePawnFiles = pawnFiles(position.whitePawns);
    const auto whiteIsolatedFiles = whitePawnFiles & ~((whitePawnFiles << 1) | (whitePawnFiles >> 1));
    whitePawnCount = countBits<NativePopcount>(position.whitePawns);
    pawnsOnBishopColor = bishopSquare < 64 ? countBits<NativePopcount>(position.whitePawns & UINT64_C(0xffffffffffff0000)
        & (petir_ueval::squareColor[bishopSquare] == 0 ? UINT64_C(0xaa55aa55aa55aa55) : UINT64_C(0x55aa55aa55aa55aa))) : 0;
    pawnTally = whitePawnCount - countBits<NativePopcount>(std::uint64_t(whitePawnFiles));
    scratchValue = 0;
    while ((pieces != 0)) {
        // Evaluate the foremost white passer before marking its file handled.
        square = popHighestBit(pieces);
        (score = pas::increment(score, petir_ueval::whitePawnSquareBonus[square]));
        if ((kingsOnOppositeWings && (petir_evalmask::squareWing[square] == petir_evalmask::squareWing[blackKingSquare]))) {
            (score = pas::increment(score, petir_ueval::whitePawnSquareBonus[square]));
        }
        if ((position.board[pas::add(square, 8)] == petir_header::blackPawn)) {
            if ((square <= petir_header::h2)) {
                (score = pas::decrement(score, petir_ueval::blockedPawnPenalty));
            }
        }
        if ((whiteIsolatedFiles & (1 << (square & 7))) != 0) {
            if ((position.blackNonPawnMaterial != 0)) {
                (scratchValue = pas::increment(scratchValue, 1));
                (score = pas::decrement(score, pas::shiftLeft(petir_ueval::isolatedPawnPenalty[(square & 7)], 1)));
                stackedOnFile = false;
                if (((petir_bitboard_mask::rookRayDownMask[square] & position.whitePawns) != 0)) {
                    (score = pas::decrement(score, pas::shiftLeft(petir_ueval::isolatedPawnPenalty[(square & 7)], 1)));
                    stackedOnFile = true;
                }
                if (((petir_bitboard_mask::rookRayUpMask[square] & (position.blackPawns | position.whitePawns)) == 0)) {
                    (score = pas::decrement(score, petir_ueval::isolatedPawnPenalty[(square & 7)]));
                    if (stackedOnFile) {
                        (score = pas::decrement(score, petir_ueval::isolatedPawnPenalty[(square & 7)]));
                    }
                }
                if (((square == petir_header::a2) || (square == petir_header::b2))) {
                    if (((((position.board[petir_header::a3] == petir_header::blackPawn) && (position.board[petir_header::b4] == petir_header::blackPawn)) || ((position.board[petir_header::b3] == petir_header::blackPawn) && (position.board[petir_header::a4] == petir_header::blackPawn))) || ((position.board[petir_header::b3] == petir_header::blackPawn) && (position.board[petir_header::a3] == petir_header::blackPawn)))) {
                        (score = pas::decrement(score, petir_ueval::potentialPasserBonus));
                    } else {
                        if (((position.board[petir_header::a4] == petir_header::blackPawn) && (position.board[petir_header::b4] == petir_header::blackPawn))) {
                            (score = pas::decrement(score, pas::shiftRight(petir_ueval::potentialPasserBonus, 1)));
                        }
                    }
                } else {
                    if (((square == petir_header::g2) || (square == petir_header::h2))) {
                        if (((((position.board[petir_header::g3] == petir_header::blackPawn) && (position.board[petir_header::h4] == petir_header::blackPawn)) || ((position.board[petir_header::h3] == petir_header::blackPawn) && (position.board[petir_header::g4] == petir_header::blackPawn))) || ((position.board[petir_header::h3] == petir_header::blackPawn) && (position.board[petir_header::g3] == petir_header::blackPawn)))) {
                            (score = pas::decrement(score, petir_ueval::potentialPasserBonus));
                        } else {
                            if (((position.board[petir_header::g4] == petir_header::blackPawn) && (position.board[petir_header::h4] == petir_header::blackPawn))) {
                                (score = pas::decrement(score, pas::shiftRight(petir_ueval::potentialPasserBonus, 1)));
                            }
                        }
                    }
                }
            }
        }
        if ((((petir_bitboard_mask::whitePassedPawnMask[square] & position.blackPawns) == 0) && ((whitePassedFiles & petir_header::powerOfTwo((square & 7))) == 0))) {
            (whitePassedCount = pas::increment(whitePassedCount, 1));
            weightedTerm = 0;
            file = (square & 7);
            (score = pas::increment(score, petir_ueval::passedPawnBonus));
            score += passedPawnActivity(position, square, true);
            termScore = pas::shiftRight(square, 3);
            whitePassedFiles = (whitePassedFiles | petir_header::powerOfTwo(file));
            if ((position.blackNonPawnMaterial == 0)) {
                if (((square >= petir_header::a6) && ((petir_bitboard_mask::whitePawnAttacks[square] & position.whiteKingBoard) != 0))) {
                    (score = pas::increment(score, petir_ueval::assuredPromotionBonus));
                    (score = pas::increment(score, pas::shiftLeft(petir_ueval::whitePassedPawnRankBonus[pas::shiftRight(square, 3)], 1)));
                    continue;
                }
                if (((((square >= petir_header::b7) && (square <= petir_header::g7)) && (position.board[pas::add(square, 8)] != petir_header::blackKing)) && ((petir_bitboard_mask::kingAttacks[square] & position.whiteKingBoard) != 0))) {
                    if ((((((blackKingSquare <= petir_header::h6) && ((petir_bitboard_mask::kingAttacks[square] & position.blackKingBoard) != 0)) && (realSideToMove == petir_header::blackSide)) && (position.whiteNonPawnMaterial == 0)) && ((blackKingSquare == pas::sub(square, 1)) || (blackKingSquare == pas::add(square, 1))))) {
                    } else {
                        (score = pas::increment(score, petir_ueval::assuredPromotionBonus));
                        (score = pas::increment(score, pas::shiftLeft(petir_ueval::whitePassedPawnRankBonus[pas::shiftRight(square, 3)], 1)));
                        continue;
                    }
                }
                if (((pas::shiftRight(square, 3) > pas::add(pas::shiftRight(blackKingSquare, 3), 1)) || ((pas::shiftRight(square, 3) > pas::shiftRight(blackKingSquare, 3)) && (realSideToMove == petir_header::whiteSide)))) {
                    (score = pas::increment(score, petir_ueval::assuredPromotionBonus));
                    (score = pas::increment(score, petir_ueval::whitePassedPawnRankBonus[termScore]));
                    continue;
                }
                if (((pas::sub(7, pas::shiftRight(square, 3)) < pas::sub(pas::abs(pas::sub((square & 7), blackKingFile)), 1)) || ((pas::sub(7, pas::shiftRight(square, 3)) == pas::sub(pas::abs(pas::sub((square & 7), blackKingFile)), 1)) && (realSideToMove == petir_header::whiteSide)))) {
                    (score = pas::increment(score, petir_ueval::assuredPromotionBonus));
                    (score = pas::increment(score, petir_ueval::whitePassedPawnRankBonus[termScore]));
                    continue;
                }
                if (((pas::shiftRight(square, 3) == 5) && ((petir_bitboard_mask::whitePassedPawnMask[square] & position.whiteKingBoard) != 0))) {
                    (score = pas::increment(score, petir_ueval::assuredPromotionBonus));
                    (score = pas::increment(score, petir_ueval::whitePassedPawnRankBonus[pas::shiftRight(square, 3)]));
                    continue;
                }
            }
            (weightedTerm = pas::increment(weightedTerm, petir_ueval::whitePassedPawnRankBonus[termScore]));
            fileAttacks = petir_bitboard_mask::rookRayDownMask[square];
            if (((fileAttacks & position.whiteRooks) != 0)) {
                if ((((petir_bitboard_mask::verticalAttacks[square][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[square]) & 63)] & position.whiteRooks) != 0) && (!(position.board[pas::add(square, 8)] < 0)))) {
                    (weightedTerm = pas::increment(weightedTerm, petir_ueval::rookBehindPassedPawnBonus));
                }
            } else {
                if (((fileAttacks & position.blackRooks) != 0)) {
                    if (((petir_bitboard_mask::verticalAttacks[square][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[square]) & 63)] & position.blackRooks) != 0)) {
                        (weightedTerm = pas::decrement(weightedTerm, pas::shiftRight(petir_ueval::rookBehindPassedPawnBonus, 1)));
                        if ((((position.whiteNonPawnMaterial == 5) && (position.blackNonPawnMaterial == 5)) && (petir_evalmask::closenessTable[square][whiteKingSquare] <= 5))) {
                            (weightedTerm = pas::decrement(weightedTerm, pas::shiftRight(petir_ueval::rookBehindPassedPawnBonus, 2)));
                        }
                    }
                }
            }
            if ((position.board[pas::add(square, 8)] < 0)) {
                (weightedTerm = pas::decrement(weightedTerm, pas::shiftRight(petir_ueval::whitePassedPawnRankBonus[termScore], 1)));
                (weightedTerm = pas::increment(weightedTerm, pas::shiftRight(petir_ueval::whitePassedPawnRankBonus[termScore], 3)));
            } else {
                if ((((square >= petir_header::a7) && (position.board[pas::add(square, 8)] == 0)) && (petir_usee::staticExchange(petir_header::whiteSide, position, square, pas::add(square, 8)) == 0))) {
                    if ((realSideToMove == petir_header::whiteSide)) {
                        (score = pas::increment(score, pas::add(petir_ueval::safeAdvanceBonus, 40)));
                    } else {
                        (score = pas::increment(score, petir_ueval::safeAdvanceBonus));
                    }
                }
            }
            if (((pas::sub(square, whiteKingSquare) <= 9) && ((petir_bitboard_mask::kingAttacks[square] & position.whiteKingBoard) != 0))) {
                (weightedTerm = pas::increment(weightedTerm, petir_ueval::whiteKingSupportedPasserBonus[termScore]));
            } else {
                if (((petir_evalmask::closenessTable[square][blackKingSquare] >= 7) && (pas::shiftRight(blackKingSquare, 3) >= pas::shiftRight(square, 3)) && (7 - pas::shiftRight(square, 3) >= pas::abs(blackKingFile - (square & 7))))) {
                    if ((petir_evalmask::closenessTable[square][blackKingSquare] >= 8)) {
                        (weightedTerm = pas::decrement(weightedTerm, pas::shiftRight(petir_ueval::whitePassedPawnRankBonus[termScore], 1)));
                    } else {
                        (weightedTerm = pas::decrement(weightedTerm, pas::shiftRight(petir_ueval::whitePassedPawnRankBonus[termScore], 2)));
                    }
                }
            }
            if ((weightedTerm > 0)) {
                (score = pas::increment(score, (pas::mul(weightedTerm, petir_ueval::endgameScale[std::min<int>(position.blackNonPawnMaterial, 41)]) / 100)));
            }
        }
    }
    whitePawnAttackMap = pas::shiftLeft((position.whitePawns & petir_bitboard_mask::notFileAMask), 7);
    whitePawnAttackMap = (whitePawnAttackMap | pas::shiftLeft((position.whitePawns & petir_bitboard_mask::notFileHMask), 9));
    (score = pas::decrement(score, petir_ueval::doubledPawnPenalty[pawnTally]));
    (score = pas::decrement(score, petir_ueval::isolatedPawnCountPenalty[scratchValue]));
    if (petir_ueval::hasSingleBit(position.whiteBishops)) {
        (score = pas::decrement(score, petir_evalmask::badBishopPenalty[pawnsOnBishopColor]));
    }
    kingZoneHits = (whitePawnAttackMap & blackKingZone);
    (blackKingPressure = pas::increment(blackKingPressure, countBits<NativePopcount>(kingZoneHits)));
    if ((((position.whiteNonPawnMaterial == pas::add(position.blackNonPawnMaterial, 3)) && (position.whiteQueens != 0)) && ((pas::sub(blackPawnCount, whitePawnCount) >= 2 && pas::sub(blackPawnCount, whitePawnCount) <= 3)))) {
        (score = pas::increment(score, 35));
    } else {
        if ((((position.blackNonPawnMaterial == pas::add(position.whiteNonPawnMaterial, 3)) && (position.blackQueens != 0)) && ((pas::sub(whitePawnCount, blackPawnCount) >= 2 && pas::sub(whitePawnCount, blackPawnCount) <= 3)))) {
            (score = pas::decrement(score, 35));
        }
    }
    termScore = 0;
    pieces = position.whiteRooks;
    while ((pieces != 0)) {
        square = popLowestBit(pieces);
        if (endgame) score += endgamePieceActivity(position, square, true);
        rankAttacks = petir_bitboard_mask::horizontalAttacks[square][(pas::shiftRight(position.allPieces, pas::add((square & 56), 1)) & 63)];
        fileAttacks = petir_bitboard_mask::verticalAttacks[square][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[square]) & 63)];
        attacks = (rankAttacks | fileAttacks);
        scratchBits = (attacks & (~position.whitePieces));
        (score = pas::increment(score, petir_ueval::rookMobilityBonus[countBits<NativePopcount>(scratchBits)]));
        whiteRookAttackMap = (whiteRookAttackMap | attacks);
        if (evaluateBlackKingSafety) {
            kingZoneHits = (attacks & blackKingZone);
            (blackKingPressure = pas::increment(blackKingPressure, countBits<NativePopcount>(kingZoneHits)));
            if (kingZoneHits != 0) {
                (blackAttackerCount = pas::increment(blackAttackerCount, 1));
            }
        }
        (score = pas::increment(score, petir_ueval::rookSquareBonus[square]));
        if (((petir_bitboard_mask::rookRayUpMask[square] & position.whitePawns) == 0)) {
            (termScore = pas::increment(termScore, petir_ueval::semiOpenFileBonus[(square & 7)]));
            nearEnemyKingWing = ((petir_evalmask::squareWing[square] == petir_evalmask::squareWing[blackKingSquare]) && (!endgame));
            if (nearEnemyKingWing) {
                (termScore = pas::increment(termScore, petir_ueval::rookSemiOpenFileNearKingBonus));
            }
            stackedOnFile = false;
            if (((petir_bitboard_mask::rookRayUpMask[square] & position.whiteRooks) != 0)) {
                (termScore = pas::increment(termScore, petir_ueval::doubledRooksBonus));
                if (nearEnemyKingWing) {
                    (termScore = pas::increment(termScore, petir_ueval::doubledRooksSemiOpenNearKingBonus));
                }
                stackedOnFile = true;
            }
            if (((fileAttacks & position.whiteQueens) != 0)) {
                (termScore = pas::increment(termScore, petir_ueval::rookQueenSemiOpenFileBonus));
                stackedOnFile = true;
                if (nearEnemyKingWing) {
                    (termScore = pas::increment(termScore, petir_ueval::rookQueenSemiOpenNearKingBonus));
                }
            }
            if (((petir_bitboard_mask::rookRayUpMask[square] & position.blackPawns) == 0)) {
                (termScore = pas::increment(termScore, petir_ueval::openFileBonus[(square & 7)]));
                if (((petir_bitboard_mask::rookRayDownMask[square] & position.whiteQueens) != 0)) {
                    (termScore = pas::increment(termScore, petir_ueval::rookQueenOpenFileBonus));
                }
                if (nearEnemyKingWing) {
                    (termScore = pas::increment(termScore, petir_ueval::rookOpenFileNearKingBonus));
                } else {
                    if (((petir_evalmask::squareWing[square] == petir_evalmask::squareWing[blackKingSquare]) && (!endgame))) {
                        (termScore = pas::increment(termScore, pas::shiftRight(petir_ueval::rookOpenFileNearKingBonus, 1)));
                    }
                }
                if (stackedOnFile) {
                    (termScore = pas::increment(termScore, petir_ueval::doubledRooksOpenFileBonus));
                    if (nearEnemyKingWing) {
                        (termScore = pas::increment(termScore, petir_ueval::doubledRooksNearKingBonus));
                    }
                }
            }
        }
        if (((pas::shiftRight(square, 3) == 6) && (((petir_bitboard_mask::rankMask[6] & position.blackPawns) != 0) || ((petir_bitboard_mask::rankMask[7] & position.blackKingBoard) != 0)))) {
            if (((rankAttacks & position.whiteRooks) != 0)) {
                (score = pas::increment(score, petir_ueval::doubledRooksSeventhBonus));
            }
            if (((rankAttacks & position.whiteQueens) != 0)) {
                (score = pas::increment(score, petir_ueval::rookQueenSeventhBonus));
            }
            (score = pas::increment(score, petir_ueval::rookOnSeventhBonus));
        } else {
            if (((pas::shiftRight(square, 3) == 7) && ((petir_bitboard_mask::rankMask[7] & position.blackKingBoard) != 0))) {
                (score = pas::increment(score, petir_ueval::rookOnEighthBonus));
            }
        }
        rookFile = (square & 7);
        if ((rookFile == 0)) {
            if ((position.board[pas::add(square, 1)] > 0)) {
                (score = pas::decrement(score, petir_ueval::blockedRookPenalty));
            }
        } else {
            if ((rookFile == 7)) {
                if ((position.board[pas::sub(square, 1)] > 0)) {
                    (score = pas::decrement(score, petir_ueval::blockedRookPenalty));
                }
            } else {
                if (((position.board[pas::sub(square, 1)] > 0) && (position.board[pas::add(square, 1)] > 0))) {
                    (score = pas::decrement(score, petir_ueval::blockedRookPenalty));
                }
            }
        }
    }
    (score = pas::increment(score, (pas::mul(termScore, petir_ueval::rookScale[std::min<int>(position.whiteNonPawnMaterial, 41)]) / 100)));
    termScore = 0;
    pieces = position.blackRooks;
    while ((pieces != 0)) {
        // Rook scores and attack unions are independent of iteration order.
        square = popLowestBit(pieces);
        if (endgame) score -= endgamePieceActivity(position, square, false);
        rankAttacks = petir_bitboard_mask::horizontalAttacks[square][(pas::shiftRight(position.allPieces, pas::add((square & 56), 1)) & 63)];
        fileAttacks = petir_bitboard_mask::verticalAttacks[square][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[square]) & 63)];
        attacks = (rankAttacks | fileAttacks);
        scratchBits = (attacks & (~position.blackPieces));
        (score = pas::decrement(score, petir_ueval::rookMobilityBonus[countBits<NativePopcount>(scratchBits)]));
        blackRookAttackMap = (blackRookAttackMap | attacks);
        if (evaluateWhiteKingSafety) {
            kingZoneHits = (attacks & whiteKingZone);
            (whiteKingPressure = pas::increment(whiteKingPressure, countBits<NativePopcount>(kingZoneHits)));
            if (kingZoneHits != 0) {
                (whiteAttackerCount = pas::increment(whiteAttackerCount, 1));
            }
        }
        (score = pas::decrement(score, petir_ueval::rookSquareBonus[square]));
        if (((petir_bitboard_mask::rookRayDownMask[square] & position.blackPawns) == 0)) {
            (termScore = pas::increment(termScore, petir_ueval::semiOpenFileBonus[(square & 7)]));
            nearEnemyKingWing = ((petir_evalmask::squareWing[square] == petir_evalmask::squareWing[whiteKingSquare]) && (!endgame));
            if (nearEnemyKingWing) {
                (termScore = pas::increment(termScore, petir_ueval::rookSemiOpenFileNearKingBonus));
            }
            stackedOnFile = false;
            if (((fileAttacks & position.blackQueens) != 0)) {
                (termScore = pas::increment(termScore, petir_ueval::rookQueenSemiOpenFileBonus));
                if (nearEnemyKingWing) {
                    (termScore = pas::increment(termScore, petir_ueval::rookQueenSemiOpenNearKingBonus));
                }
                stackedOnFile = true;
            }
            if (((petir_bitboard_mask::rookRayDownMask[square] & position.blackRooks) != 0)) {
                (termScore = pas::increment(termScore, petir_ueval::doubledRooksBonus));
                if (nearEnemyKingWing) {
                    (termScore = pas::increment(termScore, petir_ueval::doubledRooksSemiOpenNearKingBonus));
                }
                stackedOnFile = true;
            }
            if (((petir_bitboard_mask::rookRayDownMask[square] & position.whitePawns) == 0)) {
                (termScore = pas::increment(termScore, petir_ueval::openFileBonus[(square & 7)]));
                if (((petir_bitboard_mask::rookRayUpMask[square] & position.blackQueens) != 0)) {
                    (termScore = pas::increment(termScore, petir_ueval::rookQueenOpenFileBonus));
                }
                if (nearEnemyKingWing) {
                    (termScore = pas::increment(termScore, petir_ueval::rookOpenFileNearKingBonus));
                } else {
                    if (((petir_evalmask::squareWing[square] == petir_evalmask::squareWing[whiteKingSquare]) && (!endgame))) {
                        (termScore = pas::increment(termScore, pas::shiftRight(petir_ueval::rookOpenFileNearKingBonus, 1)));
                    }
                }
                if (stackedOnFile) {
                    (termScore = pas::increment(termScore, petir_ueval::doubledRooksOpenFileBonus));
                }
                if ((stackedOnFile && nearEnemyKingWing)) {
                    (termScore = pas::increment(termScore, petir_ueval::doubledRooksNearKingBonus));
                }
            }
        }
        if (((pas::shiftRight(square, 3) == 1) && (((petir_bitboard_mask::rankMask[1] & position.whitePawns) != 0) || ((petir_bitboard_mask::rankMask[0] & position.whiteKingBoard) != 0)))) {
            (score = pas::decrement(score, petir_ueval::rookOnSeventhBonus));
            if (((rankAttacks & position.blackRooks) != 0)) {
                (score = pas::decrement(score, petir_ueval::doubledRooksSeventhBonus));
            }
            if (((rankAttacks & position.blackQueens) != 0)) {
                (score = pas::decrement(score, petir_ueval::rookQueenSeventhBonus));
            }
        } else {
            if (((pas::shiftRight(square, 3) == 0) && ((petir_bitboard_mask::rankMask[0] & position.whiteKingBoard) != 0))) {
                (score = pas::decrement(score, petir_ueval::rookOnEighthBonus));
            }
        }
        rookFile = (square & 7);
        if ((rookFile == 0)) {
            if ((position.board[pas::add(square, 1)] < 0)) {
                (score = pas::increment(score, petir_ueval::blockedRookPenalty));
            }
        } else {
            if ((rookFile == 7)) {
                if ((position.board[pas::sub(square, 1)] < 0)) {
                    (score = pas::increment(score, petir_ueval::blockedRookPenalty));
                }
            } else {
                if (((position.board[pas::sub(square, 1)] < 0) && (position.board[pas::add(square, 1)] < 0))) {
                    (score = pas::increment(score, petir_ueval::blockedRookPenalty));
                }
            }
        }
    }
    (score = pas::decrement(score, (pas::mul(termScore, petir_ueval::rookScale[std::min<int>(position.blackNonPawnMaterial, 41)]) / 100)));
    pieces = position.whiteKnights;
    while ((pieces != 0)) {
        square = popLowestBit(pieces);
        if (endgame) score += endgamePieceActivity(position, square, true);
        (score = pas::increment(score, petir_ueval::whiteKnightSquareBonus[square]));
        if ((!earlyEndgame)) {
            (score = pas::increment(score, pas::mul(petir_evalmask::closenessHalfTable[square][blackKingSquare], petir_ueval::knightTropismBonus)));
        }
        attacks = petir_bitboard_mask::knightAttacks[square];
        whiteKnightAttackMap = (whiteKnightAttackMap | attacks);
        scratchBits = (attacks & (~position.whitePieces));
        (score = pas::increment(score, petir_ueval::knightMobilityBonus[countBits<NativePopcount>(scratchBits)]));
        if (evaluateBlackKingSafety) {
            kingZoneHits = (attacks & blackKingZone);
            (blackKingPressure = pas::increment(blackKingPressure, countBits<NativePopcount>(kingZoneHits)));
            if (kingZoneHits != 0) {
                (blackAttackerCount = pas::increment(blackAttackerCount, 1));
            }
        }
        if (((petir_ueval::whiteKnightOutpostBonus[square] > 0) && (!endgame))) {
            if (((petir_evalmask::whiteOutpostMask[square] & position.blackPawns) == 0)) {
                scratchBits = (petir_bitboard_mask::blackPawnAttacks[square] & position.whitePawns);
                pawnTally = countBits<NativePopcount>(scratchBits);
                whiteSupporterCase = pawnTally;
                if (((whiteSupporterCase == 1))) {
                    (score = pas::increment(score, pas::shiftRight(petir_ueval::whiteKnightOutpostBonus[square], 1)));
                }
                else if (((whiteSupporterCase == 2))) {
                    (score = pas::increment(score, petir_ueval::whiteKnightOutpostBonus[square]));
                }
                if ((((pawnTally > 0) && (position.blackKnights == 0)) && ((position.blackBishopCount == 0) || ((position.blackBishopCount == 1) && (petir_ueval::squareColor[petir_bitboard_mask::lastBit(pas::address(position.blackBishops))] != petir_ueval::squareColor[square]))))) {
                    (score = pas::increment(score, pas::shiftRight(petir_ueval::whiteKnightOutpostBonus[square], 1)));
                }
            }
        }
    }
    pieces = position.blackKnights;
    while ((pieces != 0)) {
        square = popLowestBit(pieces);
        if (endgame) score -= endgamePieceActivity(position, square, false);
        (score = pas::decrement(score, petir_ueval::blackKnightSquareBonus[square]));
        if ((!earlyEndgame)) {
            (score = pas::decrement(score, pas::mul(petir_evalmask::closenessHalfTable[square][whiteKingSquare], petir_ueval::knightTropismBonus)));
        }
        attacks = petir_bitboard_mask::knightAttacks[square];
        blackKnightAttackMap = (blackKnightAttackMap | attacks);
        scratchBits = (attacks & (~position.blackPieces));
        (score = pas::decrement(score, petir_ueval::knightMobilityBonus[countBits<NativePopcount>(scratchBits)]));
        if (evaluateWhiteKingSafety) {
            kingZoneHits = (attacks & whiteKingZone);
            (whiteKingPressure = pas::increment(whiteKingPressure, countBits<NativePopcount>(kingZoneHits)));
            if (kingZoneHits != 0) {
                (whiteAttackerCount = pas::increment(whiteAttackerCount, 1));
            }
        }
        if (((petir_ueval::blackKnightOutpostBonus[square] > 0) && (!endgame))) {
            if (((petir_evalmask::blackOutpostMask[square] & position.whitePawns) == 0)) {
                scratchBits = (petir_bitboard_mask::whitePawnAttacks[square] & position.blackPawns);
                pawnTally = countBits<NativePopcount>(scratchBits);
                blackSupporterCase = pawnTally;
                if (((blackSupporterCase == 1))) {
                    (score = pas::decrement(score, pas::shiftRight(petir_ueval::blackKnightOutpostBonus[square], 1)));
                }
                else if (((blackSupporterCase == 2))) {
                    (score = pas::decrement(score, petir_ueval::blackKnightOutpostBonus[square]));
                }
                if ((((pawnTally > 0) && (position.whiteKnights == 0)) && ((position.whiteBishopCount == 0) || ((position.whiteBishopCount == 1) && (petir_ueval::squareColor[petir_bitboard_mask::firstBit(pas::address(position.whiteBishops))] != petir_ueval::squareColor[square]))))) {
                    (score = pas::decrement(score, pas::shiftRight(petir_ueval::blackKnightOutpostBonus[square], 1)));
                }
            }
        }
    }
    pieces = position.whiteBishops;
    weightedTerm = 0;
    while ((pieces != 0)) {
        square = popLowestBit(pieces);
        attacks = petir_bitboard_mask::diagonalA8H1Attacks[square][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[square]) & 255)];
        attacks = (attacks | petir_bitboard_mask::diagonalH8A1Attacks[square][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[square]) & 255)]);
        scratchBits = (attacks & (~position.whitePieces));
        (score = pas::increment(score, petir_ueval::bishopMobilityBonus[countBits<NativePopcount>(scratchBits)]));
        if (((((!endgame) && (petir_ueval::whiteBishopOutpostBonus[square] > 0)) && ((petir_bitboard_mask::blackPawnAttacks[square] & position.whitePawns) != 0)) && ((petir_evalmask::whiteOutpostMask[square] & position.blackPawns) == 0))) {
            (score = pas::increment(score, petir_ueval::whiteBishopOutpostBonus[square]));
        }
        if (evaluateBlackKingSafety) {
            kingZoneHits = (attacks & blackKingZone);
            (blackKingPressure = pas::increment(blackKingPressure, countBits<NativePopcount>(kingZoneHits)));
            if (kingZoneHits != 0) {
                (blackAttackerCount = pas::increment(blackAttackerCount, 1));
            }
        }
        whiteBishopAttackMap = (whiteBishopAttackMap | attacks);
        (score = pas::increment(score, petir_ueval::whiteBishopSquareBonus[square]));
        if ((!earlyEndgame)) {
            if ((petir_evalmask::bishopProximityTable[square][blackKingSquare] > 0)) {
                (weightedTerm = pas::increment(weightedTerm, 1));
            }
            (score = pas::increment(score, pas::mul(petir_evalmask::bishopProximityTable[square][blackKingSquare], petir_ueval::bishopTropismBonus)));
        }
    }
    if ((weightedTerm == 2)) {
        (score = pas::increment(score, petir_ueval::twoBishopsKingAttackBonus));
    }
    pieces = position.blackBishops;
    weightedTerm = 0;
    while ((pieces != 0)) {
        square = popLowestBit(pieces);
        (score = pas::decrement(score, petir_ueval::blackBishopSquareBonus[square]));
        if ((!earlyEndgame)) {
            (score = pas::decrement(score, pas::mul(petir_evalmask::bishopProximityTable[square][whiteKingSquare], petir_ueval::bishopTropismBonus)));
            if ((petir_evalmask::bishopProximityTable[square][whiteKingSquare] > 0)) {
                (weightedTerm = pas::increment(weightedTerm, 1));
            }
        }
        attacks = petir_bitboard_mask::diagonalA8H1Attacks[square][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[square]) & 255)];
        attacks = (attacks | petir_bitboard_mask::diagonalH8A1Attacks[square][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[square]) & 255)]);
        scratchBits = (attacks & (~position.blackPieces));
        (score = pas::decrement(score, petir_ueval::bishopMobilityBonus[countBits<NativePopcount>(scratchBits)]));
        if (((((!endgame) && (petir_ueval::blackBishopOutpostBonus[square] > 0)) && ((petir_bitboard_mask::whitePawnAttacks[square] & position.blackPawns) != 0)) && ((petir_evalmask::blackOutpostMask[square] & position.whitePawns) == 0))) {
            (score = pas::decrement(score, petir_ueval::blackBishopOutpostBonus[square]));
        }
        if (evaluateWhiteKingSafety) {
            kingZoneHits = (attacks & whiteKingZone);
            (whiteKingPressure = pas::increment(whiteKingPressure, countBits<NativePopcount>(kingZoneHits)));
            if (kingZoneHits != 0) {
                (whiteAttackerCount = pas::increment(whiteAttackerCount, 1));
            }
        }
        blackBishopAttackMap = (blackBishopAttackMap | attacks);
    }
    if ((weightedTerm == 2)) {
        (score = pas::decrement(score, petir_ueval::twoBishopsKingAttackBonus));
    }
    pieces = position.whiteQueens;
    while ((pieces != 0)) {
        square = popLowestBit(pieces);
        (score = pas::increment(score, petir_ueval::whiteQueenSquareBonus[square]));
        if ((!endgame)) {
            attacks = petir_bitboard_mask::diagonalA8H1Attacks[square][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[square]) & 255)];
            attacks = (attacks | petir_bitboard_mask::diagonalH8A1Attacks[square][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[square]) & 255)]);
            attacks = petir_bitboard_mask::diagonalA8H1Attacks[square][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[square]) & 255)];
            attacks = (attacks | petir_bitboard_mask::diagonalH8A1Attacks[square][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[square]) & 255)]);
            scratchBits = petir_bitboard_mask::horizontalAttacks[square][(pas::shiftRight(position.allPieces, pas::add((square & 56), 1)) & 63)];
            kingZoneHits = petir_bitboard_mask::verticalAttacks[square][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[square]) & 63)];
            attacks = ((attacks | kingZoneHits) | scratchBits);
            queenMobilitySquares = (attacks & (~position.whitePieces));
            (score = pas::increment(score, petir_ueval::queenMobilityBonus[countBits<NativePopcount>(queenMobilitySquares)]));
            if (((scratchBits & position.whiteRooks) != 0)) {
                if (((scratchBits & blackKingZone) != 0)) {
                    whiteRookAttackMap = (whiteRookAttackMap | scratchBits);
                }
            }
            if (((kingZoneHits & position.whiteRooks) != 0)) {
                if (((kingZoneHits & blackKingZone) != 0)) {
                    whiteRookAttackMap = (whiteRookAttackMap | kingZoneHits);
                }
            }
            whiteQueenAttackMap = (whiteQueenAttackMap | attacks);
            (blackKingPressure = pas::increment(blackKingPressure, countBits<NativePopcount>((attacks & blackKingZone))));
            if ((attacks & blackKingZone) != 0) {
                (blackAttackerCount = pas::increment(blackAttackerCount, 1));
            }
        }
        if ((!earlyEndgame)) {
            (score = pas::increment(score, pas::mul(petir_evalmask::closenessHalfTable[square][blackKingSquare], petir_ueval::queenTropismBonus)));
        }
    }
    pieces = position.blackQueens;
    while ((pieces != 0)) {
        square = popLowestBit(pieces);
        (score = pas::decrement(score, petir_ueval::blackQueenSquareBonus[square]));
        if ((!endgame)) {
            attacks = petir_bitboard_mask::diagonalA8H1Attacks[square][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[square]) & 255)];
            attacks = (attacks | petir_bitboard_mask::diagonalH8A1Attacks[square][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[square]) & 255)]);
            attacks = petir_bitboard_mask::diagonalA8H1Attacks[square][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[square]) & 255)];
            attacks = (attacks | petir_bitboard_mask::diagonalH8A1Attacks[square][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[square]) & 255)]);
            scratchBits = petir_bitboard_mask::horizontalAttacks[square][(pas::shiftRight(position.allPieces, pas::add((square & 56), 1)) & 63)];
            kingZoneHits = petir_bitboard_mask::verticalAttacks[square][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[square]) & 63)];
            attacks = ((attacks | kingZoneHits) | scratchBits);
            queenMobilitySquares = (attacks & (~position.blackPieces));
            (score = pas::decrement(score, petir_ueval::queenMobilityBonus[countBits<NativePopcount>(queenMobilitySquares)]));
            blackQueenAttackMap = (blackQueenAttackMap | attacks);
            if (((scratchBits & position.blackRooks) != 0)) {
                if (((scratchBits & whiteKingZone) != 0)) {
                    blackRookAttackMap = (blackRookAttackMap | scratchBits);
                }
            }
            if (((kingZoneHits & position.blackRooks) != 0)) {
                if (((kingZoneHits & whiteKingZone) != 0)) {
                    blackRookAttackMap = (blackRookAttackMap | kingZoneHits);
                }
            }
            (whiteKingPressure = pas::increment(whiteKingPressure, countBits<NativePopcount>((attacks & whiteKingZone))));
            if ((attacks & whiteKingZone) != 0) {
                (whiteAttackerCount = pas::increment(whiteAttackerCount, 1));
            }
        }
        if ((!earlyEndgame)) {
            (score = pas::decrement(score, pas::mul(petir_evalmask::closenessHalfTable[square][whiteKingSquare], petir_ueval::queenTropismBonus)));
        }
    }
    if ((!endgame)) {
        (score = pas::decrement(score, petir_ueval::whiteKingSquarePenalty[whiteKingSquare]));
        (score = pas::increment(score, petir_ueval::blackKingSquarePenalty[blackKingSquare]));
        if (opponentPerspective) {
            scratchValue = pas::sub(3, sideToMove);
        } else {
            scratchValue = sideToMove;
        }
        if ((position.blackQueens != 0)) {
            kingZoneHits = (((((whiteQueenAttackMap | whiteRookAttackMap) | whiteBishopAttackMap) | whiteKnightAttackMap) | whitePawnAttackMap) & whiteKingZone);
            scratchBits = ((whiteKingZone & blackQueenAttackMap) & (~kingZoneHits));
            attacks = (scratchBits & blackBishopAttackMap);
            if (((attacks != 0) && (scratchValue == petir_header::blackSide))) {
                (whiteKingPressure = pas::increment(whiteKingPressure, 2));
            }
            (whiteKingPressure = pas::increment(whiteKingPressure, pas::mul(countBits<NativePopcount>(attacks), 2)));
            attacks = ((scratchBits & blackKnightAttackMap) & (~kingZoneHits));
            if (((attacks != 0) && (scratchValue == petir_header::blackSide))) {
                (whiteKingPressure = pas::increment(whiteKingPressure, 2));
            }
            (whiteKingPressure = pas::increment(whiteKingPressure, pas::mul(countBits<NativePopcount>(attacks), 2)));
            attacks = ((scratchBits & blackRookAttackMap) & (~kingZoneHits));
            if (((attacks != 0) && (scratchValue == petir_header::blackSide))) {
                (whiteKingPressure = pas::increment(whiteKingPressure, 2));
            }
            (whiteKingPressure = pas::increment(whiteKingPressure, pas::mul(countBits<NativePopcount>(attacks), 2)));
            attacks = ((scratchBits & blackPawnAttackMap) & (~kingZoneHits));
            (whiteKingPressure = pas::increment(whiteKingPressure, pas::mul(countBits<NativePopcount>(attacks), 2)));
            attacks = ((whiteKingZone & blackRookAttackMap) & blackBishopAttackMap);
            (whiteKingPressure = pas::increment(whiteKingPressure, countBits<NativePopcount>(attacks)));
            weightedTerm = petir_ueval::kingPressurePenalty[std::clamp(whiteKingPressure, 0, 63)];
            whiteAttackerCase = whiteAttackerCount;
            if (((whiteAttackerCase == 1))) {
                weightedTerm = (weightedTerm / 5);
            }
            else if (((whiteAttackerCase == 3))) {
                weightedTerm = (pas::mul(weightedTerm, 125) / 100);
            }
            else if (((whiteAttackerCase == 4))) {
                weightedTerm = (pas::mul(weightedTerm, 150) / 100);
            }
            else if (((whiteAttackerCase == 5))) {
                weightedTerm = (pas::mul(weightedTerm, 200) / 100);
            }
            (score = pas::decrement(score, weightedTerm));
        }
        if ((position.whiteQueens != 0)) {
            kingZoneHits = (((((blackQueenAttackMap | blackRookAttackMap) | blackBishopAttackMap) | blackKnightAttackMap) | blackPawnAttackMap) & blackKingZone);
            scratchBits = ((blackKingZone & whiteQueenAttackMap) & (~kingZoneHits));
            attacks = (scratchBits & whiteBishopAttackMap);
            (blackKingPressure = pas::increment(blackKingPressure, pas::mul(countBits<NativePopcount>(attacks), 2)));
            if (((attacks != 0) && (scratchValue == petir_header::whiteSide))) {
                (blackKingPressure = pas::increment(blackKingPressure, 2));
            }
            attacks = ((scratchBits & whiteKnightAttackMap) & (~kingZoneHits));
            if (((attacks != 0) && (scratchValue == petir_header::whiteSide))) {
                (blackKingPressure = pas::increment(blackKingPressure, 2));
            }
            (blackKingPressure = pas::increment(blackKingPressure, pas::mul(countBits<NativePopcount>(attacks), 2)));
            attacks = ((scratchBits & whiteRookAttackMap) & (~kingZoneHits));
            if (((attacks != 0) && (scratchValue == petir_header::whiteSide))) {
                (blackKingPressure = pas::increment(blackKingPressure, 2));
            }
            (blackKingPressure = pas::increment(blackKingPressure, pas::mul(countBits<NativePopcount>(attacks), 2)));
            attacks = ((scratchBits & whitePawnAttackMap) & (~kingZoneHits));
            (blackKingPressure = pas::increment(blackKingPressure, pas::mul(countBits<NativePopcount>(attacks), 2)));
            attacks = ((blackKingZone & whiteRookAttackMap) & whiteBishopAttackMap);
            (blackKingPressure = pas::increment(blackKingPressure, countBits<NativePopcount>(attacks)));
            weightedTerm = petir_ueval::kingPressurePenalty[std::clamp(blackKingPressure, 0, 63)];
            blackAttackerCase = blackAttackerCount;
            if (((blackAttackerCase == 1))) {
                weightedTerm = (weightedTerm / 5);
            }
            else if (((blackAttackerCase == 3))) {
                weightedTerm = (pas::mul(weightedTerm, 125) / 100);
            }
            else if (((blackAttackerCase == 4))) {
                weightedTerm = (pas::mul(weightedTerm, 150) / 100);
            }
            else if (((blackAttackerCase == 5))) {
                weightedTerm = (pas::mul(weightedTerm, 200) / 100);
            }
            (score = pas::increment(score, weightedTerm));
        }
    } else {
        scratchBits = (position.blackPawns | position.whitePawns);
        if (((petir_evalmask::filesABCDMask & scratchBits) != 0)) {
            if (((petir_evalmask::filesEFGHMask & scratchBits) != 0)) {
                (score = pas::increment(score, petir_ueval::whiteKingEndgameBothWingsBonus[whiteKingSquare]));
                (score = pas::decrement(score, petir_ueval::blackKingEndgameBothWingsBonus[blackKingSquare]));
            } else {
                (score = pas::increment(score, petir_ueval::whiteKingEndgameQueensideBonus[whiteKingSquare]));
                (score = pas::decrement(score, petir_ueval::blackKingEndgameQueensideBonus[blackKingSquare]));
            }
        } else {
            if (((petir_evalmask::filesEFGHMask & scratchBits) != 0)) {
                (score = pas::increment(score, petir_ueval::whiteKingEndgameKingsideBonus[whiteKingSquare]));
                (score = pas::decrement(score, petir_ueval::blackKingEndgameKingsideBonus[blackKingSquare]));
            } else {
                (score = pas::increment(score, petir_ueval::whiteKingEndgameBothWingsBonus[whiteKingSquare]));
                (score = pas::decrement(score, petir_ueval::blackKingEndgameBothWingsBonus[blackKingSquare]));
            }
        }
    }
    if ((position.whitePawns == 0)) {
        (score = pas::decrement(score, petir_ueval::noPawnPenalty));
    }
    if ((position.blackPawns == 0)) {
        (score = pas::increment(score, petir_ueval::noPawnPenalty));
    }
    if ((position.board[petir_header::d2] == petir_header::whitePawn)) {
        if (((position.board[petir_header::d3] != 0) && (position.board[petir_header::c1] == petir_header::whiteBishop))) {
            (score = pas::decrement(score, petir_ueval::blockedCenterPawnPenalty));
        }
    }
    if ((position.board[petir_header::e2] == petir_header::whitePawn)) {
        if (((position.board[petir_header::e3] != 0) && (position.board[petir_header::f1] == petir_header::whiteBishop))) {
            (score = pas::decrement(score, petir_ueval::blockedCenterPawnPenalty));
        }
    }
    if ((position.board[petir_header::d7] == petir_header::blackPawn)) {
        if (((position.board[petir_header::d6] != 0) && (position.board[petir_header::c8] == petir_header::blackBishop))) {
            (score = pas::increment(score, petir_ueval::blockedCenterPawnPenalty));
        }
    }
    if ((position.board[petir_header::e7] == petir_header::blackPawn)) {
        if (((position.board[petir_header::e6] != 0) && (position.board[petir_header::f8] == petir_header::blackBishop))) {
            (score = pas::increment(score, petir_ueval::blockedCenterPawnPenalty));
        }
    }
    if ((((((position.whiteNonPawnMaterial == 3) && (position.blackNonPawnMaterial == 3)) && (position.blackBishopCount == 1)) && (position.whiteBishopCount == 1)) && (petir_ueval::squareColor[petir_bitboard_mask::firstBit(pas::address(position.blackBishops))] != petir_ueval::squareColor[petir_bitboard_mask::firstBit(pas::address(position.whiteBishops))]))) {
        score = (score / 2);
        if ((petir_ueval::hasSingleBit(position.blackPawns) && (position.whitePawns == 0))) {
            square = petir_bitboard_mask::firstBit(pas::address(position.blackPawns));
            attacks = petir_bitboard_mask::rookRayDownMask[square];
            if (((whiteBishopAttackMap & attacks) != 0)) {
                score = 0;
            }
        } else {
            if ((petir_ueval::hasSingleBit(position.whitePawns) && (position.blackPawns == 0))) {
                square = petir_bitboard_mask::lastBit(pas::address(position.whitePawns));
                attacks = petir_bitboard_mask::rookRayUpMask[square];
                if (((blackBishopAttackMap & attacks) != 0)) {
                    score = 0;
                }
            }
        }
    }
    if ((position.fiftyMoveCounter >= 80)) {
        termScore = pas::sub(101, position.fiftyMoveCounter);
        score = pas::round((score * (static_cast<double>(termScore) / (20))));
    }
    if (((position.whiteNonPawnMaterial == 5) && (position.blackNonPawnMaterial == 6))) {
    }
    if (((blackPawnCount == 0) && (whitePawnCount == 0))) {
        if ((((position.whiteNonPawnMaterial == 3) && (position.blackNonPawnMaterial == 5)) || ((position.whiteNonPawnMaterial == 5) && (position.blackNonPawnMaterial == 3)))) {
            score = (score / 4);
        } else {
            if ((((position.whiteNonPawnMaterial == 12) && (position.blackNonPawnMaterial == 9)) || ((position.whiteNonPawnMaterial == 9) && (position.blackNonPawnMaterial == 12)))) {
                score = (score / 8);
            }
        }
    } else {
        if (((whitePawnCount == 1) && (blackPawnCount == 0))) {
            if (((position.whiteNonPawnMaterial == 5) && (position.blackNonPawnMaterial == 6))) {
                score = (score / 2);
            } else {
                if (((position.whiteNonPawnMaterial == 5) && (position.blackNonPawnMaterial == 5))) {
                    score = score * (score > 0 ? rookPawnScale(position, realSideToMove) : 50) / 100;
                } else {
                    if (((position.whiteNonPawnMaterial == 9) && (position.blackNonPawnMaterial == 9))) {
                        score = (score / 4);
                    } else {
                        if (((position.whiteNonPawnMaterial == 3) && (position.blackNonPawnMaterial == 3))) {
                            score = (score / 2);
                        } else {
                            if (((position.whiteNonPawnMaterial == 3) && ((position.blackNonPawnMaterial == 6) || (position.blackNonPawnMaterial == 5)))) {
                                score = (score / 4);
                            }
                        }
                    }
                }
            }
        } else {
            if (((whitePawnCount == 0) && (blackPawnCount == 1))) {
                if (((position.whiteNonPawnMaterial == 6) && (position.blackNonPawnMaterial == 5))) {
                    score = (score / 2);
                } else {
                    if (((position.whiteNonPawnMaterial == 3) && (position.blackNonPawnMaterial == 3))) {
                        score = (score / 2);
                    } else {
                        if (((position.whiteNonPawnMaterial == 5) && (position.blackNonPawnMaterial == 5))) {
                            score = score * (score < 0 ? rookPawnScale(position, realSideToMove) : 50) / 100;
                        } else {
                            if (((position.whiteNonPawnMaterial == 9) && (position.blackNonPawnMaterial == 9))) {
                                score = (score / 4);
                            } else {
                                if ((((position.whiteNonPawnMaterial == 6) || (position.whiteNonPawnMaterial == 5)) && (position.blackNonPawnMaterial == 3))) {
                                    score = (score / 4);
                                }
                            }
                        }
                    }
                }
            } else {
                if (((whitePawnCount == 2) && (blackPawnCount == 0))) {
                    if ((((position.whiteNonPawnMaterial == 0) && (position.blackNonPawnMaterial == 6)) && (position.blackBishops != 0))) {
                        score = (score / 2);
                    }
                } else {
                    if (((whitePawnCount == 0) && (blackPawnCount == 2))) {
                        if ((((position.whiteNonPawnMaterial == 6) && (position.blackNonPawnMaterial == 0)) && (position.whiteBishops != 0))) {
                            score = (score / 2);
                        }
                    } else {
                        if ((whitePawnCount == blackPawnCount)) {
                            if (((((whitePassedCount == 0) && (blackPassedCount == 0)) && (position.whiteNonPawnMaterial == 5)) && (position.blackNonPawnMaterial == 5))) {
                                score = (pas::mul(score, 75) / 100);
                            }
                        }
                    }
                }
            }
        }
    }
    if (likelyDraw) {
        score = (score / 8);
    } else {
    }
    if ((sideToMove == petir_header::whiteSide)) {
        result = score;
    } else {
        result = pas::negate(score);
    }
    return result;
}

}

std::int32_t evaluate(const petir_header::tdata& position, std::int32_t sideToMove, std::int32_t alpha, std::int32_t beta, bool opponentPerspective) {
    int kpkScore;
    if (petir_kpk::probe(position, opponentPerspective ? 3 - sideToMove : sideToMove, kpkScore)) {
        // opponentPerspective supplies the opponent's score perspective after a move.
        if (!kpkScore) return petir_header::drawScore;
        return opponentPerspective ? -kpkScore : kpkScore;
    }
#if defined(_MSC_VER) && defined(_M_X64)
    // Select once per evaluation instead of dispatching at every bit count.
    if (hasNativePopcount) {
        return evaluateImpl<true>(position, sideToMove, alpha, beta, opponentPerspective);
    }
#endif
    return evaluateImpl<false>(position, sideToMove, alpha, beta, opponentPerspective);
}

// ueval.pas:2920
std::int32_t materialForSide(std::int32_t sideToMove, std::int32_t materialScore) {
    std::int32_t result{};
    if ((sideToMove == petir_header::blackSide)) {
        result = pas::negate(materialScore);
    } else {
        result = materialScore;
    }
    return result;
}

// ueval.pas:494
bool isMaterialDraw(const petir_header::tdata& position, bool& likelyDraw) {
    bool result{};
    result = false;
    likelyDraw = false;
    if (((position.blackPawns | position.whitePawns) == 0)) {
        if ((((position.whiteNonPawnMaterial == 8) && (position.blackNonPawnMaterial == 5)) || ((position.whiteNonPawnMaterial == 5) && (position.blackNonPawnMaterial == 8)))) {
            likelyDraw = true;
            return result;
        }
        if ((position.whiteNonPawnMaterial == 0)) {
            if ((position.blackNonPawnMaterial <= 3)) {
                result = true;
                return result;
            }
            if (((position.blackNonPawnMaterial == 6) && (position.blackBishops == 0))) {
                result = true;
                return result;
            }
        }
        if ((position.blackNonPawnMaterial == 0)) {
            if ((position.whiteNonPawnMaterial <= 3)) {
                result = true;
                return result;
            }
            if (((position.whiteNonPawnMaterial == 6) && (position.whiteBishops == 0))) {
                result = true;
                return result;
            }
        }
        if ((position.whiteNonPawnMaterial == position.blackNonPawnMaterial)) {
            if ((position.whiteNonPawnMaterial == 3)) {
                result = true;
                return result;
            }
            if ((((position.whiteNonPawnMaterial == 6) && (position.blackBishops == 0)) && (position.whiteBishops == 0))) {
                result = true;
                return result;
            }
        }
        if ((((pas::sub(position.whiteNonPawnMaterial, position.blackNonPawnMaterial) == 3) && (position.whiteNonPawnMaterial == 6)) && (position.whiteBishops == 0))) {
            result = true;
            return result;
        }
        if ((((pas::sub(position.blackNonPawnMaterial, position.whiteNonPawnMaterial) == 3) && (position.blackNonPawnMaterial == 6)) && (position.blackBishops == 0))) {
            result = true;
            return result;
        }
    }
    return result;
}

// ueval.pas:595
bool isTheoreticalDraw(const petir_header::tdata& position, std::uint8_t sideToMove) {
    bool result{};
    std::int32_t otherSquare{};
    std::int32_t pawnSquare{};
    std::int32_t defendingKing{};
    std::int32_t attackingKing{};
    result = false;
    if (((position.whiteNonPawnMaterial == 0) && (position.blackNonPawnMaterial == 0))) {
        if (((position.blackPawns == 0) && petir_ueval::hasSingleBit(position.whitePawns))) {
            pawnSquare = petir_bitboard_mask::lastBit(pas::address(position.whitePawns));
            if (((pawnSquare & 7) == 7)) {
                defendingKing = petir_bitboard_mask::kingSquare(position.blackKingBoard);
                if ((((defendingKing & 7) >= 6) && (pas::shiftRight(defendingKing, 3) >= pas::shiftRight(pawnSquare, 3)))) {
                    result = true;
                    return result;
                }
                attackingKing = petir_bitboard_mask::kingSquare(position.whiteKingBoard);
                if ((((((attackingKing & 7) == 7) && (pas::shiftRight(attackingKing, 3) > pas::shiftRight(pawnSquare, 3))) && ((defendingKing & 7) == 5)) && (pas::shiftRight(defendingKing, 3) > pas::shiftRight(pawnSquare, 3)))) {
                    result = true;
                    return result;
                }
            } else {
                if (((pawnSquare & 7) == 0)) {
                    defendingKing = petir_bitboard_mask::kingSquare(position.blackKingBoard);
                    if ((((defendingKing & 7) <= 1) && (pas::shiftRight(defendingKing, 3) >= pas::shiftRight(pawnSquare, 3)))) {
                        result = true;
                        return result;
                    }
                    attackingKing = petir_bitboard_mask::kingSquare(position.whiteKingBoard);
                    if ((((((attackingKing & 7) == 0) && (pas::shiftRight(attackingKing, 3) > pas::shiftRight(pawnSquare, 3))) && ((defendingKing & 7) == 2)) && (pas::shiftRight(defendingKing, 3) > pas::shiftRight(pawnSquare, 3)))) {
                        result = true;
                        return result;
                    }
                }
            }
            return result;
            defendingKing = petir_bitboard_mask::kingSquare(position.blackKingBoard);
            attackingKing = petir_bitboard_mask::kingSquare(position.whiteKingBoard);
            if ((defendingKing >= petir_header::a8)) {
                return result;
            }
            if (((pas::sub(defendingKing, pawnSquare) == 8) && (pas::sub(pawnSquare, attackingKing) == 8))) {
                result = true;
                return result;
            }
            if (((((defendingKing < petir_header::a8) && (pas::sub(defendingKing, attackingKing) == 16)) && (pas::sub(attackingKing, pawnSquare) == 8)) && (sideToMove == petir_header::whiteSide))) {
                result = true;
                return result;
            }
            if (((pas::sub(defendingKing, attackingKing) == 16) && (sideToMove == petir_header::whiteSide))) {
                result = true;
                return result;
            }
            if ((((pas::sub(defendingKing, attackingKing) == 16) && (pas::abs(pas::sub(pawnSquare, attackingKing)) == 1)) && (sideToMove == petir_header::whiteSide))) {
                result = true;
                return result;
            }
            if (((pas::sub(defendingKing, pawnSquare) == 15) || (pas::sub(defendingKing, pawnSquare) == 16) || (pas::sub(defendingKing, pawnSquare) == 17))) {
                if (((pas::sub(pawnSquare, attackingKing) == 8) && (sideToMove == petir_header::blackSide))) {
                    result = true;
                    return result;
                }
                if ((((pas::sub(pawnSquare, attackingKing) == 7) || (pas::sub(pawnSquare, attackingKing) == 8) || (pas::sub(pawnSquare, attackingKing) == 9)) && (sideToMove == petir_header::whiteSide))) {
                    result = true;
                    return result;
                }
            }
        } else {
            if (((position.whitePawns == 0) && petir_ueval::hasSingleBit(position.blackPawns))) {
                pawnSquare = petir_bitboard_mask::lastBit(pas::address(position.blackPawns));
                if (((pawnSquare & 7) == 7)) {
                    defendingKing = petir_bitboard_mask::kingSquare(position.whiteKingBoard);
                    if ((((defendingKing & 7) >= 6) && (pas::shiftRight(defendingKing, 3) <= pas::shiftRight(pawnSquare, 3)))) {
                        result = true;
                        return result;
                    }
                    attackingKing = petir_bitboard_mask::kingSquare(position.blackKingBoard);
                    if ((((((attackingKing & 7) == 7) && (pas::shiftRight(attackingKing, 3) < pas::shiftRight(pawnSquare, 3))) && ((defendingKing & 7) == 5)) && (pas::shiftRight(defendingKing, 3) < pas::shiftRight(pawnSquare, 3)))) {
                        result = true;
                        return result;
                    }
                } else {
                    if (((pawnSquare & 7) == 0)) {
                        defendingKing = petir_bitboard_mask::kingSquare(position.whiteKingBoard);
                        if ((((defendingKing & 7) <= 1) && (pas::shiftRight(defendingKing, 3) <= pas::shiftRight(pawnSquare, 3)))) {
                            result = true;
                            return result;
                        }
                        attackingKing = petir_bitboard_mask::kingSquare(position.blackKingBoard);
                        if ((((((attackingKing & 7) == 0) && (pas::shiftRight(attackingKing, 3) < pas::shiftRight(pawnSquare, 3))) && ((defendingKing & 7) == 2)) && (pas::shiftRight(defendingKing, 3) < pas::shiftRight(pawnSquare, 3)))) {
                            result = true;
                            return result;
                        }
                    }
                }
                return result;
                defendingKing = petir_bitboard_mask::kingSquare(position.blackKingBoard);
                attackingKing = petir_bitboard_mask::kingSquare(position.whiteKingBoard);
                if (((pas::sub(pawnSquare, attackingKing) == 8) && (pas::sub(defendingKing, pawnSquare) == 8))) {
                    result = true;
                    return result;
                }
                if (((pas::sub(defendingKing, attackingKing) == 16) && (sideToMove == petir_header::blackSide))) {
                    result = true;
                    return result;
                }
            }
        }
    } else {
        if ((((position.whitePawns == 0) && (position.whiteNonPawnMaterial == 3)) && (position.blackNonPawnMaterial == 0))) {
            if (petir_ueval::hasSingleBit(position.blackPawns)) {
                if ((position.whiteBishopCount != 0)) {
                    otherSquare = petir_bitboard_mask::firstBit(pas::address(position.blackPawns));
                    defendingKing = petir_bitboard_mask::kingSquare(position.whiteKingBoard);
                    attackingKing = petir_bitboard_mask::kingSquare(position.blackKingBoard);
                    if ((otherSquare >= petir_header::a3)) {
                        if ((!(((sideToMove == petir_header::blackSide) && (petir_evalmask::chebyshevDistance(otherSquare, attackingKing) == 1)) && (petir_evalmask::chebyshevDistance(otherSquare, defendingKing) > 1)))) {
                            result = true;
                            return result;
                        }
                    }
                    pawnSquare = petir_bitboard_mask::firstBit(pas::address(position.whiteBishops));
                    if ((((petir_bitboard_mask::bishopAttacks(position, pawnSquare) & petir_bitboard_mask::rookRayDownMask[otherSquare]) != 0) || ((petir_bitboard_mask::rookRayDownMask[otherSquare] & position.whiteKingBoard) != 0))) {
                        result = true;
                        return result;
                    }
                } else {
                    otherSquare = petir_bitboard_mask::firstBit(pas::address(position.whiteKnights));
                    pawnSquare = petir_bitboard_mask::firstBit(pas::address(position.blackPawns));
                    if (((petir_bitboard_mask::knightAttacks[otherSquare] & petir_bitboard_mask::rookRayDownMask[pawnSquare]) != 0)) {
                        result = true;
                        return result;
                    }
                }
            }
        } else {
            if ((((position.blackPawns == 0) && (position.blackNonPawnMaterial == 3)) && (position.whiteNonPawnMaterial == 0))) {
                if (petir_ueval::hasSingleBit(position.whitePawns)) {
                    if ((position.blackBishopCount != 0)) {
                        otherSquare = petir_bitboard_mask::firstBit(pas::address(position.whitePawns));
                        if ((otherSquare < petir_header::a7)) {
                            defendingKing = petir_bitboard_mask::kingSquare(position.whiteKingBoard);
                            attackingKing = petir_bitboard_mask::kingSquare(position.blackKingBoard);
                            if ((!(((sideToMove == petir_header::whiteSide) && (petir_evalmask::chebyshevDistance(otherSquare, defendingKing) == 1)) && (petir_evalmask::chebyshevDistance(otherSquare, attackingKing) > 1)))) {
                                result = true;
                                return result;
                            }
                        }
                    } else {
                        otherSquare = petir_bitboard_mask::firstBit(pas::address(position.blackKnights));
                        pawnSquare = petir_bitboard_mask::firstBit(pas::address(position.whitePawns));
                        if (((petir_bitboard_mask::knightAttacks[otherSquare] & petir_bitboard_mask::rookRayUpMask[pawnSquare]) != 0)) {
                            result = true;
                            return result;
                        }
                    }
                }
            } else {
                if ((((position.whitePawns == 0) && (position.blackNonPawnMaterial == 3)) && (position.whiteNonPawnMaterial == 0))) {
                    if ((petir_ueval::hasSingleBit(position.blackPawns) && (position.blackBishops != 0))) {
                        otherSquare = petir_bitboard_mask::firstBit(pas::address(position.blackPawns));
                        if (((otherSquare & 7) == 7)) {
                            pawnSquare = petir_bitboard_mask::firstBit(pas::address(position.blackBishops));
                            if (((petir_ueval::squareColor[pawnSquare] != petir_ueval::squareColor[petir_header::h1]) && (((position.board[petir_header::h1] == petir_header::whiteKing) || (position.board[petir_header::g1] == petir_header::whiteKing)) || (position.board[petir_header::h2] == petir_header::whiteKing)))) {
                                result = true;
                                return result;
                            }
                        } else {
                            if (((otherSquare & 7) == 0)) {
                                pawnSquare = petir_bitboard_mask::firstBit(pas::address(position.blackBishops));
                                if (((petir_ueval::squareColor[pawnSquare] != petir_ueval::squareColor[petir_header::a1]) && (((position.board[petir_header::a1] == petir_header::whiteKing) || (position.board[petir_header::b1] == petir_header::whiteKing)) || (position.board[petir_header::a2] == petir_header::whiteKing)))) {
                                    result = true;
                                    return result;
                                }
                            }
                        }
                    }
                } else {
                    if ((((position.blackPawns == 0) && (position.whiteNonPawnMaterial == 3)) && (position.blackNonPawnMaterial == 0))) {
                        if ((petir_ueval::hasSingleBit(position.whitePawns) && (position.whiteBishops != 0))) {
                            otherSquare = petir_bitboard_mask::firstBit(pas::address(position.whitePawns));
                            if (((otherSquare & 7) == 7)) {
                                pawnSquare = petir_bitboard_mask::firstBit(pas::address(position.whiteBishops));
                                if (((petir_ueval::squareColor[pawnSquare] != petir_ueval::squareColor[petir_header::h8]) && (((position.board[petir_header::h8] == petir_header::blackKing) || (position.board[petir_header::g8] == petir_header::blackKing)) || (position.board[petir_header::h7] == petir_header::blackKing)))) {
                                    result = true;
                                    return result;
                                }
                            } else {
                                if (((otherSquare & 7) == 0)) {
                                    pawnSquare = petir_bitboard_mask::firstBit(pas::address(position.whiteBishops));
                                    if (((petir_ueval::squareColor[pawnSquare] != petir_ueval::squareColor[petir_header::a8]) && (((position.board[petir_header::a8] == petir_header::blackKing) || (position.board[petir_header::b8] == petir_header::blackKing)) || (position.board[petir_header::a7] == petir_header::blackKing)))) {
                                        result = true;
                                        return result;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}

// ueval.pas:483
std::uint8_t popCount(std::uint64_t bits) {
    return static_cast<std::uint8_t>(std::popcount(bits));
}

// ueval.pas:421
std::int32_t popCountUpperWords(petir_bitboard_mask::pword wordPointer) {
    std::int32_t result{};
    std::int32_t count{};
    (wordPointer = pas::increment(wordPointer, 1));
    count = petir_evalmask::bitCountTable[(*wordPointer)];
    (wordPointer = pas::increment(wordPointer, 1));
    count = pas::add(count, petir_evalmask::bitCountTable[(*wordPointer)]);
    (wordPointer = pas::increment(wordPointer, 1));
    count = pas::add(count, petir_evalmask::bitCountTable[(*wordPointer)]);
    result = count;
    return result;
}

// ueval.pas:433
std::int32_t popCountLowerWords(petir_bitboard_mask::pword wordPointer) {
    std::int32_t result{};
    std::int32_t count{};
    count = petir_evalmask::bitCountTable[(*wordPointer)];
    (wordPointer = pas::increment(wordPointer, 1));
    count = pas::add(count, petir_evalmask::bitCountTable[(*wordPointer)]);
    (wordPointer = pas::increment(wordPointer, 1));
    count = pas::add(count, petir_evalmask::bitCountTable[(*wordPointer)]);
    result = count;
    return result;
}

// ueval.pas:444
std::uint8_t popCountWords(petir_bitboard_mask::pword wordPointer) {
    std::uint64_t bits;
    std::memcpy(&bits, wordPointer, sizeof(bits));
    return petir_ueval::popCount(bits);
}

// ueval.pas:456
bool hasSingleBit(std::uint64_t bits) {
    return bits != 0 && (bits & (bits - 1)) == 0;
}

// ueval.pas:832
std::int32_t lazyMarginAdjustment(const petir_header::tdata& position) {
    std::int32_t result{};
    result = 0;
    if (((position.blackQueens == 0) && (position.whiteQueens == 0))) {
        (result = pas::decrement(result, 50));
    }
    if (((position.blackRooks == 0) && (position.whiteRooks == 0))) {
        (result = pas::decrement(result, 30));
    }
    if (((position.blackKnights == 0) && (position.whiteKnights == 0))) {
        (result = pas::decrement(result, 20));
    }
    if (((position.blackBishops == 0) && (position.whiteBishops == 0))) {
        (result = pas::decrement(result, 20));
    }
    return result;
}

}
