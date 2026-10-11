// Faithful translation of opening_book.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_opening_book {
petir_opening_book::hashbook bookTable{};
std::int32_t entryCount = 0;
std::int32_t moveIndex{};
// opening_book.pas:441
void learnFromPgn(pas::String fileName) {
    pas::File pgnFile{};
    pas::String line{};
    pas::String moveText{};
    pas::String eloText{};
    std::uint8_t plyCount{};
    std::uint8_t spacePosition{};
    std::uint8_t tokenStart{};
    petir_header::tdata position{};
    bool stopGame{};
    pas::String originalMoveText{};
    std::uint64_t hashBeforeMove{};
    std::int32_t sideToMove{};
    std::int32_t whiteElo{};
    std::int32_t blackElo{};
    std::int32_t parseCode{};
    std::uint8_t gameResult{};
    pas::assignFile(pgnFile, fileName);
    pas::reset(pgnFile);
    sideToMove = petir_header::whiteSide;
    plyCount = 0;
    stopGame = false;
    while ((!pas::endOfFile(pgnFile))) {
        pas::readLine(pgnFile, line);
        if ((line != pas::String(""))) {
            line = (line + ' ');
        }
        tokenStart = 1;
        if ((pas::upperCase(pas::copy(line, 1, 7)) == pas::upperCase(pas::String("[Result")))) {
            whiteElo = 0;
            blackElo = 0;
            if ((line[11] == '/')) {
                gameResult = petir_header::drawResult;
            } else {
                if ((line[10] == '1')) {
                    gameResult = petir_header::whiteWinResult;
                } else {
                    if ((line[10] == '0')) {
                        gameResult = petir_header::blackWinResult;
                    } else {
                        if ((line[10] == '*')) {
                            gameResult = petir_header::drawResult;
                        } else {
                            pas::showMessage(pas::String("result error"));
                            return;
                        }
                    }
                }
            }
        } else {
            if ((pas::upperCase(pas::copy(line, 1, 9)) == pas::String("[WHITEELO"))) {
                eloText = pas::copy(line, 12, 4);
                pas::parseInt(eloText, whiteElo, parseCode);
            } else {
                if ((pas::upperCase(pas::copy(line, 1, 9)) == pas::String("[BLACKELO"))) {
                    eloText = pas::copy(line, 12, 4);
                    pas::parseInt(eloText, blackElo, parseCode);
                }
            }
        }
        if (((pas::length(line) > 0) && (line[1] != '['))) {
            if ((pas::copy(line, 1, 2) == pas::String("1."))) {
                petir_udata::setInitialPosition(position);
                sideToMove = petir_header::whiteSide;
                plyCount = 0;
                stopGame = false;
                (petir_opening_book::entryCount = pas::increment(petir_opening_book::entryCount, 1));
            }
            while (((pas::findPosition(' ', line) != 0) && (!stopGame))) {
                spacePosition = pas::findPosition(' ', line);
                moveText = pas::copy(line, tokenStart, pas::sub(spacePosition, tokenStart));
                pas::deleteText(line, tokenStart, pas::add(pas::sub(spacePosition, tokenStart), 1));
                if ((!((moveText[1] >= '1' && moveText[1] <= '9')))) {
                    hashBeforeMove = petir_hashing_header::computeHashKey(position, sideToMove);
                    originalMoveText = moveText;
                    if ((moveText[pas::length(moveText)] == '+')) {
                        moveText = pas::copy(moveText, 1, pas::sub(pas::length(moveText), 1));
                    }
                    try {
                        petir_opening_book::playNotation(moveText, position, sideToMove);
                        if ((((whiteElo >= petir_opening_book::minimumElo) && (blackElo >= petir_opening_book::minimumElo)) && (plyCount >= 20))) {
                            if ((!petir_opening_book::addBookPosition(position.hashKey, gameResult, sideToMove, whiteElo, blackElo))) {
                                return;
                            }
                        }
                        (plyCount = pas::increment(plyCount, 1));
                        if ((plyCount >= 34)) {
                            stopGame = true;
                        }
                    } catch (...) {
                        stopGame = true;
                    }
                    sideToMove = pas::sub(3, sideToMove);
                }
            }
        }
    }
    pas::closeFile(pgnFile);
    pas::showMessage(pas::String("finish"));
    petir_opening_book::saveBook();
}

// opening_book.pas:212
void clearBook() {
    std::int32_t side{};
    std::int32_t slot{};
    std::int32_t lastSide{};
    std::int32_t lastSlot{};
    lastSide = 1;
    for (side = 0; side <= lastSide; ++side) {
        lastSlot = pas::sub(petir_opening_book::bookTableSize, 1);
        for (slot = 0; slot <= lastSlot; ++slot) {
            petir_opening_book::bookTable[side][slot].used = false;
        }
    }
}

// opening_book.pas:373
void loadBook() {
    pas::File bookFile{};
    pas::Array<petir_opening_book::tbook, 1, 10000> buffer{};
    std::int32_t index{};
    std::int32_t readCount{};
    std::int32_t lastWhiteIndex{};
    std::int32_t lastBlackIndex{};
    if (pas::fileExists((pas::mainForm.defaultDirectory + pas::String("\\white.book")))) {
        pas::assignFile(bookFile, (pas::mainForm.defaultDirectory + pas::String("\\white.book")));
        pas::reset(bookFile, 1);
        if ((pas::ioResult() != 0)) {
            return;
        }
        do {
            pas::blockRead(bookFile, buffer, sizeof(buffer), readCount);
            readCount = (readCount / sizeof(petir_opening_book::tbook));
            lastWhiteIndex = readCount;
            for (index = 1; index <= lastWhiteIndex; ++index) {
                petir_opening_book::entryCount = readCount;
                petir_opening_book::storeBookEntry(buffer[index], petir_header::whiteSide);
            }
        } while (!((readCount == 0)));
        pas::closeFile(bookFile);
    }
    if (pas::fileExists((pas::mainForm.defaultDirectory + pas::String("\\black.book")))) {
        pas::assignFile(bookFile, (pas::mainForm.defaultDirectory + pas::String("\\black.book")));
        pas::reset(bookFile, 1);
        if ((pas::ioResult() != 0)) {
            return;
        }
        do {
            pas::blockRead(bookFile, buffer, sizeof(buffer), readCount);
            readCount = (readCount / sizeof(petir_opening_book::tbook));
            lastBlackIndex = readCount;
            for (index = 1; index <= lastBlackIndex; ++index) {
                petir_opening_book::storeBookEntry(buffer[index], petir_header::blackSide);
            }
        } while (!((readCount == 0)));
        pas::closeFile(bookFile);
    }
}

// opening_book.pas:114
std::int32_t findBookMove(petir_header::tdata position, std::int32_t sideToMove) {
    std::int32_t result{};
    petir_header::tdata savedPosition{};
    petir_header::tmovelist moveList{};
    std::int32_t moveCount{};
    std::int32_t candidateCount{};
    pas::Array<petir_opening_book::anonymous_record_1, 1, 20> candidates{};
    std::int32_t totalWeight{};
    std::int32_t weight{};
    std::int32_t randomPick{};
    std::int32_t lastWhiteQuietIndex{};
    std::int32_t lastWhiteCaptureIndex{};
    std::int32_t lastBlackQuietIndex{};
    std::int32_t lastBlackCaptureIndex{};
    std::int32_t lastCandidateIndex{};
    savedPosition = position;
    candidateCount = 0;
    totalWeight = 0;
    result = petir_header::noMove;
    pas::fillChar(candidates, sizeof(candidates), 0);
    if ((sideToMove == petir_header::whiteSide)) {
        petir_movgen::generateWhiteQuietMoves(position, moveList, moveCount);
        lastWhiteQuietIndex = moveCount;
        for (petir_opening_book::moveIndex = 1; petir_opening_book::moveIndex <= lastWhiteQuietIndex; ++petir_opening_book::moveIndex) {
            position = savedPosition;
            petir_makemove::makeWhiteMove(moveList[petir_opening_book::moveIndex].move, position);
            if (petir_tools::whiteInCheck(position)) {
                continue;
            }
            if (petir_opening_book::probeBook(position.hashKey, sideToMove, weight)) {
                (candidateCount = pas::increment(candidateCount, 1));
                candidates[candidateCount].move = moveList[petir_opening_book::moveIndex].move;
                candidates[candidateCount].weight = weight;
                (totalWeight = pas::increment(totalWeight, weight));
            }
        }
        position = savedPosition;
        petir_movgen::generateWhiteCaptures(position, moveList, moveCount);
        lastWhiteCaptureIndex = moveCount;
        for (petir_opening_book::moveIndex = 1; petir_opening_book::moveIndex <= lastWhiteCaptureIndex; ++petir_opening_book::moveIndex) {
            position = savedPosition;
            petir_makemove::makeWhiteMove(moveList[petir_opening_book::moveIndex].move, position);
            if (petir_tools::whiteInCheck(position)) {
                continue;
            }
            if (petir_opening_book::probeBook(position.hashKey, sideToMove, weight)) {
                (candidateCount = pas::increment(candidateCount, 1));
                candidates[candidateCount].move = moveList[petir_opening_book::moveIndex].move;
                candidates[candidateCount].weight = weight;
                (totalWeight = pas::increment(totalWeight, weight));
            }
        }
    } else {
        petir_movgen::generateBlackQuietMoves(position, moveList, moveCount);
        lastBlackQuietIndex = moveCount;
        for (petir_opening_book::moveIndex = 1; petir_opening_book::moveIndex <= lastBlackQuietIndex; ++petir_opening_book::moveIndex) {
            position = savedPosition;
            petir_makemove::makeBlackMove(moveList[petir_opening_book::moveIndex].move, position);
            if (petir_tools::blackInCheck(position)) {
                continue;
            }
            if (petir_opening_book::probeBook(position.hashKey, sideToMove, weight)) {
                (candidateCount = pas::increment(candidateCount, 1));
                candidates[candidateCount].move = moveList[petir_opening_book::moveIndex].move;
                candidates[candidateCount].weight = weight;
                (totalWeight = pas::increment(totalWeight, weight));
            }
        }
        position = savedPosition;
        petir_movgen::generateBlackCaptures(position, moveList, moveCount);
        lastBlackCaptureIndex = moveCount;
        for (petir_opening_book::moveIndex = 1; petir_opening_book::moveIndex <= lastBlackCaptureIndex; ++petir_opening_book::moveIndex) {
            position = savedPosition;
            petir_makemove::makeBlackMove(moveList[petir_opening_book::moveIndex].move, position);
            if (petir_tools::blackInCheck(position)) {
                continue;
            }
            if (petir_opening_book::probeBook(position.hashKey, sideToMove, weight)) {
                (candidateCount = pas::increment(candidateCount, 1));
                candidates[candidateCount].move = moveList[petir_opening_book::moveIndex].move;
                candidates[candidateCount].weight = weight;
                (totalWeight = pas::increment(totalWeight, weight));
            }
        }
    }
    if ((candidateCount == 0)) {
        result = petir_header::noMove;
        return result;
    } else {
        randomPick = pas::add(pas::random(totalWeight), 1);
        weight = 0;
        lastCandidateIndex = candidateCount;
        for (petir_opening_book::moveIndex = 1; petir_opening_book::moveIndex <= lastCandidateIndex; ++petir_opening_book::moveIndex) {
            (weight = pas::increment(weight, candidates[petir_opening_book::moveIndex].weight));
            if ((weight >= randomPick)) {
                result = candidates[petir_opening_book::moveIndex].move;
                return result;
            }
        }
    }
    return result;
}

// opening_book.pas:40
bool probeBook(std::uint64_t key, std::uint8_t sideToMove, std::int32_t& weight) {
    bool result{};
    std::uint64_t slot{};
    std::uint64_t startSlot{};
    petir_opening_book::thashbook* entry{};
    slot = (key & pas::sub(petir_opening_book::bookTableSize, 1));
    startSlot = slot;
    (sideToMove = pas::decrement(sideToMove, 1));
    while ((petir_opening_book::bookTable[sideToMove][slot].used && (petir_opening_book::bookTable[sideToMove][slot].entry.key != key))) {
        (slot = pas::increment(slot, 1));
        if ((slot > pas::sub(petir_opening_book::bookTableSize, 1))) {
            slot = 0;
        }
        if ((slot == startSlot)) {
            result = false;
            return result;
        }
    }
    entry = pas::address(petir_opening_book::bookTable[sideToMove][slot]);
    if ((((*entry).used == false) || ((*entry).entry.weight < 4))) {
        result = false;
        return result;
    } else {
        result = true;
        (sideToMove = pas::increment(sideToMove, 1));
        if ((sideToMove == petir_header::whiteSide)) {
            weight = (*entry).entry.weight;
        } else {
            weight = (*entry).entry.weight;
        }
        if ((weight <= 0)) {
            result = false;
        }
    }
    return result;
}

// opening_book.pas:77
bool probeBookAnyWeight(std::uint64_t key, std::uint8_t sideToMove, std::int32_t& weight) {
    bool result{};
    std::uint64_t slot{};
    std::uint64_t startSlot{};
    petir_opening_book::thashbook* entry{};
    slot = (key & pas::sub(petir_opening_book::bookTableSize, 1));
    startSlot = slot;
    (sideToMove = pas::decrement(sideToMove, 1));
    while ((petir_opening_book::bookTable[sideToMove][slot].used && (petir_opening_book::bookTable[sideToMove][slot].entry.key != key))) {
        (slot = pas::increment(slot, 1));
        if ((slot > pas::sub(petir_opening_book::bookTableSize, 1))) {
            slot = 0;
        }
        if ((slot == startSlot)) {
            result = false;
            return result;
        }
    }
    entry = pas::address(petir_opening_book::bookTable[sideToMove][slot]);
    if ((((*entry).used == false) || ((*entry).entry.weight < 4))) {
        result = false;
        return result;
    } else {
        result = true;
        (sideToMove = pas::increment(sideToMove, 1));
        if ((sideToMove == petir_header::whiteSide)) {
            weight = (*entry).entry.weight;
        } else {
            weight = (*entry).entry.weight;
        }
    }
    return result;
}

// opening_book.pas:307
bool setBookWeight(std::uint64_t key, std::uint8_t sideToMove, std::int32_t newWeight) {
    bool result{};
    std::uint64_t slot{};
    std::uint64_t startSlot{};
    petir_opening_book::thashbook* entry{};
    result = true;
    slot = (key & pas::sub(petir_opening_book::bookTableSize, 1));
    startSlot = slot;
    (sideToMove = pas::decrement(sideToMove, 1));
    while ((petir_opening_book::bookTable[sideToMove][slot].used && (petir_opening_book::bookTable[sideToMove][slot].entry.key != key))) {
        (slot = pas::increment(slot, 1));
        if ((slot > pas::sub(petir_opening_book::bookTableSize, 1))) {
            slot = 0;
        }
        if ((slot == startSlot)) {
            result = false;
            pas::showMessage(pas::String("hash penuh"));
            return result;
        }
    }
    entry = pas::address(petir_opening_book::bookTable[sideToMove][slot]);
    (*entry).entry.weight = newWeight;
    return result;
}

// opening_book.pas:414
void saveBook() {
    std::int32_t slot{};
    pas::File bookFile{};
    std::int32_t lastWhiteSlot{};
    std::int32_t lastBlackSlot{};
    pas::assignFile(bookFile, (pas::mainForm.defaultDirectory + pas::String("\\white.book")));
    pas::rewrite(bookFile);
    lastWhiteSlot = pas::sub(petir_opening_book::bookTableSize, 1);
    for (slot = 0; slot <= lastWhiteSlot; ++slot) {
        if (petir_opening_book::bookTable[pas::sub(petir_header::whiteSide, 1)][slot].used) {
            pas::write(bookFile, petir_opening_book::bookTable[pas::sub(petir_header::whiteSide, 1)][slot].entry, sizeof(petir_opening_book::bookTable[pas::sub(petir_header::whiteSide, 1)][slot].entry));
        }
    }
    pas::closeFile(bookFile);
    pas::assignFile(bookFile, (pas::mainForm.defaultDirectory + pas::String("\\black.book")));
    pas::rewrite(bookFile);
    lastBlackSlot = pas::sub(petir_opening_book::bookTableSize, 1);
    for (slot = 0; slot <= lastBlackSlot; ++slot) {
        if (petir_opening_book::bookTable[pas::sub(petir_header::blackSide, 1)][slot].used) {
            pas::write(bookFile, petir_opening_book::bookTable[pas::sub(petir_header::blackSide, 1)][slot].entry, sizeof(petir_opening_book::bookTable[pas::sub(petir_header::blackSide, 1)][slot].entry));
        }
    }
    pas::closeFile(bookFile);
}

// opening_book.pas:36
void learn() {
}

// opening_book.pas:222
bool addBookPosition(std::uint64_t key, std::uint8_t gameResult, std::uint8_t sideToMove, std::int32_t whiteElo, std::int32_t blackElo) {
    bool result{};
    std::uint64_t slot{};
    std::uint64_t startSlot{};
    petir_opening_book::thashbook* entry{};
    result = true;
    slot = (key & pas::sub(petir_opening_book::bookTableSize, 1));
    startSlot = slot;
    (sideToMove = pas::decrement(sideToMove, 1));
    while ((petir_opening_book::bookTable[sideToMove][slot].used && (petir_opening_book::bookTable[sideToMove][slot].entry.key != key))) {
        (slot = pas::increment(slot, 1));
        if ((slot > pas::sub(petir_opening_book::bookTableSize, 1))) {
            slot = 0;
        }
        if ((slot == startSlot)) {
            result = false;
            pas::showMessage(pas::String("hash penuh"));
            return result;
        }
    }
    entry = pas::address(petir_opening_book::bookTable[sideToMove][slot]);
    (sideToMove = pas::increment(sideToMove, 1));
    if (((*entry).used == false)) {
        (*entry).used = true;
        (*entry).entry.key = key;
        (*entry).entry.weight = 0;
        if ((gameResult == petir_header::drawResult)) {
            ((*entry).entry.weight = pas::increment((*entry).entry.weight, 1));
        } else {
            if (((sideToMove == petir_header::whiteSide) && (gameResult == petir_header::whiteWinResult))) {
                ((*entry).entry.weight = pas::increment((*entry).entry.weight, 2));
            } else {
                if (((sideToMove == petir_header::blackSide) && (gameResult == petir_header::blackWinResult))) {
                    ((*entry).entry.weight = pas::increment((*entry).entry.weight, 2));
                }
            }
        }
    } else {
        if ((gameResult == petir_header::drawResult)) {
            ((*entry).entry.weight = pas::increment((*entry).entry.weight, 1));
        } else {
            if ((sideToMove == petir_header::whiteSide)) {
                if ((gameResult == petir_header::whiteWinResult)) {
                    ((*entry).entry.weight = pas::increment((*entry).entry.weight, 2));
                    if ((pas::sub(blackElo, whiteElo) >= 200)) {
                        ((*entry).entry.weight = pas::increment((*entry).entry.weight, 3));
                    } else {
                        if ((pas::sub(blackElo, whiteElo) >= 100)) {
                            ((*entry).entry.weight = pas::increment((*entry).entry.weight, 1));
                        }
                    }
                } else {
                    if (((*entry).entry.weight >= 2)) {
                        ((*entry).entry.weight = pas::decrement((*entry).entry.weight, 2));
                        if (((pas::sub(whiteElo, blackElo) >= 200) && ((*entry).entry.weight >= 2))) {
                            ((*entry).entry.weight = pas::decrement((*entry).entry.weight, 2));
                        } else {
                            if (((pas::sub(whiteElo, blackElo) >= 100) && ((*entry).entry.weight >= 1))) {
                                ((*entry).entry.weight = pas::decrement((*entry).entry.weight, 1));
                            }
                        }
                    }
                }
            } else {
                if ((sideToMove == petir_header::blackSide)) {
                    if ((gameResult == petir_header::blackWinResult)) {
                        ((*entry).entry.weight = pas::increment((*entry).entry.weight, 2));
                        if ((pas::sub(whiteElo, blackElo) >= 200)) {
                            ((*entry).entry.weight = pas::increment((*entry).entry.weight, 4));
                        } else {
                            if ((pas::sub(whiteElo, blackElo) >= 100)) {
                                ((*entry).entry.weight = pas::increment((*entry).entry.weight, 2));
                            } else {
                                if ((pas::sub(whiteElo, blackElo) >= 50)) {
                                    ((*entry).entry.weight = pas::increment((*entry).entry.weight, 1));
                                }
                            }
                        }
                    } else {
                        if (((*entry).entry.weight >= 2)) {
                            ((*entry).entry.weight = pas::decrement((*entry).entry.weight, 2));
                            if (((pas::sub(blackElo, whiteElo) >= 250) && ((*entry).entry.weight >= 2))) {
                                ((*entry).entry.weight = pas::decrement((*entry).entry.weight, 2));
                            } else {
                                if (((pas::sub(blackElo, whiteElo) >= 150) && ((*entry).entry.weight >= 1))) {
                                    ((*entry).entry.weight = pas::decrement((*entry).entry.weight, 1));
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}

// opening_book.pas:335
bool storeBookEntry(petir_opening_book::tbook newEntry, std::int32_t sideToMove) {
    bool result{};
    std::uint64_t slot{};
    std::uint64_t startSlot{};
    petir_opening_book::thashbook* entry{};
    result = true;
    slot = (newEntry.key & pas::sub(petir_opening_book::bookTableSize, 1));
    startSlot = slot;
    (sideToMove = pas::decrement(sideToMove, 1));
    while ((petir_opening_book::bookTable[sideToMove][slot].used && (petir_opening_book::bookTable[sideToMove][slot].entry.key != newEntry.key))) {
        (slot = pas::increment(slot, 1));
        if ((slot > pas::sub(petir_opening_book::bookTableSize, 1))) {
            slot = 0;
        }
        if ((slot == startSlot)) {
            result = false;
            pas::showMessage(pas::String("hash penuh"));
            return result;
        }
    }
    entry = pas::address(petir_opening_book::bookTable[sideToMove][slot]);
    (*entry).entry = newEntry;
    (*entry).used = true;
    return result;
}

// opening_book.pas:361
void playNotation(pas::String notation, petir_header::tdata& position, std::uint8_t sideToMove) {
    std::int32_t move{};
    if ((notation[1] != 'O')) {
        notation = petir_notation::shortToLongNotation(notation, position, sideToMove);
    }
    move = petir_notation::notationToMove(notation, position, sideToMove);
    if ((sideToMove == petir_header::whiteSide)) {
        petir_makemove::makeWhiteMove(move, position);
    } else {
        petir_makemove::makeBlackMove(move, position);
    }
}

}
