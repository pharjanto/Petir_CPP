// In-search differential check (target evaldiff): every evaluate() call from
// search runs the candidate evaluation and the baseline one (namespace
// petir_ueval_old) and aborts on the first difference. Each call also compares
// extra variants: both sides, both perspectives, a full window and several null
// windows (lazy exits), and the public helper functions on every pawn, knight
// and rook square. The engine otherwise runs normally and speaks UCI; drive it
// with diff_driver.py. A summary and coverage counts go to stderr at exit.
// EVALDIFF_DUMP=file records the first 2M calls (position + arguments).
#include "petir.hpp"
#include "stale_board.hpp"
#include <cstdio>
#include <cstdlib>

namespace petir_ueval {
std::int32_t evaluateCandidate(const petir_header::tdata& position, std::int32_t sideToMove, std::int32_t alpha, std::int32_t beta, bool opponentPerspective);
}
namespace petir_ueval_old {
std::int32_t evaluate(const petir_header::tdata& position, std::int32_t sideToMove, std::int32_t alpha, std::int32_t beta, bool opponentPerspective);
int passedPawnActivity(const petir_header::tdata& position, int pawnSquare, bool isWhite);
int connectedPassedPawns(const petir_header::tdata& position, bool isWhite);
int endgamePieceActivity(const petir_header::tdata& position, int pieceSquare, bool isWhite);
int rookPawnScale(const petir_header::tdata& position, int sideToMove);
std::int32_t materialForSide(std::int32_t sideToMove, std::int32_t materialScore);
bool isMaterialDraw(const petir_header::tdata& position, bool& likelyDraw);
bool isTheoreticalDraw(const petir_header::tdata& position, std::uint8_t sideToMove);
std::int32_t popCountUpperWords(petir_bitboard_mask::pword wordPointer);
std::int32_t popCountLowerWords(petir_bitboard_mask::pword wordPointer);
std::int32_t lazyMarginAdjustment(const petir_header::tdata& position);
}

namespace {
unsigned long long calls = 0, comparisons = 0;
// Coverage: how many calls reached each kind of position.
unsigned long long connectedWhite = 0, connectedBlack = 0, endgameActivity = 0, endgamePassers = 0,
    isolatedPawns = 0, blockedPawns = 0, noPawnBoards = 0, minorOnly = 0, staleBoards = 0;

[[noreturn]] void fail(const char* what, const petir_header::tdata& position, long long candidate, long long baseline, long long detail) {
    std::fprintf(stderr, "MISMATCH %s: candidate=%lld baseline=%lld detail=%lld after %llu calls\n", what, candidate, baseline, detail, calls);
    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) std::fprintf(stderr, "%3d", position.board[rank * 8 + file]);
        std::fprintf(stderr, "\n");
    }
    std::fprintf(stderr, "wnpm=%d bnpm=%d wbc=%d bbc=%d fifty=%d castle=%d wct=%d bct=%d\n", position.whiteNonPawnMaterial,
                 position.blackNonPawnMaterial, position.whiteBishopCount, position.blackBishopCount, position.fiftyMoveCounter,
                 position.castleRights, position.whiteCastleType, position.blackCastleType);
    std::fflush(stderr);
    std::abort();
}

void check(const char* what, const petir_header::tdata& position, long long candidate, long long baseline, long long detail = 0) {
    ++comparisons;
    if (candidate != baseline) fail(what, position, candidate, baseline, detail);
}

void compareHelpers(const petir_header::tdata& position) {
    bool candidateDraw = false, baselineDraw = false;
    check("isMaterialDraw", position, petir_ueval::isMaterialDraw(position, candidateDraw), petir_ueval_old::isMaterialDraw(position, baselineDraw));
    check("likelyDraw", position, candidateDraw, baselineDraw);
    for (int side = 1; side <= 2; ++side) {
        check("isTheoreticalDraw", position, petir_ueval::isTheoreticalDraw(position, side), petir_ueval_old::isTheoreticalDraw(position, side), side);
        check("rookPawnScale", position, petir_ueval::rookPawnScale(position, side), petir_ueval_old::rookPawnScale(position, side), side);
        check("materialForSide", position, petir_ueval::materialForSide(side, position.materialScore), petir_ueval_old::materialForSide(side, position.materialScore), side);
    }
    check("lazyMargin", position, petir_ueval::lazyMarginAdjustment(position), petir_ueval_old::lazyMarginAdjustment(position));
    check("connected white", position, petir_ueval::connectedPassedPawns(position, true), petir_ueval_old::connectedPassedPawns(position, true));
    check("connected black", position, petir_ueval::connectedPassedPawns(position, false), petir_ueval_old::connectedPassedPawns(position, false));
    std::uint64_t boards[2] = {position.whitePawns | position.whiteRooks, position.blackPieces};
    check("upperWords", position, petir_ueval::popCountUpperWords(reinterpret_cast<std::uint16_t*>(&boards[0])), petir_ueval_old::popCountUpperWords(reinterpret_cast<std::uint16_t*>(&boards[0])));
    check("lowerWords", position, petir_ueval::popCountLowerWords(reinterpret_cast<std::uint16_t*>(&boards[1])), petir_ueval_old::popCountLowerWords(reinterpret_cast<std::uint16_t*>(&boards[1])));
    for (int square = 0; square < 64; ++square) {
        const int piece = position.board[square];
        if (piece == 1 || piece == -1) {
            const bool isWhite = piece > 0;
            check("passedPawnActivity", position, petir_ueval::passedPawnActivity(position, square, isWhite), petir_ueval_old::passedPawnActivity(position, square, isWhite), square);
        }
        if (piece == 2 || piece == -2 || piece == 4 || piece == -4) {
            const bool isWhite = piece > 0;
            check("endgamePieceActivity", position, petir_ueval::endgamePieceActivity(position, square, isWhite), petir_ueval_old::endgamePieceActivity(position, square, isWhite), square);
        }
    }
}

void countCoverage(const petir_header::tdata& position) {
    using namespace petir_bitboard_mask;
    const bool endgame = position.whiteNonPawnMaterial <= 12 && position.blackNonPawnMaterial <= 12;
    const auto pawns = position.whitePawns | position.blackPawns;
    connectedWhite += petir_ueval_old::connectedPassedPawns(position, true) != 0;
    connectedBlack += petir_ueval_old::connectedPassedPawns(position, false) != 0;
    endgameActivity += endgame && pawns && (position.whiteRooks | position.blackRooks | position.whiteKnights | position.blackKnights);
    bool passer = false, isolated = false;
    for (int square = 0; square < 64; ++square) {
        const int piece = position.board[square];
        if (piece == 1 && !(whitePassedPawnMask[square] & position.blackPawns)) passer = true;
        if (piece == -1 && !(blackPassedPawnMask[square] & position.whitePawns)) passer = true;
        if (piece == 1 || piece == -1) {
            const auto own = piece > 0 ? position.whitePawns : position.blackPawns;
            const int file = square & 7;
            if (!(file > 0 && (fileMask[file - 1] & own)) && !(file < 7 && (fileMask[file + 1] & own))) isolated = true;
        }
    }
    endgamePassers += endgame && passer;
    isolatedPawns += isolated;
    blockedPawns += (((position.blackPawns & 0xffff000000000000ull) >> 8) & position.whitePawns)
                 || (((position.whitePawns & 0xffffull) << 8) & position.blackPawns);
    noPawnBoards += pawns == 0;
    const int white = position.whiteNonPawnMaterial, black = position.blackNonPawnMaterial;
    minorOnly += (white == 0 && (black == 0 || black == 3)) || (black == 0 && white == 3);
    staleBoards += eval_diff::isStaleBoard(position);
}
}

std::int32_t petir_ueval::evaluate(const petir_header::tdata& position, std::int32_t sideToMove, std::int32_t alpha, std::int32_t beta, bool opponentPerspective) {
    ++calls;
    static std::FILE* dump = [] { const char* path = std::getenv("EVALDIFF_DUMP"); return path ? std::fopen(path, "wb") : nullptr; }();
    if (dump && calls <= 2000000) {
        const std::int32_t args[4] = {sideToMove, alpha, beta, opponentPerspective};
        std::fwrite(&position, sizeof position, 1, dump);
        std::fwrite(args, sizeof args, 1, dump);
        if (calls == 2000000) { std::fclose(dump); dump = nullptr; }
    }
    const std::int32_t result = evaluateCandidate(position, sideToMove, alpha, beta, opponentPerspective);
    check("evaluate", position, result, petir_ueval_old::evaluate(position, sideToMove, alpha, beta, opponentPerspective), sideToMove * 10 + opponentPerspective);
    for (int side = 1; side <= 2; ++side) {
        for (int perspective = 0; perspective <= 1; ++perspective) {
            const bool opponent = perspective != 0;
            const int full = evaluateCandidate(position, side, -20500, 20500, opponent);
            check("full window", position, full, petir_ueval_old::evaluate(position, side, -20500, 20500, opponent), side * 10 + perspective);
            for (int offset : {-1200, -700, -400, -250, 0, 250, 400, 700, 1200}) {
                const int a = full + offset;
                check("null window", position, evaluateCandidate(position, side, a, a + 1, opponent),
                      petir_ueval_old::evaluate(position, side, a, a + 1, opponent), offset);
            }
        }
    }
    compareHelpers(position);
    countCoverage(position);
    if ((calls & ((1u << 22) - 1)) == 0) std::fprintf(stderr, "info string diff calls=%llu comparisons=%llu\n", calls, comparisons);
    return result;
}

namespace {
struct Report {
    ~Report() {
        std::fprintf(stderr, "DIFF SUMMARY calls=%llu comparisons=%llu mismatches=0\n", calls, comparisons);
        std::fprintf(stderr, "COVERAGE connectedWhite=%llu connectedBlack=%llu endgameActivity=%llu endgamePassers=%llu "
                     "isolated=%llu blocked=%llu noPawns=%llu minorOnly=%llu staleBoards=%llu\n",
                     connectedWhite, connectedBlack, endgameActivity, endgamePassers, isolatedPawns, blockedPawns,
                     noPawnBoards, minorOnly, staleBoards);
    }
} report;
}
