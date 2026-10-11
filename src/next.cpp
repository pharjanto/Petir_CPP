// Faithful translation of next.pas; Delphi source is in original/.
#include "petir.hpp"
#include <algorithm>
namespace petir_next {
// next.pas:54
void selectBestMove(petir_header::tmovelist& moveList, std::int32_t currentIndex, std::int32_t lastIndex) {
    auto* currentEntry = &moveList[currentIndex];
    if(lastIndex<=currentIndex)return;
    // Checked endpoints establish a valid range without a capacity test for
    // every score. A branch-free maximum, then a scan for its first entry,
    // selects the same record as the first-maximum rule, preserving ties.
    auto* const endEntry = &moveList[lastIndex]+1;
    int bestScore = currentEntry->score;
    for(auto* entry = currentEntry+1; entry != endEntry; ++entry) bestScore = std::max(bestScore, entry->score);
    auto* bestEntry = currentEntry;
    while(bestEntry->score != bestScore) ++bestEntry;
    if(bestEntry!=currentEntry)std::swap(*currentEntry,*bestEntry);
}

// next.pas:113
bool nextEvasionMove(std::int32_t sideToMove, petir_header::tdata& position, std::int32_t& move, std::int32_t& phase, std::int32_t& hashMove, std::int32_t& currentIndex, std::int32_t& moveCount, std::int32_t& checkerCount, petir_header::tmovelist& moveList) {
    bool result{};
    std::int32_t index{};
    std::int32_t lastIndex{};
    result = false;
    if ((phase == petir_header::hashMovePhase)) {
        if ((currentIndex == 0)) {
            if ((sideToMove == petir_header::whiteSide)) {
                petir_movgen::generateWhiteEvasions(position, moveList, moveCount, checkerCount);
            } else {
                petir_movgen::generateBlackEvasions(position, moveList, moveCount, checkerCount);
            }
            if ((moveCount == 0)) {
                result = true;
                return result;
            }
            lastIndex = moveCount;
            for (index = 1; index <= lastIndex; ++index) {
                if ((moveList[index].move == hashMove)) {
                    (moveList[index].score = pas::increment(moveList[index].score, 87000000));
                    break;
                }
            }
        }
        (currentIndex = pas::increment(currentIndex, 1));
        if ((currentIndex > moveCount)) {
            move = petir_header::noMove;
            return result;
        } else {
            petir_next::selectBestMove(moveList, currentIndex, moveCount);
            move = moveList[currentIndex].move;
            return result;
        }
    }
    habis: ;
    if ((phase == pas::add(petir_header::hashMovePhase, 2))) {
        move = petir_header::noMove;
        return result;
    }
    return result;
}

// Capture scores from the generators are the expected gain in pawns, so many
// tie (pawn takes knight = knight takes rook). Keep the gain in the high bits
// and, among equal gains, take the bigger victim and then the cheaper attacker
// first. Knight underpromotions go to the bad captures. The gain is
// score >> captureGainShift.
static constexpr int captureGainShift = 6;
static void rescoreCaptures(const petir_header::tdata& position, petir_header::tmovelist& captureList, std::int32_t captureCount) {
    for (std::int32_t index = 1; index <= captureCount; ++index) {
        const std::int32_t move = captureList[index].move;
        std::int32_t gain = captureList[index].score;
        if (((pas::shiftRight(move, 14) & 7) == petir_header::knightPromotion)) {
            gain = -1;
        }
        const std::int32_t toSquare = (pas::shiftRight(move, 7) & 127);
        const std::int32_t victim = (toSquare == petir_header::enPassantTarget) ? petir_header::whitePawn : pas::abs(position.board[toSquare]);
        const std::int32_t attacker = pas::abs(position.board[(move & 127)]);
        captureList[index].score = (gain << captureGainShift) + (victim << 3) + (7 - attacker);
    }
}

// next.pas:155
bool nextMove(std::int32_t sideToMove, std::int32_t ply, petir_header::tdata& position, std::int32_t& move, std::int32_t& phase, std::int32_t hashMove, std::int32_t& captureIndex, std::int32_t& quietIndex, std::int32_t& captureCount, std::int32_t& quietCount, petir_header::tmovelist& quietList, petir_header::tmovelist& captureList, std::int32_t& bestCaptureGain, petir_header::piece_to_history* const* continuationTables) {
    bool result{};
    std::int32_t index{};
    std::int32_t lastTriedCapture{};
    result = false;
    if ((phase == petir_header::hashMovePhase)) {
        (phase = pas::increment(phase, 1));
        if ((hashMove != 0)) {
            move = hashMove;
            return result;
        }
    }
    if ((phase == petir_header::goodCapturesPhase)) {
        if ((captureIndex == 0)) {
            if ((sideToMove == petir_header::whiteSide)) {
                petir_movgen::generateWhiteCaptures(position, captureList, captureCount);
            } else {
                petir_movgen::generateBlackCaptures(position, captureList, captureCount);
            }
            rescoreCaptures(position, captureList, captureCount);
        }
        (captureIndex = pas::increment(captureIndex, 1));
        if ((captureIndex > captureCount)) {
            (phase = pas::increment(phase, 1));
            quietIndex = 0;
        } else {
            petir_next::selectBestMove(captureList, captureIndex, captureCount);
            if ((captureList[captureIndex].move == hashMove)) {
                (captureIndex = pas::increment(captureIndex, 1));
                if ((captureIndex > captureCount)) {
                    (phase = pas::increment(phase, 1));
                    quietIndex = 0;
                    goto other;
                } else {
                    petir_next::selectBestMove(captureList, captureIndex, captureCount);
                }
            }
            if ((captureList[captureIndex].score < 0)) {
                badcap: ;
                (phase = pas::increment(phase, 1));
                quietIndex = 0;
                goto other;
            }
            move = captureList[captureIndex].move;
            const std::int32_t gain = (captureList[captureIndex].score >> captureGainShift);
            if ((gain > bestCaptureGain)) {
                bestCaptureGain = gain;
            }
            return result;
        }
    }
    other: ;
    if ((phase == petir_header::killerMove1Phase)) {
        (phase = pas::increment(phase, 1));
        if (((petir_header::firstKillers[ply] != hashMove) && petir_valid::isValidMove(position, petir_header::firstKillers[ply], sideToMove))) {
            lastTriedCapture = pas::sub(captureIndex, 1);
            for (index = 1; index <= lastTriedCapture; ++index) {
                if ((petir_header::firstKillers[ply] == captureList[index].move)) {
                    goto killer11;
                }
            }
            move = petir_header::firstKillers[ply];
            return result;
        }
    }
    killer11: ;
    // One killer per ply: after firstKillers the quiet moves follow, ordered by
    // history.
    if (((phase >= petir_header::killerMove2Phase) && (phase <= petir_header::killerMove4Phase))) {
        phase = petir_header::otherMovesPhase;
    }
    if ((phase == petir_header::otherMovesPhase)) {
        if ((quietIndex == 0)) {
            if ((sideToMove == petir_header::whiteSide)) {
                petir_movgen::generateWhiteQuietMoves(position, quietList, quietCount);
            } else {
                petir_movgen::generateBlackQuietMoves(position, quietList, quietCount);
            }
            // Add how each quiet move did after the previous move and after
            // our move before it.
            if ((continuationTables != nullptr) && ((continuationTables[0] != nullptr) || (continuationTables[1] != nullptr))) {
                for (index = 1; index <= quietCount; ++index) {
                    const std::int32_t piece = position.board[(quietList[index].move & 127)];
                    const std::int32_t toSquare = (pas::shiftRight(quietList[index].move, 7) & 127);
                    for (std::int32_t tableIndex = 0; tableIndex < 2; ++tableIndex) {
                        if ((continuationTables[tableIndex] != nullptr)) {
                            (quietList[index].score = pas::increment(quietList[index].score, (*continuationTables[tableIndex])[piece][toSquare]));
                        }
                    }
                }
            }
        }
        (quietIndex = pas::increment(quietIndex, 1));
        if ((quietIndex > quietCount)) {
            (phase = pas::increment(phase, 1));
        } else {
            if ((quietIndex <= 20)) {
                petir_next::selectBestMove(quietList, quietIndex, quietCount);
            }
            while (((quietList[quietIndex].move == hashMove) || (quietList[quietIndex].move == petir_header::firstKillers[ply]))) {
                (quietIndex = pas::increment(quietIndex, 1));
                if ((quietIndex > quietCount)) {
                    (phase = pas::increment(phase, 1));
                    goto fase3;
                } else {
                    if ((quietIndex <= 20)) {
                        petir_next::selectBestMove(quietList, quietIndex, quietCount);
                    }
                }
            }
            move = quietList[quietIndex].move;
            return result;
        }
    }
    fase3: ;
    if ((phase == petir_header::badCapturesPhase)) {
        if ((captureIndex <= captureCount)) {
            petir_next::selectBestMove(captureList, captureIndex, captureCount);
            while (((captureList[captureIndex].move == hashMove) || (captureList[captureIndex].move == petir_header::firstKillers[ply]))) {
                (captureIndex = pas::increment(captureIndex, 1));
                if ((captureIndex > captureCount)) {
                    (phase = pas::increment(phase, 1));
                    goto fase4;
                }
                petir_next::selectBestMove(captureList, captureIndex, captureCount);
            }
            move = captureList[captureIndex].move;
            (captureIndex = pas::increment(captureIndex, 1));
            return result;
        } else {
            (phase = pas::increment(phase, 1));
        }
    }
    fase4: ;
    if ((phase == petir_header::finishedPhase)) {
        move = petir_header::noMove;
    }
    return result;
}

// next.pas:77
bool nextRootMove(std::int32_t sideToMove, std::int32_t& move, std::int32_t& phase, std::int32_t& hashMove, std::int32_t& currentIndex, std::int32_t& moveCount, petir_header::tmovelist moveList) {
    bool result{};
    if ((phase == petir_header::hashMovePhase)) {
        (phase = pas::increment(phase, 1));
        if (petir_valid::isValidMove(petir_header::gamePosition, hashMove, sideToMove)) {
            move = hashMove;
            return result;
        }
    }
    if ((phase == pas::add(petir_header::hashMovePhase, 1))) {
        (currentIndex = pas::increment(currentIndex, 1));
        if ((currentIndex > moveCount)) {
            move = petir_header::noMove;
            return result;
        }
        petir_next::selectBestMove(moveList, currentIndex, moveCount);
        if ((moveList[currentIndex].move == hashMove)) {
            (currentIndex = pas::increment(currentIndex, 1));
            if ((currentIndex > moveCount)) {
                move = petir_header::noMove;
                return result;
            } else {
                petir_next::selectBestMove(moveList, currentIndex, moveCount);
            }
        }
        move = moveList[currentIndex].move;
        return result;
    }
    return result;
}

// next.pas:15
bool nextQuiescenceMove(std::int32_t sideToMove, petir_header::tdata& position, std::int32_t& move, std::int32_t& phase, std::int32_t& currentIndex, std::int32_t& moveCount, std::int32_t& score, std::int32_t checkPlies, petir_header::tmovelist& moveList) {
    bool result{};
    result = false;
    if ((phase == 0)) {
        if ((sideToMove == petir_header::whiteSide)) {
            if ((checkPlies > 0)) {
                petir_movgen::generateWhiteCapturesAndChecks(position, moveList, moveCount);
            } else {
                petir_movgen::generateWhiteCaptures(position, moveList, moveCount);
            }
        } else {
            if ((checkPlies > 0)) {
                petir_movgen::generateBlackCapturesAndChecks(position, moveList, moveCount);
            } else {
                petir_movgen::generateBlackCaptures(position, moveList, moveCount);
            }
        }
        (phase = pas::increment(phase, 1));
        if ((moveCount == 0)) {
            result = true;
            return result;
        }
    }
    if ((phase == 1)) {
        if ((currentIndex > moveCount)) {
            move = petir_header::noMove;
            return result;
        }
        petir_next::selectBestMove(moveList, currentIndex, moveCount);
        move = moveList[currentIndex].move;
        score = moveList[currentIndex].score;
        (currentIndex = pas::increment(currentIndex, 1));
        return result;
    }
    return result;
}

}
