// Faithful translation of ueval.pas; Delphi source is in original/.
//
// Conventions used throughout this file:
// - Squares run a1 = 0, b1 = 1, ... h8 = 63, so square >> 3 is the rank (0-7)
//   and square & 7 is the file (0 = a ... 7 = h).
// - Board codes are positive for White and negative for Black:
//   1 pawn, 2 knight, 3 bishop, 4 rook, 5 queen, 6 king.
// - Non-pawn material counts a knight or bishop as 3, a rook as 5 and a queen
//   as 9. A full set of pieces is 31.
// - The pas:: helpers come from the Pascal translation:
//   pas::increment(x, d) is x + d and pas::decrement(x, d) is x - d;
//   pas::add, pas::sub and pas::mul are +, - and * (with 32-bit wrap-around);
//   pas::negate(x) is -x; pas::shiftLeft and pas::shiftRight are << and >>
//   (shiftRight is a logical shift, which for the non-negative values used
//   here is a division by a power of two).
// - Inside evaluateImpl the score is from White's point of view:
//   pas::increment(score, ...) is good for White, pas::decrement(score, ...)
//   good for Black. It is turned to sideToMove's point of view at the end.
#include "petir.hpp"
#include "kpk.hpp"
#include <algorithm>
#include <bit>
#include <cstring>
#if defined(_MSC_VER) && defined(_M_X64)
#include <intrin.h>
#endif
#if defined(_M_X64) || defined(__x86_64__) || defined(__SSE2__)
#include <emmintrin.h>
#define PETIR_EVAL_SSE2 1
#endif
#if defined(_MSC_VER)
#define PETIR_EVAL_INLINE __forceinline
#define PETIR_EVAL_NOINLINE __declspec(noinline)
#else
#define PETIR_EVAL_INLINE inline __attribute__((always_inline))
#define PETIR_EVAL_NOINLINE __attribute__((noinline))
#endif
namespace petir_ueval {
namespace {
// Every square of `bits` and all squares in front of them on the same file,
// towards rank 8 (north) or towards rank 1 (south).
std::uint64_t fillNorth(std::uint64_t bits) {
    bits |= bits << 8;
    bits |= bits << 16;
    return bits | (bits << 32);
}

std::uint64_t fillSouth(std::uint64_t bits) {
    bits |= bits >> 8;
    bits |= bits >> 16;
    return bits | (bits >> 32);
}

// The squares of `bits` together with their neighbours on the files to the left
// and to the right.
std::uint64_t withNeighbourFiles(std::uint64_t bits) {
    using namespace petir_bitboard_mask;
    return bits | ((bits & notFileHMask) << 1) | ((bits & notFileAMask) >> 1);
}

// Bit f of the result is set when at least one pawn stands on file f.
std::uint8_t pawnFiles(std::uint64_t pawns) {
    pawns |= pawns >> 32;
    pawns |= pawns >> 16;
    pawns |= pawns >> 8;
    return static_cast<std::uint8_t>(pawns);
}

// Every square of the files set in `files` (bit f for file f).
std::uint64_t filesToSquares(unsigned files) {
    return std::uint64_t(files & 0xff) * UINT64_C(0x0101010101010101);
}

// Passed pawns: no enemy pawn in front of them on their own or a neighbouring
// file. This is the whitePassedPawnMask / blackPassedPawnMask test done for all
// pawns at once: a white pawn is held back exactly by the squares below some
// black pawn on its file or a neighbouring one. The self-test compares the
// masks with this geometry square by square.
std::uint64_t whitePassedPawns(std::uint64_t whitePawns, std::uint64_t blackPawns) {
    return whitePawns & ~withNeighbourFiles(fillSouth(blackPawns >> 8));
}

std::uint64_t blackPassedPawns(std::uint64_t whitePawns, std::uint64_t blackPawns) {
    return blackPawns & ~withNeighbourFiles(fillNorth(whitePawns << 8));
}

// The foremost passed pawn of each file: White's highest, Black's lowest. Since
// a pawn behind a passer on its file has more squares in front of it, a file's
// foremost passer is also its foremost pawn whenever that pawn is passed.
std::uint64_t foremostWhitePassers(std::uint64_t passers) {
    return passers & ~fillSouth(passers >> 8);
}

std::uint64_t foremostBlackPassers(std::uint64_t passers) {
    return passers & ~fillNorth(passers << 8);
}

// Bonus for each neighbouring pair of foremost passers (at most one per file in
// `passers`), from the owner's point of view and before scaling. The trailing
// pawn's rank determines the pair bonus.
int connectedPasserBonus(std::uint64_t passers, bool isWhite) {
    using namespace petir_bitboard_mask;
    const unsigned files = pawnFiles(passers);
    // Bit f: passers on files f and f + 1.
    unsigned pairs = files & (files >> 1);
    int bonus = 0;
    while (pairs) {
        const int file = std::countr_zero(pairs);
        pairs &= pairs - 1;
        // Ranks counted from the owner's side.
        const int leftSquare = std::countr_zero(passers & fileMask[file]);
        const int rightSquare = std::countr_zero(passers & fileMask[file + 1]);
        const int leftRank = isWhite ? leftSquare / 8 : 7 - leftSquare / 8;
        const int rightRank = isWhite ? rightSquare / 8 : 7 - rightSquare / 8;
        const int rank = std::min(leftRank, rightRank);
        const int separation = std::abs(leftRank - rightRank);
        bonus += connectedPassersBonus;
        // rank is the trailing (less advanced) pawn's rank: 5 is the sixth rank,
        // 4 the fifth. separation is how many ranks apart the two pawns are.
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

// Squares attacked by the piece with board code `piece` standing on `square`.
// Codes other than 1-6 and -1 to -6 (such as 0 for an empty square) attack nothing.
PETIR_EVAL_INLINE std::uint64_t boardPieceAttacks(const petir_header::tdata& position, int square, int piece) {
    using namespace petir_bitboard_mask;
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
}

// Union of the attacks of every piece on `pieces`, each taken from its board code.
std::uint64_t boardAttackMap(const petir_header::tdata& position, std::uint64_t pieces) {
    std::uint64_t result = 0;
    while (pieces) {
        const int square = std::countr_zero(pieces);
        pieces &= pieces - 1;
        result |= boardPieceAttacks(position, square, position.board[square]);
    }
    return result;
}

// endgamePieceActivity once its early exits have passed: an endgame (both sides
// at most 12 non-pawn material) with pawns on the board. enemyAttacks is
// boardAttackMap of the enemy pieces.
int endgamePieceActivityAgainst(const petir_header::tdata& position, int pieceSquare, bool isWhite,
                                std::uint64_t enemyAttacks) {
    using namespace petir_bitboard_mask;
    const int pieceType = std::abs(position.board[pieceSquare]);
    if (pieceType != 2 && pieceType != 4) return 0;
    const auto ownPieces = isWhite ? position.whitePieces : position.blackPieces;
    const auto ownPawns = isWhite ? position.whitePawns : position.blackPawns;
    const auto enemyPawns = isWhite ? position.blackPawns : position.whitePawns;
    const auto reach = boardPieceAttacks(position, pieceSquare, position.board[pieceSquare]);
    const auto pieceBit = UINT64_C(1) << pieceSquare;
    // No extra activity reward while the piece itself is under attack.
    // This conservative geometric test does not resolve pins or exchanges.
    if (enemyAttacks & pieceBit) return 0;
    // Knight: half its outpost bonus on a pawn-supported outpost that no enemy
    // pawn can ever attack.
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
    // Rook: safe squares it reaches, undefended enemy pawns it attacks, and a
    // penalty when a passive rook (3 safe squares or fewer) is the only defender
    // of attacked own pawns.
    const int safeSquares = std::popcount(reach & ~ownPieces & ~enemyAttacks);
    int bonus = 2 * std::min(6, safeSquares) - 6; // -6..+6, on top of raw mobility.
    bonus += 4 * std::min(2, std::popcount(reach & enemyPawns & ~enemyAttacks));
    if (safeSquares <= 3) {
        const auto otherDefenders = boardAttackMap(position, ownPieces & ~pieceBit);
        const auto tiedPawns = reach & ownPawns & enemyAttacks & ~otherDefenders;
        bonus -= 3 * std::min(2, std::popcount(tiedPawns));
    }
    return bonus;
}

// lazyMarginAdjustment, here so that evaluateImpl can inline it.
std::int32_t lazyMarginForTrades(const petir_header::tdata& position) {
    // Each piece type that both sides have traded off narrows the margin.
    return -50 * ((position.blackQueens | position.whiteQueens) == 0)
         - 30 * ((position.blackRooks | position.whiteRooks) == 0)
         - 20 * ((position.blackKnights | position.whiteKnights) == 0)
         - 20 * ((position.blackBishops | position.whiteBishops) == 0);
}

// Whether isTheoreticalDraw can return true: all of its rules need at most one
// minor piece on the board and no other piece.
bool mayBeTheoreticalDraw(const petir_header::tdata& position) {
    const int white = position.whiteNonPawnMaterial, black = position.blackNonPawnMaterial;
    return (white == 0 && (black == 0 || black == 3)) || (black == 0 && white == 3);
}

// Pawn-structure terms that depend on the two pawn bitboards alone, cached by
// those bitboards. An entry is used only when both stored bitboards equal the
// position's, so it returns exactly what evaluating the pawns again would give.
// The table starts zero-filled, which is the correct entry for a board without
// pawns: keep every field zero for that case. Terms that also read the board
// array, the kings or the pieces are not cached.
struct alignas(64) PawnEntry {
    std::uint64_t whitePawns, blackPawns;
    // The foremost passed pawn of each file.
    std::uint64_t whitePassers, blackPassers;
    // Sum of the pawns' square bonuses, in total and for each squareWing value.
    std::int16_t whiteSquareBonus, blackSquareBonus;
    std::int16_t whiteWingSquareBonus[3], blackWingSquareBonus[3];
    // Isolated-pawn penalties (without the potential-passer bonus), how many
    // isolated pawns there are, and on which files.
    std::int16_t whiteIsolatedPenalty, blackIsolatedPenalty;
    std::uint8_t whiteIsolatedCount, blackIsolatedCount;
    std::uint8_t whiteIsolatedFiles, blackIsolatedFiles;
    // Doubled pawns: pawns minus occupied files.
    std::uint8_t whiteDoubled, blackDoubled;
    // connectedPasserBonus of the passers above, before scaling.
    std::int16_t whiteConnectedBonus, blackConnectedBonus;
};
static_assert(sizeof(PawnEntry) == 64);

constexpr int pawnTableBits = 12;
PawnEntry pawnTable[1 << pawnTableBits];

// The terms of one colour's pawns for a PawnEntry.
struct PawnSideTerms {
    int squareBonus = 0, wingSquareBonus[3] = {0, 0, 0};
    int isolatedPenalty = 0, isolatedCount = 0, isolatedFiles = 0, doubled = 0, connectedBonus = 0;
    std::uint64_t passers = 0;
};

PawnSideTerms pawnSideTerms(std::uint64_t ownPawns, std::uint64_t enemyPawns, bool isWhite) {
    using namespace petir_bitboard_mask;
    PawnSideTerms terms;
    for (auto pawns = ownPawns; pawns; pawns &= pawns - 1) {
        const int square = std::countr_zero(pawns);
        const int bonus = isWhite ? whitePawnSquareBonus[square] : blackPawnSquareBonus[square];
        terms.squareBonus += bonus;
        terms.wingSquareBonus[petir_evalmask::squareWing[square]] += bonus;
    }
    const unsigned files = pawnFiles(ownPawns);
    terms.isolatedFiles = files & ~((files << 1) | (files >> 1)) & 0xff;
    // Each occupied file has exactly one foremost pawn; all others are doubled.
    terms.doubled = std::popcount(ownPawns) - std::popcount(files);
    // Isolated pawn: twice the file penalty; twice more when doubled (another
    // own pawn behind it); once more (and once more if doubled) when no pawn of
    // either colour is in front of it.
    for (auto pawns = ownPawns & filesToSquares(terms.isolatedFiles); pawns; pawns &= pawns - 1) {
        const int square = std::countr_zero(pawns);
        const int penalty = isolatedPawnPenalty[square & 7];
        const bool stacked = ((isWhite ? rookRayDownMask[square] : rookRayUpMask[square]) & ownPawns) != 0;
        const bool nothingInFront = ((isWhite ? rookRayUpMask[square] : rookRayDownMask[square]) & (ownPawns | enemyPawns)) == 0;
        terms.isolatedPenalty += 2 * penalty + (stacked ? 2 * penalty : 0) + (nothingInFront ? penalty * (stacked ? 2 : 1) : 0);
        ++terms.isolatedCount;
    }
    terms.passers = isWhite ? foremostWhitePassers(whitePassedPawns(ownPawns, enemyPawns))
                            : foremostBlackPassers(blackPassedPawns(enemyPawns, ownPawns));
    terms.connectedBonus = connectedPasserBonus(terms.passers, isWhite);
    return terms;
}

PETIR_EVAL_NOINLINE void fillPawnEntry(PawnEntry& entry, std::uint64_t whitePawns, std::uint64_t blackPawns) {
    const auto white = pawnSideTerms(whitePawns, blackPawns, true);
    const auto black = pawnSideTerms(blackPawns, whitePawns, false);
    entry.whitePawns = whitePawns;
    entry.blackPawns = blackPawns;
    entry.whitePassers = white.passers;
    entry.blackPassers = black.passers;
    entry.whiteSquareBonus = static_cast<std::int16_t>(white.squareBonus);
    entry.blackSquareBonus = static_cast<std::int16_t>(black.squareBonus);
    for (int wing = 0; wing < 3; ++wing) {
        entry.whiteWingSquareBonus[wing] = static_cast<std::int16_t>(white.wingSquareBonus[wing]);
        entry.blackWingSquareBonus[wing] = static_cast<std::int16_t>(black.wingSquareBonus[wing]);
    }
    entry.whiteIsolatedPenalty = static_cast<std::int16_t>(white.isolatedPenalty);
    entry.blackIsolatedPenalty = static_cast<std::int16_t>(black.isolatedPenalty);
    entry.whiteIsolatedCount = static_cast<std::uint8_t>(white.isolatedCount);
    entry.blackIsolatedCount = static_cast<std::uint8_t>(black.isolatedCount);
    entry.whiteIsolatedFiles = static_cast<std::uint8_t>(white.isolatedFiles);
    entry.blackIsolatedFiles = static_cast<std::uint8_t>(black.isolatedFiles);
    entry.whiteDoubled = static_cast<std::uint8_t>(white.doubled);
    entry.blackDoubled = static_cast<std::uint8_t>(black.doubled);
    entry.whiteConnectedBonus = static_cast<std::int16_t>(white.connectedBonus);
    entry.blackConnectedBonus = static_cast<std::int16_t>(black.connectedBonus);
}

std::size_t pawnTableIndex(std::uint64_t whitePawns, std::uint64_t blackPawns) {
    return static_cast<std::size_t>(((whitePawns * UINT64_C(0x9e3779b97f4a7c15)) ^ (blackPawns * UINT64_C(0xc2b2ae3d27d4eb4f)))
                                    >> (64 - pawnTableBits));
}

// Starts loading a table entry into the cache ahead of its use.
void prefetchPawnEntry(std::size_t index) {
#ifdef PETIR_EVAL_SSE2
    _mm_prefetch(reinterpret_cast<const char*>(&pawnTable[index]), _MM_HINT_T0);
#else
    (void)index;
#endif
}

const PawnEntry& probePawnTable(std::size_t index, std::uint64_t whitePawns, std::uint64_t blackPawns) {
    PawnEntry& entry = pawnTable[index];
    if (entry.whitePawns != whitePawns || entry.blackPawns != blackPawns) fillPawnEntry(entry, whitePawns, blackPawns);
    return entry;
}

// Bit i is set when board[first + i] holds `piece`, for the 16 squares from `first`.
std::uint32_t boardSquaresHolding(const petir_header::tdata& position, int first, int piece) {
#ifdef PETIR_EVAL_SSE2
    const __m128i squares = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&position.board.values[first]));
    return static_cast<std::uint32_t>(_mm_movemask_epi8(_mm_cmpeq_epi8(squares, _mm_set1_epi8(static_cast<char>(piece)))));
#else
    std::uint32_t holding = 0;
    for (int i = 0; i < 16; ++i) holding |= std::uint32_t(position.board[first + i] == piece) << i;
    return holding;
#endif
}
}

// Endgame refinement for a passed pawn, from the pawn owner's point of view.
// The caller passes the foremost passer of a file. The bonus rewards our king
// being closer than the enemy king to the square in front of the pawn (the
// stop square), and a free path: the stop square empty and not attacked, and
// then every square up to promotion empty and not attacked.
int passedPawnActivity(const petir_header::tdata& position, int pawnSquare, bool isWhite) {
    using namespace petir_bitboard_mask;
    // Keep this refinement in endgames and leave KPK to its existing evaluator.
    if (position.whiteNonPawnMaterial > 12 || position.blackNonPawnMaterial > 12) return 0;
    const auto allPawns = position.whitePawns | position.blackPawns;
    if (position.whiteNonPawnMaterial == 0 && position.blackNonPawnMaterial == 0
        && !(allPawns & (allPawns - 1))) return 0;
    // Only pawns on the fifth, sixth or seventh rank (counted from the owner's side).
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
    // King distance to the stop square, capped at 5: enemy king far and own king near is good.
    int bonus = 2 * weight * (kingDistance(enemyKing) - kingDistance(ownKing));
    // Stop square occupied by any piece.
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
    // Whether an enemy piece attacks the square. whitePawnAttacks[square] holds
    // the squares a white pawn on `square` would attack; a black pawn on one of
    // them attacks `square` (and the other way round for Black).
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
    return isWhite ? connectedPasserBonus(foremostWhitePassers(whitePassedPawns(position.whitePawns, position.blackPawns)), true)
                   : connectedPasserBonus(foremostBlackPassers(blackPassedPawns(position.whitePawns, position.blackPawns)), false);
}

// Piece-owner perspective. Small additions to existing mobility/PST terms.
int endgamePieceActivity(const petir_header::tdata& position, int pieceSquare, bool isWhite) {
    if (position.whiteNonPawnMaterial > 12 || position.blackNonPawnMaterial > 12) return 0;
    if (!(position.whitePawns | position.blackPawns)) return 0;
    const int pieceType = std::abs(position.board[pieceSquare]);
    if (pieceType != 2 && pieceType != 4) return 0;
    const auto enemyPieces = isWhite ? position.blackPieces : position.whitePieces;
    return endgamePieceActivityAgainst(position, pieceSquare, isWhite, boardAttackMap(position, enemyPieces));
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
    // "Strong" is the side with the pawn, "weak" the defending side. kingSquare()
    // is used here as "square of the only bit" for the pawn and the rooks too.
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

#if defined(_MSC_VER) && defined(_M_X64)
// Whether the CPU has the POPCNT instruction.
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

// The main evaluation. Returns the score from sideToMove's point of view.
// opponentPerspective: the caller wants sideToMove's score although the other
// side is to move in `position`; realSideToMove below is the side actually to
// move. alpha/beta: in a null-window search (alpha == beta - 1) the
// evaluation may stop early (lazy evaluation) when the score is far outside
// the window.
//
// Order of work:
//  1. draw detection;
//  2. cheap terms: material, trapped pieces, pawn count, side to move, rooks
//     blocked by their king;
//  3. lazy exit;
//  4. kings, castling, pawn shelter, bishops, pawn structure and passed pawns
//     (Black first, then White), rooks, knights, bishops, queens, king safety
//     or endgame king placement, small extra terms;
//  5. scaling towards a draw: opposite bishops, the 50-move rule, drawish
//     material, likely draws.
// ueval.pas:850
template<bool NativePopcount>
std::int32_t evaluateImpl(const petir_header::tdata& position, std::int32_t sideToMove, std::int32_t alpha, std::int32_t beta, bool opponentPerspective) {
    std::int32_t result{};
    // Several of these locals are scratch values reused for different things in
    // different sections (named after their main use):
    //   termScore    - pawn-shelter holes; the side-relative score in the lazy
    //                  exit; Black's scaled passer terms; White passer's rank;
    //                  the rooks' open-file bonus; 101 - fiftyMoveCounter.
    //   weightedTerm - White's scaled passer terms; bishops aiming at the enemy
    //                  king; the king-danger penalty.
    //   scratchValue - number of isolated pawns; later the side actually to move.
    //   pawnTally    - number of doubled pawns; later the pawns supporting a
    //                  knight outpost.
    //   scratchBits  - all pawns; mobility squares; pawns supporting a knight;
    //                  queen rank attacks; undefended king-zone squares the
    //                  enemy queen attacks.
    //   kingZoneHits - attacked king-zone squares; queen file attacks; defended
    //                  king-zone squares.
    //   attacks      - a piece's attacks; then queen targets also hit by another
    //                  enemy piece; a pawn's path.
    //   fileAttacks  - the ray behind a passer; a rook's file attacks.
    //   bishopSquare - square of a side's single bishop, 64 when it has none or two.
    //   stackedOnFile - rook with another rook or a queen on its file.
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
    // 1. Dead draws by material or by known endgame rules. isMaterialDraw sets
    // likelyDraw for rook + minor against rook without pawns; such scores are
    // divided by 8 at the end. isMaterialDraw only acts without pawns, and
    // isTheoreticalDraw only with at most one minor piece on the board; it is
    // given the side actually to move.
    likelyDraw = false;
    if (((position.blackPawns | position.whitePawns) == 0 && petir_ueval::isMaterialDraw(position, likelyDraw))
        || (mayBeTheoreticalDraw(position)
            && petir_ueval::isTheoreticalDraw(position, opponentPerspective ? pas::sub(3, sideToMove) : sideToMove))) {
        result = petir_header::drawScore;
        return result;
    }
    blackPawnCount = 0;
    whitePawnCount = 0;
    whitePassedCount = 0;
    blackPassedCount = 0;
    // The side actually to move in this position (see opponentPerspective).
    realSideToMove = sideToMove;
    if (opponentPerspective) {
        realSideToMove = pas::sub(3, sideToMove);
    }
    whiteAttackerCount = 0;
    blackAttackerCount = 0;
    // 2. Cheap terms. Start from the material balance.
    score = position.materialScore;
    // Trapped bishops. A white bishop on h6 (a6) facing a black pawn on g5 (b5)
    // that is defended from f6 (c6), or that the bishop cannot take without
    // losing material (SEE < 0). The next two are the same for a black bishop on
    // h3 (a3), which is good for White.
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
    // A white bishop on h7 (a7) shut in by a black pawn on g6 (b6), or about to
    // be when Black is to move with a pawn on g7 (b7). Then the same for a black
    // bishop on h2 (a2).
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
    // A knight in the far corner (a8/h8 for White, a1/h1 for Black) whose two
    // ways out both lose material.
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
    // Game phase. earlyEndgame: either side has less than 16 non-pawn material;
    // endgame: both have at most 12; realEndgame: both have at most 10.
    // Pawns gain value as pieces come off: +8 each (shiftLeft 3) in a real
    // endgame, else +4 (shiftLeft 2). With queen + minor on both sides, a knight
    // goes well with the queen.
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
    // Bonus of 10 for the side to move.
    if ((realSideToMove == petir_header::whiteSide)) {
        (score = pas::increment(score, 10));
    } else {
        (score = pas::decrement(score, 10));
    }
    // With most pieces still on: a king on f1/g1 (c1/b1) with its own rook still
    // in that corner (h1, g1, h2 / a1, a2, b1) keeps the rook out of play. The
    // same for Black on the eighth rank.
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
    // 3. Lazy evaluation: in a null-window search (alpha == beta - 1), stop here
    // when the score is so far outside the window that the remaining terms
    // cannot bring it back. The margin is 280 plus lazyMarginAdjustment (smaller
    // when piece types are traded off), plus 600 in an endgame or 200 in an
    // early endgame, where passed pawns can swing the score.
    // Complete evaluation when its score needs the later draw scaling.
    if (!likelyDraw && position.fiftyMoveCounter < 80 && alpha == pas::sub(beta, 1)) {
        lazyMargin = lazyMarginForTrades(position);
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
    // Start loading the pawn cache entry, which is used below.
    const auto pawnIndex = pawnTableIndex(position.whitePawns, position.blackPawns);
    prefetchPawnEntry(pawnIndex);
    // 4. Full evaluation. whiteKingPressure counts attacked squares in White's
    // king zone, whiteAttackerCount the black pieces attacking it (and the other
    // way round for Black). Piece attacks on a king are counted only while the
    // enemy has a queen, and never in an endgame.
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
    blackKingSquare = petir_bitboard_mask::kingSquare(position.blackKingBoard);
    whiteKingSquare = petir_bitboard_mask::kingSquare(position.whiteKingBoard);
    whiteKingFile = (whiteKingSquare & 7);
    blackKingFile = (blackKingSquare & 7);
    // Kings castled on opposite sides or at least three files apart (not in an
    // early endgame). Then pawns on the enemy king's wing count double.
    if (earlyEndgame) {
        kingsOnOppositeWings = false;
    } else {
        kingsOnOppositeWings = ((((position.whiteCastleType == petir_header::castledShort) && (position.blackCastleType == petir_header::castledLong)) || ((position.blackCastleType == petir_header::castledShort) && (position.whiteCastleType == petir_header::castledLong))) || (pas::abs(pas::sub(whiteKingFile, blackKingFile)) >= 3));
    }
    // King zone: the king's square and the squares around it.
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
    // Castling, Black first. Lost castling rights without having castled: a full
    // penalty while the enemy has a queen, half without. Having castled:
    // castledBonus, except castling long against an enemy castled short, which
    // costs castledWrongSideBonus (applied as a penalty, despite its name).
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
    // Pawn shelter in front of White's king, while Black has attacking material
    // (a queen and 16+ non-pawn material, or 19+). termScore adds up the holes
    // and indexes pawnShieldPenalty:
    //  - king on the f-h files: no white pawn on the g-file +4 (+1 more when no
    //    black pawn is there either), or the g-pawn has left g2/g3 +2; no pawn on
    //    the h-file +2 (+1), or it has left h2/h3 +2; no f-pawn +1; kings castled
    //    differently +1. Separately, neither a pawn nor a bishop on g2 costs
    //    missingFianchettoPenalty.
    //  - king on the a-c files: the same mirrored (a-file +3, b-file +4, c-file +1).
    //  - king in the centre: +2 for its own file and each neighbouring file
    //    without a white pawn.
    // Without a black queen the holes count half.
    // board[14]/[22] are g2/g3, [15]/[23] h2/h3, [8]/[16] a2/a3, [9]/[17] b2/b3.
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
    // The same for Black's king. board[54]/[46] are g7/g6, [55]/[47] h7/h6,
    // [48]/[40] a7/a6, [49]/[41] b7/b6.
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
    // Black's bishops. One bishop against none, or two against one, in an endgame
    // with pawns on both wings (a/b and g/h files): bishopOverKnightEndgameBonus
    // (half outside a real endgame). Two bishops against none: the bishop pair,
    // worth more with fewer pawns. bishopSquare is the single bishop's square for
    // the bad-bishop count below, 64 otherwise.
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
    // The terms of the pawn structure alone, from the pawn cache.
    const PawnEntry& pawnStructure = probePawnTable(pawnIndex, position.whitePawns, position.blackPawns);
    // Connected passed pawns of both colours, scaled down while the enemy has pieces.
    // Connection scoring is independent of the early exits in the passer loops.
    if (const int bonus = pawnStructure.whiteConnectedBonus)
        score += bonus * endgameScale[std::min<int>(position.blackNonPawnMaterial, 41)] / 100;
    if (const int bonus = pawnStructure.blackConnectedBonus)
        score -= bonus * endgameScale[std::min<int>(position.whiteNonPawnMaterial, 41)] / 100;
    // Black's pawns. The terms of the pawn structure alone come from the pawn
    // cache (pawnSideTerms). pawnsOnBishopColor: black pawns off their two back
    // ranks on the colour of Black's single bishop (bad bishop).
    blackPawnCount = countBits<NativePopcount>(position.blackPawns);
    pawnsOnBishopColor = bishopSquare < 64 ? countBits<NativePopcount>(position.blackPawns & UINT64_C(0x0000ffffffffffff)
        & (petir_ueval::squareColor[bishopSquare] == 0 ? UINT64_C(0xaa55aa55aa55aa55) : UINT64_C(0x55aa55aa55aa55aa))) : 0;
    pawnTally = pawnStructure.blackDoubled;
    // Piece-square value, counted twice when the kings are on opposite wings
    // and the pawn is on White's king's wing.
    (score = pas::decrement(score, pawnStructure.blackSquareBonus));
    if (kingsOnOppositeWings) {
        (score = pas::decrement(score, pawnStructure.blackWingSquareBonus[petir_evalmask::squareWing[whiteKingSquare]]));
    }
    // Pawns on their start rank (squares a7 and up) blocked by a white pawn: the
    // board squares eight below hold a white pawn.
    pieces = position.blackPawns & (std::uint64_t(boardSquaresHolding(position, petir_header::a6, petir_header::whitePawn)) << 48);
    score += petir_ueval::blockedPawnPenalty * countBits<NativePopcount>(pieces);
    // Isolated pawns, while White has pieces: the penalties of pawnSideTerms,
    // and the potential-passer bonus below for pawns on a7, b7, g7 and h7.
    scratchValue = 0;
    if (position.whiteNonPawnMaterial != 0) {
        scratchValue = pawnStructure.blackIsolatedCount;
        (score = pas::increment(score, pawnStructure.blackIsolatedPenalty));
        pieces = position.blackPawns & filesToSquares(pawnStructure.blackIsolatedFiles)
               & (petir_bitboard_mask::squareBit(petir_header::a7) | petir_bitboard_mask::squareBit(petir_header::b7)
                  | petir_bitboard_mask::squareBit(petir_header::g7) | petir_bitboard_mask::squareBit(petir_header::h7));
        while (pieces != 0) {
            square = popLowestBit(pieces);
            // A lone black pawn still on a7/b7 (g7/h7) against two white pawns on those
            // files: White may create a passer there.
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
    // Passed pawns: the foremost black pawn of each file with no white pawn in
    // front of it on its own or a neighbouring file.
    pieces = pawnStructure.blackPassers;
    while (pieces != 0) {
        square = popLowestBit(pieces);
        (blackPassedCount = pas::increment(blackPassedCount, 1));
        (score = pas::decrement(score, petir_ueval::passedPawnBonus));
        // passedPawnActivity is zero outside an endgame.
        if (endgame) score -= passedPawnActivity(position, square, false);
        termScore = 0;
        // Against a bare white king (no white pieces) some passers promote by force.
        // Each rule adds assuredPromotionBonus plus the rank bonus and skips the
        // remaining passer terms (continue):
        //  1. pawn on its sixth or seventh rank with its king diagonally in front;
        //  2. pawn on b2-g2 with its king next to it and White's king not on the
        //     promotion square, unless White's king stands beside the pawn, White is
        //     to move, Black has no pieces and Black's own king is off ranks 1-2;
        //  3. the pawn is two ranks ahead of White's king, or one with Black to move;
        //  4. rule of the square: White's king is too many files away;
        //  5. pawn on its sixth rank (rank 3) with its king in front of it.
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
        // The remaining passer terms collect in termScore, scaled down at the end
        // while White has pieces. A black rook behind the pawn helps it (unless a
        // white piece blocks the pawn); a white rook behind it holds it back, worse in
        // a pure rook ending with Black's king far from the pawn (closeness 5 or less).
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
        // Blocked by a white piece: keep 1/2 + 1/8 of the rank bonus. Otherwise a
        // pawn on its seventh rank that can promote without losing material (SEE 0)
        // earns safeAdvanceBonus (+40 with Black to move), unscaled.
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
        // Black's king next to the pawn supports it. Otherwise White's king close to
        // the pawn (closeness 7+, about three squares or less) and in front of it,
        // within reach of its path, takes away a half or a quarter of the rank bonus.
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
    // Squares attacked by black pawns; then the doubled and isolated pawn counts,
    // the bad bishop, and black pawn attacks on White's king zone.
    blackPawnAttackMap = pas::shiftRight((position.blackPawns & petir_bitboard_mask::notFileAMask), 9);
    blackPawnAttackMap = (blackPawnAttackMap | pas::shiftRight((position.blackPawns & petir_bitboard_mask::notFileHMask), 7));
    (score = pas::increment(score, petir_ueval::doubledPawnPenalty[pawnTally]));
    (score = pas::increment(score, petir_ueval::isolatedPawnCountPenalty[scratchValue]));
    if (petir_ueval::hasSingleBit(position.blackBishops)) {
        (score = pas::increment(score, petir_evalmask::badBishopPenalty[pawnsOnBishopColor]));
    }
    kingZoneHits = (blackPawnAttackMap & whiteKingZone);
    (whiteKingPressure = pas::increment(whiteKingPressure, countBits<NativePopcount>(kingZoneHits)));
    // White's bishops, as for Black above.
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
    // White's pawns, mirroring Black's above. Here termScore holds the pawn's
    // rank and weightedTerm collects the scaled passer terms.
    whitePawnCount = countBits<NativePopcount>(position.whitePawns);
    pawnsOnBishopColor = bishopSquare < 64 ? countBits<NativePopcount>(position.whitePawns & UINT64_C(0xffffffffffff0000)
        & (petir_ueval::squareColor[bishopSquare] == 0 ? UINT64_C(0xaa55aa55aa55aa55) : UINT64_C(0x55aa55aa55aa55aa))) : 0;
    pawnTally = pawnStructure.whiteDoubled;
    // Piece-square value, counted twice when the kings are on opposite wings
    // and the pawn is on Black's king's wing.
    (score = pas::increment(score, pawnStructure.whiteSquareBonus));
    if (kingsOnOppositeWings) {
        (score = pas::increment(score, pawnStructure.whiteWingSquareBonus[petir_evalmask::squareWing[blackKingSquare]]));
    }
    // Pawns on their start rank (squares h2 and below) blocked by a black pawn:
    // the board squares eight above hold a black pawn.
    pieces = position.whitePawns & boardSquaresHolding(position, petir_header::a2, petir_header::blackPawn);
    score -= petir_ueval::blockedPawnPenalty * countBits<NativePopcount>(pieces);
    // Isolated pawns, while Black has pieces, as for Black above.
    scratchValue = 0;
    if (position.blackNonPawnMaterial != 0) {
        scratchValue = pawnStructure.whiteIsolatedCount;
        (score = pas::decrement(score, pawnStructure.whiteIsolatedPenalty));
        pieces = position.whitePawns & filesToSquares(pawnStructure.whiteIsolatedFiles)
               & (petir_bitboard_mask::squareBit(petir_header::a2) | petir_bitboard_mask::squareBit(petir_header::b2)
                  | petir_bitboard_mask::squareBit(petir_header::g2) | petir_bitboard_mask::squareBit(petir_header::h2));
        while (pieces != 0) {
            square = popLowestBit(pieces);
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
    // Passed pawns, as for Black above: the foremost passer of each file.
    pieces = pawnStructure.whitePassers;
    while (pieces != 0) {
        square = popHighestBit(pieces);
        (whitePassedCount = pas::increment(whitePassedCount, 1));
        weightedTerm = 0;
        (score = pas::increment(score, petir_ueval::passedPawnBonus));
        if (endgame) score += passedPawnActivity(position, square, true);
        termScore = pas::shiftRight(square, 3);
        // Unstoppable passers against a bare black king; see the Black rules above.
        if ((position.blackNonPawnMaterial == 0)) {
            if (((square >= petir_header::a6) && ((petir_bitboard_mask::whitePawnAttacks[square] & position.whiteKingBoard) != 0))) {
                (score = pas::increment(score, petir_ueval::assuredPromotionBonus));
                (score = pas::increment(score, pas::shiftLeft(petir_ueval::whitePassedPawnRankBonus[pas::shiftRight(square, 3)], 1)));
                continue;
            }
            if (((((square >= petir_header::b7) && (square <= petir_header::g7)) && (position.board[pas::add(square, 8)] != petir_header::blackKing)) && ((petir_bitboard_mask::kingAttacks[square] & position.whiteKingBoard) != 0))) {
                if ((((((whiteKingSquare <= petir_header::h6) && ((petir_bitboard_mask::kingAttacks[square] & position.blackKingBoard) != 0)) && (realSideToMove == petir_header::blackSide)) && (position.whiteNonPawnMaterial == 0)) && ((blackKingSquare == pas::sub(square, 1)) || (blackKingSquare == pas::add(square, 1))))) {
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
    whitePawnAttackMap = pas::shiftLeft((position.whitePawns & petir_bitboard_mask::notFileAMask), 7);
    whitePawnAttackMap = (whitePawnAttackMap | pas::shiftLeft((position.whitePawns & petir_bitboard_mask::notFileHMask), 9));
    (score = pas::decrement(score, petir_ueval::doubledPawnPenalty[pawnTally]));
    (score = pas::decrement(score, petir_ueval::isolatedPawnCountPenalty[scratchValue]));
    if (petir_ueval::hasSingleBit(position.whiteBishops)) {
        (score = pas::decrement(score, petir_evalmask::badBishopPenalty[pawnsOnBishopColor]));
    }
    kingZoneHits = (whitePawnAttackMap & blackKingZone);
    (blackKingPressure = pas::increment(blackKingPressure, countBits<NativePopcount>(kingZoneHits)));
    // Three points more piece material including a queen, against two or three
    // extra enemy pawns: +35 for the side with the queen.
    if ((((position.whiteNonPawnMaterial == pas::add(position.blackNonPawnMaterial, 3)) && (position.whiteQueens != 0)) && ((pas::sub(blackPawnCount, whitePawnCount) >= 2 && pas::sub(blackPawnCount, whitePawnCount) <= 3)))) {
        (score = pas::increment(score, 35));
    } else {
        if ((((position.blackNonPawnMaterial == pas::add(position.whiteNonPawnMaterial, 3)) && (position.blackQueens != 0)) && ((pas::sub(whitePawnCount, blackPawnCount) >= 2 && pas::sub(whitePawnCount, blackPawnCount) <= 3)))) {
            (score = pas::decrement(score, 35));
        }
    }
    // Endgame activity of rooks and knights (endgamePieceActivity) needs every
    // square the enemy attacks. It is zero outside an endgame or without pawns.
    const bool pieceActivity = endgame && (position.whitePawns | position.blackPawns) != 0;
    std::uint64_t whiteAttackSquares = 0;
    std::uint64_t blackAttackSquares = 0;
    if (pieceActivity) {
        if ((position.whiteRooks | position.whiteKnights) != 0) blackAttackSquares = boardAttackMap(position, position.blackPieces);
        if ((position.blackRooks | position.blackKnights) != 0) whiteAttackSquares = boardAttackMap(position, position.whitePieces);
    }
    termScore = 0;
    // White rooks: endgame activity, mobility, attacks on Black's king zone,
    // piece-square value, half-open and open files (termScore, scaled by
    // rookScale of White's material), the seventh and eighth rank, and a rook
    // boxed in by its own pieces on both sides.
    pieces = position.whiteRooks;
    while ((pieces != 0)) {
        square = popLowestBit(pieces);
        if (pieceActivity) score += endgamePieceActivityAgainst(position, square, true, blackAttackSquares);
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
        // Half-open file: no white pawn in front of the rook. nearEnemyKingWing: the
        // rook is on the same wing as Black's king (not in an endgame).
        // stackedOnFile: another rook in front of it, or a queen on the file.
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
            // Fully open file: no black pawn in front either.
            if (((petir_bitboard_mask::rookRayUpMask[square] & position.blackPawns) == 0)) {
                (termScore = pas::increment(termScore, petir_ueval::openFileBonus[(square & 7)]));
                if (((petir_bitboard_mask::rookRayDownMask[square] & position.whiteQueens) != 0)) {
                    (termScore = pas::increment(termScore, petir_ueval::rookQueenOpenFileBonus));
                }
                if (nearEnemyKingWing) {
                    (termScore = pas::increment(termScore, petir_ueval::rookOpenFileNearKingBonus));
                }
                if (stackedOnFile) {
                    (termScore = pas::increment(termScore, petir_ueval::doubledRooksOpenFileBonus));
                    if (nearEnemyKingWing) {
                        (termScore = pas::increment(termScore, petir_ueval::doubledRooksNearKingBonus));
                    }
                }
            }
        }
        // Rook on the seventh rank with black pawns there or Black's king on the
        // eighth; or on the eighth with the king there.
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
        // Own pieces on both sides of the rook along the rank (only one side for a
        // rook on the a or h file).
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
    if (termScore != 0) {
        (score = pas::increment(score, (pas::mul(termScore, petir_ueval::rookScale[std::min<int>(position.whiteNonPawnMaterial, 41)]) / 100)));
    }
    termScore = 0;
    // Black rooks, the same mirrored.
    pieces = position.blackRooks;
    while ((pieces != 0)) {
        // Rook scores and attack unions are independent of iteration order.
        square = popLowestBit(pieces);
        if (pieceActivity) score -= endgamePieceActivityAgainst(position, square, false, whiteAttackSquares);
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
    if (termScore != 0) {
        (score = pas::decrement(score, (pas::mul(termScore, petir_ueval::rookScale[std::min<int>(position.blackNonPawnMaterial, 41)]) / 100)));
    }
    // White knights: endgame activity, piece-square value, closeness to Black's
    // king (not in an early endgame), mobility, attacks on Black's king zone, and
    // outposts: a square no black pawn can ever attack, supported by one (half
    // bonus) or two (full bonus) white pawns, plus half the bonus when Black has
    // no knight and no bishop that can attack the square.
    pieces = position.whiteKnights;
    while ((pieces != 0)) {
        square = popLowestBit(pieces);
        if (pieceActivity) score += endgamePieceActivityAgainst(position, square, true, blackAttackSquares);
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
    // Black knights, the same mirrored.
    pieces = position.blackKnights;
    while ((pieces != 0)) {
        square = popLowestBit(pieces);
        if (pieceActivity) score -= endgamePieceActivityAgainst(position, square, false, whiteAttackSquares);
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
    // White bishops: mobility, outposts, attacks on Black's king zone,
    // piece-square value and closeness to Black's king; a bonus when both bishops
    // aim at the king (weightedTerm counts them).
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
    // Black bishops, the same mirrored.
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
    // White queens: piece-square value; outside endgames mobility and attacks on
    // Black's king zone (always counted, unlike the other pieces); before the
    // endgame, closeness to Black's king. A rook lined up with the queen on a
    // rank (scratchBits) or file (kingZoneHits) that reaches Black's king zone
    // adds that line to the rook attack map.
    pieces = position.whiteQueens;
    while ((pieces != 0)) {
        square = popLowestBit(pieces);
        (score = pas::increment(score, petir_ueval::whiteQueenSquareBonus[square]));
        if ((!endgame)) {
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
    // Black queens, the same mirrored.
    pieces = position.blackQueens;
    while ((pieces != 0)) {
        square = popLowestBit(pieces);
        (score = pas::decrement(score, petir_ueval::blackQueenSquareBonus[square]));
        if ((!endgame)) {
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
        // Middlegame king placement.
        (score = pas::decrement(score, petir_ueval::whiteKingSquarePenalty[whiteKingSquare]));
        (score = pas::increment(score, petir_ueval::blackKingSquarePenalty[blackKingSquare]));
        // scratchValue: the side actually to move (the same as realSideToMove).
        if (opponentPerspective) {
            scratchValue = pas::sub(3, sideToMove);
        } else {
            scratchValue = sideToMove;
        }
        if ((position.blackQueens != 0)) {
            // Danger to White's king while Black has a queen. kingZoneHits: zone squares
            // White defends; scratchBits: zone squares Black's queen attacks that White
            // does not defend. Each of those also attacked by a black bishop, knight or
            // rook adds 2 (plus 2 once per piece type when Black is to move), each also
            // attacked by a black pawn adds 2, and each zone square hit by both a black
            // rook and a black bishop adds 1. The total indexes kingPressurePenalty, which
            // is then scaled by the number of attacking pieces: 1 attacker /5, 3 x1.25,
            // 4 x1.5, 5 x2.
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
            // The same for Black's king while White has a queen.
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
        // Endgame king placement: piece-square tables chosen by where the pawns are
        // (both wings or no pawns, queenside only, kingside only).
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
    // A side without pawns finds it harder to win.
    if ((position.whitePawns == 0)) {
        (score = pas::decrement(score, petir_ueval::noPawnPenalty));
    }
    if ((position.blackPawns == 0)) {
        (score = pas::increment(score, petir_ueval::noPawnPenalty));
    }
    // A d2/e2 pawn that cannot advance because d3/e3 is occupied, with its bishop
    // still on c1/f1 behind it. Then the same for Black.
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
    // 5. Scaling towards a draw. Bishops of opposite colours and no other pieces:
    // halve the score; a lone pawn whose path the defending bishop covers makes
    // it zero.
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
    // Fade the score out over the last 20 plies before the 50-move rule.
    if ((position.fiftyMoveCounter >= 80)) {
        termScore = pas::sub(101, position.fiftyMoveCounter);
        score = pas::round((score * (static_cast<double>(termScore) / (20))));
    }
    // Empty in the original.
    if (((position.whiteNonPawnMaterial == 5) && (position.blackNonPawnMaterial == 6))) {
    }
    // Material combinations that are hard to win despite an advantage:
    //  - no pawns: rook against minor /4, queen + minor against queen /8;
    //  - one pawn against none: rook + pawn against two minors /2, KRP-KR by
    //    rookPawnScale (or half when the other side is better), queen + pawn
    //    against queen /4, minor + pawn against minor /2, minor + pawn against a
    //    rook or two minors /4, and the same mirrored for Black;
    //  - two pawns against two minors including a bishop /2;
    //  - rook ending with equal pawns and no passers: 75%.
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
    // Rook + minor against rook without pawns (see isMaterialDraw).
    if (likelyDraw) {
        score = (score / 8);
    } else {
    }
    // Return the score from sideToMove's point of view.
    if ((sideToMove == petir_header::whiteSide)) {
        result = score;
    } else {
        result = pas::negate(score);
    }
    return result;
}

}

std::int32_t evaluate(const petir_header::tdata& position, std::int32_t sideToMove, std::int32_t alpha, std::int32_t beta, bool opponentPerspective) {
    // King + pawn against king is answered exactly by the KPK table. The probe
    // needs a board without pieces other than kings and pawns.
    int kpkScore;
    if ((position.whiteNonPawnMaterial | position.blackNonPawnMaterial) == 0 && petir_kpk::probe(position, opponentPerspective ? 3 - sideToMove : sideToMove, kpkScore)) {
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
// Pawnless endings that cannot be won: returns true for a dead draw (bare
// king against at most a minor or two knights, minor against minor, two
// knights against two knights or against a minor). Rook + minor against rook
// (8 against 5) is not dead but sets likelyDraw.
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
// Known draws with a single pawn:
//  - king and rook pawn against king: the defending king reaches the corner
//    in front of the pawn, or the attacking king is stuck on the rook file in
//    front of its pawn and held there by the defender;
//  - a minor piece against a lone pawn, including the bishop standing on or
//    covering the pawn's path, or the minor side's king standing on it;
//  - bishop + rook pawn against a bare king in the promotion corner, with the
//    bishop not covering the promotion square.
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
            // Unreachable: the return above always exits. Kept from the original.
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
                // Unreachable: the return above always exits. Kept from the original.
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
        // White minor piece against a lone black pawn.
        if ((((position.whitePawns == 0) && (position.whiteNonPawnMaterial == 3)) && (position.blackNonPawnMaterial == 0))) {
            if (petir_ueval::hasSingleBit(position.blackPawns)) {
                if ((position.whiteBishopCount != 0)) {
                    otherSquare = petir_bitboard_mask::firstBit(pas::address(position.blackPawns));
                    defendingKing = petir_bitboard_mask::kingSquare(position.whiteKingBoard);
                    attackingKing = petir_bitboard_mask::kingSquare(position.blackKingBoard);
                    // Pawn not yet on its seventh rank: a draw, unless Black is to move with its
                    // king next to the pawn and White's king is not.
                    if ((otherSquare >= petir_header::a3)) {
                        if ((!(((sideToMove == petir_header::blackSide) && (petir_evalmask::chebyshevDistance(otherSquare, attackingKing) == 1)) && (petir_evalmask::chebyshevDistance(otherSquare, defendingKing) > 1)))) {
                            result = true;
                            return result;
                        }
                    }
                    pawnSquare = petir_bitboard_mask::firstBit(pas::address(position.whiteBishops));
                    // The bishop (pawnSquare holds its square here) stands on or covers the pawn's path, or White's king stands on it.
                    if (((((petir_bitboard_mask::bishopAttacks(position, pawnSquare) | petir_bitboard_mask::squareBit(pawnSquare)) & petir_bitboard_mask::rookRayDownMask[otherSquare]) != 0) || ((petir_bitboard_mask::rookRayDownMask[otherSquare] & position.whiteKingBoard) != 0))) {
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
            // Black minor piece against a lone white pawn.
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
                        pawnSquare = petir_bitboard_mask::firstBit(pas::address(position.blackBishops));
                        // The bishop (pawnSquare holds its square here) stands on or covers the pawn's path, or Black's king stands on it.
                        if (((((petir_bitboard_mask::bishopAttacks(position, pawnSquare) | petir_bitboard_mask::squareBit(pawnSquare)) & petir_bitboard_mask::rookRayUpMask[otherSquare]) != 0) || ((petir_bitboard_mask::rookRayUpMask[otherSquare] & position.blackKingBoard) != 0))) {
                            result = true;
                            return result;
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
                // Black bishop + rook pawn against White's bare king in the promotion corner.
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
                    // White bishop + rook pawn against Black's bare king in the promotion corner.
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
// Narrows the lazy-evaluation margin for each piece type that both sides have
// traded off, since fewer pieces mean smaller positional swings.
std::int32_t lazyMarginAdjustment(const petir_header::tdata& position) {
    return lazyMarginForTrades(position);
}

}
#undef PETIR_EVAL_SSE2
#undef PETIR_EVAL_INLINE
#undef PETIR_EVAL_NOINLINE
