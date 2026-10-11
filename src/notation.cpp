// Faithful translation of notation.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_notation {
// notation.pas:256
pas::String moveToNotation(std::int32_t move, std::int32_t enPassantFile, std::int32_t sideToMove) {
    pas::String result{};
    std::int32_t fromSquare{};
    std::int32_t toSquare{};
    std::int32_t promotion{};
    result = pas::String("");
    fromSquare = (move & 127);
    toSquare = (pas::shiftRight(move, 7) & 127);
    promotion = (pas::shiftRight(move, 14) & 7);
    if ((toSquare == petir_header::shortCastleTarget)) {
        if ((sideToMove == petir_header::whiteSide)) {
            result = pas::String("e1g1");
        } else {
            result = pas::String("e8g8");
        }
    } else {
        if ((toSquare == petir_header::longCastleTarget)) {
            if ((sideToMove == petir_header::whiteSide)) {
                result = pas::String("e1c1");
            } else {
                result = pas::String("e8c8");
            }
        } else {
            if ((toSquare == petir_header::enPassantTarget)) {
                if ((sideToMove == petir_header::whiteSide)) {
                    if ((enPassantFile > (fromSquare % 8))) {
                        toSquare = pas::add(fromSquare, 9);
                    } else {
                        toSquare = pas::add(fromSquare, 7);
                    }
                } else {
                    if ((enPassantFile > (fromSquare % 8))) {
                        toSquare = pas::sub(fromSquare, 7);
                    } else {
                        toSquare = pas::sub(fromSquare, 9);
                    }
                }
                result = (petir_notation::squareName(fromSquare) + petir_notation::squareName(toSquare));
            } else {
                result = (petir_notation::squareName(fromSquare) + petir_notation::squareName(toSquare));
                if ((promotion == petir_header::queenPromotion)) {
                    result = (result + 'q');
                } else {
                    if ((promotion == petir_header::knightPromotion)) {
                        result = (result + 'n');
                    } else {
                        if ((promotion == petir_header::bishopPromotion)) {
                            result = (result + 'b');
                        } else {
                            if ((promotion == petir_header::rookPromotion)) {
                                result = (result + 'r');
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}

// notation.pas:195
std::int32_t notationToMove(pas::String notation, petir_header::tdata position, std::uint8_t sideToMove) {
    std::int32_t result{};
    std::uint8_t file{};
    std::uint8_t rank{};
    std::uint8_t fromSquare{};
    std::uint8_t toSquare{};
    if (((notation[1] >= 'a' && notation[1] <= 'h'))) {
        file = pas::sub(pas::charCode(notation[1]), 97);
        rank = pas::sub(pas::strToInt(notation[2]), 1);
        fromSquare = pas::add(pas::mul(rank, 8), file);
        file = pas::sub(pas::charCode(notation[3]), 97);
        rank = pas::sub(pas::strToInt(notation[4]), 1);
        toSquare = pas::add(pas::mul(rank, 8), file);
        if ((((fromSquare == petir_header::e1) && (toSquare == petir_header::g1)) && (position.board[petir_header::e1] == petir_header::whiteKing))) {
            result = pas::add(fromSquare, pas::shiftLeft(petir_header::shortCastleTarget, 7));
            return result;
        }
        if ((((fromSquare == petir_header::e1) && (toSquare == petir_header::c1)) && (position.board[petir_header::e1] == petir_header::whiteKing))) {
            result = pas::add(fromSquare, pas::shiftLeft(petir_header::longCastleTarget, 7));
            return result;
        }
        if ((((fromSquare == petir_header::e8) && (toSquare == petir_header::g8)) && (position.board[petir_header::e8] == petir_header::blackKing))) {
            result = pas::add(fromSquare, pas::shiftLeft(petir_header::shortCastleTarget, 7));
            return result;
        }
        if ((((fromSquare == petir_header::e8) && (toSquare == petir_header::c8)) && (position.board[petir_header::e8] == petir_header::blackKing))) {
            result = pas::add(fromSquare, pas::shiftLeft(petir_header::longCastleTarget, 7));
            return result;
        }
        if ((((position.board[fromSquare] == petir_header::whitePawn) && ((pas::sub(toSquare, fromSquare) == 7) || (pas::sub(toSquare, fromSquare) == 9))) && (position.board[toSquare] == 0))) {
            result = pas::add(fromSquare, pas::shiftLeft(petir_header::enPassantTarget, 7));
            return result;
        }
        if ((((position.board[fromSquare] == petir_header::blackPawn) && ((pas::sub(fromSquare, toSquare) == 7) || (pas::sub(fromSquare, toSquare) == 9))) && (position.board[toSquare] == 0))) {
            result = pas::add(fromSquare, pas::shiftLeft(petir_header::enPassantTarget, 7));
            return result;
        }
        result = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
        if ((pas::length(notation) > 4)) {
            if ((notation[5] == 'q')) {
                result = pas::add(result, pas::shiftLeft(petir_header::queenPromotion, 14));
            } else {
                if ((notation[5] == 'r')) {
                    result = pas::add(result, pas::shiftLeft(petir_header::rookPromotion, 14));
                } else {
                    if ((notation[5] == 'b')) {
                        result = pas::add(result, pas::shiftLeft(petir_header::bishopPromotion, 14));
                    } else {
                        if ((notation[5] == 'n')) {
                            result = pas::add(result, pas::shiftLeft(petir_header::knightPromotion, 14));
                        }
                    }
                }
            }
        }
    } else {
        if (((pas::copy(notation, 1, 3) == pas::String("O-O")) && (sideToMove == petir_header::whiteSide))) {
            result = pas::add(petir_header::e1, pas::shiftLeft(petir_header::shortCastleTarget, 7));
        } else {
            if (((pas::copy(notation, 1, 3) == pas::String("O-O")) && (sideToMove == petir_header::blackSide))) {
                result = pas::add(petir_header::e8, pas::shiftLeft(petir_header::shortCastleTarget, 7));
            } else {
                if (((pas::copy(notation, 1, 5) == pas::String("O-O-O")) && (sideToMove == petir_header::blackSide))) {
                    result = pas::add(petir_header::e8, pas::shiftLeft(petir_header::longCastleTarget, 7));
                } else {
                    if (((pas::copy(notation, 1, 5) == pas::String("O-O-O")) && (sideToMove == petir_header::whiteSide))) {
                        result = pas::add(petir_header::e1, pas::shiftLeft(petir_header::longCastleTarget, 7));
                    }
                }
            }
        }
    }
    return result;
}

// notation.pas:93
pas::String shortToLongNotation(pas::String notation, petir_header::tdata position, std::uint8_t sideToMove) {
    pas::String result{};
// notation.pas:95
    auto pawnMoveToLong = [&]() -> pas::String {
    pas::String result{};
    std::int32_t fromSquare{};
    std::int32_t toSquare{};
    std::int32_t fromFile{};
    if ((pas::length(notation) == 2)) {
        toSquare = petir_notation::squareFromName(notation);
        if ((sideToMove == petir_header::whiteSide)) {
            if ((position.board[pas::sub(toSquare, 8)] == petir_header::whitePawn)) {
                fromSquare = pas::sub(toSquare, 8);
            } else {
                fromSquare = pas::sub(toSquare, 16);
            }
        } else {
            if ((position.board[pas::add(toSquare, 8)] == petir_header::blackPawn)) {
                fromSquare = pas::add(toSquare, 8);
            } else {
                fromSquare = pas::add(toSquare, 16);
            }
        }
    } else {
        fromFile = pas::sub(pas::charCode(notation[1]), 97);
        toSquare = petir_notation::squareFromName(pas::copy(notation, 3, 2));
        if ((sideToMove == petir_header::whiteSide)) {
            if ((fromFile < (toSquare % 8))) {
                fromSquare = pas::sub(toSquare, 9);
            } else {
                fromSquare = pas::sub(toSquare, 7);
            }
        } else {
            if ((fromFile < (toSquare % 8))) {
                fromSquare = pas::add(toSquare, 7);
            } else {
                fromSquare = pas::add(toSquare, 9);
            }
        }
    }
    result = (petir_notation::squareName(fromSquare) + petir_notation::squareName(toSquare));
    return result;
};

// notation.pas:127
    auto pieceMoveToLong = [&]() -> pas::String {
    pas::String result{};
    std::int32_t fromSquare{};
    std::int32_t toSquare{};
    std::int32_t fromFile{};
    std::int32_t fromRank{};
    std::uint64_t candidates{};
    std::int32_t pieceLetter{};
    fromFile = 9;
    fromRank = 9;
    if ((pas::length(notation) == 3)) {
        toSquare = petir_notation::squareFromName(pas::copy(notation, 2, 2));
    } else {
        toSquare = petir_notation::squareFromName(pas::copy(notation, pas::sub(pas::length(notation), 1), 2));
        if ((notation[2] != 'x')) {
            if (((notation[2] >= 'a' && notation[2] <= 'h'))) {
                fromFile = pas::sub(pas::charCode(notation[2]), 97);
            } else {
                fromRank = pas::sub(pas::strToInt(notation[2]), 1);
            }
        }
    }
    pieceLetter = notation[1];
    if (((pieceLetter == 'N'))) {
        candidates = petir_bitboard_mask::knightAttacks[toSquare];
        if ((sideToMove == petir_header::whiteSide)) {
            candidates = (candidates & position.whiteKnights);
        } else {
            candidates = (candidates & position.blackKnights);
        }
    }
    else if (((pieceLetter == 'B'))) {
        candidates = petir_bitboard_mask::diagonalA8H1Attacks[toSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[toSquare]) & 255)];
        candidates = (candidates | petir_bitboard_mask::diagonalH8A1Attacks[toSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[toSquare]) & 255)]);
        if ((sideToMove == petir_header::whiteSide)) {
            candidates = (candidates & position.whiteBishops);
        } else {
            candidates = (candidates & position.blackBishops);
        }
    }
    else if (((pieceLetter == 'R'))) {
        candidates = petir_bitboard_mask::horizontalAttacks[toSquare][(pas::shiftRight(position.allPieces, pas::add((toSquare & 56), 1)) & 63)];
        candidates = (candidates | petir_bitboard_mask::verticalAttacks[toSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[toSquare]) & 63)]);
        if ((sideToMove == petir_header::whiteSide)) {
            candidates = (candidates & position.whiteRooks);
        } else {
            candidates = (candidates & position.blackRooks);
        }
    }
    else if (((pieceLetter == 'Q'))) {
        candidates = petir_bitboard_mask::horizontalAttacks[toSquare][(pas::shiftRight(position.allPieces, pas::add((toSquare & 56), 1)) & 63)];
        candidates = (candidates | petir_bitboard_mask::verticalAttacks[toSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[toSquare]) & 63)]);
        candidates = (candidates | petir_bitboard_mask::diagonalA8H1Attacks[toSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[toSquare]) & 255)]);
        candidates = (candidates | petir_bitboard_mask::diagonalH8A1Attacks[toSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[toSquare]) & 255)]);
        if ((sideToMove == petir_header::whiteSide)) {
            candidates = (candidates & position.whiteQueens);
        } else {
            candidates = (candidates & position.blackQueens);
        }
    }
    else if (((pieceLetter == 'K'))) {
        candidates = petir_bitboard_mask::kingAttacks[toSquare];
        if ((sideToMove == petir_header::whiteSide)) {
            candidates = (candidates & position.whiteKingBoard);
        } else {
            candidates = (candidates & position.blackKingBoard);
        }
    }
    if ((fromFile != 9)) {
        candidates = (candidates & petir_bitboard_mask::fileMask[fromFile]);
    } else {
        if ((fromRank != 9)) {
            candidates = (candidates & petir_bitboard_mask::rankMask[fromRank]);
        }
    }
    fromSquare = petir_bitboard_mask::firstBit(pas::address(candidates));
    result = (petir_notation::squareName(fromSquare) + petir_notation::squareName(toSquare));
    return result;
};

    if (((notation[1] >= 'a' && notation[1] <= 'h'))) {
        result = pawnMoveToLong();
    } else {
        if (((notation[1] == 'N') || (notation[1] == 'B') || (notation[1] == 'R') || (notation[1] == 'Q') || (notation[1] == 'K'))) {
            result = pieceMoveToLong();
        }
    }
    return result;
}

// notation.pas:22
pas::String squareName(std::int32_t square) {
    pas::String result{};
    std::int32_t squareCase{};
    squareCase = square;
    if (((squareCase == petir_header::a1))) {
        result = pas::String("a1");
    }
    else if (((squareCase == petir_header::a2))) {
        result = pas::String("a2");
    }
    else if (((squareCase == petir_header::a3))) {
        result = pas::String("a3");
    }
    else if (((squareCase == petir_header::a4))) {
        result = pas::String("a4");
    }
    else if (((squareCase == petir_header::a5))) {
        result = pas::String("a5");
    }
    else if (((squareCase == petir_header::a6))) {
        result = pas::String("a6");
    }
    else if (((squareCase == petir_header::a7))) {
        result = pas::String("a7");
    }
    else if (((squareCase == petir_header::a8))) {
        result = pas::String("a8");
    }
    else if (((squareCase == petir_header::b1))) {
        result = pas::String("b1");
    }
    else if (((squareCase == petir_header::b2))) {
        result = pas::String("b2");
    }
    else if (((squareCase == petir_header::b3))) {
        result = pas::String("b3");
    }
    else if (((squareCase == petir_header::b4))) {
        result = pas::String("b4");
    }
    else if (((squareCase == petir_header::b5))) {
        result = pas::String("b5");
    }
    else if (((squareCase == petir_header::b6))) {
        result = pas::String("b6");
    }
    else if (((squareCase == petir_header::b7))) {
        result = pas::String("b7");
    }
    else if (((squareCase == petir_header::b8))) {
        result = pas::String("b8");
    }
    else if (((squareCase == petir_header::c1))) {
        result = pas::String("c1");
    }
    else if (((squareCase == petir_header::c2))) {
        result = pas::String("c2");
    }
    else if (((squareCase == petir_header::c3))) {
        result = pas::String("c3");
    }
    else if (((squareCase == petir_header::c4))) {
        result = pas::String("c4");
    }
    else if (((squareCase == petir_header::c5))) {
        result = pas::String("c5");
    }
    else if (((squareCase == petir_header::c6))) {
        result = pas::String("c6");
    }
    else if (((squareCase == petir_header::c7))) {
        result = pas::String("c7");
    }
    else if (((squareCase == petir_header::c8))) {
        result = pas::String("c8");
    }
    else if (((squareCase == petir_header::d1))) {
        result = pas::String("d1");
    }
    else if (((squareCase == petir_header::d2))) {
        result = pas::String("d2");
    }
    else if (((squareCase == petir_header::d3))) {
        result = pas::String("d3");
    }
    else if (((squareCase == petir_header::d4))) {
        result = pas::String("d4");
    }
    else if (((squareCase == petir_header::d5))) {
        result = pas::String("d5");
    }
    else if (((squareCase == petir_header::d6))) {
        result = pas::String("d6");
    }
    else if (((squareCase == petir_header::d7))) {
        result = pas::String("d7");
    }
    else if (((squareCase == petir_header::d8))) {
        result = pas::String("d8");
    }
    else if (((squareCase == petir_header::e1))) {
        result = pas::String("e1");
    }
    else if (((squareCase == petir_header::e2))) {
        result = pas::String("e2");
    }
    else if (((squareCase == petir_header::e3))) {
        result = pas::String("e3");
    }
    else if (((squareCase == petir_header::e4))) {
        result = pas::String("e4");
    }
    else if (((squareCase == petir_header::e5))) {
        result = pas::String("e5");
    }
    else if (((squareCase == petir_header::e6))) {
        result = pas::String("e6");
    }
    else if (((squareCase == petir_header::e7))) {
        result = pas::String("e7");
    }
    else if (((squareCase == petir_header::e8))) {
        result = pas::String("e8");
    }
    else if (((squareCase == petir_header::f1))) {
        result = pas::String("f1");
    }
    else if (((squareCase == petir_header::f2))) {
        result = pas::String("f2");
    }
    else if (((squareCase == petir_header::f3))) {
        result = pas::String("f3");
    }
    else if (((squareCase == petir_header::f4))) {
        result = pas::String("f4");
    }
    else if (((squareCase == petir_header::f5))) {
        result = pas::String("f5");
    }
    else if (((squareCase == petir_header::f6))) {
        result = pas::String("f6");
    }
    else if (((squareCase == petir_header::f7))) {
        result = pas::String("f7");
    }
    else if (((squareCase == petir_header::f8))) {
        result = pas::String("f8");
    }
    else if (((squareCase == petir_header::g1))) {
        result = pas::String("g1");
    }
    else if (((squareCase == petir_header::g2))) {
        result = pas::String("g2");
    }
    else if (((squareCase == petir_header::g3))) {
        result = pas::String("g3");
    }
    else if (((squareCase == petir_header::g4))) {
        result = pas::String("g4");
    }
    else if (((squareCase == petir_header::g5))) {
        result = pas::String("g5");
    }
    else if (((squareCase == petir_header::g6))) {
        result = pas::String("g6");
    }
    else if (((squareCase == petir_header::g7))) {
        result = pas::String("g7");
    }
    else if (((squareCase == petir_header::g8))) {
        result = pas::String("g8");
    }
    else if (((squareCase == petir_header::h1))) {
        result = pas::String("h1");
    }
    else if (((squareCase == petir_header::h2))) {
        result = pas::String("h2");
    }
    else if (((squareCase == petir_header::h3))) {
        result = pas::String("h3");
    }
    else if (((squareCase == petir_header::h4))) {
        result = pas::String("h4");
    }
    else if (((squareCase == petir_header::h5))) {
        result = pas::String("h5");
    }
    else if (((squareCase == petir_header::h6))) {
        result = pas::String("h6");
    }
    else if (((squareCase == petir_header::h7))) {
        result = pas::String("h7");
    }
    else if (((squareCase == petir_header::h8))) {
        result = pas::String("h8");
    }
    return result;
}

// notation.pas:14
std::int32_t squareFromName(pas::String name) {
    std::int32_t result{};
    std::uint8_t file{};
    std::uint8_t rank{};
    file = pas::sub(pas::charCode(name[1]), 97);
    rank = pas::sub(pas::strToInt(name[2]), 1);
    result = pas::add(pas::mul(rank, 8), file);
    return result;
}

}
