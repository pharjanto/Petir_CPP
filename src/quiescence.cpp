// Quiescence search from search.pas; Delphi source is in original/.
#include "petir.hpp"
#include "order_stats.hpp"
namespace petir_search {
namespace {
// Called only when the side to move is not in check. Prove a legal move exists
// before stand pat, without generating every quiet move in the common case.
bool hasLegalMoveWhenNotInCheck(petir_header::tdata& position, int sideToMove, petir_header::tmovelist& moveList) {
    const bool isWhite=sideToMove==petir_header::whiteSide;
    auto pawnPushes=(isWhite?position.whitePawns<<8:position.blackPawns>>8)&~position.allPieces;
    while(pawnPushes) {
        const int toSquare=petir_bitboard_mask::firstBit(pas::address(pawnPushes));
        pawnPushes&=pawnPushes-1;
        const int fromSquare=toSquare+(isWhite?-8:8);
        const auto move=static_cast<std::uint16_t>(fromSquare|(toSquare<<7));
        if(!(isWhite?petir_tools::exposesWhiteKing(position,move):petir_tools::exposesBlackKing(position,move)))return true;
    }
    auto isLegal=[&](int move) {
        auto childPosition=position;
        const auto savedNodeCount=petir_header::nodeCount;
        if(isWhite)petir_makemove::makeWhiteMove(move,childPosition);else petir_makemove::makeBlackMove(move,childPosition);
        petir_header::nodeCount=savedNodeCount;
        return !(isWhite?petir_tools::whiteInCheck(childPosition):petir_tools::blackInCheck(childPosition));
    };
    auto kingBoard=isWhite?position.whiteKingBoard:position.blackKingBoard;
    const int fromSquare=petir_bitboard_mask::firstBit(pas::address(kingBoard));
    auto targets=petir_bitboard_mask::kingAttacks[fromSquare]&~(isWhite?position.whitePieces:position.blackPieces);
    while(targets) {
        const int toSquare=petir_bitboard_mask::firstBit(pas::address(targets));
        targets&=targets-1;
        // In a nonchecked position the old king square cannot shield an attack
        // on an adjacent destination, so an attack test suffices here.
        if(!(isWhite?petir_tools::attackedByBlack(position,toSquare):petir_tools::attackedByWhite(position,toSquare)))return true;
    }
    int moveCount=0;
    if(isWhite)petir_movgen::generateWhiteCaptures(position,moveList,moveCount);else petir_movgen::generateBlackCaptures(position,moveList,moveCount);
    for(int index=1;index<=moveCount;++index)if(isLegal(moveList[index].move))return true;
    if(isWhite)petir_movgen::generateWhiteQuietMoves(position,moveList,moveCount);else petir_movgen::generateBlackQuietMoves(position,moveList,moveCount);
    for(int index=1;index<=moveCount;++index)if(isLegal(moveList[index].move))return true;
    return false;
}
}
// search.pas:982
std::int32_t quiescenceSearch(std::int32_t sideToMove, std::int32_t alpha, std::int32_t beta, petir_header::tdata& position, std::int32_t checkPlies, std::int32_t ply, std::int32_t extensionCredit, std::int32_t staticEval, std::int32_t nodeType) {
    std::int32_t result{};
    petir_header::tdata savedPosition{};
    petir_header::tmovelist moveList;
    bool inCheck{};
    std::uint64_t pinnedMask{};
    std::int32_t futilityScore{};
    std::int32_t checkerCount{};
    std::int32_t currentAlpha{};
    std::int32_t currentBeta{};
    std::int32_t bestScore{};
    std::int32_t moveIndex{};
    std::int32_t moveCount{};
    std::int32_t score{};
    std::int32_t standPat{};
    std::int32_t toSquare{};
    std::int32_t fromSquare{};
    std::int32_t whiteEvasionCase{};
    std::int32_t blackEvasionCase{};
    std::int32_t lastMoveIndex{};
// search.pas:991
    auto captureIsDangerous = [&]() -> bool {
    bool result{};
    if ((sideToMove == petir_header::whiteSide)) {
        if ((position.board[toSquare] == petir_header::blackQueen)) {
            result = true;
            return result;
        }
        if ((((moveList[moveIndex].score > 0) && ((petir_bitboard_mask::kingAttacks[toSquare] & position.blackKingBoard) != 0)) && (position.whiteQueens != 0))) {
            result = true;
            return result;
        }
        if (((position.board[(moveList[moveIndex].move & 127)] == 1) && (toSquare >= petir_header::a7))) {
            result = true;
            return result;
        }
        if (((position.board[toSquare] == pas::negate(1)) && (toSquare <= petir_header::h2))) {
            result = true;
            return result;
        }
    } else {
        if ((position.board[toSquare] == petir_header::whiteQueen)) {
            result = true;
            return result;
        }
        if ((((moveList[moveIndex].score > 0) && ((petir_bitboard_mask::kingAttacks[toSquare] & position.whiteKingBoard) != 0)) && (position.blackQueens != 0))) {
            result = true;
            return result;
        }
        if (((position.board[(moveList[moveIndex].move & 127)] == pas::negate(1)) && (toSquare <= petir_header::h2))) {
            result = true;
            return result;
        }
        if (((position.board[toSquare] == 1) && (toSquare >= petir_header::a7))) {
            result = true;
            return result;
        }
    }
    result = false;
    return result;
};
    auto isQueenPromotion = [](std::int32_t move) -> bool {
        return (pas::shiftRight(move, 14) & 7) == petir_header::queenPromotion;
    };

    // Portability guards: stop safely and keep the original 64-ply arrays in bounds.
    if (petir::searchShouldStop()) return 0;
    if ((sideToMove == petir_header::whiteSide)) {
        inCheck = petir_tools::whiteInCheck(position);
    } else {
        inCheck = petir_tools::blackInCheck(position);
    }
    if ((!inCheck)) {
        // Stalemate is terminal even when a cached evaluation would fail high.
        if(!hasLegalMoveWhenNotInCheck(position,sideToMove,moveList))return petir_header::drawScore;
        if(ply>=63)return petir_ueval::evaluate(position,sideToMove,alpha,beta);
        if ((staticEval == petir_search::noValue)) {
            standPat = petir_ueval::evaluate(position, sideToMove, alpha, beta);
        } else {
            standPat = staticEval;
        }
        if ((standPat >= beta)) {
            result = standPat;
            return result;
        }
        if ((standPat > alpha)) {
            alpha = standPat;
        }
    }
    if ((sideToMove == petir_header::whiteSide)) {
        if (inCheck) {
            petir_movgen::generateWhiteEvasions(position, moveList, moveCount, checkerCount);
            whiteEvasionCase = moveCount;
            if (((whiteEvasionCase == 1))) {
                (checkPlies = pas::increment(checkPlies, 1));
            }
            else if (((whiteEvasionCase == 0))) {
                result = pas::add(pas::negate(petir_header::kingValue), ply);
                return result;
            }
        } else {
            if ((checkPlies > 0)) {
                petir_movgen::generateWhiteCapturesAndChecks(position, moveList, moveCount);
            } else {
                petir_movgen::generateWhiteCapturesAndPushes(position, moveList, moveCount);
            }
        }
    } else {
        if (inCheck) {
            petir_movgen::generateBlackEvasions(position, moveList, moveCount, checkerCount);
            blackEvasionCase = moveCount;
            if (((blackEvasionCase == 1))) {
                (checkPlies = pas::increment(checkPlies, 1));
            }
            else if (((blackEvasionCase == 0))) {
                result = pas::add(pas::negate(petir_header::kingValue), ply);
                return result;
            }
        } else {
            if ((checkPlies > 0)) {
                petir_movgen::generateBlackCapturesAndChecks(position, moveList, moveCount);
            } else {
                petir_movgen::generateBlackCapturesAndPushes(position, moveList, moveCount);
            }
        }
    }
    if(ply>=63)return petir_ueval::evaluate(position,sideToMove,alpha,beta);
    savedPosition = position;
    if (!inCheck && moveCount == 0) {
        return standPat;
    }
    if ((!inCheck)) {
        bestScore = standPat;
    } else {
        bestScore = pas::sub(pas::negate(petir_header::infinity), 100);
    }
    currentAlpha = alpha;
    currentBeta = beta;
    pinnedMask = inCheck?0:petir_tools::pinnedPieces(position,sideToMove);
    lastMoveIndex = moveCount;
    PETIR_ORDER_STAT(int searchedMoveCount = 0;)
    for (moveIndex = 1; moveIndex <= lastMoveIndex; ++moveIndex) {
        if(petir::searchShouldStop())return 0;
        if ((moveIndex != moveCount)) {
            petir_next::selectBestMove(moveList, moveIndex, moveCount);
        }
        toSquare = (pas::shiftRight(moveList[moveIndex].move, 7) & 127);
        if ((!inCheck)) {
            if ((moveList[moveIndex].score < 0)) {
                // As in Stockfish, queen promotions are searched even when SEE
                // says they lose; every other move with a negative score is skipped.
                if (!isQueenPromotion(moveList[moveIndex].move)) {
                    bool queenPromotionRemains = false;
                    for (int laterIndex = moveIndex + 1; laterIndex <= moveCount; ++laterIndex) {
                        if (isQueenPromotion(moveList[laterIndex].move)) {
                            queenPromotionRemains = true;
                            break;
                        }
                    }
                    if (!queenPromotionRemains) {
                        break;
                    }
                    continue;
                }
            }
            if ((((alpha == pas::sub(beta, 1)) && (toSquare <= 63)) && (position.board[toSquare] != 0))) {
                futilityScore = pas::add(pas::add(standPat, petir_header::pieceValue[position.board[toSquare]]), 70);
                if (((((futilityScore < currentAlpha) && (!petir_tools::moveGivesCheck(savedPosition, sideToMove, moveList[moveIndex].move))) && (!petir_tools::hasPawnOnSeventh(sideToMove, position))) && ((pas::shiftRight(moveList[moveIndex].move, 14) & 7) == 0))) {
                    if ((!captureIsDangerous())) {
                        if ((futilityScore > bestScore)) {
                            bestScore = futilityScore;
                        }
                        continue;
                    }
                }
            }
            fromSquare = (moveList[moveIndex].move & 127);
            if ((sideToMove == petir_header::whiteSide)) {
                if (((position.board[fromSquare] == petir_header::whiteKing) && petir_tools::attackedByBlack(position, toSquare))) {
                    continue;
                } else {
                    if ((pinnedMask & petir_bitboard_mask::squareBit(fromSquare)) && petir_tools::exposesWhiteKing(position, moveList[moveIndex].move)) {
                        continue;
                    }
                }
            } else {
                if (((position.board[fromSquare] == petir_header::blackKing) && petir_tools::attackedByWhite(position, toSquare))) {
                    continue;
                } else {
                    if ((pinnedMask & petir_bitboard_mask::squareBit(fromSquare)) && petir_tools::exposesBlackKing(position, moveList[moveIndex].move)) {
                        continue;
                    }
                }
            }
        }
        if ((sideToMove == petir_header::whiteSide)) {
            petir_makemove::makeWhiteMove(moveList[moveIndex].move, position);
        } else {
            petir_makemove::makeBlackMove(moveList[moveIndex].move, position);
        }
        if ((toSquare == petir_header::enPassantTarget)) {
            if ((((sideToMove == petir_header::whiteSide) && petir_tools::whiteInCheck(position)) || ((sideToMove == petir_header::blackSide) && petir_tools::blackInCheck(position)))) {
                position = savedPosition;
                continue;
            }
        }
        PETIR_ORDER_STAT(++searchedMoveCount;)
        score = pas::negate(petir_search::quiescenceSearch(pas::sub(3, sideToMove), pas::negate(currentBeta), pas::negate(currentAlpha), position, pas::sub(checkPlies, 1), pas::add(ply, 1), extensionCredit, petir_search::noValue, pas::negate(nodeType)));
        if(petir::searchShouldStop()) {position=savedPosition;return 0;}
        if ((score >= beta)) {
            PETIR_ORDER_STAT(petir_order_stats::quiescenceCutoff(inCheck, searchedMoveCount);)
            position = savedPosition;
            result = score;
            return result;
        }
        if ((score > bestScore)) {
            bestScore = score;
            if ((score > currentAlpha)) {
                currentAlpha = score;
            }
        }
        position = savedPosition;
    }
    PETIR_ORDER_STAT(petir_order_stats::quiescenceDone(inCheck, searchedMoveCount);)
    result = bestScore;
    return result;
}

}
