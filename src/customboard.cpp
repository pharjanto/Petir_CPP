// Faithful translation of customboard.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_customboard {
std::int32_t fenIndex{};
std::int32_t scanIndex{};
std::int32_t fenSquare{};
// customboard.pas:83
void rebuildBitboards(petir_header::tdata& position) {
    std::int32_t square{};
    std::int32_t lastSquare{};
    position.whitePieces = 0;
    position.blackPieces = 0;
    position.allPiecesRotated90 = 0;
    position.allPiecesA8H1 = 0;
    position.allPiecesH8A1 = 0;
    position.allPieces = 0;
    position.blackKingBoard = 0;
    position.whiteKingBoard = 0;
    position.blackPawns = 0;
    position.whitePawns = 0;
    position.blackKnights = 0;
    position.whiteKnights = 0;
    position.blackBishops = 0;
    position.whiteBishops = 0;
    position.blackRooks = 0;
    position.whiteRooks = 0;
    position.blackQueens = 0;
    position.whiteQueens = 0;
    position.materialScore = 0;
    position.hashKey = 0;
    position.enPassantFile = petir_header::noEnPassant;
    position.whiteNonPawnMaterial = 0;
    position.blackNonPawnMaterial = 0;
    position.whiteCastleType = 0;
    position.blackCastleType = 0;
    position.blackBishopCount = 0;
    position.whiteBishopCount = 0;
    lastSquare = 63;
    for (square = 0; square <= lastSquare; ++square) {
        if ((position.board[square] == petir_header::whitePawn)) {
            (position.materialScore = pas::increment(position.materialScore, petir_header::pawnValue));
            position.whitePawns = (position.whitePawns | petir_bitboard_mask::squareMask[square]);
            position.allPieces = (position.allPieces | petir_bitboard_mask::squareMask[square]);
            position.whitePieces = (position.whitePieces | petir_bitboard_mask::squareMask[square]);
            position.allPiecesRotated90 = (position.allPiecesRotated90 | petir_bitboard_mask::rotated90SquareMask[square]);
            position.allPiecesA8H1 = (position.allPiecesA8H1 | petir_bitboard_mask::rotatedA8H1SquareMask[square]);
            position.allPiecesH8A1 = (position.allPiecesH8A1 | petir_bitboard_mask::rotatedH8A1SquareMask[square]);
        } else {
            if ((position.board[square] == petir_header::blackPawn)) {
                (position.materialScore = pas::decrement(position.materialScore, petir_header::pawnValue));
                position.blackPawns = (position.blackPawns | petir_bitboard_mask::squareMask[square]);
                position.allPieces = (position.allPieces | petir_bitboard_mask::squareMask[square]);
                position.blackPieces = (position.blackPieces | petir_bitboard_mask::squareMask[square]);
                position.allPiecesRotated90 = (position.allPiecesRotated90 | petir_bitboard_mask::rotated90SquareMask[square]);
                position.allPiecesA8H1 = (position.allPiecesA8H1 | petir_bitboard_mask::rotatedA8H1SquareMask[square]);
                position.allPiecesH8A1 = (position.allPiecesH8A1 | petir_bitboard_mask::rotatedH8A1SquareMask[square]);
            }
        }
        if ((position.board[square] == petir_header::whiteKnight)) {
            (position.materialScore = pas::increment(position.materialScore, petir_header::knightValue));
            position.whiteKnights = (position.whiteKnights | petir_bitboard_mask::squareMask[square]);
            position.allPieces = (position.allPieces | petir_bitboard_mask::squareMask[square]);
            position.whitePieces = (position.whitePieces | petir_bitboard_mask::squareMask[square]);
            position.allPiecesRotated90 = (position.allPiecesRotated90 | petir_bitboard_mask::rotated90SquareMask[square]);
            position.allPiecesA8H1 = (position.allPiecesA8H1 | petir_bitboard_mask::rotatedA8H1SquareMask[square]);
            position.allPiecesH8A1 = (position.allPiecesH8A1 | petir_bitboard_mask::rotatedH8A1SquareMask[square]);
            (position.whiteNonPawnMaterial = pas::increment(position.whiteNonPawnMaterial, 3));
        } else {
            if ((position.board[square] == petir_header::whiteBishop)) {
                (position.materialScore = pas::increment(position.materialScore, petir_header::bishopValue));
                position.whiteBishops = (position.whiteBishops | petir_bitboard_mask::squareMask[square]);
                position.allPieces = (position.allPieces | petir_bitboard_mask::squareMask[square]);
                position.whitePieces = (position.whitePieces | petir_bitboard_mask::squareMask[square]);
                position.allPiecesRotated90 = (position.allPiecesRotated90 | petir_bitboard_mask::rotated90SquareMask[square]);
                position.allPiecesA8H1 = (position.allPiecesA8H1 | petir_bitboard_mask::rotatedA8H1SquareMask[square]);
                position.allPiecesH8A1 = (position.allPiecesH8A1 | petir_bitboard_mask::rotatedH8A1SquareMask[square]);
                (position.whiteBishopCount = pas::increment(position.whiteBishopCount, 1));
                (position.whiteNonPawnMaterial = pas::increment(position.whiteNonPawnMaterial, 3));
            } else {
                if ((position.board[square] == petir_header::whiteRook)) {
                    (position.materialScore = pas::increment(position.materialScore, petir_header::rookValue));
                    position.whiteRooks = (position.whiteRooks | petir_bitboard_mask::squareMask[square]);
                    position.allPieces = (position.allPieces | petir_bitboard_mask::squareMask[square]);
                    position.whitePieces = (position.whitePieces | petir_bitboard_mask::squareMask[square]);
                    position.allPiecesRotated90 = (position.allPiecesRotated90 | petir_bitboard_mask::rotated90SquareMask[square]);
                    position.allPiecesA8H1 = (position.allPiecesA8H1 | petir_bitboard_mask::rotatedA8H1SquareMask[square]);
                    position.allPiecesH8A1 = (position.allPiecesH8A1 | petir_bitboard_mask::rotatedH8A1SquareMask[square]);
                    (position.whiteNonPawnMaterial = pas::increment(position.whiteNonPawnMaterial, 5));
                } else {
                    if ((position.board[square] == petir_header::whiteQueen)) {
                        (position.materialScore = pas::increment(position.materialScore, petir_header::queenValue));
                        position.whiteQueens = (position.whiteQueens | petir_bitboard_mask::squareMask[square]);
                        position.allPieces = (position.allPieces | petir_bitboard_mask::squareMask[square]);
                        position.whitePieces = (position.whitePieces | petir_bitboard_mask::squareMask[square]);
                        position.allPiecesRotated90 = (position.allPiecesRotated90 | petir_bitboard_mask::rotated90SquareMask[square]);
                        position.allPiecesA8H1 = (position.allPiecesA8H1 | petir_bitboard_mask::rotatedA8H1SquareMask[square]);
                        position.allPiecesH8A1 = (position.allPiecesH8A1 | petir_bitboard_mask::rotatedH8A1SquareMask[square]);
                        (position.whiteNonPawnMaterial = pas::increment(position.whiteNonPawnMaterial, 9));
                    } else {
                        if ((position.board[square] == petir_header::whiteKing)) {
                            position.whiteKingBoard = (position.whiteKingBoard | petir_bitboard_mask::squareMask[square]);
                            position.allPieces = (position.allPieces | petir_bitboard_mask::squareMask[square]);
                            position.whitePieces = (position.whitePieces | petir_bitboard_mask::squareMask[square]);
                            position.allPiecesRotated90 = (position.allPiecesRotated90 | petir_bitboard_mask::rotated90SquareMask[square]);
                            position.allPiecesA8H1 = (position.allPiecesA8H1 | petir_bitboard_mask::rotatedA8H1SquareMask[square]);
                            position.allPiecesH8A1 = (position.allPiecesH8A1 | petir_bitboard_mask::rotatedH8A1SquareMask[square]);
                        } else {
                            if ((position.board[square] == petir_header::blackKnight)) {
                                (position.materialScore = pas::decrement(position.materialScore, petir_header::knightValue));
                                position.blackKnights = (position.blackKnights | petir_bitboard_mask::squareMask[square]);
                                position.allPieces = (position.allPieces | petir_bitboard_mask::squareMask[square]);
                                position.blackPieces = (position.blackPieces | petir_bitboard_mask::squareMask[square]);
                                position.allPiecesRotated90 = (position.allPiecesRotated90 | petir_bitboard_mask::rotated90SquareMask[square]);
                                position.allPiecesA8H1 = (position.allPiecesA8H1 | petir_bitboard_mask::rotatedA8H1SquareMask[square]);
                                position.allPiecesH8A1 = (position.allPiecesH8A1 | petir_bitboard_mask::rotatedH8A1SquareMask[square]);
                                (position.blackNonPawnMaterial = pas::increment(position.blackNonPawnMaterial, 3));
                            } else {
                                if ((position.board[square] == petir_header::blackBishop)) {
                                    (position.materialScore = pas::decrement(position.materialScore, petir_header::bishopValue));
                                    position.blackBishops = (position.blackBishops | petir_bitboard_mask::squareMask[square]);
                                    position.allPieces = (position.allPieces | petir_bitboard_mask::squareMask[square]);
                                    position.blackPieces = (position.blackPieces | petir_bitboard_mask::squareMask[square]);
                                    position.allPiecesRotated90 = (position.allPiecesRotated90 | petir_bitboard_mask::rotated90SquareMask[square]);
                                    position.allPiecesA8H1 = (position.allPiecesA8H1 | petir_bitboard_mask::rotatedA8H1SquareMask[square]);
                                    position.allPiecesH8A1 = (position.allPiecesH8A1 | petir_bitboard_mask::rotatedH8A1SquareMask[square]);
                                    (position.blackNonPawnMaterial = pas::increment(position.blackNonPawnMaterial, 3));
                                    (position.blackBishopCount = pas::increment(position.blackBishopCount, 1));
                                } else {
                                    if ((position.board[square] == petir_header::blackRook)) {
                                        (position.materialScore = pas::decrement(position.materialScore, petir_header::rookValue));
                                        position.blackRooks = (position.blackRooks | petir_bitboard_mask::squareMask[square]);
                                        position.allPieces = (position.allPieces | petir_bitboard_mask::squareMask[square]);
                                        position.blackPieces = (position.blackPieces | petir_bitboard_mask::squareMask[square]);
                                        position.allPiecesRotated90 = (position.allPiecesRotated90 | petir_bitboard_mask::rotated90SquareMask[square]);
                                        position.allPiecesA8H1 = (position.allPiecesA8H1 | petir_bitboard_mask::rotatedA8H1SquareMask[square]);
                                        position.allPiecesH8A1 = (position.allPiecesH8A1 | petir_bitboard_mask::rotatedH8A1SquareMask[square]);
                                        (position.blackNonPawnMaterial = pas::increment(position.blackNonPawnMaterial, 5));
                                    } else {
                                        if ((position.board[square] == petir_header::blackQueen)) {
                                            (position.materialScore = pas::decrement(position.materialScore, petir_header::queenValue));
                                            position.blackQueens = (position.blackQueens | petir_bitboard_mask::squareMask[square]);
                                            position.allPieces = (position.allPieces | petir_bitboard_mask::squareMask[square]);
                                            position.blackPieces = (position.blackPieces | petir_bitboard_mask::squareMask[square]);
                                            position.allPiecesRotated90 = (position.allPiecesRotated90 | petir_bitboard_mask::rotated90SquareMask[square]);
                                            position.allPiecesA8H1 = (position.allPiecesA8H1 | petir_bitboard_mask::rotatedA8H1SquareMask[square]);
                                            position.allPiecesH8A1 = (position.allPiecesH8A1 | petir_bitboard_mask::rotatedH8A1SquareMask[square]);
                                            (position.blackNonPawnMaterial = pas::increment(position.blackNonPawnMaterial, 9));
                                        } else {
                                            if ((position.board[square] == petir_header::blackKing)) {
                                                position.blackKingBoard = (position.blackKingBoard | petir_bitboard_mask::squareMask[square]);
                                                position.allPieces = (position.allPieces | petir_bitboard_mask::squareMask[square]);
                                                position.blackPieces = (position.blackPieces | petir_bitboard_mask::squareMask[square]);
                                                position.allPiecesRotated90 = (position.allPiecesRotated90 | petir_bitboard_mask::rotated90SquareMask[square]);
                                                position.allPiecesA8H1 = (position.allPiecesA8H1 | petir_bitboard_mask::rotatedA8H1SquareMask[square]);
                                                position.allPiecesH8A1 = (position.allPiecesH8A1 | petir_bitboard_mask::rotatedH8A1SquareMask[square]);
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
    }
    position.fiftyMoveCounter = 0;
    position.rooksAndQueens = (((position.blackQueens | position.whiteQueens) | position.blackRooks) | position.whiteRooks);
    position.bishopsAndQueens = (((position.blackQueens | position.whiteQueens) | position.blackBishops) | position.whiteBishops);
}

// customboard.pas:14
void readFen(pas::String fen, petir_header::tdata& position, bool& whiteToMove) {
    bool whiteLongCastle{};
    bool whiteShortCastle{};
    bool blackLongCastle{};
    bool blackShortCastle{};
    std::int32_t fenLength{};
    std::int32_t lastEmptySquare{};
    std::int32_t castlingChar{};
    petir_customboard::fenSquare = 56;
    position.castleRights = 0;
    fenLength = pas::length(fen);
    for (petir_customboard::fenIndex = 1; petir_customboard::fenIndex <= fenLength; ++petir_customboard::fenIndex) {
        if (((fen[petir_customboard::fenIndex] >= '0' && fen[petir_customboard::fenIndex] <= '9'))) {
            lastEmptySquare = pas::sub(pas::add(petir_customboard::fenSquare, pas::strToInt(fen[petir_customboard::fenIndex])), 1);
            for (petir_customboard::scanIndex = petir_customboard::fenSquare; petir_customboard::scanIndex <= lastEmptySquare; ++petir_customboard::scanIndex) {
                position.board[petir_customboard::scanIndex] = 0;
            }
            petir_customboard::fenSquare = pas::add(petir_customboard::fenSquare, pas::strToInt(fen[petir_customboard::fenIndex]));
        } else {
            if ((fen[petir_customboard::fenIndex] == 'R')) {
                position.board[petir_customboard::fenSquare] = petir_header::whiteRook;
            } else {
                if ((fen[petir_customboard::fenIndex] == 'B')) {
                    position.board[petir_customboard::fenSquare] = petir_header::whiteBishop;
                } else {
                    if ((fen[petir_customboard::fenIndex] == 'N')) {
                        position.board[petir_customboard::fenSquare] = petir_header::whiteKnight;
                    } else {
                        if ((fen[petir_customboard::fenIndex] == 'Q')) {
                            position.board[petir_customboard::fenSquare] = petir_header::whiteQueen;
                        } else {
                            if ((fen[petir_customboard::fenIndex] == 'P')) {
                                position.board[petir_customboard::fenSquare] = petir_header::whitePawn;
                            } else {
                                if ((fen[petir_customboard::fenIndex] == 'K')) {
                                    position.board[petir_customboard::fenSquare] = petir_header::whiteKing;
                                } else {
                                    if ((fen[petir_customboard::fenIndex] == 'r')) {
                                        position.board[petir_customboard::fenSquare] = petir_header::blackRook;
                                    } else {
                                        if ((fen[petir_customboard::fenIndex] == 'b')) {
                                            position.board[petir_customboard::fenSquare] = petir_header::blackBishop;
                                        } else {
                                            if ((fen[petir_customboard::fenIndex] == 'n')) {
                                                position.board[petir_customboard::fenSquare] = petir_header::blackKnight;
                                            } else {
                                                if ((fen[petir_customboard::fenIndex] == 'q')) {
                                                    position.board[petir_customboard::fenSquare] = petir_header::blackQueen;
                                                } else {
                                                    if ((fen[petir_customboard::fenIndex] == 'p')) {
                                                        position.board[petir_customboard::fenSquare] = petir_header::blackPawn;
                                                    } else {
                                                        if ((fen[petir_customboard::fenIndex] == 'k')) {
                                                            position.board[petir_customboard::fenSquare] = petir_header::blackKing;
                                                        } else {
                                                            if ((fen[petir_customboard::fenIndex] == '/')) {
                                                                petir_customboard::fenSquare = pas::sub(petir_customboard::fenSquare, 16);
                                                            } else {
                                                                if ((fen[petir_customboard::fenIndex] == ' ')) {
                                                                    if ((fen[pas::add(petir_customboard::fenIndex, 1)] == 'b')) {
                                                                        whiteToMove = false;
                                                                    } else {
                                                                        if ((fen[pas::add(petir_customboard::fenIndex, 1)] == 'w')) {
                                                                            whiteToMove = true;
                                                                        }
                                                                    }
                                                                    petir_customboard::scanIndex = petir_customboard::fenIndex;
                                                                    (petir_customboard::scanIndex = pas::increment(petir_customboard::scanIndex, 3));
                                                                    whiteLongCastle = false;
                                                                    whiteShortCastle = false;
                                                                    blackLongCastle = false;
                                                                    blackShortCastle = false;
                                                                    while (((petir_customboard::scanIndex <= pas::length(fen)) && ((fen[petir_customboard::scanIndex] == 'K') || (fen[petir_customboard::scanIndex] == 'Q') || (fen[petir_customboard::scanIndex] == 'k') || (fen[petir_customboard::scanIndex] == 'q')))) {
                                                                        castlingChar = fen[petir_customboard::scanIndex];
                                                                        if (((castlingChar == 'K'))) {
                                                                            whiteShortCastle = true;
                                                                        }
                                                                        else if (((castlingChar == 'Q'))) {
                                                                            whiteLongCastle = true;
                                                                        }
                                                                        else if (((castlingChar == 'k'))) {
                                                                            blackShortCastle = true;
                                                                        }
                                                                        else if (((castlingChar == 'q'))) {
                                                                            blackLongCastle = true;
                                                                        }
                                                                        (petir_customboard::scanIndex = pas::increment(petir_customboard::scanIndex, 1));
                                                                    }
                                                                    if (whiteShortCastle) {
                                                                        position.castleRights = (position.castleRights | petir_header::whiteShortCastleRight);
                                                                    }
                                                                    if (whiteLongCastle) {
                                                                        position.castleRights = (position.castleRights | petir_header::whiteLongCastleRight);
                                                                    }
                                                                    if (blackShortCastle) {
                                                                        position.castleRights = (position.castleRights | petir_header::blackShortCastleRight);
                                                                    }
                                                                    if (blackLongCastle) {
                                                                        position.castleRights = (position.castleRights | petir_header::blackLongCastleRight);
                                                                    }
                                                                    return;
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
                        }
                    }
                }
            }
        }
        if (((fen[petir_customboard::fenIndex] >= 'a' && fen[petir_customboard::fenIndex] <= 'z') || (fen[petir_customboard::fenIndex] >= 'A' && fen[petir_customboard::fenIndex] <= 'Z'))) {
            (petir_customboard::fenSquare = pas::increment(petir_customboard::fenSquare, 1));
        }
    }
}

}
