// Faithful translation of evalmask.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_evalmask {
pas::Array<std::uint64_t, 0, 63> whiteOutpostMask{};
pas::Array<std::uint64_t, 0, 63> blackOutpostMask{};
std::uint64_t filesABCDMask{};
std::uint64_t filesEFGHMask{};
std::uint64_t filesABMask{};
std::uint64_t filesGHMask{};
pas::Array<std::uint64_t, 0, 63> adjacentFilesMask{};
pas::Array<std::uint8_t, 0, 65535> bitCountTable{};
pas::Array<pas::Array<bool, 0, 63>, 0, 63> sameOrAdjacentFile{};
pas::Array<pas::Array<std::uint8_t, 0, 63>, 0, 63> closenessHalfTable{};
pas::Array<pas::Array<std::uint8_t, 0, 63>, 0, 63> closenessTable{};
pas::Array<pas::Array<std::uint8_t, 0, 63>, 0, 63> bishopProximityTable{};
// evalmask.pas:108
void initEvalMasks() {
    std::int32_t square{};
    std::int32_t otherSquare{};
    std::int32_t file{};
    std::int32_t count{};
    std::int32_t lastSquare{};
    std::int32_t lastOtherSquare{};
    std::int32_t lastWordValue{};
    std::int32_t lastMaskSquare{};
    std::int32_t lastFile{};
    std::int32_t lastFileSquare{};
    petir_evalmask::initDistanceTables();
    lastSquare = 63;
    for (square = 0; square <= lastSquare; ++square) {
        lastOtherSquare = 63;
        for (otherSquare = 0; otherSquare <= lastOtherSquare; ++otherSquare) {
            count = petir_evalmask::bishopDistance(square, otherSquare);
            petir_evalmask::bishopProximityTable[square][otherSquare] = pas::sub(7, count);
            if ((petir_evalmask::bishopProximityTable[square][otherSquare] <= 3)) {
                petir_evalmask::bishopProximityTable[square][otherSquare] = 0;
            } else {
                (petir_evalmask::bishopProximityTable[square][otherSquare] = pas::decrement(petir_evalmask::bishopProximityTable[square][otherSquare], 3));
            }
            petir_evalmask::sameOrAdjacentFile[square][otherSquare] = ((pas::abs(pas::sub((square % 8), (otherSquare % 8))) == 0) || (pas::abs(pas::sub((square % 8), (otherSquare % 8))) == 1));
        }
    }
    lastWordValue = 65535;
    for (square = 1; square <= lastWordValue; ++square) {
        otherSquare = square;
        count = 0;
        while ((otherSquare != 0)) {
            if (((otherSquare & 1) == 1)) {
                (count = pas::increment(count, 1));
            }
            otherSquare = pas::shiftRight(otherSquare, 1);
        }
        petir_evalmask::bitCountTable[square] = count;
    }
    petir_evalmask::bitCountTable[0] = 0;
    lastMaskSquare = 63;
    for (square = 0; square <= lastMaskSquare; ++square) {
        petir_evalmask::whiteOutpostMask[square] = 0;
        petir_evalmask::blackOutpostMask[square] = 0;
        petir_bitboard_mask::whitePassedPawnMask[square] = 0;
        petir_bitboard_mask::blackPassedPawnMask[square] = 0;
        petir_bitboard_mask::whitePawnAttacks[square] = 0;
        petir_bitboard_mask::blackPawnAttacks[square] = 0;
        if (((square % 8) < 7)) {
            if ((square <= pas::sub(63, 9))) {
                petir_bitboard_mask::whitePawnAttacks[square] = (petir_bitboard_mask::whitePawnAttacks[square] | petir_bitboard_mask::squareMask[pas::add(square, 9)]);
            }
            if ((square >= 7)) {
                petir_bitboard_mask::blackPawnAttacks[square] = (petir_bitboard_mask::blackPawnAttacks[square] | petir_bitboard_mask::squareMask[pas::sub(square, 7)]);
            }
            otherSquare = pas::add(square, 9);
            while ((otherSquare <= petir_header::h8)) {
                petir_evalmask::whiteOutpostMask[square] = (petir_evalmask::whiteOutpostMask[square] | petir_bitboard_mask::squareMask[otherSquare]);
                petir_bitboard_mask::whitePassedPawnMask[square] = (petir_bitboard_mask::whitePassedPawnMask[square] | petir_bitboard_mask::squareMask[otherSquare]);
                (otherSquare = pas::increment(otherSquare, 8));
            }
            otherSquare = pas::sub(square, 7);
            while ((otherSquare >= petir_header::a1)) {
                petir_evalmask::blackOutpostMask[square] = (petir_evalmask::blackOutpostMask[square] | petir_bitboard_mask::squareMask[otherSquare]);
                petir_bitboard_mask::blackPassedPawnMask[square] = (petir_bitboard_mask::blackPassedPawnMask[square] | petir_bitboard_mask::squareMask[otherSquare]);
                (otherSquare = pas::decrement(otherSquare, 8));
            }
        }
        if (((square % 8) > 0)) {
            if ((square <= pas::sub(63, 7))) {
                petir_bitboard_mask::whitePawnAttacks[square] = (petir_bitboard_mask::whitePawnAttacks[square] | petir_bitboard_mask::squareMask[pas::add(square, 7)]);
            }
            if ((square >= 9)) {
                petir_bitboard_mask::blackPawnAttacks[square] = (petir_bitboard_mask::blackPawnAttacks[square] | petir_bitboard_mask::squareMask[pas::sub(square, 9)]);
            }
            otherSquare = pas::add(square, 7);
            while ((otherSquare <= petir_header::h8)) {
                petir_evalmask::whiteOutpostMask[square] = (petir_evalmask::whiteOutpostMask[square] | petir_bitboard_mask::squareMask[otherSquare]);
                petir_bitboard_mask::whitePassedPawnMask[square] = (petir_bitboard_mask::whitePassedPawnMask[square] | petir_bitboard_mask::squareMask[otherSquare]);
                (otherSquare = pas::increment(otherSquare, 8));
            }
            otherSquare = pas::sub(square, 9);
            while ((otherSquare >= petir_header::a1)) {
                petir_evalmask::blackOutpostMask[square] = (petir_evalmask::blackOutpostMask[square] | petir_bitboard_mask::squareMask[otherSquare]);
                petir_bitboard_mask::blackPassedPawnMask[square] = (petir_bitboard_mask::blackPassedPawnMask[square] | petir_bitboard_mask::squareMask[otherSquare]);
                (otherSquare = pas::decrement(otherSquare, 8));
            }
        }
        petir_bitboard_mask::rookRayUpMask[square] = 0;
        otherSquare = pas::add(square, 8);
        while ((otherSquare <= petir_header::h8)) {
            petir_bitboard_mask::rookRayUpMask[square] = (petir_bitboard_mask::rookRayUpMask[square] | petir_bitboard_mask::squareMask[otherSquare]);
            petir_bitboard_mask::whitePassedPawnMask[square] = (petir_bitboard_mask::whitePassedPawnMask[square] | petir_bitboard_mask::squareMask[otherSquare]);
            (otherSquare = pas::increment(otherSquare, 8));
        }
        otherSquare = pas::sub(square, 8);
        petir_bitboard_mask::rookRayDownMask[square] = 0;
        while ((otherSquare >= petir_header::a1)) {
            petir_bitboard_mask::rookRayDownMask[square] = (petir_bitboard_mask::rookRayDownMask[square] | petir_bitboard_mask::squareMask[otherSquare]);
            petir_bitboard_mask::blackPassedPawnMask[square] = (petir_bitboard_mask::blackPassedPawnMask[square] | petir_bitboard_mask::squareMask[otherSquare]);
            (otherSquare = pas::decrement(otherSquare, 8));
        }
    }
    lastFile = 7;
    for (square = 0; square <= lastFile; ++square) {
        otherSquare = square;
        petir_bitboard_mask::fileMask[square] = 0;
        while ((otherSquare <= petir_header::h8)) {
            petir_bitboard_mask::fileMask[square] = (petir_bitboard_mask::fileMask[square] | petir_bitboard_mask::squareMask[otherSquare]);
            (otherSquare = pas::increment(otherSquare, 8));
        }
    }
    petir_evalmask::filesABCDMask = (((petir_bitboard_mask::fileMask[0] | petir_bitboard_mask::fileMask[1]) | petir_bitboard_mask::fileMask[2]) | petir_bitboard_mask::fileMask[3]);
    petir_evalmask::filesEFGHMask = (((petir_bitboard_mask::fileMask[4] | petir_bitboard_mask::fileMask[5]) | petir_bitboard_mask::fileMask[6]) | petir_bitboard_mask::fileMask[7]);
    petir_evalmask::filesABMask = (petir_bitboard_mask::fileMask[0] | petir_bitboard_mask::fileMask[1]);
    petir_evalmask::filesGHMask = (petir_bitboard_mask::fileMask[6] | petir_bitboard_mask::fileMask[7]);
    petir_bitboard_mask::notFileAMask = (~petir_bitboard_mask::fileMask[0]);
    petir_bitboard_mask::notFileHMask = (~petir_bitboard_mask::fileMask[7]);
    pas::fillChar(petir_bitboard_mask::rankMask, sizeof(petir_bitboard_mask::rankMask), 0);
    lastFileSquare = 63;
    for (square = 0; square <= lastFileSquare; ++square) {
        file = (square % 8);
        if ((file == 0)) {
            petir_evalmask::adjacentFilesMask[square] = petir_bitboard_mask::fileMask[1];
        } else {
            if ((file == 7)) {
                petir_evalmask::adjacentFilesMask[square] = petir_bitboard_mask::fileMask[6];
            } else {
                petir_evalmask::adjacentFilesMask[square] = (petir_bitboard_mask::fileMask[pas::sub(file, 1)] | petir_bitboard_mask::fileMask[pas::add(file, 1)]);
            }
        }
        petir_bitboard_mask::rankMask[(square / 8)] = (petir_bitboard_mask::rankMask[(square / 8)] | petir_bitboard_mask::squareMask[square]);
    }
}

// evalmask.pas:77
std::int32_t chebyshevDistance(std::int32_t firstSquare, std::int32_t secondSquare) {
    std::int32_t result{};
    std::int32_t firstFile{};
    std::int32_t firstRank{};
    std::int32_t secondFile{};
    std::int32_t secondRank{};
    std::int32_t fileDistance{};
    std::int32_t rankDistance{};
    firstFile = (firstSquare & 7);
    secondFile = (secondSquare & 7);
    firstRank = pas::shiftRight(firstSquare, 3);
    secondRank = pas::shiftRight(secondSquare, 3);
    fileDistance = pas::abs(pas::sub(firstFile, secondFile));
    rankDistance = pas::abs(pas::sub(firstRank, secondRank));
    if ((fileDistance > rankDistance)) {
        result = fileDistance;
    } else {
        result = rankDistance;
    }
    return result;
}

// evalmask.pas:42
std::int32_t bishopDistance(std::int32_t firstSquare, std::int32_t secondSquare) {
    std::int32_t result{};
    std::int32_t firstFile{};
    std::int32_t firstRank{};
    std::int32_t secondFile{};
    std::int32_t secondRank{};
    std::int32_t sumDiagonalGap{};
    std::int32_t differenceDiagonalGap{};
    firstFile = (firstSquare % 8);
    secondFile = (secondSquare % 8);
    firstRank = (firstSquare / 8);
    secondRank = (secondSquare / 8);
    sumDiagonalGap = pas::abs(pas::sub(pas::add(firstFile, firstRank), pas::add(secondFile, secondRank)));
    differenceDiagonalGap = pas::abs(pas::sub(pas::sub(firstFile, firstRank), pas::sub(secondFile, secondRank)));
    if ((sumDiagonalGap < differenceDiagonalGap)) {
        result = sumDiagonalGap;
    } else {
        result = differenceDiagonalGap;
    }
    return result;
}

// evalmask.pas:64
std::int32_t closenessScore(std::int32_t firstSquare, std::int32_t secondSquare) {
    std::int32_t result{};
    std::int32_t firstFile{};
    std::int32_t firstRank{};
    std::int32_t secondFile{};
    std::int32_t secondRank{};
    std::int32_t fileDistance{};
    std::int32_t rankDistance{};
    firstFile = (firstSquare % 8);
    secondFile = (secondSquare % 8);
    firstRank = (firstSquare / 8);
    secondRank = (secondSquare / 8);
    fileDistance = pas::abs(pas::sub(firstFile, secondFile));
    rankDistance = pas::abs(pas::sub(firstRank, secondRank));
    result = pas::sub(10, pas::round(pas::sqrt(pas::add(pas::mul(fileDistance, fileDistance), pas::mul(rankDistance, rankDistance)))));
    return result;
}

// evalmask.pas:93
void initDistanceTables() {
    std::int32_t square{};
    std::int32_t otherSquare{};
    std::int32_t closeness{};
    std::int32_t lastSquare{};
    std::int32_t lastOtherSquare{};
    lastSquare = 63;
    for (square = 0; square <= lastSquare; ++square) {
        lastOtherSquare = 63;
        for (otherSquare = 0; otherSquare <= lastOtherSquare; ++otherSquare) {
            closeness = petir_evalmask::closenessScore(square, otherSquare);
            if ((closeness >= 5)) {
                petir_evalmask::closenessHalfTable[square][otherSquare] = (closeness / 2);
            } else {
                petir_evalmask::closenessHalfTable[square][otherSquare] = 0;
            }
            petir_evalmask::closenessTable[square][otherSquare] = closeness;
        }
    }
}

}
