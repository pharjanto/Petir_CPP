// Petir's static evaluation and quiescence scores for a list of positions, for
// the Texel quiet filter (tools/texel/quiet_filter.py).
//
// Usage: quiet_eval FENS OUT
// FENS holds one FEN per line. OUT gets one tab-separated line per FEN, all
// scores from White's view:
//   eval  quiescence  quiescenceWithChecks  rule  inCheck
// eval is evaluate() with the full window. quiescence is quiescenceSearch()
// with the full window, captures and pushes only (checkPlies 0); the second
// one also tries checks at its first ply (checkPlies 1), as the main search
// does below one ply. rule names an exact rule that sets the score, so no
// weight can change it: kpk, materialDraw, theoreticalDraw; likelyDraw marks
// the rook + minor against rook ending whose score the evaluation divides by
// 8; "-" when none applies. A FEN that cannot be read gives the line "error".
#include "engine.hpp"
#include "kpk.hpp"
#include "search_tools.hpp"
#include <fstream>
#include <iostream>
#include <string>

namespace {
constexpr int fullWindow = 20500;

int quiescence(petir::Position position, int sideToMove, int checkPlies) {
    // Resets the lazy-evaluation margins and the node count, so a score does
    // not depend on the positions searched before it.
    petir::prepareSearch(sideToMove, false, 0);
    return petir_search::quiescenceSearch(sideToMove, -fullWindow, fullWindow, position, checkPlies, 1, 0,
                                          petir_search::noValue, petir_search::pvNode);
}

std::string exactRule(const petir::Position& position, int sideToMove) {
    int kpkScore = 0;
    if ((position.whiteNonPawnMaterial | position.blackNonPawnMaterial) == 0
        && petir_kpk::probe(position, sideToMove, kpkScore)) {
        return "kpk";
    }
    bool likelyDraw = false;
    if ((position.whitePawns | position.blackPawns) == 0 && petir_ueval::isMaterialDraw(position, likelyDraw)) {
        return "materialDraw";
    }
    // The same prefilter as ueval.cpp's mayBeTheoreticalDraw.
    const int white = position.whiteNonPawnMaterial, black = position.blackNonPawnMaterial;
    const bool mayBeTheoreticalDraw = (white == 0 && (black == 0 || black == 3)) || (black == 0 && white == 3);
    if (mayBeTheoreticalDraw && petir_ueval::isTheoreticalDraw(position, static_cast<std::uint8_t>(sideToMove))) {
        return "theoreticalDraw";
    }
    return likelyDraw ? "likelyDraw" : "-";
}
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "usage: quiet_eval FENS OUT\n";
        return 2;
    }
    // No opening book is needed: a directory without book files skips it.
    petir::initialize("no-book");
    std::ifstream fens(argv[1]);
    std::ofstream out(argv[2]);
    if (!fens || !out) {
        std::cerr << "cannot open " << (fens ? argv[2] : argv[1]) << '\n';
        return 2;
    }
    std::string fen;
    long count = 0;
    while (std::getline(fens, fen)) {
        if (!fen.empty() && fen.back() == '\r') fen.pop_back();
        if (fen.empty()) continue;
        int sideToMove = 1;
        petir::Position position{};
        try {
            position = petir::fromFen(fen, sideToMove);
        } catch (const std::exception&) {
            out << "error\n";
            continue;
        }
        if (!petir::isConsistent(position)) {
            out << "error\n";
            continue;
        }
        const int toWhite = sideToMove == petir_header::whiteSide ? 1 : -1;
        const bool inCheck = petir_search::isInCheck(sideToMove, position);
        const int eval = petir_ueval::evaluate(position, sideToMove, -fullWindow, fullWindow);
        const int captures = quiescence(position, sideToMove, 0);
        const int withChecks = quiescence(position, sideToMove, 1);
        out << toWhite * eval << '\t' << toWhite * captures << '\t' << toWhite * withChecks << '\t'
            << exactRule(position, sideToMove) << '\t' << (inCheck ? 1 : 0) << '\n';
        if (++count % 100000 == 0) std::cerr << "  " << count << " positions\n";
    }
    return 0;
}
