// Faithful translation of search.pas; Delphi source is in original/.
#include "petir.hpp"
#include "order_stats.hpp"
#include <algorithm>
#include <cstdlib>
namespace petir_search {
pas::Array<std::int32_t, 0, 63> currentMove{};
pas::Array<std::int32_t, 0, 63> currentPiece{};
pas::Array<pas::Array<std::int32_t, 0, 63>, pas::negate(6), 6> historyTried{};
pas::Array<pas::Array<std::int32_t, 0, 63>, pas::negate(6), 6> historySucceeded{};
pas::Array<petir_search::anonymous_record_1, 0, 63> searchStack{};
std::int32_t researchCount{};
// The original plies*plies step, capped well inside the history limit.
static std::int32_t historyBonus(std::int32_t plies) {return std::min(pas::mul(plies, plies), petir_header::historyLimit / 4);}
// The change for a bonus or penalty: it moves the entry toward +historyLimit or
// -historyLimit and shrinks as the entry nears that limit.
static std::int32_t historyStep(std::int32_t entry, std::int32_t bonus) {return bonus - entry * std::abs(bonus) / petir_header::historyLimit;}
// SEE pruning of quiet moves: up to this many plies left, a quiet move whose SEE
// (in pawns) is below the margin is skipped. The margin grows with the square of
// the plies left: 0 up to 2 plies (any loss), -1 at 3, -2 at 4, -3 at 5, -4 at 6.
static constexpr std::int32_t seeQuietMaxPlies = 6;
static std::int32_t seeQuietMargin(std::int32_t plies) {return -(plies * plies / 8);}
// The continuation-history table of the move made at ply d; none above the
// root or after a null move.
static petir_header::piece_to_history* continuationTable(std::int32_t ply) {
    if ((ply < 0) || (petir_search::currentMove[ply] == petir_header::noMove)) return nullptr;
    return &petir_header::continuationHistory[petir_search::currentPiece[ply]][(pas::shiftRight(petir_search::currentMove[ply], 7) & 127)];
}
// search.pas:44
void clearHistoryTables() {
    pas::fillChar(petir_search::historyTried, sizeof(petir_search::historyTried), 0);
    pas::fillChar(petir_search::historySucceeded, sizeof(petir_search::historySucceeded), 0);
}

// search.pas:72
std::int32_t mainSearch(std::int32_t searchDepth, std::int32_t sideToMove, std::int32_t alpha, std::int32_t beta, petir_header::tdata& position, bool afterNullMove, std::int32_t ply, std::int32_t extensionCredit, bool inNullMoveTree, std::int32_t reduced, std::int32_t nodeType) {
    std::int32_t result{};
    std::int32_t bestScore{};
    std::int32_t phase{};
    petir_header::tdata savedPosition{};
    // Generators write both fields in entries 1..count before move ordering.
    // Avoid clearing two 2 KiB buffers on every node; capacity checks remain.
    petir_header::tmovelist quietList;
    petir_header::tmovelist captureList;
    std::int32_t captureIndex{};
    std::int32_t quietIndex{};
    std::int32_t captureCount{};
    std::int32_t quietMoveCount{};
    std::int32_t newDepth{};
    std::int32_t nextDepth{};
    std::int32_t evasionCount{};
    std::int32_t savedExtensionCredit{};
    std::int32_t currentAlpha{};
    std::int32_t currentBeta{};
    std::int32_t score{};
    std::int32_t quickEstimate{};
    std::int32_t reduction{};
    std::uint8_t toSquare{};
    bool inCheck{};
    std::uint64_t pinnedMask{};
    bool mateThreatExtended{};
    std::uint64_t savedHashKey{};
    std::int32_t bestCaptureGain{};
    std::int32_t savedEnPassant{};
    std::int32_t remainingPlies{};
    std::int32_t legalMoveCount{};
    std::int32_t move{};
    std::int32_t hashMove{};
    std::int32_t bestMove{};
    std::int32_t staticEval{};
    bool avoidNullMove{};
    bool isRepetitionDraw{};
    bool bestIsRepetitionDraw{};
    bool doNotReduce{};
    std::int32_t remainingPliesCase{};
    std::int32_t razorPliesCase{};
    // Quiet moves searched at this node, penalised when another quiet move is best.
    std::int32_t searchedQuiets[64];
    std::int32_t searchedQuietCount = 0;
// search.pas:91
    auto movingPiece = [&]() -> std::int8_t {
    std::int8_t result{};
    result = savedPosition.board[(move & 127)];
    return result;
};

// search.pas:97
    auto destinationSquare = [&](std::int32_t toSquare) -> std::int8_t {
    std::int8_t result{};
    std::int32_t target{};
    result = pas::negate(1);
    target = toSquare;
    if (((target == petir_header::longCastleTarget))) {
        if ((sideToMove == petir_header::whiteSide)) {
            result = petir_header::c1;
        } else {
            result = petir_header::c8;
        }
    }
    else if (((target == petir_header::shortCastleTarget))) {
        if ((sideToMove == petir_header::whiteSide)) {
            result = petir_header::g1;
        } else {
            result = petir_header::g8;
        }
    }
    else {
        result = toSquare;
    }
    return result;
};

// search.pas:107
    auto staticEvaluation = [&]() -> std::int32_t {
    std::int32_t result{};
    if (((staticEval == petir_search::noValue) && (ply >= 2))) {
        result = petir_ueval::evaluate(position, sideToMove, pas::negate(petir_header::infinity), petir_header::infinity);
    } else {
        result = staticEval;
    }
    return result;
};

    // Portability guards: stop safely and keep the original 64-ply arrays in bounds.
    if (petir::searchShouldStop())return 0;
    if (ply >= 63) return petir_ueval::evaluate(position, sideToMove, alpha, beta);
    // Continuation-history tables of the opponent's move into this node and of
    // our move before it.
    petir_header::piece_to_history* const continuationTables[2] = {continuationTable(pas::sub(ply, 1)), continuationTable(pas::sub(ply, 2))};
    petir_search::searchStack[ply].checkExtended = false;
    if ((position.fiftyMoveCounter >= 101)) {
        result = petir_header::drawScore;
        return result;
    }
    hashMove = petir_header::noMove;
    avoidNullMove = false;
    if ((petir_hashing::probeHash(position.hashKey, alpha, beta, searchDepth, sideToMove, score, hashMove, avoidNullMove, position.fiftyMoveCounter) && (nodeType != petir_search::pvNode))) {
        result = score;
        return result;
    }
    savedHashKey = position.hashKey;
    remainingPlies = pas::shiftRight(searchDepth, 3);
    mateThreatExtended = false;
    if ((sideToMove == petir_header::whiteSide)) {
        if (petir_tools::whiteInCheck(position)) {
            inCheck = true;
            petir_search::searchStack[ply].checkExtended = true;
            if ((extensionCredit > 0)) {
                if (((pas::add(position.whiteNonPawnMaterial, position.blackNonPawnMaterial) > 24) || (alpha != pas::sub(beta, 1)))) {
                    (searchDepth = pas::increment(searchDepth, 6));
                } else {
                    (searchDepth = pas::increment(searchDepth, 2));
                }
            }
            goto nonullmove;
        } else {
            inCheck = false;
        }
    } else {
        if (petir_tools::blackInCheck(position)) {
            inCheck = true;
            petir_search::searchStack[ply].checkExtended = true;
            if ((extensionCredit > 0)) {
                if (((pas::add(position.whiteNonPawnMaterial, position.blackNonPawnMaterial) > 24) || (alpha != pas::sub(beta, 1)))) {
                    (searchDepth = pas::increment(searchDepth, 6));
                } else {
                    (searchDepth = pas::increment(searchDepth, 2));
                }
            }
            goto nonullmove;
        } else {
            inCheck = false;
        }
    }
    petir_search::searchStack[ply].mateThreatExtended = false;
    if (((((((remainingPlies >= 2) && (beta < pas::sub(petir_header::infinity, 100))) && (!afterNullMove)) && (petir_ueval::materialForSide(sideToMove, position.materialScore) >= pas::negate(500))) && (alpha == pas::sub(beta, 1))) && (!avoidNullMove))) {
        if ((sideToMove == petir_header::whiteSide)) {
            savedEnPassant = position.whiteNonPawnMaterial;
        } else {
            savedEnPassant = position.blackNonPawnMaterial;
        }
        if ((savedEnPassant >= 3)) {
            savedEnPassant = position.enPassantFile;
            remainingPliesCase = remainingPlies;
            if (((remainingPliesCase == 4))) {
                if ((petir_ueval::materialForSide(sideToMove, position.materialScore) >= pas::add(alpha, 500))) {
                    newDepth = pas::sub(searchDepth, 32);
                } else {
                    if ((nodeType == petir_search::pvNode)) {
                        newDepth = pas::sub(searchDepth, 24);
                    } else {
                        newDepth = pas::sub(searchDepth, 32);
                    }
                }
            }
            else {
                newDepth = pas::sub(searchDepth, 32);
            }
            if ((position.enPassantFile != petir_header::noEnPassant)) {
                position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]);
                position.enPassantFile = petir_header::noEnPassant;
                position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[petir_header::noEnPassant]);
            }
            reduction = 0;
            petir_search::searchStack[ply].evaluated = false;
            petir_search::currentMove[ply] = petir_header::noMove;
            if ((newDepth < petir_header::plyUnits)) {
                if (((position.whiteNonPawnMaterial <= 9) && (position.blackNonPawnMaterial <= 9))) {
                    score = pas::negate(petir_search::quiescenceSearch(pas::sub(3, sideToMove), pas::negate(beta), pas::negate(pas::sub(beta, 1)), position, 0, pas::add(ply, 1), extensionCredit, petir_search::noValue, pas::negate(nodeType)));
                } else {
                    score = pas::negate(petir_search::quiescenceSearch(pas::sub(3, sideToMove), pas::negate(beta), pas::negate(pas::sub(beta, 1)), position, 1, pas::add(ply, 1), extensionCredit, petir_search::noValue, pas::negate(nodeType)));
                }
            } else {
                score = pas::negate(petir_search::mainSearch(newDepth, pas::sub(3, sideToMove), pas::negate(beta), pas::negate(pas::sub(beta, 1)), position, true, pas::add(ply, 1), extensionCredit, true, 0, pas::negate(nodeType)));
            }
            if(petir::searchShouldStop()) {position.hashKey=savedHashKey;position.enPassantFile=savedEnPassant;return 0;}
            if ((score >= beta)) {
                position.hashKey = savedHashKey;
                position.enPassantFile = savedEnPassant;
                result = score;
                return result;
            }
            if (((score == pas::add(pas::add(pas::negate(petir_header::kingValue), ply), 2)) && (extensionCredit > 0))) {
                petir_search::searchStack[ply].mateThreatExtended = true;
                (searchDepth = pas::increment(searchDepth, 6));
                (extensionCredit = pas::decrement(extensionCredit, 6));
                mateThreatExtended = true;
            }
            position.hashKey = savedHashKey;
            position.enPassantFile = savedEnPassant;
        }
    }
    nonullmove: ;
    doNotReduce = false;
    if ((petir_search::searchStack[pas::sub(ply, 1)].evaluated && ((pas::sub(petir_search::searchStack[pas::sub(ply, 1)].staticEval, petir_ueval::materialForSide(sideToMove, position.materialScore)) >= 90) || (petir_search::searchStack[pas::sub(ply, 1)].staticEval >= beta)))) {
        doNotReduce = true;
    }
    phase = 0;
    savedPosition = position;
    pinnedMask = inCheck?0:petir_tools::pinnedPieces(position,sideToMove);
    captureIndex = 0;
    bestMove = petir_header::noMove;
    currentAlpha = alpha;
    currentBeta = beta;
    bestScore = pas::sub(pas::negate(petir_header::infinity), 100);
    legalMoveCount = 0;
    nextDepth = pas::sub(searchDepth, petir_header::plyUnits);
    savedExtensionCredit = extensionCredit;
    bestCaptureGain = 0;
    remainingPlies = pas::shiftRight(searchDepth, 3);
    bestIsRepetitionDraw = false;
    PETIR_ORDER_STAT(petir_order_stats::SearchNode orderStats(remainingPlies, sideToMove, hashMove, nodeType, inCheck);)
    do {
        if(petir::searchShouldStop())return 0;
        petir_search::searchStack[ply].evaluated = false;
        isRepetitionDraw = false;
        avoidNullMove = false;
        staticEval = petir_search::noValue;
        extensionCredit = savedExtensionCredit;
        newDepth = nextDepth;
        reduction = 0;
        if ((!inCheck)) {
            petir_next::nextMove(sideToMove, ply, position, move, phase, hashMove, captureIndex, quietIndex, captureCount, quietMoveCount, quietList, captureList, bestCaptureGain, continuationTables);
            if ((move == petir_header::noMove)) {
                break;
            }
            toSquare = (pas::shiftRight(move, 7) & 127);
            if ((sideToMove == petir_header::whiteSide)) {
                if ((position.board[(move & 127)] == petir_header::whiteKing)) {
                    if (petir_tools::attackedByBlack(position, toSquare)) {
                        continue;
                    }
                } else {
                    if ((pinnedMask & petir_bitboard_mask::squareBit(move & 127)) && petir_tools::exposesWhiteKing(position, move)) {
                        continue;
                    }
                }
            } else {
                if ((position.board[(move & 127)] == petir_header::blackKing)) {
                    if (petir_tools::attackedByWhite(position, toSquare)) {
                        continue;
                    }
                } else {
                    if ((pinnedMask & petir_bitboard_mask::squareBit(move & 127)) && petir_tools::exposesBlackKing(position, move)) {
                        continue;
                    }
                }
            }
            // SEE pruning of quiet moves (as in Stockfish): at non-PV nodes with
            // few plies left, skip a late quiet move that loses material by SEE,
            // unless it gives check. Hash and killer moves are never pruned, and
            // at least one move must already have a non-mated score.
            if ((phase == petir_header::otherMovesPhase) && (remainingPlies <= seeQuietMaxPlies)
                && (nodeType != petir_search::pvNode) && (pas::sub(beta, alpha) == 1)
                && (legalMoveCount > 0) && !mateThreatExtended && (bestScore > pas::add(pas::negate(petir_header::kingValue), 100))
                && (toSquare <= 63)
                && (petir_usee::staticExchange(sideToMove, position, (move & 127), toSquare) < seeQuietMargin(remainingPlies))
                && !petir_tools::moveGivesCheck(position, sideToMove, move)) {
                (legalMoveCount = pas::increment(legalMoveCount, 1));
                continue;
            }
        } else {
            if (petir_next::nextEvasionMove(sideToMove, position, move, phase, hashMove, captureIndex, evasionCount, captureCount, quietList)) {
                result = pas::add(pas::negate(petir_header::kingValue), ply);
                return result;
            }
            if ((move == petir_header::noMove)) {
                break;
            }
            toSquare = (pas::shiftRight(move, 7) & 127);
            if (((legalMoveCount == 0) && (extensionCredit > 0))) {
                if ((evasionCount == 1)) {
                    (newDepth = pas::increment(newDepth, petir_header::singleReplyExtension));
                    (nextDepth = pas::increment(nextDepth, petir_header::singleReplyExtension));
                    (extensionCredit = pas::increment(extensionCredit, 8));
                    (savedExtensionCredit = pas::increment(savedExtensionCredit, 8));
                    (newDepth = pas::decrement(newDepth, (newDepth & 7)));
                }
                if ((captureCount == 2)) {
                    (newDepth = pas::increment(newDepth, petir_header::doubleCheckExtension));
                    (nextDepth = pas::increment(nextDepth, petir_header::doubleCheckExtension));
                    (extensionCredit = pas::decrement(extensionCredit, petir_header::doubleCheckExtension));
                    (savedExtensionCredit = pas::decrement(savedExtensionCredit, petir_header::doubleCheckExtension));
                }
                (extensionCredit = pas::decrement(extensionCredit, petir_header::checkExtension));
                (savedExtensionCredit = pas::decrement(savedExtensionCredit, petir_header::checkExtension));
            }
        }
        if ((sideToMove == petir_header::whiteSide)) {
            petir_makemove::makeWhiteMove(move, position);
        } else {
            petir_makemove::makeBlackMove(move, position);
        }
        // Children at full depth probe the hash table first; overlap that
        // memory access with the extension and reduction work below.
        if (nextDepth >= petir_header::plyUnits) {
            petir_hashing::prefetchHash(position.hashKey, pas::sub(3, sideToMove));
        }
        if ((toSquare == petir_header::enPassantTarget)) {
            if ((((sideToMove == petir_header::whiteSide) && petir_tools::whiteInCheck(position)) || ((sideToMove == petir_header::blackSide) && petir_tools::blackInCheck(position)))) {
                position = savedPosition;
                continue;
            }
        }
        if ((position.lastCaptureType == 0)) {
            (petir_search::historyTried[movingPiece()][destinationSquare(toSquare)] = pas::increment(petir_search::historyTried[movingPiece()][destinationSquare(toSquare)], pas::mul(remainingPlies, remainingPlies)));
            if (searchedQuietCount < 64) {
                searchedQuiets[searchedQuietCount++] = move;
            }
        }
        petir_search::currentMove[ply] = move;
        petir_search::currentPiece[ply] = movingPiece();
        (legalMoveCount = pas::increment(legalMoveCount, 1));
        PETIR_ORDER_STAT(orderStats.move(move, hashMove, phase, quietIndex, quietMoveCount, quietList);)
        if ((petir_repetition::addRepetition(position.hashKey, pas::sub(2, sideToMove)) == 1)) {
            score = petir_header::drawScore;
            isRepetitionDraw = true;
            goto search3;
        }
        petir_search::searchStack[ply].phase = phase;
        petir_search::searchStack[ply].legalMoveCount = legalMoveCount;
        if ((((position.whiteNonPawnMaterial == 0) && (position.blackNonPawnMaterial == 0)) && (nodeType == petir_search::pvNode))) {
            if (((position.lastCaptureType == 1) && (extensionCredit > 0))) {
                (newDepth = pas::increment(newDepth, 4));
                (extensionCredit = pas::decrement(extensionCredit, 4));
            }
            if ((((((position.lastCaptureType >= petir_header::whiteKnight) && (position.lastCaptureType <= petir_header::whiteQueen)) && (position.materialScore >= pas::negate(385))) && (position.materialScore <= 385)) && (extensionCredit > 0))) {
                (extensionCredit = pas::decrement(extensionCredit, 16));
                (newDepth = pas::increment(newDepth, 16));
                goto search1;
            }
        }
        if ((((((((toSquare <= 63) && (pas::abs(position.board[toSquare]) == petir_header::whitePawn)) && (nodeType == petir_search::pvNode)) && (extensionCredit > 0)) && (((sideToMove == petir_header::whiteSide) && (toSquare >= petir_header::a7)) || ((sideToMove == petir_header::blackSide) && (toSquare <= petir_header::h2)))) && (!inCheck)) && (petir_usee::staticExchange(sideToMove, savedPosition, (move & 127), toSquare) >= 0))) {
            (newDepth = pas::increment(newDepth, 8));
            (extensionCredit = pas::decrement(extensionCredit, 8));
            goto search1;
        }
        if ((((((((alpha != pas::sub(beta, 1)) && (position.lastCaptureType > 1)) && (position.lastCaptureType < 9)) && (position.lastCaptureType == savedPosition.lastCaptureType)) && (!inCheck)) && (extensionCredit > 0)) && (!mateThreatExtended))) {
            (newDepth = pas::increment(newDepth, 8));
            (extensionCredit = pas::decrement(extensionCredit, 8));
            goto search1;
        }
        if (((((((((remainingPlies <= 2) && (!inCheck)) && (position.lastCaptureType == 0)) && (pas::sub(beta, alpha) == 1)) && (!petir_search::searchStack[pas::sub(ply, 1)].mateThreatExtended)) && (!petir_search::searchStack[pas::sub(ply, 1)].checkExtended)) && (nodeType != petir_search::pvNode)) && (move != hashMove))) {
            razorPliesCase = remainingPlies;
            if (((razorPliesCase == 1))) {
                quickEstimate = petir_ueval::materialForSide(sideToMove, position.materialScore);
                if ((pas::add(position.whiteNonPawnMaterial, position.blackNonPawnMaterial) <= 24)) {
                    (quickEstimate = pas::increment(quickEstimate, 300));
                }
                if ((((((((pas::add(quickEstimate, 250) < currentAlpha) && (!petir_tools::moveGivesCheck(savedPosition, sideToMove, move))) && (position.whiteNonPawnMaterial > 3)) && (position.blackNonPawnMaterial > 3)) && (!petir_tools::hasPawnOnSeventh(sideToMove, position))) && (position.whitePawns != 0)) && (position.blackPawns != 0))) {
                    score = quickEstimate;
                    goto search3;
                }
            }
            else if (((razorPliesCase == 2))) {
                quickEstimate = petir_ueval::materialForSide(sideToMove, position.materialScore);
                if ((pas::add(position.whiteNonPawnMaterial, position.blackNonPawnMaterial) <= 24)) {
                    (quickEstimate = pas::increment(quickEstimate, 300));
                }
                if (((((((((pas::add(quickEstimate, 300) < currentAlpha) && (!petir_tools::moveGivesCheck(savedPosition, sideToMove, move))) && (position.whiteNonPawnMaterial > 5)) && (position.blackNonPawnMaterial > 5)) && (!petir_tools::hasPawnOnSeventh(sideToMove, position))) && (!petir_tools::hasFreeSixthRankPawn(sideToMove, position))) && (position.whitePawns != 0)) && (position.blackPawns != 0))) {
                    score = quickEstimate;
                    goto search3;
                }
            }
        }
        if ((sideToMove == petir_header::whiteSide)) {
            if ((((((((((((((!inCheck) && (remainingPlies >= 2)) && (ply >= 2)) && (position.lastCaptureType == 0)) && (phase > petir_header::killerMove3Phase)) && (alpha == currentAlpha)) && (pas::sub(beta, alpha) == 1)) && (!petir_tools::hasPawnOnSeventh(petir_header::whiteSide, position))) && (!mateThreatExtended)) && (!petir_tools::isGoodWhiteMove(position, toSquare))) && (((position.whiteNonPawnMaterial >= 6) && (position.blackNonPawnMaterial >= 6)) || (((position.whiteNonPawnMaterial >= 3) && (position.blackNonPawnMaterial >= 3)) && (remainingPlies == 2)))) && (!petir_tools::whiteMoveThreatensKing(position, move, quickEstimate))) && (!petir_tools::exposesBlackKing(savedPosition, move)))) {
                if ((((((quickEstimate == 0) || ((((legalMoveCount > 5) && (petir_search::searchStack[pas::sub(ply, 2)].phase > petir_header::killerMove4Phase)) && (petir_search::searchStack[pas::sub(ply, 1)].legalMoveCount <= 3)) && (!petir_search::searchStack[pas::sub(ply, 1)].mateThreatExtended))) && (legalMoveCount > 3)) && (petir_search::historyTried[movingPiece()][destinationSquare(toSquare)] >= petir_search::minHistoryTries)) && (petir_search::historySucceeded[movingPiece()][destinationSquare(toSquare)] <= pas::shiftRight(petir_search::historyTried[movingPiece()][destinationSquare(toSquare)], 3)))) {
                    (newDepth = pas::decrement(newDepth, 8));
                    reduction = 8;
                    if ((petir_search::historySucceeded[movingPiece()][destinationSquare(toSquare)] <= pas::shiftRight(petir_search::historyTried[movingPiece()][destinationSquare(toSquare)], 4))) {
                        (newDepth = pas::decrement(newDepth, 8));
                        reduction = 16;
                    }
                    goto search1;
                }
                if ((remainingPlies <= 8)) {
                    score = petir_ueval::evaluate(position, sideToMove, pas::negate(petir_header::infinity), petir_header::infinity, true);
                    staticEval = pas::negate(score);
                    petir_search::searchStack[ply].evaluated = true;
                    petir_search::searchStack[ply].staticEval = staticEval;
                    if (((pas::add(pas::add(pas::add(score, quickEstimate), petir_search::futilityMargins[remainingPlies]), 30) < currentAlpha) && (!petir_tools::hasFreeSixthRankPawn(sideToMove, position)))) {
                        (newDepth = pas::decrement(newDepth, 24));
                        reduction = 24;
                    } else {
                        if ((pas::add(pas::add(score, quickEstimate), petir_search::futilityMargins[remainingPlies]) < currentAlpha)) {
                            (newDepth = pas::decrement(newDepth, 16));
                            reduction = 16;
                        } else {
                            if ((pas::sub(pas::add(pas::add(score, quickEstimate), petir_search::futilityMargins[remainingPlies]), 5) < currentAlpha)) {
                                (newDepth = pas::decrement(newDepth, 8));
                                reduction = 8;
                            } else {
                                if (((quickEstimate > 0) && (pas::add(pas::add(pas::add(score, petir_search::futilityMargins[remainingPlies]), 40), pas::shiftRight(quickEstimate, 1)) < currentAlpha))) {
                                    newDepth = 0;
                                } else {
                                    if ((((bestCaptureGain > 1) && (quickEstimate == 0)) && (score < currentAlpha))) {
                                        if ((bestCaptureGain >= 5)) {
                                            (newDepth = pas::decrement(newDepth, 16));
                                            reduction = 16;
                                        } else {
                                            (newDepth = pas::decrement(newDepth, 8));
                                            reduction = 8;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                ;
            }
        } else {
            if ((((((((((((((!inCheck) && (remainingPlies >= 2)) && (ply >= 2)) && (position.lastCaptureType == 0)) && (phase > petir_header::killerMove3Phase)) && (alpha == currentAlpha)) && (pas::sub(beta, alpha) == 1)) && (!petir_tools::blackHasPawnOnSeventh(position))) && (!mateThreatExtended)) && (!petir_tools::isGoodBlackMove(position, toSquare))) && (((position.whiteNonPawnMaterial >= 6) && (position.blackNonPawnMaterial >= 6)) || (((position.whiteNonPawnMaterial >= 3) && (position.blackNonPawnMaterial >= 3)) && (remainingPlies == 2)))) && (!petir_tools::blackMoveThreatensKing(position, move, quickEstimate))) && (!petir_tools::exposesWhiteKing(savedPosition, move)))) {
                if ((((((quickEstimate == 0) || ((((legalMoveCount > 5) && (petir_search::searchStack[pas::sub(ply, 2)].phase > petir_header::killerMove4Phase)) && (petir_search::searchStack[pas::sub(ply, 1)].legalMoveCount <= 3)) && (!petir_search::searchStack[pas::sub(ply, 1)].mateThreatExtended))) && (legalMoveCount > 3)) && (petir_search::historyTried[movingPiece()][destinationSquare(toSquare)] >= petir_search::minHistoryTries)) && (petir_search::historySucceeded[movingPiece()][destinationSquare(toSquare)] <= pas::shiftRight(petir_search::historyTried[movingPiece()][destinationSquare(toSquare)], 3)))) {
                    (newDepth = pas::decrement(newDepth, 8));
                    reduction = 8;
                    if ((petir_search::historySucceeded[movingPiece()][destinationSquare(toSquare)] <= pas::shiftRight(petir_search::historyTried[movingPiece()][destinationSquare(toSquare)], 4))) {
                        (newDepth = pas::decrement(newDepth, 8));
                        reduction = 16;
                    }
                    goto search1;
                }
                if ((remainingPlies <= 8)) {
                    score = petir_ueval::evaluate(position, sideToMove, pas::negate(petir_header::infinity), petir_header::infinity, true);
                    staticEval = pas::negate(score);
                    petir_search::searchStack[ply].evaluated = true;
                    petir_search::searchStack[ply].staticEval = staticEval;
                    if (((pas::add(pas::add(pas::add(score, quickEstimate), petir_search::futilityMargins[remainingPlies]), 30) < currentAlpha) && (!petir_tools::hasFreeSixthRankPawn(sideToMove, position)))) {
                        (newDepth = pas::decrement(newDepth, 24));
                        reduction = 24;
                    } else {
                        if ((pas::add(pas::add(score, quickEstimate), petir_search::futilityMargins[remainingPlies]) < currentAlpha)) {
                            (newDepth = pas::decrement(newDepth, 16));
                            reduction = 16;
                        } else {
                            if ((pas::sub(pas::add(pas::add(score, quickEstimate), petir_search::futilityMargins[remainingPlies]), 5) < currentAlpha)) {
                                (newDepth = pas::decrement(newDepth, 8));
                                reduction = 8;
                            } else {
                                if (((quickEstimate > 0) && (pas::add(pas::add(pas::add(score, petir_search::futilityMargins[remainingPlies]), 40), pas::shiftRight(quickEstimate, 1)) < currentAlpha))) {
                                    newDepth = 0;
                                } else {
                                    if ((((bestCaptureGain > 1) && (quickEstimate == 0)) && (score < currentAlpha))) {
                                        if ((bestCaptureGain >= 5)) {
                                            (newDepth = pas::decrement(newDepth, 16));
                                            reduction = 16;
                                        } else {
                                            (newDepth = pas::decrement(newDepth, 8));
                                            reduction = 8;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                ;
            }
        }
        // 4.39 search.pas:758: require poor history before reducing a quiet move.
        if (!inCheck && (ply <= 6) && (nodeType != petir_search::pvNode)
            && (position.lastCaptureType == 0) && (ply >= 2) && (legalMoveCount > 12) && !mateThreatExtended
            && (reduction == 0) && !petir_search::searchStack[pas::sub(ply, 1)].mateThreatExtended
            && !petir_search::searchStack[pas::sub(ply, 2)].mateThreatExtended
            && !petir_tools::hasPawnOnSeventh(sideToMove, position) && !doNotReduce
            && !petir_tools::isGoodMove(position, toSquare, sideToMove)
            && (petir_search::historyTried[movingPiece()][destinationSquare(toSquare)] >= petir_search::minHistoryTries)
            && (petir_search::historySucceeded[movingPiece()][destinationSquare(toSquare)] <= pas::shiftRight(petir_search::historyTried[movingPiece()][destinationSquare(toSquare)], 6))
            && !petir_tools::moveGivesCheck(savedPosition, sideToMove, move)) {
            (newDepth = pas::decrement(newDepth, 8));
            reduction = 8;
            goto search1;
        }
        if ((((((((((((((!inCheck) && (remainingPlies >= 4)) && (ply >= 2)) && (nodeType != petir_search::pvNode)) && (((nodeType == petir_search::cutNode) && (legalMoveCount > 3)) || ((nodeType == petir_search::allNode) && (legalMoveCount > 6)))) && (!mateThreatExtended)) && (phase > petir_header::killerMove3Phase)) && (reduction == 0)) && (position.lastCaptureType == 0)) && (!petir_tools::moveGivesCheck(savedPosition, sideToMove, move))) && (!petir_tools::hasPawnOnSeventh(sideToMove, position))) && (!petir_tools::isGoodMove(position, toSquare, sideToMove))) && (!doNotReduce))) {
            (newDepth = pas::decrement(newDepth, 8));
            reduction = 8;
        }
        search1: ;
        PETIR_ORDER_STAT(orderStats.searching(reduction);)
        if ((newDepth < 8)) {
            if ((sideToMove == petir_header::whiteSide)) {
                if ((petir_tools::whiteForksOrWinsPawn(toSquare, position) && (extensionCredit > 0))) {
                    newDepth = 8;
                    extensionCredit = 0;
                }
            } else {
                if ((petir_tools::blackForksOrWinsPawn(toSquare, position) && (extensionCredit > 0))) {
                    newDepth = 8;
                    extensionCredit = 0;
                }
            }
        }
        if ((newDepth < petir_header::plyUnits)) {
            if ((legalMoveCount == 1)) {
                score = pas::negate(petir_search::quiescenceSearch(pas::sub(3, sideToMove), pas::negate(currentBeta), pas::negate(currentAlpha), position, 1, pas::add(ply, 1), extensionCredit, staticEval, pas::negate(nodeType)));
            } else {
                score = pas::negate(petir_search::quiescenceSearch(pas::sub(3, sideToMove), pas::negate(currentBeta), pas::negate(currentAlpha), position, 1, pas::add(ply, 1), extensionCredit, staticEval, petir_search::cutNode));
            }
        } else {
            if ((legalMoveCount == 1)) {
                score = pas::negate(petir_search::mainSearch(newDepth, pas::sub(3, sideToMove), pas::negate(currentBeta), pas::negate(currentAlpha), position, false, pas::add(ply, 1), extensionCredit, inNullMoveTree, reduction, pas::negate(nodeType)));
            } else {
                score = pas::negate(petir_search::mainSearch(newDepth, pas::sub(3, sideToMove), pas::negate(currentBeta), pas::negate(currentAlpha), position, false, pas::add(ply, 1), extensionCredit, inNullMoveTree, reduction, petir_search::cutNode));
            }
        }
        search2: ;
        if(petir::searchShouldStop()) {
            petir_repetition::removeRepetition(position.hashKey,pas::sub(2,sideToMove));position=savedPosition;return 0;
        }
        if ((((score > currentAlpha) && (score < beta)) && (legalMoveCount > 1))) {
            PETIR_ORDER_STAT(orderStats.pvsResearch();)
            if ((reduction != 0)) {
                (newDepth = pas::increment(newDepth, reduction));
            }
            if ((newDepth < petir_header::plyUnits)) {
                score = pas::negate(petir_search::quiescenceSearch(pas::sub(3, sideToMove), pas::negate(beta), pas::negate(currentAlpha), position, 1, pas::add(ply, 1), extensionCredit, petir_search::noValue, petir_search::pvNode));
            } else {
                score = pas::negate(petir_search::mainSearch(newDepth, pas::sub(3, sideToMove), pas::negate(beta), pas::negate(currentAlpha), position, false, pas::add(ply, 1), extensionCredit, inNullMoveTree, reduction, petir_search::pvNode));
            }
            PETIR_ORDER_STAT(orderStats.pvsResult(score > currentAlpha);)
        } else {
            if ((score >= beta)) {
                if ((reduction != 0)) {
                    PETIR_ORDER_STAT(orderStats.lmrResearch();)
                    (newDepth = pas::increment(newDepth, reduction));
                    reduction = 0;
                    if ((newDepth < petir_header::plyUnits)) {
                        score = pas::negate(petir_search::quiescenceSearch(pas::sub(3, sideToMove), pas::negate(score), pas::negate(currentAlpha), position, 1, pas::add(ply, 1), extensionCredit, petir_search::noValue, petir_search::pvNode));
                    } else {
                        score = pas::negate(petir_search::mainSearch(newDepth, pas::sub(3, sideToMove), pas::negate(score), pas::negate(currentAlpha), position, false, pas::add(ply, 1), extensionCredit, inNullMoveTree, reduction, petir_search::pvNode));
                    }
                    PETIR_ORDER_STAT(orderStats.lmrResult(score >= beta);)
                }
            }
        }
        search3: ;
        if(petir::searchShouldStop()) {
            petir_repetition::removeRepetition(position.hashKey,pas::sub(2,sideToMove));position=savedPosition;return 0;
        }
        if ((score > bestScore)) {
            bestScore = score;
            bestIsRepetitionDraw = isRepetitionDraw;
            if ((score > currentAlpha)) {
                bestMove = move;
                currentAlpha = score;
                PETIR_ORDER_STAT(orderStats.raised(legalMoveCount);)
                if ((bestScore >= beta)) {
                    PETIR_ORDER_STAT(orderStats.cutoff(legalMoveCount);)
                    if ((position.lastCaptureType == 0)) {
                        (petir_search::historySucceeded[movingPiece()][destinationSquare(toSquare)] = pas::increment(petir_search::historySucceeded[movingPiece()][destinationSquare(toSquare)], pas::mul(remainingPlies, remainingPlies)));
                    }
                    if (((position.lastCaptureType == 0) && (!inCheck))) {
                        petir_search::addHistory(remainingPlies, ply, move, sideToMove, movingPiece());
                        petir_search::updateContinuation(continuationTables, movingPiece(), toSquare, historyBonus(remainingPlies));
                        petir_search::penaliseQuiets(remainingPlies, sideToMove, savedPosition, continuationTables, searchedQuiets, searchedQuietCount, move);
                    }
                    petir_repetition::removeRepetition(position.hashKey, pas::sub(2, sideToMove));
                    position = savedPosition;
                    break;
                }
            }
        }
        currentBeta = pas::add(currentAlpha, 1);
        petir_repetition::removeRepetition(position.hashKey, pas::sub(2, sideToMove));
        position = savedPosition;
    } while (!((1 == 2)));
    if (petir::searchShouldStop()) {
        return result;
    }
    PETIR_ORDER_STAT(orderStats.finish(bestScore, alpha, beta, legalMoveCount);)
    if ((legalMoveCount == 0)) {
        bestMove = petir_header::noMove;
        result = petir_header::drawScore;
        bestScore = result;
    } else {
        result = bestScore;
        if ((bestMove == petir_header::noMove)) {
            bestMove = hashMove;
        }
    }
    if ((!bestIsRepetitionDraw)) {
        if ((bestScore <= alpha)) {
            petir_hashing::storeHash(savedHashKey, sideToMove, searchDepth, bestMove, bestScore, petir_header::upperBound, position.fiftyMoveCounter);
        } else {
            if ((bestScore >= beta)) {
                petir_hashing::storeHash(savedHashKey, sideToMove, searchDepth, bestMove, bestScore, petir_header::lowerBound, position.fiftyMoveCounter);
            } else {
                petir_hashing::storeHash(savedHashKey, sideToMove, searchDepth, bestMove, bestScore, petir_header::exactBound, position.fiftyMoveCounter);
                toSquare = (pas::shiftRight(bestMove, 7) & 127);
                if ((((bestMove != petir_header::noMove) && (toSquare != petir_header::enPassantTarget)) && ((toSquare > 63) || (position.board[toSquare] == 0)))) {
                    petir_search::updateHistory(sideToMove, position.board[(bestMove & 127)], (pas::shiftRight(bestMove, 7) & 127), historyBonus(remainingPlies));
                    petir_search::updateContinuation(continuationTables, position.board[(bestMove & 127)], (pas::shiftRight(bestMove, 7) & 127), historyBonus(remainingPlies));
                    petir_search::penaliseQuiets(remainingPlies, sideToMove, position, continuationTables, searchedQuiets, searchedQuietCount, bestMove);
                }
            }
        }
    }
    return result;
}

// Moves the entry toward +historyLimit for a bonus and toward -historyLimit for
// a penalty; the step shrinks as the entry nears that limit.
void updateHistory(std::int32_t sideToMove, std::int32_t piece, std::int32_t toSquare, std::int32_t bonus) {
    auto& entry = (sideToMove == petir_header::whiteSide ? petir_header::whiteHistory : petir_header::blackHistory)[piece][toSquare];
    entry += historyStep(entry, bonus);
}

// The same update for the move's entry in each continuation-history table.
void updateContinuation(petir_header::piece_to_history* const* continuationTables, std::int32_t piece, std::int32_t toSquare, std::int32_t bonus) {
    for (std::int32_t tableIndex = 0; tableIndex < 2; ++tableIndex) {
        if (continuationTables[tableIndex] != nullptr) {
            auto& entry = (*continuationTables[tableIndex])[piece][toSquare];
            entry = static_cast<std::int16_t>(entry + historyStep(entry, bonus));
        }
    }
}

// The other quiet moves searched at the node lose what the best one gains.
void penaliseQuiets(std::int32_t plies, std::int32_t sideToMove, const petir_header::tdata& position, petir_header::piece_to_history* const* continuationTables, const std::int32_t* quietMoves, std::int32_t quietCount, std::int32_t bestMove) {
    for (std::int32_t index = 0; index < quietCount; ++index) {
        if (quietMoves[index] != bestMove) {
            const std::int32_t piece = position.board[(quietMoves[index] & 127)];
            const std::int32_t toSquare = (pas::shiftRight(quietMoves[index], 7) & 127);
            petir_search::updateHistory(sideToMove, piece, toSquare, -historyBonus(plies));
            petir_search::updateContinuation(continuationTables, piece, toSquare, -historyBonus(plies));
        }
    }
}

// search.pas:51
void addHistory(std::int32_t plies, std::int32_t ply, std::int32_t move, std::int32_t sideToMove, std::int32_t piece) {
    petir_search::updateHistory(sideToMove, piece, (pas::shiftRight(move, 7) & 127), historyBonus(plies));
    petir_header::firstKillers[ply] = move;
}

}
