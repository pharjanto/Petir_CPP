// Faithful translation of bitboard.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_bitboard {
// bitboard.pas:110
void initMasks() {
    std::int32_t index{};
    std::int32_t secondIndex{};
    std::int32_t file{};
    std::int32_t rank{};
    std::int32_t startCoordinate{};
    std::int32_t diagonalLastIndex{};
    std::int32_t diagonalPatternMax{};
    std::int32_t pathSquare{};
    std::int32_t raySquare{};
    std::int32_t paddingLength{};
    std::int32_t paddingPatternMax{};
    std::uint64_t mask{};
    std::int32_t lastSquare{};
    std::int32_t lastRaySquare{};
    std::int32_t lastDirectionFrom{};
    std::int32_t lastDirectionTo{};
    std::int32_t lastCodeFrom{};
    std::int32_t lastCodeTo{};
    std::int32_t lastPathFrom{};
    std::int32_t lastPathTo{};
    std::int32_t lastRotatedSquare{};
    std::int32_t lastKnightSquare{};
    std::int32_t lastKnightDirection{};
    std::int32_t lastKingSquare{};
    std::int32_t lastKingDirection{};
    std::int32_t lastSliderSquare{};
    std::int32_t lastOccupancy{};
    std::int32_t lastDiagonalSquare{};
    std::int32_t lastA8H1Pattern{};
    std::int32_t lastA8H1Padding{};
    std::int32_t lastH8A1Pattern{};
    std::int32_t lastH8A1Padding{};
    std::int32_t lastWordValue{};
    mask = 1;
    lastSquare = 63;
    for (index = 0; index <= lastSquare; ++index) {
        petir_bitboard_mask::squareMask[index] = mask;
        petir_bitboard_mask::squareMaskInverted[index] = (~mask);
        mask = (mask * 2);
    }
    lastRaySquare = 63;
    for (index = 0; index <= lastRaySquare; ++index) {
        raySquare = index;
        file = (raySquare % 8);
        petir_bitboard_mask::rookRayRightMask[index] = 0;
        while ((file < 7)) {
            (raySquare = pas::increment(raySquare, 1));
            petir_bitboard_mask::rookRayRightMask[index] = (petir_bitboard_mask::rookRayRightMask[index] | petir_bitboard_mask::squareMask[raySquare]);
            file = (raySquare % 8);
        }
        raySquare = index;
        file = (raySquare % 8);
        petir_bitboard_mask::rookRayLeftMask[index] = 0;
        while ((file > 0)) {
            (raySquare = pas::increment(raySquare, pas::negate(1)));
            petir_bitboard_mask::rookRayLeftMask[index] = (petir_bitboard_mask::rookRayLeftMask[index] | petir_bitboard_mask::squareMask[raySquare]);
            file = (raySquare % 8);
        }
        raySquare = index;
        file = (raySquare % 8);
        rank = (raySquare / 8);
        petir_bitboard_mask::bishopRayPlus9Mask[index] = 0;
        while (((file < 7) && (rank < 7))) {
            (raySquare = pas::increment(raySquare, 9));
            petir_bitboard_mask::bishopRayPlus9Mask[index] = (petir_bitboard_mask::bishopRayPlus9Mask[index] | petir_bitboard_mask::squareMask[raySquare]);
            file = (raySquare % 8);
            rank = (raySquare / 8);
        }
        raySquare = index;
        file = (raySquare % 8);
        rank = (raySquare / 8);
        petir_bitboard_mask::bishopRayPlus7Mask[index] = 0;
        while (((file > 0) && (rank < 7))) {
            (raySquare = pas::increment(raySquare, 7));
            petir_bitboard_mask::bishopRayPlus7Mask[index] = (petir_bitboard_mask::bishopRayPlus7Mask[index] | petir_bitboard_mask::squareMask[raySquare]);
            file = (raySquare % 8);
            rank = (raySquare / 8);
        }
        raySquare = index;
        file = (raySquare % 8);
        rank = (raySquare / 8);
        petir_bitboard_mask::bishopRayMinus9Mask[index] = 0;
        while (((file > 0) && (rank > 0))) {
            (raySquare = pas::increment(raySquare, pas::negate(9)));
            petir_bitboard_mask::bishopRayMinus9Mask[index] = (petir_bitboard_mask::bishopRayMinus9Mask[index] | petir_bitboard_mask::squareMask[raySquare]);
            file = (raySquare % 8);
            rank = (raySquare / 8);
        }
        raySquare = index;
        file = (index % 8);
        rank = (index / 8);
        petir_bitboard_mask::bishopRayMinus7Mask[index] = 0;
        while (((file < 7) && (rank > 0))) {
            (raySquare = pas::increment(raySquare, pas::negate(7)));
            petir_bitboard_mask::bishopRayMinus7Mask[index] = (petir_bitboard_mask::bishopRayMinus7Mask[index] | petir_bitboard_mask::squareMask[raySquare]);
            file = (raySquare % 8);
            rank = (raySquare / 8);
        }
        petir_bitboard_mask::bishopRaysUpMask[index] = (petir_bitboard_mask::bishopRayPlus7Mask[index] | petir_bitboard_mask::bishopRayPlus9Mask[index]);
        petir_bitboard_mask::bishopRaysDownMask[index] = (petir_bitboard_mask::bishopRayMinus7Mask[index] | petir_bitboard_mask::bishopRayMinus9Mask[index]);
    }
    lastDirectionFrom = 63;
    for (index = 0; index <= lastDirectionFrom; ++index) {
        lastDirectionTo = 63;
        for (secondIndex = 0; secondIndex <= lastDirectionTo; ++secondIndex) {
            petir_bitboard_mask::rayDirection[index][secondIndex] = 0;
            if (((index % 8) == (secondIndex % 8))) {
                if ((index > secondIndex)) {
                    petir_bitboard_mask::rayDirection[index][secondIndex] = pas::negate(8);
                } else {
                    if ((secondIndex > index)) {
                        petir_bitboard_mask::rayDirection[index][secondIndex] = 8;
                    }
                }
            }
            if (((index / 8) == (secondIndex / 8))) {
                if ((index > secondIndex)) {
                    petir_bitboard_mask::rayDirection[index][secondIndex] = pas::negate(1);
                } else {
                    if ((secondIndex > index)) {
                        petir_bitboard_mask::rayDirection[index][secondIndex] = 1;
                    }
                }
            }
            if ((pas::add((index / 8), (index % 8)) == pas::add((secondIndex / 8), (secondIndex % 8)))) {
                if ((index > secondIndex)) {
                    petir_bitboard_mask::rayDirection[index][secondIndex] = pas::negate(7);
                } else {
                    if ((secondIndex > index)) {
                        petir_bitboard_mask::rayDirection[index][secondIndex] = 7;
                    }
                }
            }
            if ((pas::sub((index / 8), (index % 8)) == pas::sub((secondIndex / 8), (secondIndex % 8)))) {
                if ((index > secondIndex)) {
                    petir_bitboard_mask::rayDirection[index][secondIndex] = pas::negate(9);
                } else {
                    if ((secondIndex > index)) {
                        petir_bitboard_mask::rayDirection[index][secondIndex] = 9;
                    }
                }
            }
        }
    }
    lastCodeFrom = 63;
    for (index = 0; index <= lastCodeFrom; ++index) {
        lastCodeTo = 63;
        for (secondIndex = 0; secondIndex <= lastCodeTo; ++secondIndex) {
            petir_bitboard_mask::rayDirectionCode[index][secondIndex] = 0;
            if ((petir_bitboard_mask::rayDirection[index][secondIndex] == 7)) {
                petir_bitboard_mask::rayDirectionCode[index][secondIndex] = 8;
            } else {
                if ((petir_bitboard_mask::rayDirection[index][secondIndex] == pas::negate(7))) {
                    petir_bitboard_mask::rayDirectionCode[index][secondIndex] = 7;
                } else {
                    if ((petir_bitboard_mask::rayDirection[index][secondIndex] == 9)) {
                        petir_bitboard_mask::rayDirectionCode[index][secondIndex] = 6;
                    } else {
                        if ((petir_bitboard_mask::rayDirection[index][secondIndex] == pas::negate(9))) {
                            petir_bitboard_mask::rayDirectionCode[index][secondIndex] = 5;
                        } else {
                            if ((petir_bitboard_mask::rayDirection[index][secondIndex] == 1)) {
                                petir_bitboard_mask::rayDirectionCode[index][secondIndex] = 4;
                            } else {
                                if ((petir_bitboard_mask::rayDirection[index][secondIndex] == pas::negate(1))) {
                                    petir_bitboard_mask::rayDirectionCode[index][secondIndex] = 3;
                                } else {
                                    if ((petir_bitboard_mask::rayDirection[index][secondIndex] == 8)) {
                                        petir_bitboard_mask::rayDirectionCode[index][secondIndex] = 2;
                                    } else {
                                        if ((petir_bitboard_mask::rayDirection[index][secondIndex] == pas::negate(8))) {
                                            petir_bitboard_mask::rayDirectionCode[index][secondIndex] = 1;
                                        } else {
                                            ;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    lastPathFrom = 63;
    for (index = 0; index <= lastPathFrom; ++index) {
        lastPathTo = 63;
        for (secondIndex = 0; secondIndex <= lastPathTo; ++secondIndex) {
            petir_bitboard_mask::betweenMask[index][secondIndex] = 0;
            if ((((index % 8) == (secondIndex % 8)) && (pas::abs(pas::sub(index, secondIndex)) >= 16))) {
                if ((secondIndex > index)) {
                    pathSquare = index;
                    raySquare = secondIndex;
                } else {
                    pathSquare = secondIndex;
                    raySquare = index;
                }
                (pathSquare = pas::increment(pathSquare, 8));
                while ((pathSquare < raySquare)) {
                    petir_bitboard_mask::betweenMask[index][secondIndex] = (petir_bitboard_mask::betweenMask[index][secondIndex] | petir_bitboard_mask::squareMask[pathSquare]);
                    (pathSquare = pas::increment(pathSquare, 8));
                }
            }
            if ((((index / 8) == (secondIndex / 8)) && (pas::abs(pas::sub(index, secondIndex)) >= 2))) {
                if ((secondIndex > index)) {
                    pathSquare = index;
                    raySquare = secondIndex;
                } else {
                    pathSquare = secondIndex;
                    raySquare = index;
                }
                (pathSquare = pas::increment(pathSquare, 1));
                while ((pathSquare < raySquare)) {
                    petir_bitboard_mask::betweenMask[index][secondIndex] = (petir_bitboard_mask::betweenMask[index][secondIndex] | petir_bitboard_mask::squareMask[pathSquare]);
                    (pathSquare = pas::increment(pathSquare, 1));
                }
            }
            if (((pas::add((index / 8), (index % 8)) == pas::add((secondIndex / 8), (secondIndex % 8))) && (pas::abs(pas::sub(index, secondIndex)) >= 14))) {
                if ((secondIndex > index)) {
                    pathSquare = index;
                    raySquare = secondIndex;
                } else {
                    pathSquare = secondIndex;
                    raySquare = index;
                }
                (pathSquare = pas::increment(pathSquare, 7));
                while ((pathSquare < raySquare)) {
                    petir_bitboard_mask::betweenMask[index][secondIndex] = (petir_bitboard_mask::betweenMask[index][secondIndex] | petir_bitboard_mask::squareMask[pathSquare]);
                    (pathSquare = pas::increment(pathSquare, 7));
                }
            }
            if (((pas::sub((index / 8), (index % 8)) == pas::sub((secondIndex / 8), (secondIndex % 8))) && (pas::abs(pas::sub(index, secondIndex)) >= 18))) {
                if ((secondIndex > index)) {
                    pathSquare = index;
                    raySquare = secondIndex;
                } else {
                    pathSquare = secondIndex;
                    raySquare = index;
                }
                (pathSquare = pas::increment(pathSquare, 9));
                while ((pathSquare < raySquare)) {
                    petir_bitboard_mask::betweenMask[index][secondIndex] = (petir_bitboard_mask::betweenMask[index][secondIndex] | petir_bitboard_mask::squareMask[pathSquare]);
                    (pathSquare = pas::increment(pathSquare, 9));
                }
            }
        }
    }
    lastRotatedSquare = 63;
    for (index = 0; index <= lastRotatedSquare; ++index) {
        petir_bitboard_mask::rotated90SquareMask[index] = petir_bitboard_mask::squareMask[petir_bitboard::rotated90Index[index]];
        petir_bitboard_mask::rotatedA8H1SquareMask[index] = petir_bitboard_mask::squareMask[petir_bitboard::rotatedA8H1Index[index]];
        petir_bitboard_mask::rotatedH8A1SquareMask[index] = petir_bitboard_mask::squareMask[petir_bitboard::rotatedH8A1Index[index]];
    }
    lastKnightSquare = 63;
    for (index = 0; index <= lastKnightSquare; ++index) {
        mask = 0;
        lastKnightDirection = 8;
        for (secondIndex = 1; secondIndex <= lastKnightDirection; ++secondIndex) {
            file = (index % 8);
            (file = pas::increment(file, petir_bitboard::knightFileDelta[secondIndex]));
            rank = (index / 8);
            (rank = pas::increment(rank, petir_bitboard::knightRankDelta[secondIndex]));
            if ((((file >= 0 && file <= 7)) && ((rank >= 0 && rank <= 7)))) {
                mask = (mask | petir_bitboard_mask::squareMask[pas::add(pas::mul(rank, 8), file)]);
            }
        }
        petir_bitboard_mask::knightAttacks[index] = mask;
    }
    lastKingSquare = 63;
    for (index = 0; index <= lastKingSquare; ++index) {
        mask = 0;
        lastKingDirection = 8;
        for (secondIndex = 1; secondIndex <= lastKingDirection; ++secondIndex) {
            file = (index % 8);
            (file = pas::increment(file, petir_bitboard::kingFileDelta[secondIndex]));
            rank = (index / 8);
            (rank = pas::increment(rank, petir_bitboard::kingRankDelta[secondIndex]));
            if ((((file >= 0 && file <= 7)) && ((rank >= 0 && rank <= 7)))) {
                mask = (mask | petir_bitboard_mask::squareMask[pas::add(pas::mul(rank, 8), file)]);
            }
        }
        petir_bitboard_mask::kingAttacks[index] = mask;
    }
    lastSliderSquare = 63;
    for (secondIndex = 0; secondIndex <= lastSliderSquare; ++secondIndex) {
        lastOccupancy = 255;
        for (index = 0; index <= lastOccupancy; ++index) {
            mask = 0;
            file = (secondIndex % 8);
            rank = (secondIndex / 8);
            startCoordinate = file;
            if ((file > 0)) {
                (file = pas::decrement(file, 1));
                while (((file >= 0) && ((index & petir_bitboard_mask::squareMask[file]) == 0))) {
                    mask = (mask | petir_bitboard_mask::squareMask[pas::sub(secondIndex, pas::sub(startCoordinate, file))]);
                    (file = pas::decrement(file, 1));
                }
                if ((file >= 0)) {
                    mask = (mask | petir_bitboard_mask::squareMask[pas::sub(secondIndex, pas::sub(startCoordinate, file))]);
                }
            }
            file = startCoordinate;
            if ((file < 7)) {
                (file = pas::increment(file, 1));
                while (((file <= 7) && ((index & petir_bitboard_mask::squareMask[file]) == 0))) {
                    mask = (mask | petir_bitboard_mask::squareMask[pas::add(secondIndex, pas::sub(file, startCoordinate))]);
                    (file = pas::increment(file, 1));
                }
                if ((file <= 7)) {
                    mask = (mask | petir_bitboard_mask::squareMask[pas::add(secondIndex, pas::sub(file, startCoordinate))]);
                }
            }
            petir_bitboard_mask::horizontalAttacks[secondIndex][(pas::shiftRight(index, 1) & 63)] = mask;
            mask = 0;
            startCoordinate = rank;
            if ((rank > 0)) {
                (rank = pas::decrement(rank, 1));
                while (((rank >= 0) && ((index & petir_bitboard_mask::squareMask[rank]) == 0))) {
                    mask = (mask | petir_bitboard_mask::squareMask[pas::sub(secondIndex, pas::mul(pas::sub(startCoordinate, rank), 8))]);
                    (rank = pas::decrement(rank, 1));
                }
                if ((rank >= 0)) {
                    mask = (mask | petir_bitboard_mask::squareMask[pas::sub(secondIndex, pas::mul(pas::sub(startCoordinate, rank), 8))]);
                }
            }
            rank = startCoordinate;
            if ((rank < 7)) {
                (rank = pas::increment(rank, 1));
                while (((rank <= 7) && ((index & petir_bitboard_mask::squareMask[rank]) == 0))) {
                    mask = (mask | petir_bitboard_mask::squareMask[pas::add(secondIndex, pas::mul(pas::sub(rank, startCoordinate), 8))]);
                    (rank = pas::increment(rank, 1));
                }
                if ((rank <= 7)) {
                    mask = (mask | petir_bitboard_mask::squareMask[pas::add(secondIndex, pas::mul(pas::sub(rank, startCoordinate), 8))]);
                }
            }
            petir_bitboard_mask::verticalAttacks[secondIndex][(pas::shiftRight(index, 1) & 63)] = mask;
        }
    }
    lastDiagonalSquare = petir_header::h8;
    for (secondIndex = petir_header::a1; secondIndex <= lastDiagonalSquare; ++secondIndex) {
        diagonalLastIndex = pas::sub(petir_bitboard::diagonalA8H1Length[secondIndex], 1);
        diagonalPatternMax = (petir_bitboard_mask::squareMask[pas::add(diagonalLastIndex, 1)] - 1);
        paddingLength = pas::sub(6, diagonalLastIndex);
        paddingPatternMax = (petir_bitboard_mask::squareMask[pas::add(paddingLength, 1)] - 1);
        lastA8H1Pattern = diagonalPatternMax;
        for (index = 0; index <= lastA8H1Pattern; ++index) {
            mask = 0;
            file = petir_bitboard::diagonalA8H1Position[secondIndex];
            startCoordinate = file;
            if ((file > 0)) {
                (file = pas::decrement(file, 1));
                while (((file >= 0) && ((index & petir_bitboard_mask::squareMask[file]) == 0))) {
                    mask = (mask | petir_bitboard_mask::squareMask[pas::sub(secondIndex, pas::mul(pas::sub(startCoordinate, file), 7))]);
                    (file = pas::decrement(file, 1));
                }
                if ((file >= 0)) {
                    mask = (mask | petir_bitboard_mask::squareMask[pas::sub(secondIndex, pas::mul(pas::sub(startCoordinate, file), 7))]);
                }
            }
            file = startCoordinate;
            if ((file < diagonalLastIndex)) {
                (file = pas::increment(file, 1));
                while (((file <= diagonalLastIndex) && ((index & petir_bitboard_mask::squareMask[file]) == 0))) {
                    mask = (mask | petir_bitboard_mask::squareMask[pas::add(secondIndex, pas::mul(pas::sub(file, startCoordinate), 7))]);
                    (file = pas::increment(file, 1));
                }
                if ((file <= diagonalLastIndex)) {
                    mask = (mask | petir_bitboard_mask::squareMask[pas::add(secondIndex, pas::mul(pas::sub(file, startCoordinate), 7))]);
                }
            }
            petir_bitboard_mask::diagonalA8H1Attacks[secondIndex][index] = mask;
            if ((paddingLength >= 0)) {
                lastA8H1Padding = paddingPatternMax;
                for (pathSquare = 0; pathSquare <= lastA8H1Padding; ++pathSquare) {
                    petir_bitboard_mask::diagonalA8H1Attacks[secondIndex][pas::add(pas::shiftLeft(pathSquare, pas::add(diagonalLastIndex, 1)), index)] = mask;
                }
            }
        }
        diagonalLastIndex = pas::sub(petir_bitboard::diagonalH8A1Length[secondIndex], 1);
        diagonalPatternMax = (petir_bitboard_mask::squareMask[pas::add(diagonalLastIndex, 1)] - 1);
        paddingLength = pas::sub(6, diagonalLastIndex);
        paddingPatternMax = (petir_bitboard_mask::squareMask[pas::add(paddingLength, 1)] - 1);
        lastH8A1Pattern = diagonalPatternMax;
        for (index = 0; index <= lastH8A1Pattern; ++index) {
            mask = 0;
            file = petir_bitboard::diagonalH8A1Position[secondIndex];
            startCoordinate = file;
            if ((file > 0)) {
                (file = pas::decrement(file, 1));
                while (((file >= 0) && ((index & petir_bitboard_mask::squareMask[pas::sub(diagonalLastIndex, file)]) == 0))) {
                    mask = (mask | petir_bitboard_mask::squareMask[pas::sub(secondIndex, pas::mul(pas::sub(startCoordinate, file), 9))]);
                    (file = pas::decrement(file, 1));
                }
                if ((file >= 0)) {
                    mask = (mask | petir_bitboard_mask::squareMask[pas::sub(secondIndex, pas::mul(pas::sub(startCoordinate, file), 9))]);
                }
            }
            file = startCoordinate;
            if ((file < diagonalLastIndex)) {
                (file = pas::increment(file, 1));
                while (((file <= diagonalLastIndex) && ((index & petir_bitboard_mask::squareMask[pas::sub(diagonalLastIndex, file)]) == 0))) {
                    mask = (mask | petir_bitboard_mask::squareMask[pas::add(secondIndex, pas::mul(pas::sub(file, startCoordinate), 9))]);
                    (file = pas::increment(file, 1));
                }
                if ((file <= diagonalLastIndex)) {
                    mask = (mask | petir_bitboard_mask::squareMask[pas::add(secondIndex, pas::mul(pas::sub(file, startCoordinate), 9))]);
                }
            }
            petir_bitboard_mask::diagonalH8A1Attacks[secondIndex][index] = mask;
            if ((paddingLength >= 0)) {
                lastH8A1Padding = paddingPatternMax;
                for (pathSquare = 0; pathSquare <= lastH8A1Padding; ++pathSquare) {
                    petir_bitboard_mask::diagonalH8A1Attacks[secondIndex][pas::add(pas::shiftLeft(pathSquare, pas::add(diagonalLastIndex, 1)), index)] = mask;
                }
            }
        }
    }
    lastWordValue = 65535;
    for (index = 1; index <= lastWordValue; ++index) {
        secondIndex = index;
        mask = 1;
        while (((secondIndex & 1) == 0)) {
            secondIndex = pas::shiftRight(secondIndex, 1);
            (mask = pas::increment(mask, 1));
        }
        petir_bitboard_mask::firstBitTable[index] = (mask - 1);
    }
    petir_bitboard_mask::firstBitTable[0] = 0;
}

}
