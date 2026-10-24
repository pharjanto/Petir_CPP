// Faithful translation of pv.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_pv {
// pv.pas:52
pas::String getPv(petir_header::tdata position, std::int32_t sideToMove, std::int32_t maxLength) {
    // PV reconstruction is bookkeeping, not work charged to a node budget.
    // Keep legacy counting for searches without an explicit node limit.
    struct RestoreNodes {
        std::int64_t saved=petir_header::nodeCount;
        ~RestoreNodes() {if(petir::searchLimits.nodes>=0)petir_header::nodeCount=saved;}
    } restoreNodes;
    pas::String result{};
    std::int32_t score{};
    std::int32_t hashMove{};
    std::int32_t length{};
    bool avoidNullMove{};
    std::int32_t alpha{};
    std::int32_t beta{};
    hashMove = petir_header::noMove;
    result = pas::String("");
    alpha = pas::add(petir_header::infinity, 50000);
    beta = 0;
    if ((!petir_hashing::probeHash(position.hashKey, alpha, beta, 0, sideToMove, score, hashMove, avoidNullMove, position.fiftyMoveCounter))) {
        hashMove = petir_header::noMove;
    }
    length = 0;
    while ((hashMove != petir_header::noMove)) {
        (length = pas::increment(length, 1));
        result = (result + petir_notation::moveToNotation(hashMove, position.enPassantFile, sideToMove));
        if ((length >= pas::add(maxLength, 1))) {
            return result;
        }
        if ((sideToMove == petir_header::blackSide)) {
            petir_makemove::makeBlackMove(hashMove, position);
        } else {
            petir_makemove::makeWhiteMove(hashMove, position);
        }
        sideToMove = pas::sub(3, sideToMove);
        if ((!petir_hashing::probeHash(position.hashKey, alpha, beta, 0, sideToMove, score, hashMove, avoidNullMove, position.fiftyMoveCounter))) {
            hashMove = petir_header::noMove;
        }
        result = (result + ' ');
    }
    return result;
}

// pv.pas:24
void printPv(std::int32_t& lineNumber, pas::String pvText, std::int32_t depth, std::int32_t seconds, std::int32_t score) {
    pas::String line{};
    if ((!petir_header::useWinboard)) {
    } else {
        if (petir_header::useWinboard) {
            line = pas::intToStr(depth) + ' ' + pas::intToStr(score) + ' ' + pas::intToStr(static_cast<std::int32_t>(petir::searchTime.elapsedMs()/10)) + ' ' + pas::intToStr(petir_header::nodeCount) + ' ' + pvText;
            ;
            pas::sendWinboard(line);
        }
    }
}

// pv.pas:12
pas::String formatTime(std::int32_t seconds) {
    pas::String result{};
    pas::String hoursText{};
    pas::String minutesText{};
    pas::String secondsText{};
    hoursText = pas::intToStr((seconds / 3600));
    if ((pas::strToInt(hoursText) < 10)) {
        hoursText = ('0' + hoursText);
    }
    minutesText = pas::intToStr(((seconds % 3600) / 60));
    if ((pas::strToInt(minutesText) < 10)) {
        minutesText = ('0' + minutesText);
    }
    secondsText = pas::intToStr((seconds % 60));
    if ((pas::strToInt(secondsText) < 10)) {
        secondsText = ('0' + secondsText);
    }
    result = ((((hoursText + ':') + minutesText) + ':') + secondsText);
    return result;
}

}
