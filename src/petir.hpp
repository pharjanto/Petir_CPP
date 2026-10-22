// Generated from the bundled Delphi source; see tools/translate.py.
#pragma once
#include "pascal_runtime.hpp"
#include "time_manager.hpp"
#ifdef PETIR_HASH_STATS
namespace petir_hash_stats { struct Probe; }
#endif
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
#include <intrin.h>
#include <xmmintrin.h>
#include <new>
#endif
namespace petir_header {
inline constexpr auto version = 1;
inline constexpr auto a1 = 0;
inline constexpr auto b1 = 1;
inline constexpr auto c1 = 2;
inline constexpr auto d1 = 3;
inline constexpr auto e1 = 4;
inline constexpr auto f1 = 5;
inline constexpr auto g1 = 6;
inline constexpr auto h1 = 7;
inline constexpr auto a2 = 8;
inline constexpr auto b2 = 9;
inline constexpr auto c2 = 10;
inline constexpr auto d2 = 11;
inline constexpr auto e2 = 12;
inline constexpr auto f2 = 13;
inline constexpr auto g2 = 14;
inline constexpr auto h2 = 15;
inline constexpr auto a3 = 16;
inline constexpr auto b3 = 17;
inline constexpr auto c3 = 18;
inline constexpr auto d3 = 19;
inline constexpr auto e3 = 20;
inline constexpr auto f3 = 21;
inline constexpr auto g3 = 22;
inline constexpr auto h3 = 23;
inline constexpr auto a4 = 24;
inline constexpr auto b4 = 25;
inline constexpr auto c4 = 26;
inline constexpr auto d4 = 27;
inline constexpr auto e4 = 28;
inline constexpr auto f4 = 29;
inline constexpr auto g4 = 30;
inline constexpr auto h4 = 31;
inline constexpr auto a5 = 32;
inline constexpr auto b5 = 33;
inline constexpr auto c5 = 34;
inline constexpr auto d5 = 35;
inline constexpr auto e5 = 36;
inline constexpr auto f5 = 37;
inline constexpr auto g5 = 38;
inline constexpr auto h5 = 39;
inline constexpr auto a6 = 40;
inline constexpr auto b6 = 41;
inline constexpr auto c6 = 42;
inline constexpr auto d6 = 43;
inline constexpr auto e6 = 44;
inline constexpr auto f6 = 45;
inline constexpr auto g6 = 46;
inline constexpr auto h6 = 47;
inline constexpr auto a7 = 48;
inline constexpr auto b7 = 49;
inline constexpr auto c7 = 50;
inline constexpr auto d7 = 51;
inline constexpr auto e7 = 52;
inline constexpr auto f7 = 53;
inline constexpr auto g7 = 54;
inline constexpr auto h7 = 55;
inline constexpr auto a8 = 56;
inline constexpr auto b8 = 57;
inline constexpr auto c8 = 58;
inline constexpr auto d8 = 59;
inline constexpr auto e8 = 60;
inline constexpr auto f8 = 61;
inline constexpr auto g8 = 62;
inline constexpr auto h8 = 63;
inline constexpr auto rank1 = 0;
inline constexpr auto rank2 = 1;
inline constexpr auto rank3 = 2;
inline constexpr auto rank4 = 3;
inline constexpr auto rank5 = 4;
inline constexpr auto rank6 = 5;
inline constexpr auto rank7 = 6;
inline constexpr auto rank8 = 7;
inline constexpr auto whitePawn = 1;
inline constexpr auto whiteKnight = 2;
inline constexpr auto whiteBishop = 3;
inline constexpr auto whiteRook = 4;
inline constexpr auto whiteQueen = 5;
inline constexpr auto whiteKing = 6;
inline constexpr auto blackPawn = pas::negate(1);
inline constexpr auto blackKnight = pas::negate(2);
inline constexpr auto blackBishop = pas::negate(3);
inline constexpr auto blackRook = pas::negate(4);
inline constexpr auto blackQueen = pas::negate(5);
inline constexpr auto blackKing = pas::negate(6);
inline constexpr auto exactBound = 0;
inline constexpr auto upperBound = 1;
inline constexpr auto lowerBound = 2;
inline constexpr auto whiteSide = 1;
inline constexpr auto blackSide = 2;
inline constexpr auto gameOver = 3;
inline constexpr auto maxNode = 1;
inline constexpr auto minNode = 2;
inline constexpr auto noEnPassant = 10;
inline constexpr auto noCastleTarget = 64;
inline constexpr auto shortCastleTarget = 65;
inline constexpr auto longCastleTarget = 66;
inline constexpr auto enPassantTarget = 67;
inline constexpr auto noMove = 0;
inline constexpr auto twoPlayers = 0;
inline constexpr auto playerVsComputer = 1;
inline constexpr auto computerVsPlayer = 2;
inline constexpr auto computerVsComputer = 3;
inline constexpr auto notCastled = 0;
inline constexpr auto castledShort = 1;
inline constexpr auto castledLong = 2;
inline constexpr auto whiteShortCastleRight = 1;
inline constexpr auto whiteLongCastleRight = 2;
inline constexpr auto blackShortCastleRight = 4;
inline constexpr auto blackLongCastleRight = 8;
inline constexpr auto whiteCastleRights = 3;
inline constexpr auto blackCastleRights = 12;
inline constexpr auto whiteWinResult = 0;
inline constexpr auto drawResult = 1;
inline constexpr auto blackWinResult = 2;
inline constexpr auto queenPromotion = 1;
inline constexpr auto knightPromotion = 2;
inline constexpr auto bishopPromotion = 3;
inline constexpr auto rookPromotion = 4;
inline pas::Array<std::uint16_t, 0, 12> powersOfTwo = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
// powersOfTwo[exponent] as the int it promotes to; checked builds keep the table's range check.
inline std::int32_t powerOfTwo(std::int64_t exponent) {
#ifdef PETIR_CHECK_BOUNDS
    (void)powersOfTwo[exponent];
#endif
    return std::int32_t(1) << exponent;
}
inline constexpr auto hashMovePhase = 0;
inline constexpr auto goodCapturesPhase = 1;
inline constexpr auto killerMove1Phase = 2;
inline constexpr auto killerMove2Phase = 3;
inline constexpr auto killerMove3Phase = 4;
inline constexpr auto killerMove4Phase = 5;
inline constexpr auto otherMovesPhase = 6;
inline constexpr auto badCapturesPhase = 7;
inline constexpr auto finishedPhase = 8;
inline constexpr auto minutesPerGame = 0;
inline constexpr auto secondsPerMove = 1;
inline constexpr auto movesInMinutes = 2;
inline constexpr auto minutesPerGameWithIncrement = 3;
inline constexpr auto fixedDepth = 4;
inline constexpr auto ponderOff = 0;
inline constexpr auto ponderFillHash = 1;
inline constexpr auto ponderGuess = 2;
inline constexpr auto ponderCombine = 3;
inline constexpr auto pawnValue = 98;
inline constexpr auto bishopValue = 343;
inline constexpr auto knightValue = 330;
inline constexpr auto rookValue = 503;
inline constexpr auto queenValue = 1036;
inline constexpr auto kingValue = 20000;
inline pas::Array<std::uint16_t, pas::negate(6), 6> pieceValue = {petir_header::kingValue, petir_header::queenValue, petir_header::rookValue, petir_header::bishopValue, petir_header::knightValue, petir_header::pawnValue, 0, petir_header::pawnValue, petir_header::knightValue, petir_header::bishopValue, petir_header::rookValue, petir_header::queenValue, petir_header::kingValue};
inline constexpr auto infinity = 20000;
inline constexpr auto historySize = 65536;
using tkillermove = std::int32_t;
struct tmoverecord {
    std::int32_t move;
    std::int32_t score;
};
using pmoverecord = petir_header::tmoverecord*;
// Move generation can exceed the legacy 100-entry limit after promotions.
// Keep this capacity guard even when general Pascal bounds checks are off.
// The throw lives out of line so each access is one compare and branch.
#if defined(_MSC_VER)
[[noreturn]] __declspec(noinline) void moveListIndexError(std::int64_t index);
#else
[[noreturn]] __attribute__((noinline)) void moveListIndexError(std::int64_t index);
#endif
struct tmovelist : pas::Array<petir_header::tmoverecord, 1, 256> {
    tmoverecord& operator[](std::int64_t index) {
        if(static_cast<std::uint64_t>(index-1)>=256) [[unlikely]] moveListIndexError(index);
        return values[index-1];
    }
    const tmoverecord& operator[](std::int64_t index) const {
        if(static_cast<std::uint64_t>(index-1)>=256) [[unlikely]] moveListIndexError(index);
        return values[index-1];
    }
};
using tarr = pas::Array<std::int8_t, 0, 63>;
struct tdata {
    petir_header::tarr board;
    std::uint8_t enPassantFile;
    std::uint8_t castleRights;
    std::uint8_t whiteCastleType;
    std::uint8_t blackCastleType;
    std::uint8_t fiftyMoveCounter;
    std::uint8_t lastCaptureType;
    std::uint8_t whiteNonPawnMaterial;
    std::uint8_t blackNonPawnMaterial;
    std::uint8_t blackBishopCount;
    std::uint8_t whiteBishopCount;
    bool pawnEnding;
    std::int32_t materialScore;
    std::uint64_t hashKey;
    std::uint64_t whitePieces;
    std::uint64_t blackPieces;
    std::uint64_t allPieces;
    std::uint64_t allPiecesRotated90;
    std::uint64_t allPiecesA8H1;
    std::uint64_t allPiecesH8A1;
    std::uint64_t blackPawns;
    std::uint64_t whitePawns;
    std::uint64_t blackBishops;
    std::uint64_t whiteBishops;
    std::uint64_t blackKnights;
    std::uint64_t whiteKnights;
    std::uint64_t blackRooks;
    std::uint64_t whiteRooks;
    std::uint64_t blackQueens;
    std::uint64_t whiteQueens;
    std::uint64_t blackKingBoard;
    std::uint64_t whiteKingBoard;
    std::uint64_t rooksAndQueens;
    std::uint64_t bishopsAndQueens;
};
extern std::int32_t hashStoreCount;
extern std::int32_t hashCollisionCount;
extern std::int32_t futilityFailCount;
extern std::int32_t futilityTryCount;
extern std::int32_t seeTimeTotal;
extern std::int32_t evalTimeMs;
extern std::int32_t seeTimeMs;
extern std::int32_t moveGenTimeMs;
extern bool futilityPruningEnabled;
extern bool lazyEvalEnabled;
extern bool quiescencePruningEnabled;
extern bool nullMoveEnabled;
extern bool etcEnabled;
extern std::int32_t lazyMarginNodeStamp;
extern std::int32_t pawnEvalScore;
extern std::int32_t knightEvalScore;
extern std::int32_t bishopEvalScore;
extern std::int32_t rookEvalScore;
extern std::int32_t queenEvalScore;
extern std::int32_t kingShelterEvalScore;
extern std::int32_t kingPressureEvalScore;
extern std::int32_t tropismEvalScore;
extern std::int32_t quiescenceMargin;
extern std::int32_t futilityMargin;
extern std::int32_t threadPriority;
extern std::int32_t hashTableSize;
extern std::int32_t resignThreshold;
extern std::int32_t resignCounter;
extern std::int32_t drawCounter;
extern std::int32_t nodeCount;
extern std::int32_t quiescenceNodeCount;
extern bool stalemateFound;
extern std::int32_t playerMode;
extern std::int32_t engineSide;
extern std::int32_t iterationDepth;
extern bool useMtdf;
// Quiet-move history stays within +/-historyLimit (see petir_search::updateHistory).
inline constexpr std::int32_t historyLimit = 16384;
extern pas::Array<pas::Array<std::int32_t, 0, 67>, pas::negate(6), 6> whiteHistory;
extern pas::Array<pas::Array<std::int32_t, 0, 67>, pas::negate(6), 6> blackHistory;
// Continuation history: how a quiet move [piece][to] did right after an
// earlier move, keyed by that move's [piece][to]; the piece's sign gives the
// side that made it. Entries stay within +/-historyLimit.
using piece_to_history = pas::Array<pas::Array<std::int16_t, 0, 67>, pas::negate(6), 6>;
extern pas::Array<pas::Array<piece_to_history, 0, 67>, pas::negate(6), 6> continuationHistory;
extern pas::Array<petir_header::tkillermove, pas::negate(2), 63> firstKillers;
extern pas::Array<petir_header::tkillermove, pas::negate(2), 63> secondKillers;
extern std::int32_t cutoffCount;
extern std::int32_t hashCutoffCount;
extern std::int32_t lazyEvalExitCount;
extern std::int32_t futilityPruneCount;
extern std::int32_t extendedFutilityPruneCount;
extern std::int32_t razorCount;
extern std::int32_t firstMoveCutoffCount;
extern pas::StringList debugStringList;
extern std::int32_t lazyMargin1;
extern std::int32_t lazyMargin2;
extern std::int32_t lazyMargin3;
extern std::int32_t adjustedLazyMargin1;
extern std::int32_t adjustedLazyMargin2;
extern std::int32_t adjustedLazyMargin3;
extern std::int32_t drawScore;
extern std::int32_t hashHitCount;
extern std::int32_t etcCutoffCount;
extern std::int32_t maxDepthReached;
extern bool isPondering;
extern std::atomic_bool stopSearch;
extern bool threadKilled;
extern std::int32_t maxSearchPly;
extern std::int32_t movesOutOfBook;
extern std::int32_t whiteClock;
extern std::int32_t blackClock;
extern std::int32_t fixedMoveTime;
extern bool useTimer;
extern std::int32_t timeFactor;
extern std::atomic<std::int32_t> timeElapsed;
extern std::int32_t timeLimit;
extern std::int32_t currentSide;
extern std::int32_t rootBestMove;
extern pas::Array<pas::Array<std::uint64_t, 0, 127>, pas::negate(6), 6> zobristPieceKeys;
extern pas::Array<pas::Array<std::uint64_t, 0, 127>, pas::negate(6), 6> zobristPieceKeys2;
extern pas::Array<std::uint64_t, 0, 255> zobristCastleKeys;
extern pas::Array<std::uint64_t, 0, 255> zobristCastleKeys2;
extern pas::Array<std::uint64_t, 0, petir_header::noEnPassant> zobristEnPassantKeys;
extern pas::Array<std::uint64_t, 0, petir_header::noEnPassant> zobristEnPassantKeys2;
extern std::int32_t searchStartTime;
extern std::int32_t outputLineNumber;
extern double maxExtensionFactor;
extern std::int32_t gamePlyCount;
extern bool computerThinking;
extern bool useBook;
extern std::uint8_t ponderMode;
extern std::int32_t plyUnits;
extern std::int32_t checkExtension;
extern std::int32_t mateThreatExtension;
extern std::int32_t pawnRecaptureExtension;
extern std::int32_t knightRecaptureExtension;
extern std::int32_t rookRecaptureExtension;
extern std::int32_t queenRecaptureExtension;
extern std::int32_t singleReplyExtension;
extern std::int32_t twoReplyExtension;
extern std::int32_t doubleCheckExtension;
extern std::int32_t pawnPushExtensionMiddlegame;
extern std::int32_t pawnPushExtensionEndgame;
extern std::int32_t pawnPushExtensionLateEndgame;
extern petir_header::tdata gamePosition;
extern std::int32_t timeMode;
extern std::int32_t movesPerTimeControl;
extern std::int32_t timeIncrement;
extern bool runningTestSuite;
extern bool testSolved;
extern pas::Array<std::int32_t, 1, 5> testAnswers;
extern bool useWinboard;
extern std::int32_t pvReferenceTime;
extern std::int32_t errorCount;
extern std::uint64_t noEnPassantHashKey;
extern pas::File debugFile;
}
namespace petir_bitboard_mask {
using pword = std::uint16_t*;
inline pas::Array<std::uint8_t, 0, 63> diagonalH8A1Shift = {28, 21, 15, 10, 6, 3, 1, 0, 36, 28, 21, 15, 10, 6, 3, 1, 43, 36, 28, 21, 15, 10, 6, 3, 49, 43, 36, 28, 21, 15, 10, 6, 54, 49, 43, 36, 28, 21, 15, 10, 58, 54, 49, 43, 36, 28, 21, 15, 61, 58, 54, 49, 43, 36, 28, 21, 63, 61, 58, 54, 49, 43, 36, 28};
inline pas::Array<std::uint8_t, 0, 63> diagonalA8H1Shift = {0, 1, 3, 6, 10, 15, 21, 28, 1, 3, 6, 10, 15, 21, 28, 36, 3, 6, 10, 15, 21, 28, 36, 43, 6, 10, 15, 21, 28, 36, 43, 49, 10, 15, 21, 28, 36, 43, 49, 54, 15, 21, 28, 36, 43, 49, 54, 58, 21, 28, 36, 43, 49, 54, 58, 61, 28, 36, 43, 49, 54, 58, 61, 63};
inline pas::Array<std::uint8_t, 0, 63> verticalShift = {57, 49, 41, 33, 25, 17, 9, 1, 57, 49, 41, 33, 25, 17, 9, 1, 57, 49, 41, 33, 25, 17, 9, 1, 57, 49, 41, 33, 25, 17, 9, 1, 57, 49, 41, 33, 25, 17, 9, 1, 57, 49, 41, 33, 25, 17, 9, 1, 57, 49, 41, 33, 25, 17, 9, 1, 57, 49, 41, 33, 25, 17, 9, 1};
extern pas::Array<std::uint64_t, 0, 63> squareMask;
extern pas::Array<std::uint64_t, 0, 63> squareMaskInverted;
// squareMask[square] and squareMaskInverted[square] computed with a shift. Unlike table loads,
// these stay in registers across the position stores in move making; checked
// builds keep the tables' range check.
inline std::uint64_t squareBit(std::int64_t square) {
#ifdef PETIR_CHECK_BOUNDS
    (void)squareMask[square];
#endif
    return UINT64_C(1) << square;
}
inline std::uint64_t squareBitInverted(std::int64_t square) {
#ifdef PETIR_CHECK_BOUNDS
    (void)squareMaskInverted[square];
#endif
    return ~(UINT64_C(1) << square);
}
// Square of a king bitboard. A king board holds exactly one bit, so firstBit
// and lastBit agree; one forward scan replaces lastBit's two scans.
inline std::int32_t kingSquare(std::uint64_t kingBoard) {
#ifdef PETIR_CHECK_BOUNDS
    if (!kingBoard || (kingBoard & (kingBoard - 1))) throw std::out_of_range("King bitboard does not hold exactly one king");
#endif
#if defined(_MSC_VER) && defined(_M_X64)
    unsigned long square;
    _BitScanForward64(&square, kingBoard);
    return static_cast<std::int32_t>(square);
#else
    return std::countr_zero(kingBoard);
#endif
}
extern pas::Array<std::uint64_t, 0, 63> rotated90SquareMask;
extern pas::Array<std::uint64_t, 0, 63> rotatedA8H1SquareMask;
extern pas::Array<std::uint64_t, 0, 63> rotatedH8A1SquareMask;
extern pas::Array<std::uint64_t, 0, 63> whitePassedPawnMask;
extern pas::Array<std::uint64_t, 0, 63> blackPassedPawnMask;
extern pas::Array<std::uint64_t, 0, 63> knightAttacks;
extern pas::Array<std::uint64_t, 0, 63> kingAttacks;
extern std::uint64_t notFileAMask;
extern std::uint64_t notFileHMask;
extern pas::Array<pas::Array<std::uint64_t, 0, 63>, 0, 63> horizontalAttacks;
extern pas::Array<pas::Array<std::uint64_t, 0, 63>, 0, 63> verticalAttacks;
extern pas::Array<pas::Array<std::uint64_t, 0, 255>, 0, 63> diagonalA8H1Attacks;
extern pas::Array<pas::Array<std::uint64_t, 0, 255>, 0, 63> diagonalH8A1Attacks;
extern pas::Array<std::uint8_t, 0, 65535> firstBitTable;
extern pas::Array<std::uint64_t, 0, 63> whiteOutpostMask;
extern pas::Array<std::uint64_t, 0, 63> blackOutpostMask;
extern pas::Array<std::uint64_t, 0, 63> whitePawnAttacks;
extern pas::Array<std::uint64_t, 0, 63> blackPawnAttacks;
extern pas::Array<std::uint64_t, 0, 7> fileMask;
extern pas::Array<std::uint64_t, 0, 7> rankMask;
extern pas::Array<pas::Array<std::uint64_t, 0, 63>, 0, 63> betweenMask;
extern pas::Array<pas::Array<std::int8_t, 0, 63>, 0, 63> rayDirection;
extern pas::Array<pas::Array<std::int8_t, 0, 63>, 0, 63> rayDirectionCode;
extern pas::Array<std::uint64_t, 0, 63> rookRayUpMask;
extern pas::Array<std::uint64_t, 0, 63> rookRayDownMask;
extern pas::Array<std::uint64_t, 0, 63> rookRayLeftMask;
extern pas::Array<std::uint64_t, 0, 63> rookRayRightMask;
extern pas::Array<std::uint64_t, 0, 63> bishopRayPlus7Mask;
extern pas::Array<std::uint64_t, 0, 63> bishopRayPlus9Mask;
extern pas::Array<std::uint64_t, 0, 63> bishopRayMinus7Mask;
extern pas::Array<std::uint64_t, 0, 63> bishopRayMinus9Mask;
extern pas::Array<std::uint64_t, 0, 63> bishopRaysUpMask;
extern pas::Array<std::uint64_t, 0, 63> bishopRaysDownMask;
}
namespace petir_bitboard {
inline pas::Array<std::uint8_t, 0, 63> rotated90Index = {56, 48, 40, 32, 24, 16, 8, 0, 57, 49, 41, 33, 25, 17, 9, 1, 58, 50, 42, 34, 26, 18, 10, 2, 59, 51, 43, 35, 27, 19, 11, 3, 60, 52, 44, 36, 28, 20, 12, 4, 61, 53, 45, 37, 29, 21, 13, 5, 62, 54, 46, 38, 30, 22, 14, 6, 63, 55, 47, 39, 31, 23, 15, 7};
inline pas::Array<std::uint8_t, 0, 63> rotatedA8H1Index = {0, 1, 3, 6, 10, 15, 21, 28, 2, 4, 7, 11, 16, 22, 29, 36, 5, 8, 12, 17, 23, 30, 37, 43, 9, 13, 18, 24, 31, 38, 44, 49, 14, 19, 25, 32, 39, 45, 50, 54, 20, 26, 33, 40, 46, 51, 55, 58, 27, 34, 41, 47, 52, 56, 59, 61, 35, 42, 48, 53, 57, 60, 62, 63};
inline pas::Array<std::uint8_t, 0, 63> rotatedH8A1Index = {35, 27, 20, 14, 9, 5, 2, 0, 42, 34, 26, 19, 13, 8, 4, 1, 48, 41, 33, 25, 18, 12, 7, 3, 53, 47, 40, 32, 24, 17, 11, 6, 57, 52, 46, 39, 31, 23, 16, 10, 60, 56, 51, 45, 38, 30, 22, 15, 62, 59, 55, 50, 44, 37, 29, 21, 63, 61, 58, 54, 49, 43, 36, 28};
inline pas::Array<std::uint8_t, 0, 63> diagonalA8H1Length = {1, 2, 3, 4, 5, 6, 7, 8, 2, 3, 4, 5, 6, 7, 8, 7, 3, 4, 5, 6, 7, 8, 7, 6, 4, 5, 6, 7, 8, 7, 6, 5, 5, 6, 7, 8, 7, 6, 5, 4, 6, 7, 8, 7, 6, 5, 4, 3, 7, 8, 7, 6, 5, 4, 3, 2, 8, 7, 6, 5, 4, 3, 2, 1};
inline pas::Array<std::uint8_t, 0, 63> diagonalH8A1Length = {8, 7, 6, 5, 4, 3, 2, 1, 7, 8, 7, 6, 5, 4, 3, 2, 6, 7, 8, 7, 6, 5, 4, 3, 5, 6, 7, 8, 7, 6, 5, 4, 4, 5, 6, 7, 8, 7, 6, 5, 3, 4, 5, 6, 7, 8, 7, 6, 2, 3, 4, 5, 6, 7, 8, 7, 1, 2, 3, 4, 5, 6, 7, 8};
inline pas::Array<std::uint8_t, 0, 63> diagonalA8H1Position = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 2, 2, 2, 2, 2, 2, 1, 0, 3, 3, 3, 3, 3, 2, 1, 0, 4, 4, 4, 4, 3, 2, 1, 0, 5, 5, 5, 4, 3, 2, 1, 0, 6, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0};
inline pas::Array<std::uint8_t, 0, 63> diagonalH8A1Position = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 2, 2, 2, 2, 2, 2, 0, 1, 2, 3, 3, 3, 3, 3, 0, 1, 2, 3, 4, 4, 4, 4, 0, 1, 2, 3, 4, 5, 5, 5, 0, 1, 2, 3, 4, 5, 6, 6, 0, 1, 2, 3, 4, 5, 6, 7};
inline pas::Array<std::int32_t, 1, 8> knightFileDelta = {1, 2, 2, 1, pas::negate(1), pas::negate(2), pas::negate(2), pas::negate(1)};
inline pas::Array<std::int32_t, 1, 8> knightRankDelta = {2, 1, pas::negate(1), pas::negate(2), pas::negate(2), pas::negate(1), 1, 2};
inline pas::Array<std::int32_t, 1, 8> kingFileDelta = {0, 1, 1, 1, 0, pas::negate(1), pas::negate(1), pas::negate(1)};
inline pas::Array<std::int32_t, 1, 8> kingRankDelta = {pas::negate(1), pas::negate(1), 0, 1, 1, 1, 0, pas::negate(1)};
}
namespace petir_hashing_header {
inline constexpr auto usedFlag = 64;
inline constexpr auto oldFlag = 128;
inline constexpr std::uint8_t generationMask = 63;
extern std::uint8_t generation;
#pragma pack(push, 1)
struct thashrecord {
    std::uint8_t flags;
    std::uint8_t boundType;
    std::uint64_t key;
    std::uint8_t depth;
    std::uint16_t move;
    std::int16_t score;
    std::uint8_t fiftyMoveCounter; // Uses the former unused byte; entry stays 16 bytes.
};
#pragma pack(pop)
inline bool isOld(const thashrecord& entry) {return (entry.flags&generationMask)!=generation;}
// From the search just before the current one.
inline bool isPreviousGeneration(const thashrecord& entry) {return ((entry.flags+1)&generationMask)==generation;}
// Cache-line aligned, so a two-entry bucket never straddles two lines.
template<class T> struct line_allocator {
    using value_type = T;
    line_allocator() = default;
    template<class U> line_allocator(const line_allocator<U>&) {}
    T* allocate(std::size_t count) {return static_cast<T*>(::operator new(count*sizeof(T), std::align_val_t{64}));}
    void deallocate(T* pointer, std::size_t) {::operator delete(pointer, std::align_val_t{64});}
    template<class U> bool operator==(const line_allocator<U>&) const {return true;}
};
using thashdata = std::vector<petir_hashing_header::thashrecord, line_allocator<petir_hashing_header::thashrecord>>;
extern petir_hashing_header::thashdata hashTable;
// The two-entry bucket a key belongs to (hashTableSize is a power of two).
inline thashrecord* bucket(std::uint64_t key) {return hashTable.data() + (key & pas::sub(petir_header::hashTableSize, 2));}
extern std::int32_t discardedRandom;
}
namespace petir_udata {
}
namespace petir_evalmask {
inline pas::Array<std::uint8_t, 0, 63> squareWing = {0, 0, 0, 2, 2, 1, 1, 1, 0, 0, 0, 2, 2, 1, 1, 1, 0, 0, 0, 2, 2, 1, 1, 1, 0, 0, 0, 2, 2, 1, 1, 1, 0, 0, 0, 2, 2, 1, 1, 1, 0, 0, 0, 2, 2, 1, 1, 1, 0, 0, 0, 2, 2, 1, 1, 1, 0, 0, 0, 2, 2, 1, 1, 1};
inline pas::Array<std::uint8_t, 0, 8> badBishopPenalty = {8, 14, 23, 32, 42, 50, 52, 54, 49};
extern pas::Array<std::uint64_t, 0, 63> whiteOutpostMask;
extern pas::Array<std::uint64_t, 0, 63> blackOutpostMask;
extern std::uint64_t filesABCDMask;
extern std::uint64_t filesEFGHMask;
extern std::uint64_t filesABMask;
extern std::uint64_t filesGHMask;
extern pas::Array<std::uint64_t, 0, 63> adjacentFilesMask;
extern pas::Array<std::uint8_t, 0, 65535> bitCountTable;
extern pas::Array<pas::Array<bool, 0, 63>, 0, 63> sameOrAdjacentFile;
extern pas::Array<pas::Array<std::uint8_t, 0, 63>, 0, 63> closenessHalfTable;
extern pas::Array<pas::Array<std::uint8_t, 0, 63>, 0, 63> closenessTable;
extern pas::Array<pas::Array<std::uint8_t, 0, 63>, 0, 63> bishopProximityTable;
}
namespace petir_makemove {
}
namespace petir_movgen {
inline constexpr auto pawnOrderValue = 1;
inline constexpr auto bishopOrderValue = 3;
inline constexpr auto knightOrderValue = 3;
inline constexpr auto rookOrderValue = 5;
inline constexpr auto queenOrderValue = 9;
inline constexpr auto kingOrderValue = 99;
inline pas::Array<std::uint8_t, pas::negate(6), 6> pieceOrderValue = {petir_movgen::kingOrderValue, petir_movgen::queenOrderValue, petir_movgen::rookOrderValue, petir_movgen::bishopOrderValue, petir_movgen::knightOrderValue, petir_movgen::pawnOrderValue, 0, petir_movgen::pawnOrderValue, petir_movgen::knightOrderValue, petir_movgen::bishopOrderValue, petir_movgen::rookOrderValue, petir_movgen::queenOrderValue, petir_movgen::kingOrderValue};
}
namespace petir_tools {
using pinteger = std::int32_t*;
}
namespace petir_usee {
inline constexpr auto pawnSeeValue = 1;
inline constexpr auto bishopSeeValue = 3;
inline constexpr auto knightSeeValue = 3;
inline constexpr auto rookSeeValue = 5;
inline constexpr auto queenSeeValue = 9;
inline constexpr auto kingSeeValue = 99;
inline pas::Array<std::uint8_t, pas::negate(6), 6> pieceSeeValue = {petir_usee::kingSeeValue, petir_usee::queenSeeValue, petir_usee::rookSeeValue, petir_usee::bishopSeeValue, petir_usee::knightSeeValue, petir_usee::pawnSeeValue, 0, petir_usee::pawnSeeValue, petir_usee::knightSeeValue, petir_usee::bishopSeeValue, petir_usee::rookSeeValue, petir_usee::queenSeeValue, petir_usee::kingSeeValue};
}
namespace petir_valid {
}
namespace petir_hashing {
}
namespace petir_repetition {
// Lookups compare full keys and search adds/removes entries in stack order, so
// counts do not depend on table size while a free slot remains. 16384 slots per
// side exceed the positions of any legal game plus the search path, and keep
// the table cache-resident (originally 65536 slots, 1.25 MiB in total).
inline constexpr auto repetitionTableSize = 16384;
#pragma pack(push, 1)
struct treprecord {
    bool used;
    std::uint64_t key;
    std::int8_t count;
};
#pragma pack(pop)
using tarrreprecord = pas::Array<pas::Array<petir_repetition::treprecord, 0, pas::sub(petir_repetition::repetitionTableSize, 1)>, 0, 1>;
extern petir_repetition::tarrreprecord repetitionTable;
}
namespace petir_next {
}
// The evaluation's scalar weights are constants in the engine. The Texel tuner
// (tools/texel/tuner) defines PETIR_EVAL_TUNING to make them variables it can
// change between evaluations. The weight tables are variables in both builds.
#ifdef PETIR_EVAL_TUNING
#define PETIR_EVAL_WEIGHT inline
#else
#define PETIR_EVAL_WEIGHT inline constexpr
#endif
namespace petir_ueval {
inline constexpr auto queensideMajority = 1;
inline constexpr auto kingsideMajority = 2;
inline constexpr auto noMajority = 0;
// A tapered weight holds a middlegame and an endgame value in one number,
// middlegame * 65536 + endgame, so that one addition sums both. The
// evaluation sums the tapered terms apart from the others and blends the
// two sums by the game phase. The sums stay well inside 16 bits each.
constexpr std::int32_t taper(int middlegame, int endgame) {
    return middlegame * 65536 + endgame;
}
constexpr int endgamePart(std::int32_t tapered) {
    return static_cast<std::int16_t>(tapered & 0xffff);
}
constexpr int middlegamePart(std::int32_t tapered) {
    return (tapered - endgamePart(tapered)) / 65536;
}
inline pas::Array<std::uint8_t, 0, 63> whiteBishopOutpostBonus = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 38, 45, 0, 0, 0, 0, 0, 0, 31, 43, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
inline pas::Array<std::uint8_t, 0, 63> blackBishopOutpostBonus = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 31, 43, 0, 0, 0, 0, 0, 0, 38, 45, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
inline pas::Array<std::uint8_t, 0, 63> whiteKnightOutpostBonus = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 35, 41, 0, 0, 0, 0, 0, 31, 39, 78, 53, 0, 0, 0, 0, 24, 29, 10, 27, 0, 0, 0, 0, 0, 16, 28, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
inline pas::Array<std::uint8_t, 0, 63> blackKnightOutpostBonus = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 28, 0, 0, 0, 0, 0, 24, 29, 10, 27, 0, 0, 0, 0, 31, 39, 78, 53, 0, 0, 0, 0, 0, 35, 41, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
inline pas::Array<std::int32_t, 0, 63> whiteKnightSquareBonus = {taper(-84, -37), taper(-44, -24), taper(-43, -11), taper(-25, -4), taper(-16, -6), taper(-8, -23), taper(-39, -9), taper(-40, -25), taper(-45, -27), taper(-28, 2), taper(-22, -21), taper(-7, -12), taper(-5, -14), taper(-8, -20), taper(-13, -4), taper(-11, -5), taper(-42, -23), taper(-26, -10), taper(-18, -4), taper(12, 6), taper(15, 5), taper(-11, -16), taper(-4, -10), taper(-17, -2), taper(-19, 10), taper(8, 1), taper(10, 17), taper(4, 18), taper(14, 20), taper(17, 20), taper(32, 2), taper(5, 12), taper(1, 5), taper(-10, 11), taper(4, 22), taper(29, 25), taper(-3, 25), taper(27, 25), taper(-5, 29), taper(32, 6), taper(-21, -1), taper(5, 6), taper(18, 17), taper(25, 22), taper(80, 5), taper(57, 23), taper(33, 7), taper(-7, 18), taper(-21, 2), taper(-14, 5), taper(-1, 6), taper(45, 2), taper(34, 0), taper(41, -19), taper(-1, 6), taper(7, -10), taper(-131, 21), taper(-76, 15), taper(-58, 13), taper(-9, 7), taper(-6, 8), taper(-7, 7), taper(-12, 10), taper(-89, 5)};
inline pas::Array<std::int32_t, 0, 63> blackKnightSquareBonus = {taper(-131, 21), taper(-76, 15), taper(-58, 13), taper(-9, 7), taper(-6, 8), taper(-7, 7), taper(-12, 10), taper(-89, 5), taper(-21, 2), taper(-14, 5), taper(-1, 6), taper(45, 2), taper(34, 0), taper(41, -19), taper(-1, 6), taper(7, -10), taper(-21, -1), taper(5, 6), taper(18, 17), taper(25, 22), taper(80, 5), taper(57, 23), taper(33, 7), taper(-7, 18), taper(1, 5), taper(-10, 11), taper(4, 22), taper(29, 25), taper(-3, 25), taper(27, 25), taper(-5, 29), taper(32, 6), taper(-19, 10), taper(8, 1), taper(10, 17), taper(4, 18), taper(14, 20), taper(17, 20), taper(32, 2), taper(5, 12), taper(-42, -23), taper(-26, -10), taper(-18, -4), taper(12, 6), taper(15, 5), taper(-11, -16), taper(-4, -10), taper(-17, -2), taper(-45, -27), taper(-28, 2), taper(-22, -21), taper(-7, -12), taper(-5, -14), taper(-8, -20), taper(-13, -4), taper(-11, -5), taper(-84, -37), taper(-44, -24), taper(-43, -11), taper(-25, -4), taper(-16, -6), taper(-8, -23), taper(-39, -9), taper(-40, -25)};
inline pas::Array<std::int32_t, 0, 63> whiteBishopSquareBonus = {taper(-12, -9), taper(19, -18), taper(-20, -6), taper(-20, -1), taper(-11, -4), taper(-6, -1), taper(12, -12), taper(-37, -6), taper(-5, 2), taper(-16, -8), taper(17, -15), taper(-13, 1), taper(0, -3), taper(2, -8), taper(-4, -9), taper(6, -25), taper(-24, 6), taper(-2, 8), taper(-2, 12), taper(6, 6), taper(-1, 16), taper(4, 4), taper(9, -6), taper(10, -13), taper(-17, 3), taper(-8, 11), taper(-3, 23), taper(9, 24), taper(18, 16), taper(-10, 17), taper(7, 3), taper(13, -15), taper(-21, 6), taper(-1, 18), taper(3, 22), taper(15, 28), taper(5, 28), taper(31, 15), taper(-2, 20), taper(-7, 8), taper(-12, 9), taper(12, 17), taper(12, 17), taper(21, 15), taper(47, 13), taper(80, 14), taper(52, 9), taper(-3, 22), taper(-25, 16), taper(-5, 13), taper(-3, 10), taper(6, 9), taper(-9, 18), taper(-14, 16), taper(-26, 15), taper(4, 0), taper(-40, 22), taper(-29, 18), taper(-24, 10), taper(-20, 12), taper(-24, 12), taper(-28, 8), taper(18, -1), taper(-54, 10)};
inline pas::Array<std::int32_t, 0, 63> blackBishopSquareBonus = {taper(-40, 22), taper(-29, 18), taper(-24, 10), taper(-20, 12), taper(-24, 12), taper(-28, 8), taper(18, -1), taper(-54, 10), taper(-25, 16), taper(-5, 13), taper(-3, 10), taper(6, 9), taper(-9, 18), taper(-14, 16), taper(-26, 15), taper(4, 0), taper(-12, 9), taper(12, 17), taper(12, 17), taper(21, 15), taper(47, 13), taper(80, 14), taper(52, 9), taper(-3, 22), taper(-21, 6), taper(-1, 18), taper(3, 22), taper(15, 28), taper(5, 28), taper(31, 15), taper(-2, 20), taper(-7, 8), taper(-17, 3), taper(-8, 11), taper(-3, 23), taper(9, 24), taper(18, 16), taper(-10, 17), taper(7, 3), taper(13, -15), taper(-24, 6), taper(-2, 8), taper(-2, 12), taper(6, 6), taper(-1, 16), taper(4, 4), taper(9, -6), taper(10, -13), taper(-5, 2), taper(-16, -8), taper(17, -15), taper(-13, 1), taper(0, -3), taper(2, -8), taper(-4, -9), taper(6, -25), taper(-12, -9), taper(19, -18), taper(-20, -6), taper(-20, -1), taper(-11, -4), taper(-6, -1), taper(12, -12), taper(-37, -6)};
inline pas::Array<std::int32_t, 0, 63> rookSquareBonus = {taper(-118, 90), taper(-108, 89), taper(-104, 90), taper(-97, 82), taper(-93, 76), taper(-96, 78), taper(-95, 76), taper(-117, 78), taper(-118, 90), taper(-108, 89), taper(-104, 90), taper(-97, 82), taper(-93, 76), taper(-96, 78), taper(-95, 76), taper(-117, 78), taper(-118, 90), taper(-108, 89), taper(-104, 90), taper(-97, 82), taper(-93, 76), taper(-96, 78), taper(-95, 76), taper(-117, 78), taper(-118, 90), taper(-108, 89), taper(-104, 90), taper(-97, 82), taper(-93, 76), taper(-96, 78), taper(-95, 76), taper(-117, 78), taper(-118, 90), taper(-108, 89), taper(-104, 90), taper(-97, 82), taper(-93, 76), taper(-96, 78), taper(-95, 76), taper(-117, 78), taper(-118, 90), taper(-108, 89), taper(-104, 90), taper(-97, 82), taper(-93, 76), taper(-96, 78), taper(-95, 76), taper(-117, 78), taper(-118, 90), taper(-108, 89), taper(-104, 90), taper(-97, 82), taper(-93, 76), taper(-96, 78), taper(-95, 76), taper(-117, 78), taper(-118, 90), taper(-108, 89), taper(-104, 90), taper(-97, 82), taper(-93, 76), taper(-96, 78), taper(-95, 76), taper(-117, 78)};
inline pas::Array<std::int16_t, 0, 63> whiteKingSquarePenalty = {45, 29, 63, 131, 106, 53, 13, 20, 11, 20, 53, 78, 74, 42, pas::negate(3), 14, 50, 32, 66, 92, 84, 68, 37, 60, 98, 69, 59, 84, 90, 86, 65, 124, 71, 59, 92, 99, 118, 87, 50, 67, 88, 55, 73, 114, 109, 41, 64, 80, 25, 55, 72, 97, 109, 64, 110, 41, 86, 92, 98, 62, 40, 38, 101, 116};
inline pas::Array<std::int16_t, 0, 63> blackKingSquarePenalty = {86, 92, 98, 62, 40, 38, 101, 116, 25, 55, 72, 97, 109, 64, 110, 41, 88, 55, 73, 114, 109, 41, 64, 80, 71, 59, 92, 99, 118, 87, 50, 67, 98, 69, 59, 84, 90, 86, 65, 124, 50, 32, 66, 92, 84, 68, 37, 60, 11, 20, 53, 78, 74, 42, pas::negate(3), 14, 45, 29, 63, 131, 106, 53, 13, 20};
inline pas::Array<std::int32_t, 0, 63> whitePawnSquareBonus = {taper(0, 0), taper(0, 0), taper(0, 0), taper(0, 0), taper(0, 0), taper(0, 0), taper(0, 0), taper(0, 0), taper(-37, 22), taper(-45, 30), taper(-44, 31), taper(-55, 31), taper(-42, 42), taper(-20, 29), taper(-31, 28), taper(-42, 11), taper(-38, 21), taper(-35, 25), taper(-43, 23), taper(-44, 27), taper(-32, 27), taper(-22, 22), taper(-22, 21), taper(-33, 12), taper(-30, 29), taper(-26, 32), taper(-19, 18), taper(-16, 11), taper(-13, 14), taper(-10, 19), taper(-10, 24), taper(-17, 12), taper(-16, 38), taper(-12, 32), taper(-5, 24), taper(3, 5), taper(19, 9), taper(21, 13), taper(-4, 32), taper(-10, 24), taper(25, 58), taper(33, 59), taper(54, 41), taper(61, 26), taper(86, 28), taper(81, 53), taper(62, 58), taper(28, 56), taper(102, 45), taper(98, 52), taper(142, 42), taper(126, 43), taper(102, 47), taper(53, 58), taper(-13, 75), taper(-4, 67), taper(0, 0), taper(0, 0), taper(0, 0), taper(0, 0), taper(0, 0), taper(0, 0), taper(0, 0), taper(0, 0)};
inline pas::Array<std::int32_t, 0, 63> blackPawnSquareBonus = {taper(0, 0), taper(0, 0), taper(0, 0), taper(0, 0), taper(0, 0), taper(0, 0), taper(0, 0), taper(0, 0), taper(102, 45), taper(98, 52), taper(142, 42), taper(126, 43), taper(102, 47), taper(53, 58), taper(-13, 75), taper(-4, 67), taper(25, 58), taper(33, 59), taper(54, 41), taper(61, 26), taper(86, 28), taper(81, 53), taper(62, 58), taper(28, 56), taper(-16, 38), taper(-12, 32), taper(-5, 24), taper(3, 5), taper(19, 9), taper(21, 13), taper(-4, 32), taper(-10, 24), taper(-30, 29), taper(-26, 32), taper(-19, 18), taper(-16, 11), taper(-13, 14), taper(-10, 19), taper(-10, 24), taper(-17, 12), taper(-38, 21), taper(-35, 25), taper(-43, 23), taper(-44, 27), taper(-32, 27), taper(-22, 22), taper(-22, 21), taper(-33, 12), taper(-37, 22), taper(-45, 30), taper(-44, 31), taper(-55, 31), taper(-42, 42), taper(-20, 29), taper(-31, 28), taper(-42, 11), taper(0, 0), taper(0, 0), taper(0, 0), taper(0, 0), taper(0, 0), taper(0, 0), taper(0, 0), taper(0, 0)};
inline pas::Array<std::int16_t, 0, 63> whiteKingEndgameKingsideBonus = {pas::negate(103), pas::negate(84), pas::negate(56), pas::negate(1), 8, 22, 24, 26, pas::negate(121), pas::negate(87), pas::negate(38), pas::negate(10), 8, 16, 13, 17, pas::negate(98), pas::negate(64), pas::negate(26), 1, 14, 21, 12, 14, pas::negate(68), pas::negate(46), pas::negate(17), 9, 18, 25, 14, 16, pas::negate(75), pas::negate(53), pas::negate(9), 19, 31, 41, 32, 6, pas::negate(76), pas::negate(45), pas::negate(19), 18, 36, 51, 65, 31, pas::negate(124), pas::negate(69), pas::negate(20), 8, 36, 55, 73, 12, pas::negate(78), pas::negate(75), pas::negate(61), pas::negate(18), 2, 28, 60, pas::negate(52)};
inline pas::Array<std::int16_t, 0, 63> whiteKingEndgameQueensideBonus = {26, 24, 22, 8, pas::negate(1), pas::negate(56), pas::negate(84), pas::negate(103), 17, 13, 16, 8, pas::negate(10), pas::negate(38), pas::negate(87), pas::negate(121), 14, 12, 21, 14, 1, pas::negate(26), pas::negate(64), pas::negate(98), 16, 14, 25, 18, 9, pas::negate(17), pas::negate(46), pas::negate(68), 6, 32, 41, 31, 19, pas::negate(9), pas::negate(53), pas::negate(75), 31, 65, 51, 36, 18, pas::negate(19), pas::negate(45), pas::negate(76), 12, 73, 55, 36, 8, pas::negate(20), pas::negate(69), pas::negate(124), pas::negate(52), 60, 28, 2, pas::negate(18), pas::negate(61), pas::negate(75), pas::negate(78)};
inline pas::Array<std::int16_t, 0, 63> blackKingEndgameQueensideBonus = {pas::negate(52), 60, 28, 2, pas::negate(18), pas::negate(61), pas::negate(75), pas::negate(78), 12, 73, 55, 36, 8, pas::negate(20), pas::negate(69), pas::negate(124), 31, 65, 51, 36, 18, pas::negate(19), pas::negate(45), pas::negate(76), 6, 32, 41, 31, 19, pas::negate(9), pas::negate(53), pas::negate(75), 16, 14, 25, 18, 9, pas::negate(17), pas::negate(46), pas::negate(68), 14, 12, 21, 14, 1, pas::negate(26), pas::negate(64), pas::negate(98), 17, 13, 16, 8, pas::negate(10), pas::negate(38), pas::negate(87), pas::negate(121), 26, 24, 22, 8, pas::negate(1), pas::negate(56), pas::negate(84), pas::negate(103)};
inline pas::Array<std::int16_t, 0, 63> blackKingEndgameKingsideBonus = {pas::negate(78), pas::negate(75), pas::negate(61), pas::negate(18), 2, 28, 60, pas::negate(52), pas::negate(124), pas::negate(69), pas::negate(20), 8, 36, 55, 73, 12, pas::negate(76), pas::negate(45), pas::negate(19), 18, 36, 51, 65, 31, pas::negate(75), pas::negate(53), pas::negate(9), 19, 31, 41, 32, 6, pas::negate(68), pas::negate(46), pas::negate(17), 9, 18, 25, 14, 16, pas::negate(98), pas::negate(64), pas::negate(26), 1, 14, 21, 12, 14, pas::negate(121), pas::negate(87), pas::negate(38), pas::negate(10), 8, 16, 13, 17, pas::negate(103), pas::negate(84), pas::negate(56), pas::negate(1), 8, 22, 24, 26};
inline pas::Array<std::int16_t, 0, 63> whiteKingEndgameBothWingsBonus = {pas::negate(64), pas::negate(36), pas::negate(19), pas::negate(11), pas::negate(30), pas::negate(40), pas::negate(57), pas::negate(94), pas::negate(40), pas::negate(29), pas::negate(14), pas::negate(8), pas::negate(9), pas::negate(23), pas::negate(47), pas::negate(57), pas::negate(17), pas::negate(5), 8, 15, 14, 0, pas::negate(19), pas::negate(34), 21, 27, 33, 38, 36, 29, 13, 8, 23, 49, 64, 65, 65, 59, 46, 19, 38, 58, 75, 85, 84, 74, 66, 41, pas::negate(3), 50, 68, 73, 76, 61, 66, 9, pas::negate(61), 31, 44, 36, 28, 27, 24, pas::negate(35)};
inline pas::Array<std::int16_t, 0, 63> blackKingEndgameBothWingsBonus = {pas::negate(61), 31, 44, 36, 28, 27, 24, pas::negate(35), pas::negate(3), 50, 68, 73, 76, 61, 66, 9, 38, 58, 75, 85, 84, 74, 66, 41, 23, 49, 64, 65, 65, 59, 46, 19, 21, 27, 33, 38, 36, 29, 13, 8, pas::negate(17), pas::negate(5), 8, 15, 14, 0, pas::negate(19), pas::negate(34), pas::negate(40), pas::negate(29), pas::negate(14), pas::negate(8), pas::negate(9), pas::negate(23), pas::negate(47), pas::negate(57), pas::negate(64), pas::negate(36), pas::negate(19), pas::negate(11), pas::negate(30), pas::negate(40), pas::negate(57), pas::negate(94)};
inline pas::Array<std::int32_t, 0, 63> whiteQueenSquareBonus = {taper(6, -51), taper(7, -61), taper(15, -66), taper(16, -55), taper(23, -87), taper(3, -93), taper(18, -128), taper(2, -53), taper(0, -45), taper(11, -49), taper(14, -60), taper(23, -66), taper(20, -63), taper(27, -103), taper(35, -134), taper(23, -87), taper(-9, -23), taper(0, -24), taper(10, -29), taper(3, -30), taper(8, -31), taper(11, -34), taper(16, -40), taper(18, -51), taper(-9, -9), taper(-12, 9), taper(-4, 0), taper(-5, 15), taper(-10, 13), taper(-6, 8), taper(5, 0), taper(5, 8), taper(-15, 18), taper(-19, 33), taper(-19, 39), taper(-13, 37), taper(2, 42), taper(-6, 51), taper(3, 50), taper(-1, 27), taper(-12, 10), taper(-21, 38), taper(-30, 58), taper(-17, 61), taper(11, 59), taper(21, 57), taper(30, 42), taper(1, 53), taper(-31, 33), taper(-57, 61), taper(-34, 66), taper(-37, 72), taper(-45, 82), taper(-26, 75), taper(-14, 65), taper(47, 25), taper(-16, 23), taper(10, 22), taper(2, 32), taper(-14, 47), taper(-19, 64), taper(34, 31), taper(72, 1), taper(42, 19)};
inline pas::Array<std::int32_t, 0, 63> blackQueenSquareBonus = {taper(-16, 23), taper(10, 22), taper(2, 32), taper(-14, 47), taper(-19, 64), taper(34, 31), taper(72, 1), taper(42, 19), taper(-31, 33), taper(-57, 61), taper(-34, 66), taper(-37, 72), taper(-45, 82), taper(-26, 75), taper(-14, 65), taper(47, 25), taper(-12, 10), taper(-21, 38), taper(-30, 58), taper(-17, 61), taper(11, 59), taper(21, 57), taper(30, 42), taper(1, 53), taper(-15, 18), taper(-19, 33), taper(-19, 39), taper(-13, 37), taper(2, 42), taper(-6, 51), taper(3, 50), taper(-1, 27), taper(-9, -9), taper(-12, 9), taper(-4, 0), taper(-5, 15), taper(-10, 13), taper(-6, 8), taper(5, 0), taper(5, 8), taper(-9, -23), taper(0, -24), taper(10, -29), taper(3, -30), taper(8, -31), taper(11, -34), taper(16, -40), taper(18, -51), taper(0, -45), taper(11, -49), taper(14, -60), taper(23, -66), taper(20, -63), taper(27, -103), taper(35, -134), taper(23, -87), taper(6, -51), taper(7, -61), taper(15, -66), taper(16, -55), taper(23, -87), taper(3, -93), taper(18, -128), taper(2, -53)};
inline pas::Array<std::uint8_t, 0, 64> squareColor = {0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 2};
PETIR_EVAL_WEIGHT auto twoBishopsKingAttackBonus = 1;
PETIR_EVAL_WEIGHT auto lostCastlingPenalty = 43;
PETIR_EVAL_WEIGHT auto castledBonus = 10;
PETIR_EVAL_WEIGHT auto castledWrongSideBonus = 10;
PETIR_EVAL_WEIGHT auto trappedBishopPenalty = 92;
PETIR_EVAL_WEIGHT auto trappedBishopPenalty2 = 56;
PETIR_EVAL_WEIGHT auto trappedKnightPenalty = 122;
PETIR_EVAL_WEIGHT auto undevelopedCenterPawnPenalty = 10;
PETIR_EVAL_WEIGHT auto blockedCenterPawnPenalty = 66;
PETIR_EVAL_WEIGHT auto rookSemiOpenFileNearKingBonus = 8;
PETIR_EVAL_WEIGHT auto doubledRooksSemiOpenNearKingBonus = -5;
PETIR_EVAL_WEIGHT auto rookOpenFileNearKingBonus = 13;
PETIR_EVAL_WEIGHT auto doubledRooksBonus = 35;
PETIR_EVAL_WEIGHT auto doubledRooksNearKingBonus = 17;
PETIR_EVAL_WEIGHT auto rookQueenOpenFileBonus = 24;
PETIR_EVAL_WEIGHT auto rookQueenSemiOpenFileBonus = 21;
PETIR_EVAL_WEIGHT auto rookQueenSemiOpenNearKingBonus = 2;
PETIR_EVAL_WEIGHT auto doubledRooksOpenFileBonus = -14;
inline pas::Array<std::int16_t, 0, 6> doubledPawnPenalty = {0, 10, 23, 45, 65, 90, 120};
PETIR_EVAL_WEIGHT auto backwardPawnPenalty = 3;
inline pas::Array<std::uint8_t, 0, 7> isolatedPawnPenalty = {5, 6, 8, 10, 9, 7, 6, 6};
PETIR_EVAL_WEIGHT auto missingPawnPenalty = 45;
inline pas::Array<std::uint8_t, 0, 7> whitePotentialPasserBonus = {0, 0, 3, 4, 5, 6, 6, 6};
inline pas::Array<std::uint8_t, 0, 7> blackPotentialPasserBonus = {6, 6, 6, 5, 4, 3, 0, 0};
inline pas::Array<std::int16_t, 0, 8> isolatedPawnCountPenalty = {0, pas::negate(7), pas::negate(10), pas::negate(20), pas::negate(30), -42, -46, -48, -48};
PETIR_EVAL_WEIGHT auto rookOnSeventhBonus = 27;
PETIR_EVAL_WEIGHT auto rookOnEighthBonus = 79;
PETIR_EVAL_WEIGHT auto safeAdvanceBonus = 48;
PETIR_EVAL_WEIGHT auto doubledRooksSeventhBonus = 27;
PETIR_EVAL_WEIGHT auto rookQueenSeventhBonus = 24;
PETIR_EVAL_WEIGHT auto blockedRookPenalty = -2;
PETIR_EVAL_WEIGHT auto passedPawnBonus = 4;
PETIR_EVAL_WEIGHT auto mobilityBonus = 2;
PETIR_EVAL_WEIGHT auto queenTropismBonus = 4;
PETIR_EVAL_WEIGHT auto bishopTropismBonus = 3;
PETIR_EVAL_WEIGHT auto knightTropismBonus = 3;
PETIR_EVAL_WEIGHT auto blockedPawnPenalty = -23;
PETIR_EVAL_WEIGHT auto connectedPassersBonus = 6;
PETIR_EVAL_WEIGHT auto connectedPassersAdvancedBonus = 61;
PETIR_EVAL_WEIGHT auto connectedPassersFarAdvancedBonus = 110;
inline pas::Array<std::uint8_t, 0, 8> bishopPairBonus = {0, 7, 35, 42, 41, 34, 23, 0, 0};
PETIR_EVAL_WEIGHT auto noPawnPenalty = 32;
// Material imbalances, a bonus for the side named first (materialImbalanceScore
// in ueval.cpp); the piece types not named are equal on both sides:
//  - minorForPawnsBonus / rookForPawnsBonus: a minor piece / a rook against
//    1, 2 or 3+ pawns (the index);
//  - exchangeBonus: a rook against a minor piece, indexed by the minor side's
//    extra pawns 0, 1 or 2+ (0 also when the rook side has more pawns);
//  - two minors against a rook, a queen against two rooks, a queen against a
//    rook and a minor piece: whatever the pawns.
inline pas::Array<std::int16_t, 1, 3> minorForPawnsBonus = {0, 52, 66};
inline pas::Array<std::int16_t, 1, 3> rookForPawnsBonus = {0, pas::negate(21), 75};
inline pas::Array<std::int16_t, 0, 2> exchangeBonus = {31, 28, 23};
PETIR_EVAL_WEIGHT auto minorsForRookBonus = 36;
PETIR_EVAL_WEIGHT auto queenForRooksBonus = 20;
PETIR_EVAL_WEIGHT auto queenForRookMinorBonus = 23;
PETIR_EVAL_WEIGHT auto missingFianchettoPenalty = 18;
PETIR_EVAL_WEIGHT auto blockedKingPenalty = 8;
PETIR_EVAL_WEIGHT auto safePromotionBonus = 225;
PETIR_EVAL_WEIGHT auto safePromotionBonus1 = 125;
PETIR_EVAL_WEIGHT auto weakStructurePenalty = 6;
PETIR_EVAL_WEIGHT auto bishopOverKnightEndgameBonus = 10;
inline pas::Array<std::int8_t, 0, 8> knightLowMobilityBonus = {pas::negate(25), pas::negate(5), 0, 0, 0, 0, 0, 0, 0};
inline pas::Array<std::int32_t, 0, 8> knightMobilityBonus = {taper(-77, -96), taper(-55, -28), taper(-42, 8), taper(-32, 24), taper(-25, 35), taper(-18, 47), taper(-10, 44), taper(-4, 41), taper(6, 25)};
inline pas::Array<std::int32_t, 0, 15> bishopMobilityBonus = {taper(-58, -43), taper(-46, -12), taper(-35, 12), taper(-28, 29), taper(-19, 40), taper(-13, 49), taper(-9, 54), taper(-5, 56), taper(-3, 57), taper(6, 53), taper(15, 48), taper(39, 37), taper(52, 41), taper(73, 22), taper(37, 37), taper(38, 38)};
inline pas::Array<std::int32_t, 0, 15> rookMobilityBonus = {taper(-64, -5), taper(-57, 22), taper(-52, 23), taper(-48, 27), taper(-49, 40), taper(-45, 46), taper(-42, 54), taper(-38, 62), taper(-35, 75), taper(-33, 87), taper(-32, 95), taper(-30, 99), taper(-31, 103), taper(-17, 106), taper(-13, 103), taper(24, 24)};
inline pas::Array<std::int32_t, 0, 31> queenMobilityBonus = {taper(-86, -93), taper(-75, -138), taper(-82, -125), taper(-85, -75), taper(-81, -85), taper(-80, -66), taper(-79, -53), taper(-79, -36), taper(-79, -26), taper(-77, -19), taper(-77, -9), taper(-77, 4), taper(-77, 11), taper(-76, 18), taper(-75, 25), taper(-71, 27), taper(-73, 34), taper(-74, 39), taper(-71, 38), taper(-66, 36), taper(-70, 44), taper(-59, 35), taper(-43, 23), taper(-43, 23), taper(-43, 23), taper(-43, 23), taper(-43, 23), taper(-43, 23), taper(-43, 23), taper(-43, 23), taper(-43, 23), taper(-43, 23)};
PETIR_EVAL_WEIGHT auto assuredPromotionBonus = 44;
PETIR_EVAL_WEIGHT auto potentialPasserBonus = 70;
PETIR_EVAL_WEIGHT auto rookBlockedByKingPenalty = 42;
inline pas::Array<std::uint8_t, 0, 7> semiOpenFileBonus = {5, 2, 6, 3, 0, 4, 13, 16};
inline pas::Array<std::uint8_t, 0, 7> openFileBonus = {18, 20, 14, 20, 29, 16, 30, 46};
inline pas::Array<std::uint8_t, 1, 7> whitePassedPawnRankBonus = {0, 0, 17, 47, 76, 122, 0};
inline pas::Array<std::uint8_t, 0, 6> blackPassedPawnRankBonus = {0, 122, 76, 47, 17, 0, 0};
PETIR_EVAL_WEIGHT auto rookBehindPassedPawnBonus = 18;
PETIR_EVAL_WEIGHT auto blockedPassedPawnPenalty = 15;
inline pas::Array<std::uint8_t, 1, 7> whiteKingSupportedPasserBonus = {12, 11, 18, 2, 0, 0, 0};
inline pas::Array<std::uint8_t, 0, 6> blackKingSupportedPasserBonus = {0, 0, 0, 2, 18, 11, 12};
PETIR_EVAL_WEIGHT auto blockedBishopPenalty2 = 5;
PETIR_EVAL_WEIGHT auto offsideBishopPenalty = 20;
inline pas::Array<std::uint8_t, 0, 8> blockedBishopPenalty = {0, 2, 5, 12, 30, 50, 70, 100, 100};
inline pas::Array<std::uint8_t, 0, 14> pawnShieldPenalty = {0, 20, 14, 36, 32, 40, 46, 60, 82, 103, 135, 100, 120, 180, 200};
inline pas::Array<std::int32_t, 0, 63> kingPressurePenalty = {pas::negate(12), pas::negate(42), 0, 35, 66, 93, 126, 157, 189, 213, 249, 271, 281, 276, 313, 317, 310, 359, 385, 400, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500};
inline pas::Array<std::uint8_t, 0, 41> endgameScale = {100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 90, 80, 70, 60, 50, 40, 30, 20, 20, 10, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
inline pas::Array<std::uint8_t, 0, 41> rookScale = {0, 0, 0, 0, 0, 0, 0, 0, 20, 20, 20, 20, 20, 20, 20, 20, 50, 60, 70, 70, 80, 90, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100};
PETIR_EVAL_WEIGHT auto queenKnightEndgameBonus = 9;
PETIR_EVAL_WEIGHT auto pawnEndgameBonus = 10;
PETIR_EVAL_WEIGHT auto pawnEndgameBonus2 = 5;
// Weights that ueval.cpp used to write as plain numbers.
PETIR_EVAL_WEIGHT auto sideToMoveBonus = 3;
// Per own pawn once either side has less than 16 non-pawn material:
// realEndgamePawnBonus when both have at most 10, else earlyEndgamePawnBonus.
PETIR_EVAL_WEIGHT auto earlyEndgamePawnBonus = -2;
PETIR_EVAL_WEIGHT auto realEndgamePawnBonus = 3;
// Three points more piece material, including a queen, against two or three
// extra enemy pawns.
PETIR_EVAL_WEIGHT auto queenAgainstPawnsBonus = -16;
// Added to safeAdvanceBonus when the passer's side is to move.
PETIR_EVAL_WEIGHT auto safeAdvanceOnMoveBonus = 79;
// Added to the connected-passer bonus when the two passers stand one rank
// apart: the trailing one on its sixth rank or beyond, or on its fifth.
PETIR_EVAL_WEIGHT auto staggeredFarAdvancedPassersBonus = -58;
PETIR_EVAL_WEIGHT auto staggeredAdvancedPassersBonus = -24;
// passedPawnActivity, each times the passer's weight (1, 2 or 3 on its fifth,
// sixth or seventh rank).
PETIR_EVAL_WEIGHT auto passerKingDistanceBonus = 6;
PETIR_EVAL_WEIGHT auto passerStopBlockedPenalty = -8;
PETIR_EVAL_WEIGHT auto passerStopAttackedPenalty = -3;
PETIR_EVAL_WEIGHT auto passerSafeStopBonus = 20;
PETIR_EVAL_WEIGHT auto passerFreePathBonus = 2;
// endgamePieceActivityAgainst.
PETIR_EVAL_WEIGHT auto knightBlockadeBonus = 9;
PETIR_EVAL_WEIGHT auto rookSafeSquareBonus = -3;
PETIR_EVAL_WEIGHT auto rookPawnTargetBonus = 37;
PETIR_EVAL_WEIGHT auto rookTiedDefenderPenalty = 22;
// Candidate passers in pawn endings: the bonus by rank (counted from the
// pawn's side) times candidateFileQuarters / 4, and the three-against-three
// breakthrough.
PETIR_EVAL_WEIGHT int candidateRankBonus[] = {0, 53, 48, 73, 78, 100, 0, 0};
PETIR_EVAL_WEIGHT int candidateFileQuarters[] = {6, 5, 4, 4, 4, 4, 5, 6};
PETIR_EVAL_WEIGHT auto breakthroughBonus = 16;
}
namespace petir_search {
struct anonymous_record_1 {
    std::int32_t phase;
    std::int32_t legalMoveCount;
    bool mateThreatExtended;
    bool checkExtended;
    bool evaluated;
    std::int32_t staticEval;
};
extern pas::Array<std::int32_t, 0, 63> currentMove;
// The piece that made currentMove, for the continuation-history key.
extern pas::Array<std::int32_t, 0, 63> currentPiece;
inline constexpr auto minHistoryTries = pas::mul(50, 6);
inline constexpr auto minHistoryThreshold = 0.10;
inline constexpr auto allNode = pas::negate(1);
inline constexpr auto pvNode = 0;
inline constexpr auto cutNode = 1;
extern pas::Array<pas::Array<std::int32_t, 0, 63>, pas::negate(6), 6> historyTried;
extern pas::Array<pas::Array<std::int32_t, 0, 63>, pas::negate(6), 6> historySucceeded;
extern pas::Array<petir_search::anonymous_record_1, 0, 63> searchStack;
extern std::int32_t researchCount;
inline pas::Array<std::int32_t, 2, 8> futilityMargins = {16, 22, 30, 37, 55, 92, 142};
inline constexpr auto noValue = pas::negate(200000);
}
namespace petir_root {
extern std::int32_t stableBestMove;
extern std::int32_t earlyStopTimeLimit;
extern std::int32_t originalTimeLimit;
extern bool newPvFound;
}
namespace petir_uiterative {
extern std::int32_t completedMove,completedDepth,completedScore;
extern std::string completedPv;
inline pas::Array<std::int32_t, 2, 12> timeReductionPercent = {1, 5, 15, 20, 20, 25, 30, 30, 30, 30, 30};
extern std::int32_t extendedTimeLimit;
}
namespace petir_customboard {
extern std::int32_t fenIndex;
extern std::int32_t scanIndex;
extern std::int32_t fenSquare;
}
namespace petir_notation {
}
namespace petir_pv {
}
namespace petir_opening_book {
struct anonymous_record_1 {
    std::uint16_t move;
    std::int32_t weight;
};
inline constexpr auto bookTableSize = (262144 / 2);
inline constexpr auto minimumElo = 2540;
#pragma pack(push, 1)
struct tbook {
    std::uint64_t key;
    std::uint16_t weight;
};
#pragma pack(pop)
#pragma pack(push, 1)
struct thashbook {
    petir_opening_book::tbook entry;
    bool used;
};
#pragma pack(pop)
using hashbook = pas::Array<pas::Array<petir_opening_book::thashbook, 0, pas::sub(petir_opening_book::bookTableSize, 1)>, 0, 1>;
extern petir_opening_book::hashbook bookTable;
extern std::int32_t entryCount;
extern std::int32_t moveIndex;
}
namespace petir_etc {
}
namespace petir_undoredo {
struct tundomoves {
    petir_header::tdata position;
    std::int32_t whiteClock;
    std::int32_t blackClock;
};
struct tundo {
    std::int32_t count;
    std::int32_t maxCount;
    pas::Array<petir_undoredo::tundomoves, 1, 400> entries;
};
}
namespace petir_header {
}
namespace petir_bitboard_mask {
std::int32_t firstBit(const std::uint16_t* bitboard);
std::int32_t lastBit(const std::uint16_t* bitboard);
std::uint64_t bishopAttacks(const petir_header::tdata& position, std::int32_t square);
std::uint64_t rookAttacks(petir_header::tdata& position, std::int32_t square);
}
namespace petir_bitboard {
void initMasks();
}
namespace petir_udata {
std::uint8_t squareAt(std::uint8_t rank, std::uint8_t file);
void setInitialPosition(petir_header::tdata& position);
}
namespace petir_evalmask {
void initEvalMasks();
std::int32_t chebyshevDistance(std::int32_t firstSquare, std::int32_t secondSquare);
std::int32_t bishopDistance(std::int32_t firstSquare, std::int32_t secondSquare);
std::int32_t closenessScore(std::int32_t firstSquare, std::int32_t secondSquare);
void initDistanceTables();
}
namespace petir_makemove {
void makeWhiteMove(std::int32_t move, petir_header::tdata& position);
void makeBlackMove(std::int32_t move, petir_header::tdata& position);
void checkPositionConsistency(petir_header::tdata position);
void castleWhiteShort(petir_header::tdata& position);
void castleWhiteLong(petir_header::tdata& position);
void castleBlackShort(petir_header::tdata& position);
void castleBlackLong(petir_header::tdata& position);
void makeBlackEnPassant(petir_header::tdata& position, std::int32_t fromSquare, std::int32_t toSquare);
void makeWhiteEnPassant(petir_header::tdata& position, std::int32_t fromSquare, std::int32_t toSquare);
void promoteWhitePawn(petir_header::tdata& position, std::int32_t square, std::int32_t promotion);
void promoteBlackPawn(petir_header::tdata& position, std::int32_t square, std::int32_t promotion);
}
namespace petir_movgen {
void generateBlackQuietMoves(petir_header::tdata& position, petir_header::tmovelist& moveList, std::int32_t& moveCountOut);
void generateBlackCaptures(petir_header::tdata& position, petir_header::tmovelist& moveList, std::int32_t& moveCountOut);
void generateBlackCapturesAndPushes(petir_header::tdata& position, petir_header::tmovelist& moveList, std::int32_t& moveCountOut);
void generateBlackCapturesAndChecks(petir_header::tdata& position, petir_header::tmovelist& moveList, std::int32_t& moveCountOut);
void generateWhiteCapturesAndChecks(petir_header::tdata& position, petir_header::tmovelist& moveList, std::int32_t& moveCountOut);
void generateWhiteCaptures(petir_header::tdata& position, petir_header::tmovelist& moveList, std::int32_t& moveCountOut);
void generateWhiteCapturesAndPushes(petir_header::tdata& position, petir_header::tmovelist& moveList, std::int32_t& moveCountOut);
void generateWhiteQuietMoves(petir_header::tdata& position, petir_header::tmovelist& moveList, std::int32_t& moveCountOut);
void generateWhiteEvasions(petir_header::tdata& position, petir_header::tmovelist& moveList, std::int32_t& moveCountOut, std::int32_t& checkerCount);
void generateBlackEvasions(petir_header::tdata& position, petir_header::tmovelist& moveList, std::int32_t& moveCountOut, std::int32_t& checkerCount);
}
namespace petir_tools {
std::uint64_t pinnedPieces(const petir_header::tdata& position, int side);
bool attackedByBlack(petir_header::tdata& position, std::int32_t square);
bool attackedByWhite(petir_header::tdata& position, std::int32_t square);
bool whiteInCheck(petir_header::tdata& position);
bool blackInCheck(petir_header::tdata& position);
void clearKillers();
bool exposesWhiteKing(petir_header::tdata& position, std::uint16_t move);
bool exposesWhiteKingFrom(petir_header::tdata& position, std::uint16_t fromSquare, std::uint16_t toSquare, std::uint16_t kingSquare);
bool exposesBlackKingFrom(petir_header::tdata& position, std::uint16_t fromSquare, std::uint16_t toSquare, std::uint16_t kingSquare);
bool exposesBlackKing(petir_header::tdata& position, std::uint16_t move);
bool extendPassedPawnPush(petir_header::tdata& position, std::int32_t& newDepth, std::int32_t sideToMove, std::int32_t move, std::int32_t& extensionCredit);
bool isWinningMaterial(std::int32_t materialScore, std::uint8_t sideToMove);
bool isLosingMaterial(std::int32_t materialScore, std::uint8_t sideToMove);
bool opponentInCheck(petir_header::tdata& position, std::int32_t sideToMove);
bool moveGivesCheck(petir_header::tdata& position, std::uint16_t sideToMove, std::uint16_t move);
bool isPawnPush(petir_header::tdata& position, std::uint16_t sideToMove, std::uint16_t move);
bool isCapture(petir_header::tdata& position, std::uint16_t move);
bool whiteForksOrWinsPawn(std::int32_t square, petir_header::tdata& position);
bool blackForksOrWinsPawn(std::int32_t square, petir_header::tdata& position);
std::int32_t capturedValue(std::int32_t toSquare, petir_header::tdata& position);
bool whiteMoveThreatensKing(petir_header::tdata& position, std::uint16_t move, std::int32_t& threatenedValue);
bool blackMoveThreatensKing(petir_header::tdata& position, std::uint16_t move, std::int32_t& threatenedValue);
bool hasPawnOnSeventh(std::int32_t sideToMove, petir_header::tdata& position);
bool whiteHasPawnOnSeventh(petir_header::tdata& position);
bool blackHasPawnOnSeventh(petir_header::tdata& position);
bool hasFreeSixthRankPawn(std::int32_t sideToMove, petir_header::tdata& position);
bool isGoodWhiteMove(petir_header::tdata& position, std::int32_t square);
bool isGoodBlackMove(petir_header::tdata& position, std::int32_t square);
bool moveThreatensKing(std::uint8_t sideToMove, petir_header::tdata& position, std::uint16_t move, std::int32_t& threatenedValue);
std::int32_t nonPawnMaterial(petir_header::tdata& position, std::int32_t side);
pas::String getFen(petir_header::tdata position, std::int32_t sideToMove);
bool isGoodMove(petir_header::tdata& position, std::int32_t square, std::int32_t sideToMove);
bool whiteMoveGivesCheck(petir_header::tdata& position, std::uint16_t move);
std::uint8_t popCount(std::uint64_t bits);
bool blackMoveGivesCheck(petir_header::tdata& position, std::uint16_t move);
bool blackMoveGivesCheckOrSpecial(petir_header::tdata& position, std::uint16_t move, std::int32_t& threatenedValue);
}
namespace petir_usee {
std::int32_t staticExchange(std::int32_t sideToMove, const petir_header::tdata& position, std::int32_t fromSquare, std::int32_t toSquare);
void addXray(std::uint64_t& attackers, std::int32_t square, std::int32_t target, const petir_header::tdata& position);
void initXray();
}
namespace petir_valid {
bool isValidMove(petir_header::tdata& position, std::int32_t move, std::int32_t sideToMove);
}
namespace petir_hashing_header {
void initHashKeys();
std::uint64_t computeHashKey(petir_header::tdata& position, std::int32_t sideToMove);
void clearHashTable();
void advanceHashGeneration();
void allocateHashTable();
void clearReverseTable();
void reverseTable();
void coba();
}
namespace petir_hashing {
void storeHash(std::uint64_t key, std::int32_t sideToMove, std::int32_t depth, std::int32_t move, std::int32_t score, std::int32_t boundType, std::uint8_t fiftyMoveCounter);
bool probeHash(std::uint64_t key, std::int32_t& alpha, std::int32_t& beta, std::int32_t depth, std::int32_t sideToMove, std::int32_t& score, std::int32_t& move, bool& avoidNullMove, std::uint8_t fiftyMoveCounter
#ifdef PETIR_HASH_STATS
    , petir_hash_stats::Probe* diagnostic = nullptr
#endif
);
// Cache hint only: starts loading the entry probeHash will read for this key.
inline void prefetchHash(std::uint64_t key, std::int32_t sideToMove) {
    if (sideToMove == petir_header::blackSide) key = ~key;
    const auto* entry = petir_hashing_header::bucket(key);
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
    _mm_prefetch(reinterpret_cast<const char*>(entry), _MM_HINT_T0);
#elif defined(__GNUC__)
    __builtin_prefetch(entry);
#else
    (void)entry;
#endif
}
}
namespace petir_repetition {
void clearRepetitionTable();
std::int32_t addRepetition(std::uint64_t key, std::int32_t side);
void removeRepetition(std::uint64_t key, std::int32_t side);
bool isRepetition(std::uint64_t key, std::int32_t side);
}
namespace petir_next {
void selectBestMove(petir_header::tmovelist& moveList, std::int32_t currentIndex, std::int32_t lastIndex);
bool nextEvasionMove(std::int32_t sideToMove, petir_header::tdata& position, std::int32_t& move, std::int32_t& phase, std::int32_t& hashMove, std::int32_t& currentIndex, std::int32_t& moveCount, std::int32_t& checkerCount, petir_header::tmovelist& moveList);
bool nextMove(std::int32_t sideToMove, std::int32_t ply, petir_header::tdata& position, std::int32_t& move, std::int32_t& phase, std::int32_t hashMove, std::int32_t& captureIndex, std::int32_t& quietIndex, std::int32_t& captureCount, std::int32_t& quietCount, petir_header::tmovelist& quietList, petir_header::tmovelist& captureList, std::int32_t& bestCaptureGain, petir_header::piece_to_history* const* continuationTables);
bool nextRootMove(std::int32_t sideToMove, std::int32_t& move, std::int32_t& phase, std::int32_t& hashMove, std::int32_t& currentIndex, std::int32_t& moveCount, petir_header::tmovelist moveList);
bool nextQuiescenceMove(std::int32_t sideToMove, petir_header::tdata& position, std::int32_t& move, std::int32_t& phase, std::int32_t& currentIndex, std::int32_t& moveCount, std::int32_t& score, std::int32_t checkPlies, petir_header::tmovelist& moveList);
}
namespace petir_ueval {
std::int32_t evaluate(const petir_header::tdata& position, std::int32_t sideToMove, std::int32_t alpha, std::int32_t beta, bool opponentPerspective = false);
// Pawn-owner perspective; caller supplies a frontmost passed pawn.
int passedPawnActivity(const petir_header::tdata& position, int pawnSquare, bool isWhite);
// Unscaled owner-perspective bonus for neighboring foremost passed pawns.
int connectedPassedPawns(const petir_header::tdata& position, bool isWhite);
// Candidate/breakthrough potential, owner perspective before phase/draw scaling.
int candidatePawnPotential(const petir_header::tdata& position, bool isWhite, bool useCache = true);
// Piece-owner perspective; caller supplies its rook or knight square.
int endgamePieceActivity(const petir_header::tdata& position, int pieceSquare, bool isWhite);
// Soft KRP-KR percentage; pass the actual side to move (also for opponentPerspective).
int rookPawnScale(const petir_header::tdata& position, int sideToMove);
std::int32_t materialForSide(std::int32_t sideToMove, std::int32_t materialScore);
bool isMaterialDraw(const petir_header::tdata& position, bool& likelyDraw);
bool isTheoreticalDraw(const petir_header::tdata& position, std::uint8_t sideToMove);
std::uint8_t popCount(std::uint64_t bits);
std::int32_t popCountUpperWords(petir_bitboard_mask::pword wordPointer);
std::int32_t popCountLowerWords(petir_bitboard_mask::pword wordPointer);
std::uint8_t popCountWords(petir_bitboard_mask::pword wordPointer);
bool hasSingleBit(std::uint64_t bits);
std::int32_t lazyMarginAdjustment(const petir_header::tdata& position);
}
namespace petir_search {
void clearHistoryTables();
std::int32_t mainSearch(std::int32_t searchDepth, std::int32_t sideToMove, std::int32_t alpha, std::int32_t beta, petir_header::tdata& position, bool afterNullMove, std::int32_t ply, std::int32_t extensionCredit, bool inNullMoveTree, std::int32_t reduced, std::int32_t nodeType);
std::int32_t quiescenceSearch(std::int32_t sideToMove, std::int32_t alpha, std::int32_t beta, petir_header::tdata& position, std::int32_t checkPlies, std::int32_t ply, std::int32_t extensionCredit, std::int32_t staticEval, std::int32_t nodeType);
void addHistory(std::int32_t plies, std::int32_t ply, std::int32_t move, std::int32_t sideToMove, std::int32_t piece);
void updateHistory(std::int32_t sideToMove, std::int32_t piece, std::int32_t toSquare, std::int32_t bonus);
void updateContinuation(petir_header::piece_to_history* const* continuationTables, std::int32_t piece, std::int32_t toSquare, std::int32_t bonus);
void penaliseQuiets(std::int32_t plies, std::int32_t sideToMove, const petir_header::tdata& position, petir_header::piece_to_history* const* continuationTables, const std::int32_t* quietMoves, std::int32_t quietCount, std::int32_t bestMove);
}
namespace petir_root {
std::int32_t searchRoot(std::int32_t searchDepth, std::int32_t sideToMove, std::int32_t rootNodeType, std::int32_t alpha, std::int32_t beta, petir_header::tdata position, bool afterNullMove, std::int32_t ply, std::int32_t extensionCredit);
}
namespace petir_uiterative {
std::int32_t iterativeDeepening(std::int32_t requestedDepth, std::int32_t sideToMove, std::int32_t rootNodeType, petir_header::tdata position);
std::int32_t mtdfSearch(petir_header::tdata position, std::uint8_t searchDepth, std::uint8_t sideToMove, std::int32_t firstGuess);
std::int32_t aspirationSearch(std::int32_t searchDepth, std::int32_t sideToMove, std::int32_t rootNodeType, std::int32_t previousScore, petir_header::tdata position);
bool hasForcedMove(std::int32_t sideToMove, petir_header::tdata position);
}
namespace petir_customboard {
void rebuildBitboards(petir_header::tdata& position);
void readFen(pas::String fen, petir_header::tdata& position, bool& whiteToMove);
}
namespace petir_notation {
pas::String moveToNotation(std::int32_t move, std::int32_t enPassantFile, std::int32_t sideToMove);
std::int32_t notationToMove(pas::String notation, petir_header::tdata position, std::uint8_t sideToMove);
pas::String shortToLongNotation(pas::String notation, petir_header::tdata position, std::uint8_t sideToMove);
pas::String squareName(std::int32_t square);
std::int32_t squareFromName(pas::String name);
}
namespace petir_pv {
pas::String getPv(petir_header::tdata position, std::int32_t sideToMove, std::int32_t maxLength);
void printPv(std::int32_t& lineNumber, pas::String pvText, std::int32_t depth, std::int32_t seconds, std::int32_t score);
pas::String formatTime(std::int32_t seconds);
}
namespace petir_opening_book {
void learnFromPgn(pas::String fileName);
void clearBook();
void loadBook();
std::int32_t findBookMove(petir_header::tdata position, std::int32_t sideToMove);
bool probeBook(std::uint64_t key, std::uint8_t sideToMove, std::int32_t& weight);
bool probeBookAnyWeight(std::uint64_t key, std::uint8_t sideToMove, std::int32_t& weight);
bool setBookWeight(std::uint64_t key, std::uint8_t sideToMove, std::int32_t newWeight);
void saveBook();
void learn();
bool addBookPosition(std::uint64_t key, std::uint8_t gameResult, std::uint8_t sideToMove, std::int32_t whiteElo, std::int32_t blackElo);
bool storeBookEntry(petir_opening_book::tbook newEntry, std::int32_t sideToMove);
void playNotation(pas::String notation, petir_header::tdata& position, std::uint8_t sideToMove);
}
namespace petir_etc {
void updateBlackHashKey(std::int32_t move, petir_header::tdata& position);
void updateWhiteHashKey(std::int32_t move, petir_header::tdata& position);
void updateHashKey(petir_header::tdata& position, std::int32_t move, std::int32_t sideToMove);
}
namespace petir_undoredo {
void resetUndo(petir_undoredo::tundo& undo);
void addUndo(petir_undoredo::tundo& undo, petir_header::tdata position, std::int32_t whiteClock, std::int32_t blackClock);
bool takeBack(petir_undoredo::tundo& undo, petir_header::tdata& position, std::int32_t& whiteClock, std::int32_t& blackClock, std::uint8_t& sideToMove);
bool moveForward(petir_undoredo::tundo& undo, petir_header::tdata& position, std::int32_t& whiteClock, std::int32_t& blackClock, std::uint8_t& sideToMove);
}
