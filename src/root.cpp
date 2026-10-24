// Faithful translation of root.pas; Delphi source is in original/.
#include "petir.hpp"
#include "order_stats.hpp"
#include <algorithm>
namespace petir_root {
std::int32_t stableBestMove{};
std::int32_t earlyStopTimeLimit{};
std::int32_t originalTimeLimit{};
bool newPvFound{};
// root.pas:16
std::int32_t searchRoot(std::int32_t searchDepth, std::int32_t sideToMove, std::int32_t rootNodeType, std::int32_t alpha, std::int32_t beta, petir_header::tdata position, bool afterNullMove, std::int32_t ply, std::int32_t extensionCredit) {
    std::int32_t result{};
    std::int32_t bestScore{};
    std::int32_t phase{};
    std::int32_t currentAlpha{};
    std::int32_t currentBeta{};
    std::int32_t score{};
    bool checkExtended{};
    bool inCheck{};
    bool isFirstMove{};
    bool finished{};
    bool isRepetitionDraw{};
    bool avoidNullMove{};
    petir_header::tdata savedPosition{};
    std::int8_t capturedPiece{};
    std::int32_t bestCaptureGain{};
    std::int32_t nullWindowScore{};
    petir_header::tmovelist quietList{};
    petir_header::tmovelist captureList{};
    pas::String pvText{};
    std::int32_t previousBestMove{};
    std::int32_t legalMoveCount{};
    std::int32_t move{};
    std::int32_t hashMove{};
    std::int32_t captureIndex{};
    std::int32_t quietIndex{};
    std::int32_t captureCount{};
    std::int32_t quietMoveCount{};
    std::int32_t newDepth{};
    std::int32_t nextDepth{};
    std::uint64_t childHashKey{};
    std::uint64_t rootHashKey{};
    std::int32_t checkerCount{};
    std::int32_t savedExtensionCredit{};
    std::int32_t remainingPlies{};
    std::int32_t toSquare{};
    std::int32_t timeExtensionCount{};
    std::int32_t lastAnswerIndex{};
    std::int32_t lastAnswerIndex2{};
    remainingPlies = (searchDepth / petir_header::plyUnits);
    if(petir::searchShouldStop(true))return 0;
    pas::fillChar(petir_search::currentMove, sizeof(petir_search::currentMove), 0);
    hashMove = petir_header::noMove;
    // Retain the hash move for ordering, but search root moves at every depth.
    // Cached score cutoffs remain available below the root.
    (void)petir_hashing::probeHash(position.hashKey, alpha, beta, searchDepth, sideToMove, score, hashMove, avoidNullMove, position.fiftyMoveCounter);
    rootHashKey = position.hashKey;
    (petir_header::nodeCount = pas::increment(petir_header::nodeCount, 1));
    finished = false;
    phase = 0;
    savedPosition = position;
    captureIndex = 0;
    currentAlpha = alpha;
    currentBeta = beta;
    if ((rootNodeType == petir_header::maxNode)) {
        bestScore = pas::sub(pas::negate(petir_header::infinity), 100);
    } else {
        bestScore = pas::add(petir_header::infinity, 100);
    }
    isFirstMove = true;
    legalMoveCount = 0;
    nextDepth = pas::sub(searchDepth, petir_header::plyUnits);
    timeExtensionCount = 0;
    if ((sideToMove == petir_header::whiteSide)) {
        if (petir_tools::whiteInCheck(position)) {
            inCheck = true;
        } else {
            inCheck = false;
        }
    } else {
        if (petir_tools::blackInCheck(position)) {
            inCheck = true;
        } else {
            inCheck = false;
        }
    }
    savedExtensionCredit = extensionCredit;
    petir_search::clearHistoryTables();
    PETIR_ORDER_STAT(petir_order_stats::RootNode orderStats;)
    do {
        if(petir::searchShouldStop(true))return 0;
        petir_search::searchStack[ply].checkExtended = false;
        pas::fillChar(petir_search::searchStack, sizeof(petir_search::searchStack), 0);
        extensionCredit = savedExtensionCredit;
        checkExtended = false;
        isRepetitionDraw = false;
        newDepth = nextDepth;
        if ((!inCheck)) {
            petir_next::nextMove(sideToMove, ply, position, move, phase, hashMove, captureIndex, quietIndex, captureCount, quietMoveCount, quietList, captureList, bestCaptureGain, nullptr);
        } else {
            if (petir_next::nextEvasionMove(sideToMove, position, move, phase, hashMove, captureIndex, captureCount, checkerCount, captureList)) {
                result = pas::sub(pas::negate(petir_header::kingValue), remainingPlies);
                return result;
            }
            if ((extensionCredit > 0)) {
                petir_search::searchStack[ply].checkExtended = true;
                if ((captureCount == 1)) {
                    (newDepth = pas::increment(newDepth, petir_header::singleReplyExtension));
                    (extensionCredit = pas::decrement(extensionCredit, petir_header::singleReplyExtension));
                } else {
                    if ((captureCount == 2)) {
                        (newDepth = pas::increment(newDepth, petir_header::twoReplyExtension));
                        (extensionCredit = pas::decrement(extensionCredit, petir_header::twoReplyExtension));
                    }
                }
                if ((checkerCount == 2)) {
                    (newDepth = pas::increment(newDepth, petir_header::doubleCheckExtension));
                    (extensionCredit = pas::decrement(extensionCredit, petir_header::doubleCheckExtension));
                }
                (newDepth = pas::increment(newDepth, petir_header::checkExtension));
                (extensionCredit = pas::decrement(extensionCredit, petir_header::checkExtension));
                checkExtended = true;
            }
        }
        if ((move == petir_header::noMove)) {
            break;
        }
        if(petir::searchLimits.restrictRootMoves &&
            std::find(petir::searchLimits.rootMoves.begin(),petir::searchLimits.rootMoves.end(),move)==petir::searchLimits.rootMoves.end())continue;
        toSquare = (pas::shiftRight(move, 7) & 127);
        if ((toSquare <= 63)) {
            capturedPiece = position.board[toSquare];
        } else {
            if ((toSquare == petir_header::enPassantTarget)) {
                capturedPiece = 1;
            } else {
                capturedPiece = 0;
            }
        }
        if (((capturedPiece == 0) && ((pas::shiftRight(move, 14) & 7) != 0))) {
            capturedPiece = 1;
        }
        if ((sideToMove == petir_header::whiteSide)) {
            petir_makemove::makeWhiteMove(move, position);
            if (petir_tools::whiteInCheck(position)) {
                position = savedPosition;
                continue;
            }
        } else {
            petir_makemove::makeBlackMove(move, position);
            if (petir_tools::blackInCheck(position)) {
                position = savedPosition;
                continue;
            }
        }
        (legalMoveCount = pas::increment(legalMoveCount, 1));
        PETIR_ORDER_STAT(orderStats.move(legalMoveCount);)
        petir_search::searchStack[ply].phase = phase;
        petir_search::searchStack[ply].legalMoveCount = legalMoveCount;
        petir_search::searchStack[ply].mateThreatExtended = false;
        childHashKey = position.hashKey;
        if ((petir_repetition::addRepetition(position.hashKey, pas::sub(2, sideToMove)) == 2)) {
            score = 0;
            isRepetitionDraw = true;
            goto search2;
        }
        petir_search::currentMove[ply] = move;
        petir_search::currentPiece[ply] = savedPosition.board[(move & 127)];
        if (((((!checkExtended) && (toSquare <= 63)) && (pas::abs(position.board[toSquare]) == petir_header::whitePawn)) && petir_tools::extendPassedPawnPush(position, newDepth, sideToMove, move, extensionCredit))) {
            petir_search::searchStack[ply].checkExtended = true;
            goto search1;
        }
        search1: ;
        if ((legalMoveCount == 1)) {
            score = pas::negate(petir_search::mainSearch(newDepth, pas::sub(3, sideToMove), pas::negate(currentBeta), pas::negate(currentAlpha), position, afterNullMove, pas::add(ply, 1), extensionCredit, false, 0, petir_search::pvNode));
        } else {
            score = pas::negate(petir_search::mainSearch(newDepth, pas::sub(3, sideToMove), pas::negate(currentBeta), pas::negate(currentAlpha), position, afterNullMove, pas::add(ply, 1), extensionCredit, false, 0, petir_search::cutNode));
        }
        nullWindowScore = score;
        search2: ;
        if (petir::searchShouldStop(true)) {
            petir_repetition::removeRepetition(childHashKey, pas::sub(2, sideToMove));
            position = savedPosition;
            return result;
        }
        previousBestMove = petir_header::rootBestMove;
        if ((rootNodeType == petir_header::maxNode)) {
            if ((((((!petir_header::useMtdf) && (score > currentAlpha)) && (score < beta)) && (!isFirstMove)) && (!isRepetitionDraw))) {
                PETIR_ORDER_STAT(orderStats.research();)
                petir_header::rootBestMove = move;
                if (petir_header::runningTestSuite) {
                    lastAnswerIndex = 5;
                    for (checkerCount = 1; checkerCount <= lastAnswerIndex; ++checkerCount) {
                        if ((move == petir_header::testAnswers[checkerCount])) {
                            petir_header::testSolved = true;
                        }
                    }
                }
                score = pas::negate(petir_search::mainSearch(newDepth, pas::sub(3, sideToMove), pas::negate(beta), pas::negate(currentAlpha), position, afterNullMove, pas::add(ply, 1), extensionCredit, false, 0, petir_search::pvNode));
                if (petir::searchShouldStop(true)) {
                    petir_repetition::removeRepetition(childHashKey, pas::sub(2, sideToMove));
                    position = savedPosition;
                    return result;
                }
                PETIR_ORDER_STAT(orderStats.researchResult(score > currentAlpha);)
                if ((score < nullWindowScore)) {
                    petir_header::rootBestMove = previousBestMove;
                }
            }
            if ((score > bestScore)) {
                bestScore = score;
                if ((score > currentAlpha)) {
                    PETIR_ORDER_STAT(orderStats.raised(legalMoveCount);)
                    if ((!petir_header::useMtdf)) {
                        pvText = petir_pv::getPv(position, pas::sub(3, sideToMove), pas::add(remainingPlies, 3));
                        pvText = ((petir_notation::moveToNotation(move, savedPosition.enPassantFile, sideToMove) + ' ') + pvText);
                        petir_pv::printPv(petir_header::outputLineNumber, pvText, remainingPlies, pas::floor((static_cast<double>(pas::sub(pas::tickCount(), petir_header::searchStartTime)) / (1000))), score);
                    }
                    if (petir_header::runningTestSuite) {
                        lastAnswerIndex2 = 5;
                        for (checkerCount = 1; checkerCount <= lastAnswerIndex2; ++checkerCount) {
                            if ((move == petir_header::testAnswers[checkerCount])) {
                                petir_header::testSolved = true;
                            }
                        }
                    }
                    if ((previousBestMove != move)) {
                        if ((((petir_header::timeMode == petir_header::movesInMinutes) || (petir_header::timeMode == petir_header::minutesPerGameWithIncrement)) && (timeExtensionCount <= 6))) {
                            petir::searchTime.adjustSoft(0.1);
                            (timeExtensionCount = pas::increment(timeExtensionCount, 1));
                        }
                    }
                    petir_header::rootBestMove = move;
                    if ((petir_header::rootBestMove != petir_root::stableBestMove)) {
                        petir_root::stableBestMove = petir_header::noMove;
                    }
                    currentAlpha = score;
                    if ((bestScore >= beta)) {
                        if ((capturedPiece == 0)) {
                            // Castle codes 65/66 are valid history indices, but
                            // not board squares. Read the piece from its source.
                            petir_search::addHistory(remainingPlies, ply, move, sideToMove, savedPosition.board[move & 127]);
                        }
                        (petir_header::cutoffCount = pas::increment(petir_header::cutoffCount, 1));
                        if (isFirstMove) {
                            (petir_header::firstMoveCutoffCount = pas::increment(petir_header::firstMoveCutoffCount, 1));
                        }
                        petir_repetition::removeRepetition(childHashKey, pas::sub(2, sideToMove));
                        position = savedPosition;
                        result = bestScore;
                        break;
                    }
                }
            }
            if ((!petir_header::useMtdf)) {
                currentBeta = pas::add(currentAlpha, 1);
            }
        }
        isFirstMove = false;
        petir_repetition::removeRepetition(childHashKey, pas::sub(2, sideToMove));
        position = savedPosition;
    } while (!(finished));
    PETIR_ORDER_STAT(orderStats.finish();)
    if ((legalMoveCount == 0)) {
        result = petir_header::drawScore;
        bestScore = result;
        petir_header::stalemateFound = true;
    }
    result = bestScore;
    if ((petir_header::rootBestMove == petir_header::noMove)) {
        petir_header::rootBestMove = hashMove;
    }
    if (petir::searchShouldStop(true)) {
        return result;
    }
    // A score over a subset of moves is not a bound for the unrestricted root.
    // Child entries still describe ordinary positions and can be retained.
    if(petir::searchLimits.restrictRootMoves)return result;
    if ((bestScore <= alpha)) {
        petir_hashing::storeHash(rootHashKey, sideToMove, searchDepth, petir_header::rootBestMove, bestScore, petir_header::upperBound, position.fiftyMoveCounter);
    } else {
        if ((bestScore >= beta)) {
            petir_hashing::storeHash(rootHashKey, sideToMove, searchDepth, petir_header::rootBestMove, bestScore, petir_header::lowerBound, position.fiftyMoveCounter);
        } else {
            petir_hashing::storeHash(rootHashKey, sideToMove, searchDepth, petir_header::rootBestMove, bestScore, petir_header::exactBound, position.fiftyMoveCounter);
        }
    }
    return result;
}

// Root singularity test for time management: search every root move except
// bestMove with a null window at bestScore - margin, at about half the
// iteration depth, and stop at the first move that reaches it. Positions in
// check and mate scores never count as singular.
bool rootMoveIsSingular(std::int32_t sideToMove, petir_header::tdata position, std::int32_t bestMove, std::int32_t bestScore, std::int32_t iterationDepth) {
    static constexpr std::int32_t marginCp = 50;
    static constexpr std::int32_t extensionCredit = 80;
    if (bestMove == petir_header::noMove) return false;
    if (pas::abs(bestScore) >= petir_header::kingValue - 100) return false;
    const bool inCheck = (sideToMove == petir_header::whiteSide) ? petir_tools::whiteInCheck(position) : petir_tools::blackInCheck(position);
    if (inCheck) return false;
    const std::int32_t singularBeta = bestScore - marginCp;
    const std::int32_t testDepth = (iterationDepth * petir_header::plyUnits - petir_header::plyUnits) / 2;
    const std::int32_t childDepth = testDepth - petir_header::plyUnits;
    const std::int32_t opponent = pas::sub(3, sideToMove);
    const std::int32_t repetitionSide = pas::sub(2, sideToMove);
    const petir_header::tdata savedPosition = position;
    std::int32_t phase = 0, move = 0, captureIndex = 0, quietIndex = 0, captureCount = 0, quietMoveCount = 0, bestCaptureGain = 0;
    petir_header::tmovelist quietList{};
    petir_header::tmovelist captureList{};
    petir_search::clearHistoryTables();
    while (true) {
        if (petir::searchShouldStop(true)) return false;
        pas::fillChar(petir_search::searchStack, sizeof(petir_search::searchStack), 0);
        petir_next::nextMove(sideToMove, 0, position, move, phase, bestMove, captureIndex, quietIndex, captureCount, quietMoveCount, quietList, captureList, bestCaptureGain, nullptr);
        if (move == petir_header::noMove) break;
        if (move == bestMove) continue;
        if (petir::searchLimits.restrictRootMoves &&
            std::find(petir::searchLimits.rootMoves.begin(), petir::searchLimits.rootMoves.end(), move) == petir::searchLimits.rootMoves.end()) continue;
        if (sideToMove == petir_header::whiteSide) {
            petir_makemove::makeWhiteMove(move, position);
            if (petir_tools::whiteInCheck(position)) {position = savedPosition; continue;}
        } else {
            petir_makemove::makeBlackMove(move, position);
            if (petir_tools::blackInCheck(position)) {position = savedPosition; continue;}
        }
        petir_search::searchStack[0].phase = phase;
        const std::uint64_t childHashKey = position.hashKey;
        std::int32_t score = 0;
        if (petir_repetition::addRepetition(childHashKey, repetitionSide) != 2) {
            petir_search::currentMove[0] = move;
            petir_search::currentPiece[0] = savedPosition.board[(move & 127)];
            score = pas::negate(petir_search::mainSearch(childDepth, opponent, pas::negate(singularBeta), pas::add(pas::negate(singularBeta), 1), position, false, 1, extensionCredit, false, 0, petir_search::cutNode));
        }
        petir_repetition::removeRepetition(childHashKey, repetitionSide);
        position = savedPosition;
        if (petir::searchShouldStop(true)) return false;
        if (score >= singularBeta) return false;
    }
    return true;
}

}
