// Faithful translation of uiterative.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_uiterative {
std::int32_t extendedTimeLimit{};
std::int32_t completedMove{},completedDepth{},completedScore{};
std::string completedPv;
// uiterative.pas:145
std::int32_t iterativeDeepening(std::int32_t requestedDepth, std::int32_t sideToMove, std::int32_t rootNodeType, petir_header::tdata position) {
    std::int32_t result{};
    std::int32_t currentDepth{};
    std::int32_t score{};
    std::int32_t previousScore{};
    std::int32_t iterationScore{};
    std::int64_t lastIterationMs{},iterationStartMs{};
    bool scoreDropExtended{};
    std::int32_t lastBestMove{};
    std::int32_t stableIterations{};
    requestedDepth=std::clamp(requestedDepth,1,49);
    currentDepth = 1;
    completedMove=completedDepth=completedScore=0;completedPv.clear();
    const auto interrupted=[] {return petir::searchShouldStop(true);};
    const auto recordCompletedIteration=[&](int depth,int score) {
        completedMove=petir_header::rootBestMove;completedDepth=depth;completedScore=score;
        const int savedNodeCount=petir_header::nodeCount;
        try {completedPv=petir_pv::getPv(position,sideToMove,depth+(petir_header::useMtdf?3:4)).text;}
        catch(...) {petir_header::nodeCount=savedNodeCount;throw;}
        petir_header::nodeCount=savedNodeCount;
        const auto bestMoveText=petir_notation::moveToNotation(completedMove,position.enPassantFile,sideToMove).text;
        // The root move is authoritative even if a TT child was replaced.
        const auto firstSpace=completedPv.find(' ');
        completedPv=bestMoveText+(firstSpace==std::string::npos?"":completedPv.substr(firstSpace));
    };
    const auto restoreCompleted=[] {petir_header::rootBestMove=completedMove;return completedScore;};
    petir_root::originalTimeLimit = petir_header::timeLimit;
    petir_header::maxSearchPly = pas::round((currentDepth * 2.00));
    iterationScore = 0;
    petir_header::outputLineNumber = 1;
    scoreDropExtended = false;
    petir_uiterative::extendedTimeLimit = petir_header::timeLimit;
    petir_root::earlyStopTimeLimit = (petir_header::timeLimit / 3);
    petir_header::maxExtensionFactor = 1.00;
    if (petir_uiterative::hasForcedMove(sideToMove, position)) {
        completedMove=petir_header::rootBestMove;
        return result;
    }
    iterationStartMs=petir::TimeManager::nowMs();
    if ((!petir_header::useMtdf)) {
        petir_header::iterationDepth = currentDepth;
        score = petir_root::searchRoot(pas::mul(currentDepth, petir_header::plyUnits), sideToMove, petir_header::maxNode, pas::sub(pas::negate(petir_header::infinity), 500), pas::add(petir_header::infinity, 500), position, false, 0, pas::round(((petir_header::maxExtensionFactor * petir_header::iterationDepth) * petir_header::plyUnits)));
    } else {
        petir_header::iterationDepth = currentDepth;
        score = petir_uiterative::mtdfSearch(position, pas::mul(currentDepth, petir_header::plyUnits), sideToMove, 0);
    }
    if(interrupted())return restoreCompleted();
    recordCompletedIteration(currentDepth,score);
    lastIterationMs=petir::TimeManager::nowMs()-iterationStartMs;
    if (petir_header::testSolved) {
        return result;
    }
    lastBestMove = petir_header::rootBestMove;
    stableIterations = 0;
    while (currentDepth<requestedDepth && !interrupted() && petir::searchTime.canStartIteration(lastIterationMs)) {
        (currentDepth = pas::increment(currentDepth, 1));
        petir_header::iterationDepth = currentDepth;
        petir_header::maxSearchPly = pas::round(pas::mul(currentDepth, 2));
        iterationStartMs=petir::TimeManager::nowMs();
        petir_root::newPvFound = true;
        if ((!petir_header::useMtdf)) {
            iterationScore = petir_uiterative::aspirationSearch(pas::mul(currentDepth, petir_header::plyUnits), sideToMove, rootNodeType, previousScore, position);
        } else {
            iterationScore = petir_uiterative::mtdfSearch(position, pas::mul(currentDepth, petir_header::plyUnits), sideToMove, previousScore);
        }
        if (interrupted()) {
            return restoreCompleted();
        }
        recordCompletedIteration(currentDepth,iterationScore);
        if ((((iterationScore < 0) && (score >= 10)) && (!scoreDropExtended))) {
            petir::searchTime.adjustSoft(0.5);
            scoreDropExtended = true;
        }
        if ((((lastBestMove == petir_header::rootBestMove) && petir_root::newPvFound) && (currentDepth > 7))) {
            (stableIterations = pas::increment(stableIterations, 1));
            if ((((stableIterations >= 2) && (stableIterations <= 12)) && (iterationScore > pas::negate(20)))) {
                petir::searchTime.adjustSoft(-static_cast<double>(petir_uiterative::timeReductionPercent[stableIterations])/100);
            }
        } else {
            if ((stableIterations >= 4)) {
                petir::searchTime.adjustSoft(0.125);
            }
            stableIterations = 0;
            lastBestMove = petir_header::rootBestMove;
        }
        if (petir_header::testSolved) {
            return result;
        }
        lastIterationMs=petir::TimeManager::nowMs()-iterationStartMs;
        previousScore = score;
        score = iterationScore;
    }
    petir_header::iterationDepth = requestedDepth;
    result = score;
    return result;
}

// uiterative.pas:14
std::int32_t mtdfSearch(petir_header::tdata position, std::uint8_t searchDepth, std::uint8_t sideToMove, std::int32_t firstGuess) {
    std::int32_t result{};
    std::int32_t previousScore{};
    std::int32_t beta{};
    std::int32_t score{};
    double momentum{};
    pas::String pvText{};
    std::int32_t previousBestMove{};
    std::int32_t globalUpper{};
    std::int32_t globalLower{};
    bool raisedOnce{};
    bool loweredOnce{};
    momentum = 1.5;
    score = firstGuess;
    globalUpper = pas::add(petir_header::infinity, 100);
    globalLower = pas::sub(pas::negate(petir_header::infinity), 100);
    beta = score;
    previousBestMove = 0;
    raisedOnce = false;
    loweredOnce = false;
    do {
        if(petir::searchShouldStop(true))return score;
        previousScore = score;
        score = petir_root::searchRoot(searchDepth, sideToMove, petir_header::maxNode, pas::sub(beta, 1), beta, position, false, 0, pas::round((petir_header::maxExtensionFactor * searchDepth)));
        if(petir::searchShouldStop(true))return score;
        if ((score < beta)) {
            globalUpper = score;
            beta = score;
            if ((!raisedOnce)) {
                beta = pas::add(beta, pas::round((momentum * pas::sub(score, previousScore))));
            }
            loweredOnce = true;
        } else {
            globalLower = score;
            beta = pas::add(score, 1);
            if ((!loweredOnce)) {
                beta = pas::add(beta, pas::round((momentum * pas::sub(score, previousScore))));
            }
            if ((petir_header::rootBestMove != previousBestMove)) {
                pvText = petir_pv::getPv(position, sideToMove, pas::add((searchDepth / petir_header::plyUnits), 3));
                petir_pv::printPv(petir_header::outputLineNumber, pvText, (searchDepth / petir_header::plyUnits), pas::floor((static_cast<double>(pas::sub(pas::tickCount(), petir_header::searchStartTime)) / (1000))), score);
            }
            raisedOnce = true;
            previousBestMove = petir_header::rootBestMove;
        }
    } while (!((globalLower >= globalUpper)));
    petir_header::rootBestMove = previousBestMove;
    result = score;
    return result;
}

// uiterative.pas:68
std::int32_t aspirationSearch(std::int32_t searchDepth, std::int32_t sideToMove, std::int32_t rootNodeType, std::int32_t previousScore, petir_header::tdata position) {
    std::int32_t result{};
    std::int32_t score{};
    std::int32_t firstScore{};
    std::int32_t window{};
    std::int32_t extensionCredit{};
    window = 35;
    extensionCredit = 80;
    score = petir_root::searchRoot(searchDepth, sideToMove, petir_header::maxNode, pas::sub(previousScore, window), pas::add(previousScore, window), position, false, 0, extensionCredit);
    if(petir::searchShouldStop(true))return score;
    firstScore = score;
    if (((score >= pas::add(previousScore, window)) && (score < petir_header::infinity))) {
        score = petir_root::searchRoot(searchDepth, sideToMove, petir_header::maxNode, firstScore, pas::add(petir_header::infinity, 500), position, false, 0, extensionCredit);
        if(petir::searchShouldStop(true))return score;
        if ((score < firstScore)) {
            score = petir_root::searchRoot(searchDepth, sideToMove, petir_header::maxNode, pas::sub(pas::negate(petir_header::infinity), 500), pas::add(petir_header::infinity, 500), position, false, 0, extensionCredit);
        }
    } else {
        if ((score <= pas::sub(previousScore, window))) {
            petir::searchTime.adjustSoft(0.5);
            score = petir_root::searchRoot(searchDepth, sideToMove, petir_header::maxNode, pas::sub(pas::negate(petir_header::infinity), 500), score, position, false, 0, extensionCredit);
            if(petir::searchShouldStop(true))return score;
            if ((score > firstScore)) {
                score = petir_root::searchRoot(searchDepth, sideToMove, petir_header::maxNode, pas::sub(pas::negate(petir_header::infinity), 500), pas::add(petir_header::infinity, 500), position, false, 0, extensionCredit);
            }
        }
    }
    result = score;
    return result;
}

// uiterative.pas:93
bool hasForcedMove(std::int32_t sideToMove, petir_header::tdata position) {
    bool result{};
    bool inCheck{};
    petir_header::tmovelist moveList{};
    std::int32_t moveCount{};
    std::int32_t index{};
    std::int32_t goodCaptureCount{};
    bool pawnCaptureAvailable{};
    std::int32_t lastIndex{};
    inCheck = false;
    petir_root::stableBestMove = petir_header::noMove;
    pawnCaptureAvailable = false;
    result = false;
    if ((((sideToMove == petir_header::whiteSide) && petir_tools::whiteInCheck(position)) || ((sideToMove == petir_header::blackSide) && petir_tools::blackInCheck(position)))) {
        inCheck = true;
    }
    if (inCheck) {
        if ((sideToMove == petir_header::whiteSide)) {
            petir_movgen::generateWhiteEvasions(position, moveList, moveCount, index);
        } else {
            petir_movgen::generateBlackEvasions(position, moveList, moveCount, index);
        }
        if ((moveCount == 1)) {
            petir_header::rootBestMove = moveList[1].move;
            result = true;
            return result;
        }
    } else {
        if ((sideToMove == petir_header::whiteSide)) {
            petir_movgen::generateWhiteCaptures(position, moveList, moveCount);
        } else {
            petir_movgen::generateBlackCaptures(position, moveList, moveCount);
        }
        goodCaptureCount = 0;
        lastIndex = moveCount;
        for (index = 1; index <= lastIndex; ++index) {
            if ((moveList[index].score > 1)) {
                (goodCaptureCount = pas::increment(goodCaptureCount, 1));
            } else {
                if ((moveList[index].score > 0)) {
                    pawnCaptureAvailable = true;
                }
            }
        }
        ;
    }
    return result;
}

}
