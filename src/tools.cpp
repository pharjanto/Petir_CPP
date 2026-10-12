// Faithful translation of tools.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_tools {
namespace {
// Empty-board geometry identifies potential pinners without occupancy lookups.
template<bool Diagonal>
constexpr std::array<std::uint64_t,64> makeSliderRays() {
    std::array<std::uint64_t,64> rays{};
    for(int fromSquare=0;fromSquare<64;++fromSquare)for(int toSquare=0;toSquare<64;++toSquare) {
        const int fileDelta=fromSquare%8-toSquare%8,rankDelta=fromSquare/8-toSquare/8;
        if(fromSquare!=toSquare && (Diagonal ? fileDelta*fileDelta==rankDelta*rankDelta : fileDelta==0 || rankDelta==0))
            rays[fromSquare]|=UINT64_C(1)<<toSquare;
    }
    return rays;
}
constexpr auto rookRays=makeSliderRays<false>();
constexpr auto bishopRays=makeSliderRays<true>();
}

// Used only for nonchecked move filtering. An unpinned nonking piece cannot
// expose its king by moving; en passant still needs the post-move check.
std::uint64_t pinnedPieces(const petir_header::tdata& position,int side) {
    using namespace petir_bitboard_mask;
    const bool isWhite=side==petir_header::whiteSide;
    const auto kingBoard=isWhite?position.whiteKingBoard:position.blackKingBoard;
    const int kingIndex=firstBit(pas::address(kingBoard));
    const auto enemyQueens=isWhite?position.blackQueens:position.whiteQueens;
    auto pinners=((enemyQueens|(isWhite?position.blackRooks:position.whiteRooks)) & rookRays[kingIndex])
                |((enemyQueens|(isWhite?position.blackBishops:position.whiteBishops)) & bishopRays[kingIndex]);
    std::uint64_t pins=0;
    while(pinners) {
        const int pinnerSquare=firstBit(pas::address(pinners));
        pinners &= pinners-1;
        const auto blockers=betweenMask[kingIndex][pinnerSquare] & position.allPieces;
        if(blockers && !(blockers & (blockers-1)))pins |= blockers;
    }
    return pins & (isWhite?position.whitePieces:position.blackPieces);
}

// tools.pas:1620
bool attackedByBlack(petir_header::tdata& position, std::int32_t square) {
    bool result{};
    std::uint64_t attackers{};
    if ((square > 63)) {
        result = false;
        return result;
    }
    if (((petir_bitboard_mask::knightAttacks[square] & position.blackKnights) != 0)) {
        result = true;
        return result;
    }
    attackers = petir_bitboard_mask::horizontalAttacks[square][(pas::shiftRight(position.allPieces, pas::add((square & 56), 1)) & 63)];
    attackers = (attackers | petir_bitboard_mask::verticalAttacks[square][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[square]) & 63)]);
    if (((attackers & (position.blackQueens | position.blackRooks)) != 0)) {
        result = true;
        return result;
    }
    attackers = petir_bitboard_mask::diagonalA8H1Attacks[square][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[square]) & 255)];
    attackers = (attackers | petir_bitboard_mask::diagonalH8A1Attacks[square][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[square]) & 255)]);
    if (((attackers & (position.blackQueens | position.blackBishops)) != 0)) {
        result = true;
        return result;
    }
    if (((((square & 7) > 0) && (square <= pas::sub(63, 7))) && (position.board[pas::add(square, 7)] == petir_header::blackPawn))) {
        result = true;
        return result;
    }
    if (((((square & 7) < 7) && (square <= pas::sub(63, 9))) && (position.board[pas::add(square, 9)] == petir_header::blackPawn))) {
        result = true;
        return result;
    }
    if (((petir_bitboard_mask::kingAttacks[square] & position.blackKingBoard) != 0)) {
        result = true;
        return result;
    }
    result = false;
    return result;
}

// tools.pas:1662
bool attackedByWhite(petir_header::tdata& position, std::int32_t square) {
    bool result{};
    std::uint64_t attackers{};
    if ((square > 63)) {
        result = false;
        return result;
    }
    if (((petir_bitboard_mask::knightAttacks[square] & position.whiteKnights) != 0)) {
        result = true;
        return result;
    }
    attackers = petir_bitboard_mask::horizontalAttacks[square][(pas::shiftRight(position.allPieces, pas::add((square & 56), 1)) & 63)];
    attackers = (attackers | petir_bitboard_mask::verticalAttacks[square][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[square]) & 63)]);
    if (((attackers & (position.whiteQueens | position.whiteRooks)) != 0)) {
        result = true;
        return result;
    }
    attackers = petir_bitboard_mask::diagonalA8H1Attacks[square][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[square]) & 255)];
    attackers = (attackers | petir_bitboard_mask::diagonalH8A1Attacks[square][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[square]) & 255)]);
    if (((attackers & (position.whiteQueens | position.whiteBishops)) != 0)) {
        result = true;
        return result;
    }
    if (((((square & 7) < 7) && (square > 6)) && (position.board[pas::sub(square, 7)] == petir_header::whitePawn))) {
        result = true;
        return result;
    }
    if (((((square & 7) > 0) && (square > 8)) && (position.board[pas::sub(square, 9)] == petir_header::whitePawn))) {
        result = true;
        return result;
    }
    if (((petir_bitboard_mask::kingAttacks[square] & position.whiteKingBoard) != 0)) {
        result = true;
        return result;
    }
    result = false;
    return result;
}

// tools.pas:1545
bool whiteInCheck(petir_header::tdata& position) {
    bool result{};
    std::uint64_t attackers{};
    std::int32_t kingSquare{};
    kingSquare = petir_bitboard_mask::kingSquare(position.whiteKingBoard);
    if (((petir_bitboard_mask::knightAttacks[kingSquare] & position.blackKnights) != 0)) {
        result = true;
        return result;
    }
    attackers = petir_bitboard_mask::horizontalAttacks[kingSquare][(pas::shiftRight(position.allPieces, pas::add((kingSquare & 56), 1)) & 63)];
    attackers = (attackers | petir_bitboard_mask::verticalAttacks[kingSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[kingSquare]) & 63)]);
    if (((attackers & (position.blackQueens | position.blackRooks)) != 0)) {
        result = true;
        return result;
    }
    attackers = petir_bitboard_mask::diagonalA8H1Attacks[kingSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[kingSquare]) & 255)];
    attackers = (attackers | petir_bitboard_mask::diagonalH8A1Attacks[kingSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[kingSquare]) & 255)]);
    if (((attackers & (position.blackQueens | position.blackBishops)) != 0)) {
        result = true;
        return result;
    }
    if (((petir_bitboard_mask::whitePawnAttacks[kingSquare] & position.blackPawns) != 0)) {
        result = true;
        return result;
    }
    if (((petir_bitboard_mask::kingAttacks[kingSquare] & position.blackKingBoard) != 0)) {
        result = true;
        return result;
    }
    result = false;
    return result;
}

// tools.pas:1581
bool blackInCheck(petir_header::tdata& position) {
    bool result{};
    std::uint64_t attackers{};
    std::int32_t kingSquare{};
    kingSquare = petir_bitboard_mask::kingSquare(position.blackKingBoard);
    if (((petir_bitboard_mask::knightAttacks[kingSquare] & position.whiteKnights) != 0)) {
        result = true;
        return result;
    }
    attackers = petir_bitboard_mask::horizontalAttacks[kingSquare][(pas::shiftRight(position.allPieces, pas::add((kingSquare & 56), 1)) & 63)];
    attackers = (attackers | petir_bitboard_mask::verticalAttacks[kingSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[kingSquare]) & 63)]);
    if (((attackers & (position.whiteQueens | position.whiteRooks)) != 0)) {
        result = true;
        return result;
    }
    attackers = petir_bitboard_mask::diagonalA8H1Attacks[kingSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[kingSquare]) & 255)];
    attackers = (attackers | petir_bitboard_mask::diagonalH8A1Attacks[kingSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[kingSquare]) & 255)]);
    if (((attackers & (position.whiteQueens | position.whiteBishops)) != 0)) {
        result = true;
        return result;
    }
    if (((petir_bitboard_mask::blackPawnAttacks[kingSquare] & position.whitePawns) != 0)) {
        result = true;
        return result;
    }
    if (((petir_bitboard_mask::kingAttacks[kingSquare] & position.whiteKingBoard) != 0)) {
        result = true;
        return result;
    }
    result = false;
    return result;
}

// tools.pas:1514
void clearKillers() {
    std::int32_t ply{};
    std::int32_t lastPly{};
    lastPly = 63;
    for (ply = 0; ply <= lastPly; ++ply) {
        petir_header::firstKillers[ply] = petir_header::noMove;
        petir_header::secondKillers[ply] = petir_header::noMove;
    }
}

// tools.pas:1326
bool exposesWhiteKing(petir_header::tdata& position, std::uint16_t move) {
    bool result{};
    std::int32_t direction{};
    std::int32_t fromSquare{};
    std::int32_t absoluteDirection{};
    std::int32_t toSquare{};
    std::int32_t kingSquare{};
    std::uint64_t occupancyWithoutMover{};
    fromSquare = (move & 127);
    kingSquare = petir_bitboard_mask::kingSquare(position.whiteKingBoard);
    direction = petir_bitboard_mask::rayDirection[kingSquare][fromSquare];
    if ((direction == 0)) {
        result = false;
        return result;
    }
    absoluteDirection = pas::abs(direction);
    toSquare = (pas::shiftRight(move, 7) & 127);
    if ((toSquare == petir_header::enPassantTarget)) {
        if ((position.enPassantFile > (fromSquare & 7))) {
            toSquare = pas::sub(fromSquare, 7);
        } else {
            toSquare = pas::sub(fromSquare, 9);
        }
    } else {
        if ((toSquare > 63)) {
            result = false;
            return result;
        }
    }
    if ((pas::abs(petir_bitboard_mask::rayDirection[fromSquare][toSquare]) == absoluteDirection)) {
        result = false;
        return result;
    }
    if ((absoluteDirection == 7)) {
        occupancyWithoutMover = (position.allPiecesA8H1 & (~petir_bitboard_mask::rotatedA8H1SquareMask[fromSquare]));
        if (((petir_bitboard_mask::diagonalA8H1Attacks[kingSquare][(pas::shiftRight(occupancyWithoutMover, petir_bitboard_mask::diagonalA8H1Shift[kingSquare]) & 255)] & (position.blackQueens | position.blackBishops)) != 0)) {
            result = true;
            return result;
        }
    } else {
        if ((absoluteDirection == 9)) {
            occupancyWithoutMover = (position.allPiecesH8A1 & (~petir_bitboard_mask::rotatedH8A1SquareMask[fromSquare]));
            if (((petir_bitboard_mask::diagonalH8A1Attacks[kingSquare][(pas::shiftRight(occupancyWithoutMover, petir_bitboard_mask::diagonalH8A1Shift[kingSquare]) & 255)] & (position.blackQueens | position.blackBishops)) != 0)) {
                result = true;
                return result;
            }
        } else {
            if ((absoluteDirection == 8)) {
                occupancyWithoutMover = (position.allPiecesRotated90 & (~petir_bitboard_mask::rotated90SquareMask[fromSquare]));
                if (((petir_bitboard_mask::verticalAttacks[kingSquare][(pas::shiftRight(occupancyWithoutMover, petir_bitboard_mask::verticalShift[kingSquare]) & 63)] & (position.blackQueens | position.blackRooks)) != 0)) {
                    result = true;
                    return result;
                }
            } else {
                if ((absoluteDirection == 1)) {
                    occupancyWithoutMover = (position.allPieces & (~petir_bitboard_mask::squareBit(fromSquare)));
                    if (((petir_bitboard_mask::horizontalAttacks[kingSquare][(pas::shiftRight(occupancyWithoutMover, pas::add((kingSquare & 56), 1)) & 63)] & (position.blackQueens | position.blackRooks)) != 0)) {
                        result = true;
                        return result;
                    }
                }
            }
        }
    }
    result = false;
    return result;
}

// tools.pas:1396
bool exposesWhiteKingFrom(petir_header::tdata& position, std::uint16_t fromSquare, std::uint16_t toSquare, std::uint16_t kingSquare) {
    bool result{};
    std::int32_t direction{};
    std::int32_t absoluteDirection{};
    std::uint64_t occupancyWithoutMover{};
    direction = petir_bitboard_mask::rayDirection[kingSquare][fromSquare];
    if ((direction == 0)) {
        result = false;
        return result;
    }
    absoluteDirection = pas::abs(direction);
    if ((pas::abs(petir_bitboard_mask::rayDirection[fromSquare][toSquare]) == absoluteDirection)) {
        result = false;
        return result;
    }
    if ((absoluteDirection == 7)) {
        occupancyWithoutMover = (position.allPiecesA8H1 & (~petir_bitboard_mask::rotatedA8H1SquareMask[fromSquare]));
        if (((petir_bitboard_mask::diagonalA8H1Attacks[kingSquare][(pas::shiftRight(occupancyWithoutMover, petir_bitboard_mask::diagonalA8H1Shift[kingSquare]) & 255)] & (position.whiteQueens | position.whiteBishops)) != 0)) {
            result = true;
            return result;
        }
    } else {
        if ((absoluteDirection == 9)) {
            occupancyWithoutMover = (position.allPiecesH8A1 & (~petir_bitboard_mask::rotatedH8A1SquareMask[fromSquare]));
            if (((petir_bitboard_mask::diagonalH8A1Attacks[kingSquare][(pas::shiftRight(occupancyWithoutMover, petir_bitboard_mask::diagonalH8A1Shift[kingSquare]) & 255)] & (position.whiteQueens | position.whiteBishops)) != 0)) {
                result = true;
                return result;
            }
        } else {
            if ((absoluteDirection == 8)) {
                occupancyWithoutMover = (position.allPiecesRotated90 & (~petir_bitboard_mask::rotated90SquareMask[fromSquare]));
                if (((petir_bitboard_mask::verticalAttacks[kingSquare][(pas::shiftRight(occupancyWithoutMover, petir_bitboard_mask::verticalShift[kingSquare]) & 63)] & (position.whiteQueens | position.whiteRooks)) != 0)) {
                    result = true;
                    return result;
                }
            } else {
                if ((absoluteDirection == 1)) {
                    occupancyWithoutMover = (position.allPieces & (~petir_bitboard_mask::squareBit(fromSquare)));
                    if (((petir_bitboard_mask::horizontalAttacks[kingSquare][(pas::shiftRight(occupancyWithoutMover, pas::add((kingSquare & 56), 1)) & 63)] & (position.whiteQueens | position.whiteRooks)) != 0)) {
                        result = true;
                        return result;
                    }
                }
            }
        }
    }
    result = false;
    return result;
}

// tools.pas:1454
bool exposesBlackKingFrom(petir_header::tdata& position, std::uint16_t fromSquare, std::uint16_t toSquare, std::uint16_t kingSquare) {
    bool result{};
    std::int32_t direction{};
    std::int32_t absoluteDirection{};
    std::uint64_t occupancyWithoutMover{};
    direction = petir_bitboard_mask::rayDirection[kingSquare][fromSquare];
    if ((direction == 0)) {
        result = false;
        return result;
    }
    absoluteDirection = pas::abs(direction);
    if ((pas::abs(petir_bitboard_mask::rayDirection[fromSquare][toSquare]) == absoluteDirection)) {
        result = false;
        return result;
    }
    if ((absoluteDirection == 7)) {
        occupancyWithoutMover = (position.allPiecesA8H1 & (~petir_bitboard_mask::rotatedA8H1SquareMask[fromSquare]));
        if (((petir_bitboard_mask::diagonalA8H1Attacks[kingSquare][(pas::shiftRight(occupancyWithoutMover, petir_bitboard_mask::diagonalA8H1Shift[kingSquare]) & 255)] & (position.blackQueens | position.blackBishops)) != 0)) {
            result = true;
            return result;
        }
    } else {
        if ((absoluteDirection == 9)) {
            occupancyWithoutMover = (position.allPiecesH8A1 & (~petir_bitboard_mask::rotatedH8A1SquareMask[fromSquare]));
            if (((petir_bitboard_mask::diagonalH8A1Attacks[kingSquare][(pas::shiftRight(occupancyWithoutMover, petir_bitboard_mask::diagonalH8A1Shift[kingSquare]) & 255)] & (position.blackQueens | position.blackBishops)) != 0)) {
                result = true;
                return result;
            }
        } else {
            if ((absoluteDirection == 8)) {
                occupancyWithoutMover = (position.allPiecesRotated90 & (~petir_bitboard_mask::rotated90SquareMask[fromSquare]));
                if (((petir_bitboard_mask::verticalAttacks[kingSquare][(pas::shiftRight(occupancyWithoutMover, petir_bitboard_mask::verticalShift[kingSquare]) & 63)] & (position.blackQueens | position.blackRooks)) != 0)) {
                    result = true;
                    return result;
                }
            } else {
                if ((absoluteDirection == 1)) {
                    occupancyWithoutMover = (position.allPieces & (~petir_bitboard_mask::squareBit(fromSquare)));
                    if (((petir_bitboard_mask::horizontalAttacks[kingSquare][(pas::shiftRight(occupancyWithoutMover, pas::add((kingSquare & 56), 1)) & 63)] & (position.blackQueens | position.blackRooks)) != 0)) {
                        result = true;
                        return result;
                    }
                }
            }
        }
    }
    result = false;
    return result;
}

// tools.pas:1253
bool exposesBlackKing(petir_header::tdata& position, std::uint16_t move) {
    bool result{};
    std::int32_t direction{};
    std::int32_t fromSquare{};
    std::int32_t absoluteDirection{};
    std::int32_t toSquare{};
    std::int32_t kingSquare{};
    std::uint64_t occupancyWithoutMover{};
    fromSquare = (move & 127);
    kingSquare = petir_bitboard_mask::kingSquare(position.blackKingBoard);
    direction = petir_bitboard_mask::rayDirection[kingSquare][fromSquare];
    if ((direction == 0)) {
        result = false;
        return result;
    }
    toSquare = (pas::shiftRight(move, 7) & 127);
    if ((toSquare == petir_header::enPassantTarget)) {
        if ((position.enPassantFile > (fromSquare & 7))) {
            toSquare = pas::add(fromSquare, 9);
        } else {
            toSquare = pas::add(fromSquare, 7);
        }
    } else {
        if ((toSquare > 63)) {
            result = false;
            return result;
        }
    }
    absoluteDirection = pas::abs(direction);
    if ((pas::abs(petir_bitboard_mask::rayDirection[fromSquare][toSquare]) == absoluteDirection)) {
        result = false;
        return result;
    }
    if ((absoluteDirection == 7)) {
        occupancyWithoutMover = (position.allPiecesA8H1 & (~petir_bitboard_mask::rotatedA8H1SquareMask[fromSquare]));
        if (((petir_bitboard_mask::diagonalA8H1Attacks[kingSquare][(pas::shiftRight(occupancyWithoutMover, petir_bitboard_mask::diagonalA8H1Shift[kingSquare]) & 255)] & (position.whiteQueens | position.whiteBishops)) != 0)) {
            result = true;
            return result;
        }
    } else {
        if ((absoluteDirection == 9)) {
            occupancyWithoutMover = (position.allPiecesH8A1 & (~petir_bitboard_mask::rotatedH8A1SquareMask[fromSquare]));
            if (((petir_bitboard_mask::diagonalH8A1Attacks[kingSquare][(pas::shiftRight(occupancyWithoutMover, petir_bitboard_mask::diagonalH8A1Shift[kingSquare]) & 255)] & (position.whiteQueens | position.whiteBishops)) != 0)) {
                result = true;
                return result;
            }
        } else {
            if ((absoluteDirection == 8)) {
                occupancyWithoutMover = (position.allPiecesRotated90 & (~petir_bitboard_mask::rotated90SquareMask[fromSquare]));
                if (((petir_bitboard_mask::verticalAttacks[kingSquare][(pas::shiftRight(occupancyWithoutMover, petir_bitboard_mask::verticalShift[kingSquare]) & 63)] & (position.whiteQueens | position.whiteRooks)) != 0)) {
                    result = true;
                    return result;
                }
            } else {
                if ((absoluteDirection == 1)) {
                    occupancyWithoutMover = (position.allPieces & (~petir_bitboard_mask::squareBit(fromSquare)));
                    if (((petir_bitboard_mask::horizontalAttacks[kingSquare][(pas::shiftRight(occupancyWithoutMover, pas::add((kingSquare & 56), 1)) & 63)] & (position.whiteQueens | position.whiteRooks)) != 0)) {
                        result = true;
                        return result;
                    }
                }
            }
        }
    }
    result = false;
    return result;
}

// tools.pas:1203
bool extendPassedPawnPush(petir_header::tdata& position, std::int32_t& newDepth, std::int32_t sideToMove, std::int32_t move, std::int32_t& extensionCredit) {
    bool result{};
    std::uint8_t toSquare{};
    result = false;
    toSquare = (move & 127);
    if ((sideToMove == petir_header::blackSide)) {
        toSquare = pas::sub(63, toSquare);
    }
    if ((toSquare <= petir_header::h4)) {
        return result;
    }
    if ((toSquare <= petir_header::h5)) {
        if (((position.whiteNonPawnMaterial > 15) && (position.blackNonPawnMaterial > 15))) {
            return result;
        }
        if ((newDepth > 24)) {
            return result;
        }
        if ((sideToMove == petir_header::whiteSide)) {
            if (((position.board[pas::add(toSquare, 16)] != 0) || (position.board[pas::add(toSquare, 24)] != 0))) {
                return result;
            }
            if ((((toSquare & 7) < 7) && (position.board[pas::add(toSquare, 17)] == petir_header::blackPawn))) {
                return result;
            }
            if ((((toSquare & 7) > 0) && (position.board[pas::add(toSquare, 15)] == petir_header::blackPawn))) {
                return result;
            }
            result = true;
            (newDepth = pas::increment(newDepth, 8));
            (extensionCredit = pas::decrement(extensionCredit, 8));
            return result;
        } else {
            toSquare = (move & 127);
            if (((position.board[pas::sub(toSquare, 16)] != 0) || (position.board[pas::sub(toSquare, 24)] != 0))) {
                return result;
            }
            if ((((toSquare & 7) < 7) && (position.board[pas::sub(toSquare, 15)] == petir_header::whitePawn))) {
                return result;
            }
            if ((((toSquare & 7) > 0) && (position.board[pas::sub(toSquare, 17)] == petir_header::whitePawn))) {
                return result;
            }
            result = true;
            (newDepth = pas::increment(newDepth, 8));
            (extensionCredit = pas::decrement(extensionCredit, 8));
            return result;
        }
        return result;
    }
    if (((position.whiteNonPawnMaterial <= 12) && (position.blackNonPawnMaterial <= 12))) {
        (newDepth = pas::increment(newDepth, 8));
        (extensionCredit = pas::decrement(extensionCredit, 8));
        result = true;
        return result;
    } else {
        if (((position.whiteNonPawnMaterial <= 17) && (position.blackNonPawnMaterial <= 17))) {
            (newDepth = pas::increment(newDepth, 8));
            (extensionCredit = pas::decrement(extensionCredit, 8));
            result = true;
            return result;
        } else {
            (newDepth = pas::increment(newDepth, petir_header::pawnPushExtensionMiddlegame));
            (extensionCredit = pas::decrement(extensionCredit, petir_header::pawnPushExtensionMiddlegame));
            result = true;
            return result;
        }
    }
    return result;
}

// tools.pas:1179
bool isWinningMaterial(std::int32_t materialScore, std::uint8_t sideToMove) {
    bool result{};
    result = false;
    if (((sideToMove == petir_header::blackSide) && (petir_header::gamePosition.materialScore <= pas::negate(petir_header::rookValue)))) {
        result = true;
    } else {
        if (((sideToMove == petir_header::whiteSide) && (petir_header::gamePosition.materialScore >= petir_header::rookValue))) {
            result = true;
        }
    }
    return result;
}

// tools.pas:1188
bool isLosingMaterial(std::int32_t materialScore, std::uint8_t sideToMove) {
    bool result{};
    result = false;
    if (((sideToMove == petir_header::blackSide) && (petir_header::gamePosition.materialScore >= petir_header::rookValue))) {
        result = true;
    } else {
        if (((sideToMove == petir_header::whiteSide) && (petir_header::gamePosition.materialScore <= pas::negate(petir_header::rookValue)))) {
            result = true;
        }
    }
    return result;
}

// tools.pas:1160
bool opponentInCheck(petir_header::tdata& position, std::int32_t sideToMove) {
    bool result{};
    std::int32_t side{};
    result = false;
    side = sideToMove;
    if (((side == petir_header::whiteSide))) {
        if (petir_tools::blackInCheck(position)) {
            result = true;
        }
    }
    else if (((side == petir_header::blackSide))) {
        if (petir_tools::whiteInCheck(position)) {
            result = true;
        }
    }
    return result;
}

// tools.pas:1169
bool moveGivesCheck(petir_header::tdata& position, std::uint16_t sideToMove, std::uint16_t move) {
    bool result{};
    std::int32_t side{};
    result = false;
    side = sideToMove;
    if (((side == petir_header::whiteSide))) {
        if ((petir_tools::whiteMoveGivesCheck(position, move) || petir_tools::exposesBlackKing(position, move))) {
            result = true;
        }
    }
    else if (((side == petir_header::blackSide))) {
        if ((petir_tools::blackMoveGivesCheck(position, move) || petir_tools::exposesWhiteKing(position, move))) {
            result = true;
        }
    }
    return result;
}

// tools.pas:1197
bool isPawnPush(petir_header::tdata& position, std::uint16_t sideToMove, std::uint16_t move) {
    bool result{};
    return result;
}

// tools.pas:280
bool isCapture(petir_header::tdata& position, std::uint16_t move) {
    bool result{};
    std::uint8_t toSquare{};
    result = true;
    toSquare = (pas::shiftRight(move, 7) & 127);
    if ((toSquare == petir_header::enPassantTarget)) {
        return result;
    }
    if (((toSquare > 63) || (position.board[toSquare] == 0))) {
        result = false;
    }
    return result;
}

// tools.pas:797
bool whiteForksOrWinsPawn(std::int32_t square, petir_header::tdata& position) {
    bool result{};
    std::uint64_t targets{};
    std::int32_t targetSquare{};
    std::int32_t attackedCount{};
    std::int32_t movedPiece{};
    result = false;
    if ((square > 63)) {
        return result;
    }
    movedPiece = position.board[square];
    if (((movedPiece == petir_header::whitePawn))) {
        attackedCount = 0;
        if ((((square & 7) > 0) && (position.board[pas::add(square, 7)] < pas::negate(1)))) {
            (attackedCount = pas::increment(attackedCount, 1));
        } else {
            return result;
        }
        if ((((square & 7) < 7) && (position.board[pas::add(square, 9)] < pas::negate(1)))) {
            (attackedCount = pas::increment(attackedCount, 1));
        }
        result = (attackedCount == 2);
    }
    else if (((movedPiece == petir_header::whiteKnight))) {
        targets = (petir_bitboard_mask::knightAttacks[square] & position.blackPieces);
        attackedCount = 0;
        while ((targets != 0)) {
            targetSquare = petir_bitboard_mask::firstBit(pas::address(targets));
            targets = (targets & petir_bitboard_mask::squareBitInverted(targetSquare));
            if ((position.board[targetSquare] <= petir_header::blackRook)) {
                (attackedCount = pas::increment(attackedCount, 1));
            }
            if ((((position.board[targetSquare] == petir_header::blackPawn) && (position.board[pas::sub(targetSquare, 8)] == petir_header::whitePawn)) && (petir_usee::staticExchange(petir_header::whiteSide, position, square, targetSquare) > 0))) {
                result = true;
                return result;
            }
        }
        result = (attackedCount >= 2);
    }
    else if (((movedPiece == petir_header::whiteBishop))) {
        if ((position.blackNonPawnMaterial <= 17)) {
            targets = petir_bitboard_mask::diagonalA8H1Attacks[square][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[square]) & 255)];
            targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[square][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[square]) & 255)]);
            targets = (targets & position.blackPawns);
            while ((targets != 0)) {
                targetSquare = petir_bitboard_mask::firstBit(pas::address(targets));
                if (((position.board[pas::sub(targetSquare, 8)] == petir_header::whitePawn) && (petir_usee::staticExchange(petir_header::whiteSide, position, square, targetSquare) > 0))) {
                    result = true;
                    return result;
                }
                targets = (targets & petir_bitboard_mask::squareBitInverted(targetSquare));
            }
        }
    }
    else if (((movedPiece == petir_header::whiteRook))) {
        if ((position.blackNonPawnMaterial <= 17)) {
            targets = petir_bitboard_mask::horizontalAttacks[square][(pas::shiftRight(position.allPieces, pas::add((square & 56), 1)) & 63)];
            targets = (targets | petir_bitboard_mask::verticalAttacks[square][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[square]) & 63)]);
            targets = (targets & position.blackPawns);
            while ((targets != 0)) {
                targetSquare = petir_bitboard_mask::firstBit(pas::address(targets));
                if (((position.board[pas::sub(targetSquare, 8)] == petir_header::whitePawn) && (petir_usee::staticExchange(petir_header::whiteSide, position, square, targetSquare) > 0))) {
                    result = true;
                    return result;
                }
                targets = (targets & petir_bitboard_mask::squareBitInverted(targetSquare));
            }
        }
    }
    else if (((movedPiece == petir_header::whiteKing))) {
        if ((position.blackNonPawnMaterial <= 3)) {
            targets = (petir_bitboard_mask::kingAttacks[square] & position.blackPawns);
            while ((targets != 0)) {
                targetSquare = petir_bitboard_mask::firstBit(pas::address(targets));
                targets = (targets & petir_bitboard_mask::squareBitInverted(targetSquare));
                if ((((petir_bitboard_mask::kingAttacks[targetSquare] & position.blackKingBoard) == 0) && ((petir_bitboard_mask::whitePawnAttacks[targetSquare] & position.blackPawns) == 0))) {
                    result = true;
                    return result;
                }
            }
        }
    }
    return result;
}

// tools.pas:890
bool blackForksOrWinsPawn(std::int32_t square, petir_header::tdata& position) {
    bool result{};
    std::uint64_t targets{};
    std::int32_t targetSquare{};
    std::int32_t attackedCount{};
    std::int32_t movedPiece{};
    result = false;
    if ((square > 63)) {
        return result;
    }
    movedPiece = position.board[square];
    if (((movedPiece == petir_header::blackPawn))) {
        attackedCount = 0;
        if ((((square & 7) > 0) && (position.board[pas::sub(square, 9)] > 1))) {
            (attackedCount = pas::increment(attackedCount, 1));
        } else {
            return result;
        }
        if ((((square & 7) < 7) && (position.board[pas::sub(square, 7)] > 1))) {
            (attackedCount = pas::increment(attackedCount, 1));
        } else {
            return result;
        }
        result = (attackedCount == 2);
    }
    else if (((movedPiece == petir_header::blackKnight))) {
        targets = (petir_bitboard_mask::knightAttacks[square] & position.whitePieces);
        attackedCount = 0;
        while ((targets != 0)) {
            targetSquare = petir_bitboard_mask::firstBit(pas::address(targets));
            targets = (targets & petir_bitboard_mask::squareBitInverted(targetSquare));
            if ((position.board[targetSquare] >= petir_header::whiteRook)) {
                (attackedCount = pas::increment(attackedCount, 1));
            }
            if ((((position.board[targetSquare] == petir_header::whitePawn) && (position.board[pas::add(targetSquare, 8)] == petir_header::blackPawn)) && (petir_usee::staticExchange(petir_header::blackSide, position, square, targetSquare) > 0))) {
                result = true;
                return result;
            }
        }
        result = (attackedCount >= 2);
    }
    else if (((movedPiece == petir_header::blackBishop))) {
        if ((position.whiteNonPawnMaterial <= 17)) {
            targets = petir_bitboard_mask::diagonalA8H1Attacks[square][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[square]) & 255)];
            targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[square][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[square]) & 255)]);
            targets = (targets & position.whitePawns);
            while ((targets != 0)) {
                targetSquare = petir_bitboard_mask::firstBit(pas::address(targets));
                if (((position.board[pas::add(targetSquare, 8)] == petir_header::blackPawn) && (petir_usee::staticExchange(petir_header::blackSide, position, square, targetSquare) > 0))) {
                    result = true;
                    return result;
                }
                targets = (targets & petir_bitboard_mask::squareBitInverted(targetSquare));
            }
        }
    }
    else if (((movedPiece == petir_header::blackRook))) {
        if ((position.whiteNonPawnMaterial <= 17)) {
            targets = petir_bitboard_mask::horizontalAttacks[square][(pas::shiftRight(position.allPieces, pas::add((square & 56), 1)) & 63)];
            targets = (targets | petir_bitboard_mask::verticalAttacks[square][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[square]) & 63)]);
            targets = (targets & position.whitePawns);
            while ((targets != 0)) {
                targetSquare = petir_bitboard_mask::firstBit(pas::address(targets));
                if (((position.board[pas::add(targetSquare, 8)] == petir_header::blackPawn) && (petir_usee::staticExchange(petir_header::blackSide, position, square, targetSquare) > 0))) {
                    result = true;
                    return result;
                }
                targets = (targets & petir_bitboard_mask::squareBitInverted(targetSquare));
            }
        }
    }
    else if (((movedPiece == petir_header::blackKing))) {
        if ((position.whiteNonPawnMaterial <= 3)) {
            targets = (petir_bitboard_mask::kingAttacks[square] & position.whitePawns);
            while ((targets != 0)) {
                targetSquare = petir_bitboard_mask::firstBit(pas::address(targets));
                targets = (targets & petir_bitboard_mask::squareBitInverted(targetSquare));
                if ((((petir_bitboard_mask::kingAttacks[targetSquare] & position.whiteKingBoard) == 0) && ((petir_bitboard_mask::blackPawnAttacks[targetSquare] & position.whitePawns) == 0))) {
                    result = true;
                    return result;
                }
            }
        }
    }
    return result;
}

// tools.pas:268
std::int32_t capturedValue(std::int32_t toSquare, petir_header::tdata& position) {
    std::int32_t result{};
    if ((position.lastCaptureType == 0)) {
        result = 0;
    } else {
        if ((toSquare == petir_header::enPassantTarget)) {
            result = petir_header::pawnValue;
        } else {
            if ((toSquare >= petir_header::shortCastleTarget)) {
                result = 0;
            } else {
                result = petir_header::pieceValue[position.board[toSquare]];
            }
        }
        (result = pas::increment(result, 50));
    }
    return result;
}

// tools.pas:497
bool whiteMoveThreatensKing(petir_header::tdata& position, std::uint16_t move, std::int32_t& threatenedValue) {
    bool result{};
    std::uint64_t attackedSquares{};
    std::uint64_t kingZone{};
    std::uint64_t occupancyWithoutSlider{};
    std::uint8_t toSquare{};
    std::uint8_t kingSquare{};
    std::int32_t square{};
    std::int32_t file{};
    std::int32_t rank{};
    std::int32_t movedPiece{};
    threatenedValue = 0;
    kingSquare = petir_bitboard_mask::kingSquare(position.blackKingBoard);
    toSquare = (pas::shiftRight(move, 7) & 127);
    if ((toSquare > 63)) {
        if (((toSquare == petir_header::shortCastleTarget) || (toSquare == petir_header::longCastleTarget))) {
            threatenedValue = 50;
            result = false;
            return result;
        }
        result = true;
        return result;
    }
    result = false;
    attackedSquares = 0;
    kingZone = (petir_bitboard_mask::kingAttacks[kingSquare] | petir_bitboard_mask::squareBit(kingSquare));
    movedPiece = position.board[toSquare];
    if (((movedPiece == petir_header::whitePawn))) {
        if ((toSquare >= petir_header::a7)) {
            result = true;
            return result;
        }
        attackedSquares = petir_bitboard_mask::whitePawnAttacks[toSquare];
    }
    else if (((movedPiece == petir_header::whiteKnight))) {
        attackedSquares = petir_bitboard_mask::knightAttacks[toSquare];
    }
    else if (((movedPiece == petir_header::whiteBishop))) {
        attackedSquares = 0;
        file = (toSquare & 7);
        rank = pas::shiftRight(toSquare, 3);
        (file = pas::increment(file, 1));
        (rank = pas::increment(rank, 1));
        while (((((file <= 7) && (rank <= 7)) && (file >= 0)) && (rank >= 0))) {
            square = pas::add(pas::shiftLeft(rank, 3), file);
            attackedSquares = (attackedSquares | petir_bitboard_mask::squareBit(square));
            if ((position.board[square] == petir_header::whitePawn)) {
                (file = pas::increment(file, 1));
                (rank = pas::increment(rank, 1));
                square = pas::add(pas::shiftLeft(rank, 3), file);
                attackedSquares = (attackedSquares | petir_bitboard_mask::squareBit(square));
                break;
            }
            if ((position.board[square] == petir_header::whiteQueen)) {
                (file = pas::increment(file, 1));
                (rank = pas::increment(rank, 1));
                continue;
            }
            if ((position.board[square] != 0)) {
                break;
            }
            (file = pas::increment(file, 1));
            (rank = pas::increment(rank, 1));
        }
        file = (toSquare & 7);
        rank = pas::shiftRight(toSquare, 3);
        (file = pas::decrement(file, 1));
        (rank = pas::increment(rank, 1));
        while (((((file <= 7) && (rank <= 7)) && (file >= 0)) && (rank >= 0))) {
            square = pas::add(pas::shiftLeft(rank, 3), file);
            attackedSquares = (attackedSquares | petir_bitboard_mask::squareBit(square));
            if ((position.board[square] == petir_header::whitePawn)) {
                (file = pas::decrement(file, 1));
                (rank = pas::increment(rank, 1));
                square = pas::add(pas::shiftLeft(rank, 3), file);
                attackedSquares = (attackedSquares | petir_bitboard_mask::squareBit(square));
                break;
            }
            if ((position.board[square] == petir_header::whiteQueen)) {
                (file = pas::decrement(file, 1));
                (rank = pas::increment(rank, 1));
                continue;
            }
            if ((position.board[square] != 0)) {
                break;
            }
            (file = pas::decrement(file, 1));
            (rank = pas::increment(rank, 1));
        }
        file = (toSquare & 7);
        rank = pas::shiftRight(toSquare, 3);
        (file = pas::increment(file, 1));
        (rank = pas::decrement(rank, 1));
        while (((((file <= 7) && (rank <= 7)) && (file >= 0)) && (rank >= 0))) {
            square = pas::add(pas::shiftLeft(rank, 3), file);
            attackedSquares = (attackedSquares | petir_bitboard_mask::squareBit(square));
            if ((position.board[square] == petir_header::whiteQueen)) {
                (file = pas::increment(file, 1));
                (rank = pas::decrement(rank, 1));
                continue;
            }
            if ((position.board[square] != 0)) {
                break;
            }
            (file = pas::increment(file, 1));
            (rank = pas::decrement(rank, 1));
        }
        file = (toSquare & 7);
        rank = pas::shiftRight(toSquare, 3);
        (file = pas::decrement(file, 1));
        (rank = pas::decrement(rank, 1));
        while (((((file <= 7) && (rank <= 7)) && (file >= 0)) && (rank >= 0))) {
            square = pas::add(pas::shiftLeft(rank, 3), file);
            attackedSquares = (attackedSquares | petir_bitboard_mask::squareBit(square));
            if ((position.board[square] == petir_header::whiteQueen)) {
                (file = pas::decrement(file, 1));
                (rank = pas::decrement(rank, 1));
                continue;
            }
            if ((position.board[square] != 0)) {
                break;
            }
            (file = pas::decrement(file, 1));
            (rank = pas::decrement(rank, 1));
        }
    }
    else if (((movedPiece == petir_header::whiteRook))) {
        occupancyWithoutSlider = (position.allPieces & (~position.whiteQueens));
        attackedSquares = petir_bitboard_mask::horizontalAttacks[toSquare][(pas::shiftRight(occupancyWithoutSlider, pas::add((toSquare & 56), 1)) & 63)];
        square = pas::add(toSquare, 8);
        while ((square <= 63)) {
            attackedSquares = (attackedSquares | petir_bitboard_mask::squareBit(square));
            if (((position.board[square] == petir_header::whiteRook) || (position.board[square] == petir_header::whiteQueen))) {
                square = pas::add(square, 8);
                continue;
            }
            if ((position.board[square] != 0)) {
                break;
            }
            square = pas::add(square, 8);
        }
        square = pas::sub(toSquare, 8);
        while ((square >= 0)) {
            attackedSquares = (attackedSquares | petir_bitboard_mask::squareBit(square));
            if (((position.board[square] == petir_header::whiteRook) || (position.board[square] == petir_header::whiteQueen))) {
                square = pas::sub(square, 8);
                continue;
            }
            if ((position.board[square] != 0)) {
                break;
            }
            square = pas::sub(square, 8);
        }
    }
    else if (((movedPiece == petir_header::whiteQueen))) {
        attackedSquares = petir_bitboard_mask::diagonalA8H1Attacks[toSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[toSquare]) & 255)];
        attackedSquares = (attackedSquares | petir_bitboard_mask::diagonalH8A1Attacks[toSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[toSquare]) & 255)]);
        occupancyWithoutSlider = (position.allPieces & (~position.whiteRooks));
        attackedSquares = (attackedSquares | petir_bitboard_mask::horizontalAttacks[toSquare][(pas::shiftRight(occupancyWithoutSlider, pas::add((toSquare & 56), 1)) & 63)]);
        square = pas::add(toSquare, 8);
        while ((square <= 63)) {
            attackedSquares = (attackedSquares | petir_bitboard_mask::squareBit(square));
            if (((position.board[square] == petir_header::whiteRook) || (position.board[square] == petir_header::whiteQueen))) {
                square = pas::add(square, 8);
                continue;
            }
            if ((position.board[square] != 0)) {
                break;
            }
            square = pas::add(square, 8);
        }
        square = pas::sub(toSquare, 8);
        while ((square >= 0)) {
            attackedSquares = (attackedSquares | petir_bitboard_mask::squareBit(square));
            if (((position.board[square] == petir_header::whiteRook) || (position.board[square] == petir_header::whiteQueen))) {
                square = pas::sub(square, 8);
                continue;
            }
            if ((position.board[square] != 0)) {
                break;
            }
            square = pas::sub(square, 8);
        }
    }
    else if (((movedPiece == petir_header::whiteKing))) {
        attackedSquares = petir_bitboard_mask::kingAttacks[toSquare];
    }
    threatenedValue = 0;
    if (((attackedSquares & kingZone) != 0)) {
        result = true;
        return result;
    }
    if (((attackedSquares & position.blackQueens) != 0)) {
        threatenedValue = petir_header::queenValue;
    } else {
        if (((attackedSquares & position.blackRooks) != 0)) {
            threatenedValue = petir_header::rookValue;
        } else {
            if (((attackedSquares & position.blackBishops) != 0)) {
                threatenedValue = petir_header::bishopValue;
            } else {
                if (((attackedSquares & position.blackKnights) != 0)) {
                    threatenedValue = petir_header::knightValue;
                } else {
                    if (((attackedSquares & position.blackPawns) != 0)) {
                        threatenedValue = petir_header::pawnValue;
                    }
                }
            }
        }
    }
    return result;
}

// tools.pas:298
bool blackMoveThreatensKing(petir_header::tdata& position, std::uint16_t move, std::int32_t& threatenedValue) {
    bool result{};
    std::uint64_t attackedSquares{};
    std::uint64_t kingZone{};
    std::uint64_t occupancyWithoutSlider{};
    std::uint8_t toSquare{};
    std::uint8_t kingSquare{};
    std::int32_t square{};
    std::int32_t file{};
    std::int32_t rank{};
    std::int32_t movedPiece{};
    threatenedValue = 0;
    kingSquare = petir_bitboard_mask::kingSquare(position.whiteKingBoard);
    toSquare = (pas::shiftRight(move, 7) & 127);
    if ((toSquare > 63)) {
        if (((toSquare == petir_header::shortCastleTarget) || (toSquare == petir_header::longCastleTarget))) {
            threatenedValue = 50;
            result = false;
            return result;
        }
        result = true;
        return result;
    }
    result = false;
    attackedSquares = 0;
    kingZone = (petir_bitboard_mask::kingAttacks[kingSquare] | petir_bitboard_mask::squareBit(kingSquare));
    movedPiece = position.board[toSquare];
    if (((movedPiece == petir_header::blackPawn))) {
        if ((toSquare <= petir_header::h2)) {
            result = true;
            return result;
        }
        attackedSquares = petir_bitboard_mask::blackPawnAttacks[toSquare];
    }
    else if (((movedPiece == petir_header::blackKnight))) {
        attackedSquares = petir_bitboard_mask::knightAttacks[toSquare];
    }
    else if (((movedPiece == petir_header::blackBishop))) {
        attackedSquares = 0;
        file = (toSquare & 7);
        rank = pas::shiftRight(toSquare, 3);
        (file = pas::increment(file, 1));
        (rank = pas::increment(rank, 1));
        while (((((file <= 7) && (rank <= 7)) && (file >= 0)) && (rank >= 0))) {
            square = pas::add(pas::shiftLeft(rank, 3), file);
            attackedSquares = (attackedSquares | petir_bitboard_mask::squareBit(square));
            if ((position.board[square] == petir_header::blackQueen)) {
                (file = pas::increment(file, 1));
                (rank = pas::increment(rank, 1));
                continue;
            }
            if ((position.board[square] != 0)) {
                break;
            }
            (file = pas::increment(file, 1));
            (rank = pas::increment(rank, 1));
        }
        file = (toSquare & 7);
        rank = pas::shiftRight(toSquare, 3);
        (file = pas::decrement(file, 1));
        (rank = pas::increment(rank, 1));
        while (((((file <= 7) && (rank <= 7)) && (file >= 0)) && (rank >= 0))) {
            square = pas::add(pas::shiftLeft(rank, 3), file);
            attackedSquares = (attackedSquares | petir_bitboard_mask::squareBit(square));
            if ((position.board[square] == petir_header::blackQueen)) {
                (file = pas::decrement(file, 1));
                (rank = pas::increment(rank, 1));
                continue;
            }
            if ((position.board[square] != 0)) {
                break;
            }
            (file = pas::decrement(file, 1));
            (rank = pas::increment(rank, 1));
        }
        file = (toSquare & 7);
        rank = pas::shiftRight(toSquare, 3);
        (file = pas::increment(file, 1));
        (rank = pas::decrement(rank, 1));
        while (((((file <= 7) && (rank <= 7)) && (file >= 0)) && (rank >= 0))) {
            square = pas::add(pas::shiftLeft(rank, 3), file);
            attackedSquares = (attackedSquares | petir_bitboard_mask::squareBit(square));
            if ((position.board[square] == petir_header::blackPawn)) {
                (file = pas::increment(file, 1));
                (rank = pas::decrement(rank, 1));
                square = pas::add(pas::shiftLeft(rank, 3), file);
                attackedSquares = (attackedSquares | petir_bitboard_mask::squareBit(square));
                break;
            }
            if ((position.board[square] == petir_header::blackQueen)) {
                (file = pas::increment(file, 1));
                (rank = pas::decrement(rank, 1));
                continue;
            }
            if ((position.board[square] != 0)) {
                break;
            }
            (file = pas::increment(file, 1));
            (rank = pas::decrement(rank, 1));
        }
        file = (toSquare & 7);
        rank = pas::shiftRight(toSquare, 3);
        (file = pas::decrement(file, 1));
        (rank = pas::decrement(rank, 1));
        while (((((file <= 7) && (rank <= 7)) && (file >= 0)) && (rank >= 0))) {
            square = pas::add(pas::shiftLeft(rank, 3), file);
            attackedSquares = (attackedSquares | petir_bitboard_mask::squareBit(square));
            if ((position.board[square] == petir_header::blackPawn)) {
                (file = pas::decrement(file, 1));
                (rank = pas::decrement(rank, 1));
                square = pas::add(pas::shiftLeft(rank, 3), file);
                attackedSquares = (attackedSquares | petir_bitboard_mask::squareBit(square));
                break;
            }
            if ((position.board[square] == petir_header::blackQueen)) {
                (file = pas::decrement(file, 1));
                (rank = pas::decrement(rank, 1));
                continue;
            }
            if ((position.board[square] != 0)) {
                break;
            }
            (file = pas::decrement(file, 1));
            (rank = pas::decrement(rank, 1));
        }
        attackedSquares = petir_bitboard_mask::diagonalA8H1Attacks[toSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[toSquare]) & 255)];
        attackedSquares = (attackedSquares | petir_bitboard_mask::diagonalH8A1Attacks[toSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[toSquare]) & 255)]);
    }
    else if (((movedPiece == petir_header::blackRook))) {
        occupancyWithoutSlider = (position.allPieces & (~position.blackQueens));
        attackedSquares = petir_bitboard_mask::horizontalAttacks[toSquare][(pas::shiftRight(occupancyWithoutSlider, pas::add((toSquare & 56), 1)) & 63)];
        square = pas::add(toSquare, 8);
        while ((square <= 63)) {
            attackedSquares = (attackedSquares | petir_bitboard_mask::squareBit(square));
            if (((position.board[square] == petir_header::blackRook) || (position.board[square] == petir_header::blackQueen))) {
                square = pas::add(square, 8);
                continue;
            }
            if ((position.board[square] != 0)) {
                break;
            }
            square = pas::add(square, 8);
        }
        square = pas::sub(toSquare, 8);
        while ((square >= 0)) {
            attackedSquares = (attackedSquares | petir_bitboard_mask::squareBit(square));
            if (((position.board[square] == petir_header::blackRook) || (position.board[square] == petir_header::blackQueen))) {
                square = pas::sub(square, 8);
                continue;
            }
            if ((position.board[square] != 0)) {
                break;
            }
            square = pas::sub(square, 8);
        }
    }
    else if (((movedPiece == petir_header::blackQueen))) {
        attackedSquares = petir_bitboard_mask::diagonalA8H1Attacks[toSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[toSquare]) & 255)];
        attackedSquares = (attackedSquares | petir_bitboard_mask::diagonalH8A1Attacks[toSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[toSquare]) & 255)]);
        occupancyWithoutSlider = (position.allPieces & (~position.blackRooks));
        attackedSquares = (attackedSquares | petir_bitboard_mask::horizontalAttacks[toSquare][(pas::shiftRight(occupancyWithoutSlider, pas::add((toSquare & 56), 1)) & 63)]);
        square = pas::add(toSquare, 8);
        while ((square <= 63)) {
            attackedSquares = (attackedSquares | petir_bitboard_mask::squareBit(square));
            if (((position.board[square] == petir_header::blackRook) || (position.board[square] == petir_header::blackQueen))) {
                square = pas::add(square, 8);
                continue;
            }
            if ((position.board[square] != 0)) {
                break;
            }
            square = pas::add(square, 8);
        }
        square = pas::sub(toSquare, 8);
        while ((square >= 0)) {
            attackedSquares = (attackedSquares | petir_bitboard_mask::squareBit(square));
            if (((position.board[square] == petir_header::blackRook) || (position.board[square] == petir_header::blackQueen))) {
                square = pas::sub(square, 8);
                continue;
            }
            if ((position.board[square] != 0)) {
                break;
            }
            square = pas::sub(square, 8);
        }
    }
    else if (((movedPiece == petir_header::blackKing))) {
        attackedSquares = petir_bitboard_mask::kingAttacks[toSquare];
    }
    threatenedValue = 0;
    if (((attackedSquares & kingZone) != 0)) {
        result = true;
        return result;
    }
    if (((attackedSquares & position.whiteQueens) != 0)) {
        threatenedValue = petir_header::queenValue;
    } else {
        if (((attackedSquares & position.whiteRooks) != 0)) {
            threatenedValue = petir_header::rookValue;
        } else {
            if (((attackedSquares & position.whiteBishops) != 0)) {
                threatenedValue = petir_header::bishopValue;
            } else {
                if (((attackedSquares & position.whiteKnights) != 0)) {
                    threatenedValue = petir_header::knightValue;
                } else {
                    if (((attackedSquares & position.whitePawns) != 0)) {
                        threatenedValue = petir_header::pawnValue;
                    }
                }
            }
        }
    }
    return result;
}

// tools.pas:239
bool hasPawnOnSeventh(std::int32_t sideToMove, petir_header::tdata& position) {
    bool result{};
    if ((sideToMove == petir_header::blackSide)) {
        result = ((petir_bitboard_mask::rankMask[1] & position.blackPawns) != 0);
    } else {
        result = ((petir_bitboard_mask::rankMask[6] & position.whitePawns) != 0);
    }
    return result;
}

// tools.pas:251
bool whiteHasPawnOnSeventh(petir_header::tdata& position) {
    bool result{};
    result = ((petir_bitboard_mask::rankMask[6] & position.whitePawns) != 0);
    return result;
}

// tools.pas:259
bool blackHasPawnOnSeventh(petir_header::tdata& position) {
    bool result{};
    result = ((petir_bitboard_mask::rankMask[1] & position.blackPawns) != 0);
    return result;
}

// tools.pas:143
bool hasFreeSixthRankPawn(std::int32_t sideToMove, petir_header::tdata& position) {
    bool result{};
    if ((sideToMove == petir_header::blackSide)) {
        result = false;
        if ((((position.board[petir_header::d3] == petir_header::blackPawn) && (position.board[petir_header::d2] == 0)) && (position.board[petir_header::d1] == 0))) {
            result = true;
            return result;
        }
        if ((((position.board[petir_header::e3] == petir_header::blackPawn) && (position.board[petir_header::e2] == 0)) && (position.board[petir_header::e1] == 0))) {
            result = true;
            return result;
        }
        if ((((position.board[petir_header::a3] == petir_header::blackPawn) && (position.board[petir_header::a2] == 0)) && (position.board[petir_header::a1] == 0))) {
            result = true;
            return result;
        }
        if ((((position.board[petir_header::b3] == petir_header::blackPawn) && (position.board[petir_header::b2] == 0)) && (position.board[petir_header::b1] == 0))) {
            result = true;
            return result;
        }
        if ((((position.board[petir_header::c3] == petir_header::blackPawn) && (position.board[petir_header::c2] == 0)) && (position.board[petir_header::c1] == 0))) {
            result = true;
            return result;
        }
        if ((((position.board[petir_header::f3] == petir_header::blackPawn) && (position.board[petir_header::f2] == 0)) && (position.board[petir_header::f1] == 0))) {
            result = true;
            return result;
        }
        if ((((position.board[petir_header::g3] == petir_header::blackPawn) && (position.board[petir_header::g2] == 0)) && (position.board[petir_header::g1] == 0))) {
            result = true;
            return result;
        }
        if ((((position.board[petir_header::h3] == petir_header::blackPawn) && (position.board[petir_header::h2] == 0)) && (position.board[petir_header::h1] == 0))) {
            result = true;
            return result;
        }
    } else {
        result = false;
        if ((((position.board[petir_header::d6] == petir_header::whitePawn) && (position.board[petir_header::d7] == 0)) && (position.board[petir_header::d8] == 0))) {
            result = true;
            return result;
        }
        if ((((position.board[petir_header::e6] == petir_header::whitePawn) && (position.board[petir_header::e7] == 0)) && (position.board[petir_header::e8] == 0))) {
            result = true;
            return result;
        }
        if ((((position.board[petir_header::a6] == petir_header::whitePawn) && (position.board[petir_header::a7] == 0)) && (position.board[petir_header::a8] == 0))) {
            result = true;
            return result;
        }
        if ((((position.board[petir_header::b6] == petir_header::whitePawn) && (position.board[petir_header::b7] == 0)) && (position.board[petir_header::b8] == 0))) {
            result = true;
            return result;
        }
        if ((((position.board[petir_header::c6] == petir_header::whitePawn) && (position.board[petir_header::c7] == 0)) && (position.board[petir_header::c8] == 0))) {
            result = true;
            return result;
        }
        if ((((position.board[petir_header::f6] == petir_header::whitePawn) && (position.board[petir_header::f7] == 0)) && (position.board[petir_header::f8] == 0))) {
            result = true;
            return result;
        }
        if ((((position.board[petir_header::g6] == petir_header::whitePawn) && (position.board[petir_header::g7] == 0)) && (position.board[petir_header::g8] == 0))) {
            result = true;
            return result;
        }
        if ((((position.board[petir_header::h6] == petir_header::whitePawn) && (position.board[petir_header::h7] == 0)) && (position.board[petir_header::h8] == 0))) {
            result = true;
            return result;
        }
    }
    return result;
}

// tools.pas:107
bool isGoodWhiteMove(petir_header::tdata& position, std::int32_t square) {
    bool result{};
    result = false;
    if ((square > 63)) {
        return result;
    }
    if (((position.board[square] == petir_header::whiteRook) && (square >= petir_header::a7))) {
        result = true;
    } else {
        if (((position.board[square] == petir_header::whitePawn) && ((petir_bitboard_mask::whitePassedPawnMask[square] & position.blackPawns) == 0))) {
            result = true;
        }
    }
    return result;
}

// tools.pas:121
bool isGoodBlackMove(petir_header::tdata& position, std::int32_t square) {
    bool result{};
    result = false;
    if ((square > 63)) {
        return result;
    }
    if (((position.board[square] == petir_header::blackRook) && (square <= petir_header::h2))) {
        result = true;
    } else {
        if (((position.board[square] == petir_header::blackPawn) && ((petir_bitboard_mask::blackPassedPawnMask[square] & position.whitePawns) == 0))) {
            result = true;
        }
    }
    return result;
}

// tools.pas:290
bool moveThreatensKing(std::uint8_t sideToMove, petir_header::tdata& position, std::uint16_t move, std::int32_t& threatenedValue) {
    bool result{};
    if ((sideToMove == petir_header::whiteSide)) {
        result = petir_tools::whiteMoveThreatensKing(position, move, threatenedValue);
    } else {
        result = petir_tools::blackMoveThreatensKing(position, move, threatenedValue);
    }
    return result;
}

// tools.pas:99
std::int32_t nonPawnMaterial(petir_header::tdata& position, std::int32_t side) {
    std::int32_t result{};
    if ((side == petir_header::whiteSide)) {
        result = position.whiteNonPawnMaterial;
    } else {
        result = position.blackNonPawnMaterial;
    }
    return result;
}

// tools.pas:44
pas::String getFen(petir_header::tdata position, std::int32_t sideToMove) {
    pas::String result{};
    std::int32_t index{};
    std::int32_t rankStart{};
    std::int32_t mirroredFile{};
    std::int32_t square{};
    pas::String rankText{};
    std::int32_t emptyCount{};
    std::int32_t lastIndex{};
    std::int32_t piece{};
    result = pas::String("");
    rankText = pas::String("");
    emptyCount = 0;
    lastIndex = 0;
    for (index = 63; index >= lastIndex; --index) {
        rankStart = pas::mul((index / 8), 8);
        mirroredFile = pas::sub(7, (index % 8));
        square = pas::add(rankStart, mirroredFile);
        if ((position.board[square] != 0)) {
            if ((emptyCount != 0)) {
                rankText = (rankText + pas::intToStr(emptyCount));
                emptyCount = 0;
            }
        }
        piece = position.board[square];
        if (((piece == petir_header::blackPawn))) {
            rankText = (rankText + 'p');
        }
        else if (((piece == petir_header::blackKnight))) {
            rankText = (rankText + 'n');
        }
        else if (((piece == petir_header::blackBishop))) {
            rankText = (rankText + 'b');
        }
        else if (((piece == petir_header::blackRook))) {
            rankText = (rankText + 'r');
        }
        else if (((piece == petir_header::blackQueen))) {
            rankText = (rankText + 'q');
        }
        else if (((piece == petir_header::blackKing))) {
            rankText = (rankText + 'k');
        }
        else if (((piece == petir_header::whitePawn))) {
            rankText = (rankText + 'P');
        }
        else if (((piece == petir_header::whiteKnight))) {
            rankText = (rankText + 'N');
        }
        else if (((piece == petir_header::whiteBishop))) {
            rankText = (rankText + 'B');
        }
        else if (((piece == petir_header::whiteRook))) {
            rankText = (rankText + 'R');
        }
        else if (((piece == petir_header::whiteQueen))) {
            rankText = (rankText + 'Q');
        }
        else if (((piece == petir_header::whiteKing))) {
            rankText = (rankText + 'K');
        }
        else if (((piece == 0))) {
            emptyCount = pas::add(emptyCount, 1);
        }
        if (((index % 8) == 0)) {
            if ((emptyCount != 0)) {
                rankText = (rankText + pas::intToStr(emptyCount));
            }
            result = (result + rankText);
            if (((index <= 56) && (index > 0))) {
                result = (result + '/');
            }
            rankText = pas::String("");
            emptyCount = 0;
        }
    }
    result = (result + ' ');
    if ((sideToMove == petir_header::whiteSide)) {
        result = (result + 'w');
    } else {
        result = (result + 'b');
    }
    result = (result + pas::String(" - -"));
    return result;
}

// tools.pas:135
bool isGoodMove(petir_header::tdata& position, std::int32_t square, std::int32_t sideToMove) {
    bool result{};
    if ((sideToMove == petir_header::whiteSide)) {
        result = petir_tools::isGoodWhiteMove(position, square);
    } else {
        result = petir_tools::isGoodBlackMove(position, square);
    }
    return result;
}

// tools.pas:691
bool whiteMoveGivesCheck(petir_header::tdata& position, std::uint16_t move) {
    bool result{};
    std::uint8_t fromSquare{};
    std::uint8_t toSquare{};
    std::int32_t kingSquare{};
    std::int32_t promotion{};
    std::uint64_t attackedSquares{};
    std::int8_t piece{};
    std::int32_t movedPiece{};
    std::int32_t bishopDirection{};
    std::int32_t rookDirection{};
    std::int32_t queenDirection{};
    result = false;
    kingSquare = petir_bitboard_mask::kingSquare(position.blackKingBoard);
    fromSquare = (move & 127);
    toSquare = (pas::shiftRight(move, 7) & 127);
    promotion = (pas::shiftRight(move, 14) & 7);
    piece = position.board[fromSquare];
    if ((promotion == petir_header::queenPromotion)) {
        piece = petir_header::whiteQueen;
    } else {
        if ((promotion == petir_header::knightPromotion)) {
            piece = petir_header::whiteKnight;
        }
    }
    if (((toSquare == 65) || (toSquare == 66))) {
        if ((toSquare == 65)) {
            toSquare = petir_header::f1;
        } else {
            toSquare = petir_header::d1;
        }
        piece = petir_header::whiteRook;
    } else {
        if ((toSquare == 67)) {
            if ((position.enPassantFile > (fromSquare & 7))) {
                toSquare = pas::add(fromSquare, 9);
            } else {
                toSquare = pas::add(fromSquare, 7);
            }
        }
    }
    movedPiece = piece;
    if (((movedPiece == petir_header::whitePawn))) {
        if (((petir_bitboard_mask::whitePawnAttacks[toSquare] & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
            result = true;
        }
    }
    else if (((movedPiece == petir_header::whiteKnight))) {
        if (((petir_bitboard_mask::knightAttacks[toSquare] & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
            result = true;
        }
    }
    else if (((movedPiece == petir_header::whiteBishop))) {
        bishopDirection = pas::abs(petir_bitboard_mask::rayDirection[toSquare][kingSquare]);
        if (((bishopDirection == 7))) {
            attackedSquares = petir_bitboard_mask::diagonalA8H1Attacks[toSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[toSquare]) & 255)];
            if (((attackedSquares & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
                result = true;
            }
        }
        else if (((bishopDirection == 9))) {
            attackedSquares = petir_bitboard_mask::diagonalH8A1Attacks[toSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[toSquare]) & 255)];
            if (((attackedSquares & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
                result = true;
            }
        }
    }
    else if (((movedPiece == petir_header::whiteRook))) {
        rookDirection = pas::abs(petir_bitboard_mask::rayDirection[toSquare][kingSquare]);
        if (((rookDirection == 1))) {
            attackedSquares = petir_bitboard_mask::horizontalAttacks[toSquare][(pas::shiftRight(position.allPieces, pas::add((toSquare & 56), 1)) & 63)];
            if (((attackedSquares & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
                result = true;
            }
        }
        else if (((rookDirection == 8))) {
            attackedSquares = petir_bitboard_mask::verticalAttacks[toSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[toSquare]) & 63)];
            if (((attackedSquares & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
                result = true;
            }
        }
    }
    else if (((movedPiece == petir_header::whiteQueen))) {
        queenDirection = pas::abs(petir_bitboard_mask::rayDirection[toSquare][kingSquare]);
        if (((queenDirection == 1))) {
            attackedSquares = petir_bitboard_mask::horizontalAttacks[toSquare][(pas::shiftRight(position.allPieces, pas::add((toSquare & 56), 1)) & 63)];
            if (((attackedSquares & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
                result = true;
            }
        }
        else if (((queenDirection == 8))) {
            attackedSquares = petir_bitboard_mask::verticalAttacks[toSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[toSquare]) & 63)];
            if (((attackedSquares & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
                result = true;
            }
        }
        else if (((queenDirection == 7))) {
            attackedSquares = petir_bitboard_mask::diagonalA8H1Attacks[toSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[toSquare]) & 255)];
            if (((attackedSquares & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
                result = true;
            }
        }
        else if (((queenDirection == 9))) {
            attackedSquares = petir_bitboard_mask::diagonalH8A1Attacks[toSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[toSquare]) & 255)];
            if (((attackedSquares & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
                result = true;
            }
        }
    }
    return result;
}

// tools.pas:787
std::uint8_t popCount(std::uint64_t bits) {
    std::uint8_t result{};
    result = 0;
    while ((bits != 0)) {
        bits = (bits & (bits - 1));
        (result = pas::increment(result, 1));
    }
    return result;
}

// tools.pas:984
bool blackMoveGivesCheck(petir_header::tdata& position, std::uint16_t move) {
    bool result{};
    std::uint8_t fromSquare{};
    std::uint8_t toSquare{};
    std::int32_t kingSquare{};
    std::int32_t promotion{};
    std::uint64_t attackedSquares{};
    std::int8_t piece{};
    std::int32_t movedPiece{};
    std::int32_t bishopDirection{};
    std::int32_t rookDirection{};
    std::int32_t queenDirection{};
    result = false;
    kingSquare = petir_bitboard_mask::kingSquare(position.whiteKingBoard);
    fromSquare = (move & 127);
    toSquare = (pas::shiftRight(move, 7) & 127);
    promotion = (pas::shiftRight(move, 14) & 7);
    piece = position.board[fromSquare];
    if ((promotion == petir_header::queenPromotion)) {
        piece = petir_header::blackQueen;
    } else {
        if ((promotion == petir_header::knightPromotion)) {
            piece = petir_header::blackKnight;
        }
    }
    if (((toSquare == 65) || (toSquare == 66))) {
        if ((toSquare == 65)) {
            toSquare = petir_header::f8;
        } else {
            toSquare = petir_header::d8;
        }
        piece = petir_header::blackRook;
    } else {
        if ((toSquare == 67)) {
            if ((position.enPassantFile > (fromSquare & 7))) {
                toSquare = pas::sub(fromSquare, 7);
            } else {
                toSquare = pas::sub(fromSquare, 9);
            }
        }
    }
    movedPiece = piece;
    if (((movedPiece == petir_header::blackPawn))) {
        if (((petir_bitboard_mask::blackPawnAttacks[toSquare] & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
            result = true;
        }
    }
    else if (((movedPiece == petir_header::blackKnight))) {
        if (((petir_bitboard_mask::knightAttacks[toSquare] & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
            result = true;
        }
    }
    else if (((movedPiece == petir_header::blackBishop))) {
        bishopDirection = pas::abs(petir_bitboard_mask::rayDirection[toSquare][kingSquare]);
        if (((bishopDirection == 7))) {
            attackedSquares = petir_bitboard_mask::diagonalA8H1Attacks[toSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[toSquare]) & 255)];
            if (((attackedSquares & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
                result = true;
            }
        }
        else if (((bishopDirection == 9))) {
            attackedSquares = petir_bitboard_mask::diagonalH8A1Attacks[toSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[toSquare]) & 255)];
            if (((attackedSquares & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
                result = true;
            }
        }
    }
    else if (((movedPiece == petir_header::blackRook))) {
        rookDirection = pas::abs(petir_bitboard_mask::rayDirection[toSquare][kingSquare]);
        if (((rookDirection == 1))) {
            attackedSquares = petir_bitboard_mask::horizontalAttacks[toSquare][(pas::shiftRight(position.allPieces, pas::add((toSquare & 56), 1)) & 63)];
            if (((attackedSquares & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
                result = true;
            }
        }
        else if (((rookDirection == 8))) {
            attackedSquares = petir_bitboard_mask::verticalAttacks[toSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[toSquare]) & 63)];
            if (((attackedSquares & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
                result = true;
            }
        }
    }
    else if (((movedPiece == petir_header::blackQueen))) {
        queenDirection = pas::abs(petir_bitboard_mask::rayDirection[toSquare][kingSquare]);
        if (((queenDirection == 1))) {
            attackedSquares = petir_bitboard_mask::horizontalAttacks[toSquare][(pas::shiftRight(position.allPieces, pas::add((toSquare & 56), 1)) & 63)];
            if (((attackedSquares & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
                result = true;
            }
        }
        else if (((queenDirection == 8))) {
            attackedSquares = petir_bitboard_mask::verticalAttacks[toSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[toSquare]) & 63)];
            if (((attackedSquares & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
                result = true;
            }
        }
        else if (((queenDirection == 7))) {
            attackedSquares = petir_bitboard_mask::diagonalA8H1Attacks[toSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[toSquare]) & 255)];
            if (((attackedSquares & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
                result = true;
            }
        }
        else if (((queenDirection == 9))) {
            attackedSquares = petir_bitboard_mask::diagonalH8A1Attacks[toSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[toSquare]) & 255)];
            if (((attackedSquares & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
                result = true;
            }
        }
    }
    return result;
}

// tools.pas:1079
bool blackMoveGivesCheckOrSpecial(petir_header::tdata& position, std::uint16_t move, std::int32_t& threatenedValue) {
    bool result{};
    std::uint8_t fromSquare{};
    std::uint8_t toSquare{};
    std::int32_t kingSquare{};
    std::uint64_t attackedSquares{};
    std::uint64_t kingBit{};
    std::int32_t movedPiece{};
    std::int32_t bishopDirection{};
    std::int32_t rookDirection{};
    std::int32_t queenDirection{};
    result = false;
    kingSquare = petir_bitboard_mask::kingSquare(position.whiteKingBoard);
    fromSquare = (move & 127);
    toSquare = (pas::shiftRight(move, 7) & 127);
    if ((toSquare > 63)) {
        result = true;
        return result;
    }
    kingBit = petir_bitboard_mask::squareBit(kingSquare);
    movedPiece = position.board[fromSquare];
    if (((movedPiece == petir_header::blackPawn))) {
        if (((petir_bitboard_mask::blackPawnAttacks[toSquare] & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
            result = true;
        }
    }
    else if (((movedPiece == petir_header::blackKnight))) {
        if (((petir_bitboard_mask::knightAttacks[toSquare] & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
            result = true;
        }
    }
    else if (((movedPiece == petir_header::blackBishop))) {
        bishopDirection = pas::abs(petir_bitboard_mask::rayDirection[toSquare][kingSquare]);
        if (((bishopDirection == 7))) {
            attackedSquares = petir_bitboard_mask::diagonalA8H1Attacks[toSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[toSquare]) & 255)];
            if (((attackedSquares & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
                result = true;
            }
        }
        else if (((bishopDirection == 9))) {
            attackedSquares = petir_bitboard_mask::diagonalH8A1Attacks[toSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[toSquare]) & 255)];
            if (((attackedSquares & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
                result = true;
            }
        }
    }
    else if (((movedPiece == petir_header::blackRook))) {
        rookDirection = pas::abs(petir_bitboard_mask::rayDirection[toSquare][kingSquare]);
        if (((rookDirection == 1))) {
            attackedSquares = petir_bitboard_mask::horizontalAttacks[toSquare][(pas::shiftRight(position.allPieces, pas::add((toSquare & 56), 1)) & 63)];
            if (((attackedSquares & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
                result = true;
            }
        }
        else if (((rookDirection == 8))) {
            attackedSquares = petir_bitboard_mask::verticalAttacks[toSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[toSquare]) & 63)];
            if (((attackedSquares & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
                result = true;
            }
        }
    }
    else if (((movedPiece == petir_header::blackQueen))) {
        queenDirection = pas::abs(petir_bitboard_mask::rayDirection[toSquare][kingSquare]);
        if (((queenDirection == 1))) {
            attackedSquares = petir_bitboard_mask::horizontalAttacks[toSquare][(pas::shiftRight(position.allPieces, pas::add((toSquare & 56), 1)) & 63)];
            if (((attackedSquares & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
                result = true;
            }
        }
        else if (((queenDirection == 8))) {
            attackedSquares = petir_bitboard_mask::verticalAttacks[toSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[toSquare]) & 63)];
            if (((attackedSquares & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
                result = true;
            }
        }
        else if (((queenDirection == 7))) {
            attackedSquares = petir_bitboard_mask::diagonalA8H1Attacks[toSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[toSquare]) & 255)];
            if (((attackedSquares & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
                result = true;
            }
        }
        else if (((queenDirection == 9))) {
            attackedSquares = petir_bitboard_mask::diagonalH8A1Attacks[toSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[toSquare]) & 255)];
            if (((attackedSquares & petir_bitboard_mask::squareBit(kingSquare)) != 0)) {
                result = true;
            }
        }
    }
    return result;
}

}
