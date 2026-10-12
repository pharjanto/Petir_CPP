// Random-board differential test (target evalrandom): the candidate ueval.cpp
// (petir_ueval) against the baseline (petir_ueval_old) on positions from random
// games, random endgame material, random pawn structures, and the boards after
// every pseudo-legal capture of either side. King captures leave the captured
// king's bitboard set (stale boards), so those are covered too. Each position is
// also tried with other fifty-move counters and castling states. Aborts on the
// first difference and exits with code 1.
// Usage: evalrandom [GAMES] [ENDGAME_ROUNDS] [PAWN_BOARDS] [SEED]
#include "engine.hpp"
#include "stale_board.hpp"
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>
#include <vector>

namespace petir_ueval {
std::int32_t evaluateCandidate(const petir_header::tdata& position, std::int32_t sideToMove, std::int32_t alpha, std::int32_t beta, bool opponentPerspective);
// The rest of the engine calls evaluate(); give it the candidate.
std::int32_t evaluate(const petir_header::tdata& position, std::int32_t sideToMove, std::int32_t alpha, std::int32_t beta, bool opponentPerspective) {
    return evaluateCandidate(position, sideToMove, alpha, beta, opponentPerspective);
}
}
namespace petir_ueval_old {
std::int32_t evaluate(const petir_header::tdata& position, std::int32_t sideToMove, std::int32_t alpha, std::int32_t beta, bool opponentPerspective);
int passedPawnActivity(const petir_header::tdata& position, int pawnSquare, bool isWhite);
int connectedPassedPawns(const petir_header::tdata& position, bool isWhite);
int endgamePieceActivity(const petir_header::tdata& position, int pieceSquare, bool isWhite);
int rookPawnScale(const petir_header::tdata& position, int sideToMove);
bool isMaterialDraw(const petir_header::tdata& position, bool& likelyDraw);
bool isTheoreticalDraw(const petir_header::tdata& position, std::uint8_t sideToMove);
std::int32_t lazyMarginAdjustment(const petir_header::tdata& position);
}

using petir::Position;
namespace {
std::mt19937 rng;
unsigned long long comparisons = 0, positionsChecked = 0, staleBoards = 0, kingCaptures = 0;

[[noreturn]] void fail(const char* what, const Position& position, long long candidate, long long baseline, long long detail) {
    std::fprintf(stderr, "MISMATCH %s: candidate=%lld baseline=%lld detail=%lld\n", what, candidate, baseline, detail);
    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) std::fprintf(stderr, "%3d", position.board[rank * 8 + file]);
        std::fprintf(stderr, "\n");
    }
    std::fprintf(stderr, "fen %s\n", petir::toFen(position, 1, 1).c_str());
    std::fprintf(stderr, "wnpm=%d bnpm=%d fifty=%d castle=%d wct=%d bct=%d\n", position.whiteNonPawnMaterial,
                 position.blackNonPawnMaterial, position.fiftyMoveCounter, position.castleRights,
                 position.whiteCastleType, position.blackCastleType);
    std::exit(1);
}

void check(const char* what, const Position& position, long long candidate, long long baseline, long long detail = 0) {
    ++comparisons;
    if (candidate != baseline) fail(what, position, candidate, baseline, detail);
}

void compareEval(const Position& position) {
    for (int side = 1; side <= 2; ++side) {
        for (int perspective = 0; perspective <= 1; ++perspective) {
            const bool opponent = perspective != 0;
            const int full = petir_ueval::evaluate(position, side, -20500, 20500, opponent);
            check("full window", position, full, petir_ueval_old::evaluate(position, side, -20500, 20500, opponent), side * 10 + perspective);
            for (int offset : {-1500, -1000, -700, -450, -300, -150, -50, 0, 50, 150, 300, 450, 700, 1000, 1500}) {
                const int alpha = full + offset;
                check("null window", position, petir_ueval::evaluate(position, side, alpha, alpha + 1, opponent),
                      petir_ueval_old::evaluate(position, side, alpha, alpha + 1, opponent), offset);
            }
        }
    }
}

void compareHelpers(const Position& position) {
    bool candidateDraw = false, baselineDraw = false;
    check("isMaterialDraw", position, petir_ueval::isMaterialDraw(position, candidateDraw), petir_ueval_old::isMaterialDraw(position, baselineDraw));
    check("likelyDraw", position, candidateDraw, baselineDraw);
    for (int side = 1; side <= 2; ++side) {
        check("isTheoreticalDraw", position, petir_ueval::isTheoreticalDraw(position, side), petir_ueval_old::isTheoreticalDraw(position, side), side);
        check("rookPawnScale", position, petir_ueval::rookPawnScale(position, side), petir_ueval_old::rookPawnScale(position, side), side);
    }
    check("lazyMargin", position, petir_ueval::lazyMarginAdjustment(position), petir_ueval_old::lazyMarginAdjustment(position));
    check("connected white", position, petir_ueval::connectedPassedPawns(position, true), petir_ueval_old::connectedPassedPawns(position, true));
    check("connected black", position, petir_ueval::connectedPassedPawns(position, false), petir_ueval_old::connectedPassedPawns(position, false));
    for (int square = 0; square < 64; ++square) {
        const int piece = position.board[square];
        for (bool isWhite : {true, false}) {
            if (piece == 1 || piece == -1)
                check("passedPawnActivity", position, petir_ueval::passedPawnActivity(position, square, isWhite),
                      petir_ueval_old::passedPawnActivity(position, square, isWhite), square);
            check("endgamePieceActivity", position, petir_ueval::endgamePieceActivity(position, square, isWhite),
                  petir_ueval_old::endgamePieceActivity(position, square, isWhite), square);
        }
    }
}

void audit(const Position& position, bool variants) {
    ++positionsChecked;
    staleBoards += eval_diff::isStaleBoard(position);
    compareEval(position);
    compareHelpers(position);
    if (!variants) return;
    // The fifty-move fade, the lazy-exit guard and the castling terms.
    for (int fifty : {79, 80, 90, 100, 101}) {
        Position copy = position;
        copy.fiftyMoveCounter = static_cast<std::uint8_t>(fifty);
        compareEval(copy);
    }
    for (int white = 0; white <= 2; ++white)
        for (int black = 0; black <= 2; ++black) {
            Position copy = position;
            copy.whiteCastleType = static_cast<std::uint8_t>(white);
            copy.blackCastleType = static_cast<std::uint8_t>(black);
            copy.castleRights = static_cast<std::uint8_t>(rng() & 15);
            compareEval(copy);
        }
}

// Positions after every capture either side can make, legal or not.
void auditCaptureChildren(const Position& position) {
    for (int mover : {1, 2}) {
        petir_header::tmovelist list{};
        int count = 0;
        Position parent = position;
        if (mover == 1) petir_movgen::generateWhiteCaptures(parent, list, count);
        else petir_movgen::generateBlackCaptures(parent, list, count);
        for (int k = 1; k <= count; ++k) {
            const int to = (list[k].move >> 7) & 127;
            const bool king = to < 64 && std::abs(position.board[to]) == 6;
            Position child = position;
            if (mover == 1) petir_makemove::makeWhiteMove(list[k].move, child);
            else petir_makemove::makeBlackMove(list[k].move, child);
            kingCaptures += king;
            audit(child, false);
        }
    }
}

bool fromBoard(Position& board, int side, Position& out) {
    int dummy;
    try { out = petir::fromFen(petir::toFen(board, side, 1), dummy); } catch (...) { return false; }
    return !(petir_tools::whiteInCheck(out) && petir_tools::blackInCheck(out));
}

// Kings plus the pieces of `set` (upper case White, lower case Black) on random squares.
bool randomBoard(const std::string& set, Position& out, int& side) {
    Position board{};
    board.enPassantFile = petir_header::noEnPassant;
    std::vector<int> free;
    for (int square = 0; square < 64; ++square) free.push_back(square);
    std::shuffle(free.begin(), free.end(), rng);
    std::size_t at = 0;
    board.board[free[at++]] = 6;
    board.board[free[at++]] = -6;
    const bool swap = rng() & 1;
    for (char c : set) {
        const int type = static_cast<int>(std::string(" PNBRQ").find(static_cast<char>(std::toupper(c))));
        int sign = std::isupper(static_cast<unsigned char>(c)) ? 1 : -1;
        if (swap) sign = -sign;
        int square = free[at++];
        if (type == 1)
            while (square < 8 || square >= 56) {
                if (at >= free.size()) return false;
                square = free[at++];
            }
        board.board[square] = static_cast<std::int8_t>(type * sign);
    }
    side = 1 + (rng() & 1);
    return fromBoard(board, side, out);
}
}

int main(int argc, char** argv) {
    petir::initialize(EVAL_DIFF_BOOKS);
    const int games = argc > 1 ? std::atoi(argv[1]) : 200;
    const int endgameRounds = argc > 2 ? std::atoi(argv[2]) : 800;
    const int pawnBoards = argc > 3 ? std::atoi(argv[3]) : 40000;
    rng.seed(argc > 4 ? std::atoi(argv[4]) : 20261012);
    const char* seeds[] = {
        petir::startFen,
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "r1bq1rk1/pp2bppp/2n2n2/3p4/3P4/2NBPN2/PP3PPP/R2Q1RK1 w - - 0 10",
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
        "2rq1rk1/pb1nbppp/1p2pn2/2pp4/2PP4/1PNBPN2/PB1Q1PPP/2R2RK1 w - - 0 12",
        "r2qr1k1/1b1nbppp/p2p1n2/1pp1p3/4P3/1BPP1N1P/PP1N1PP1/R1BQR1K1 w - - 0 13",
        "rnbqkb1r/pp3ppp/4pn2/2pp4/3P4/2P1PN2/PP3PPP/RNBQKB1R w KQkq - 0 5",
        "8/pp3k2/2p1p1p1/3pP1P1/3P1P2/2P5/PP4K1/8 w - - 0 30",
        "4r1k1/1p3ppp/p1p5/3r4/3P4/P1R2P2/1P3KPP/4R3 w - - 0 25",
        "6k1/5ppp/8/2n5/8/2N5/5PPP/6K1 w - - 0 30"};
    // 1. Random games.
    for (int game = 0; game < games; ++game) {
        int side = 1;
        Position position = petir::fromFen(seeds[game % std::size(seeds)], side);
        for (int ply = 0; ply < 220; ++ply) {
            audit(position, ply % 4 == 0);
            if (ply % 3 == 0) auditCaptureChildren(position);
            const auto moves = petir::legalMoves(position, side);
            if (moves.empty()) break;
            petir::makeMove(position, side, moves[rng() % moves.size()]);
            side = 3 - side;
        }
    }
    std::fprintf(stderr, "games done: %llu positions, %llu comparisons\n", positionsChecked, comparisons);
    // 2. Random endgame material, so the endgame, draw and scaling rules are reached.
    const char* sets[] = {"P", "PP", "Pp", "N", "B", "NP", "BP", "Bp", "Np", "RP", "Rr", "RPr", "RPrp", "QP", "Qq", "QPq",
                          "PPp", "PPpp", "BPb", "BPn", "NPn", "RBr", "RNr", "BPPb", "PPP", "ppp", "RPPrp", "BPPpp", "NPPpp",
                          "QPPqp", "RRrr", "RNrn", "RNPrnp", "RRPPrrpp", "NNPPnnpp", "RRPrp", "RPPPrppp", "NPPPnppp",
                          "BPPPbppp", "QRPqrp", "PPPPpppp", "PPPPPppppp", "RBPPrnpp", "RNPPrbpp", "QNPqnp", "Rb", "Rn",
                          "RBrr", "Qr", "NN", "BB", "NB", "RPb", "RPn", "Bpp", "Npp", "Rpp", "Qpp", "PPPppp"};
    for (int round = 0; round < endgameRounds; ++round)
        for (const char* set : sets) {
            Position position;
            int side;
            if (!randomBoard(set, position, side)) continue;
            audit(position, round % 8 == 0);
            if (round % 4 == 0) auditCaptureChildren(position);
        }
    std::fprintf(stderr, "endgames done: %llu positions, %llu comparisons\n", positionsChecked, comparisons);
    // 3. Random pawn structures with random pieces: passers, doubled, isolated
    // and blocked pawns, outposts and open files in every combination.
    for (int n = 0; n < pawnBoards; ++n) {
        std::string set;
        const int whitePawns = rng() % 9, blackPawns = rng() % 9;
        set.append(whitePawns, 'P');
        set.append(blackPawns, 'p');
        const char* pieces = "NBRQnbrq";
        const int extra = rng() % 9;
        for (int k = 0; k < extra; ++k) set.push_back(pieces[rng() % 8]);
        Position position;
        int side;
        if (!randomBoard(set, position, side)) continue;
        audit(position, n % 8 == 0);
        if (n % 4 == 0) auditCaptureChildren(position);
    }
    std::printf("PASS positions=%llu comparisons=%llu stale=%llu kingCaptures=%llu mismatches=0\n", positionsChecked,
                comparisons, staleBoards, kingCaptures);
    return 0;
}
