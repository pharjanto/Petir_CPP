// Faithful translation of makemove.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_makemove {
// makemove.pas:413
void makeWhiteMove(std::int32_t move, petir_header::tdata& position) {
    std::int32_t capturedPiece{};
    std::int32_t fromSquare{};
    std::int32_t toSquare{};
    std::int32_t promotion{};
    std::int32_t specialTarget{};
    (petir_header::nodeCount = pas::increment(petir_header::nodeCount, 1));
    fromSquare = (move & 127);
    toSquare = (pas::shiftRight(move, 7) & 127);
    (position.fiftyMoveCounter = pas::increment(position.fiftyMoveCounter, 1));
    specialTarget = toSquare;
    if (((specialTarget == petir_header::shortCastleTarget))) {
        position.lastCaptureType = 0;
        petir_makemove::castleWhiteShort(position);
    }
    else if (((specialTarget == petir_header::longCastleTarget))) {
        position.lastCaptureType = 0;
        petir_makemove::castleWhiteLong(position);
    }
    else if (((specialTarget == petir_header::enPassantTarget))) {
        petir_makemove::makeWhiteEnPassant(position, fromSquare, toSquare);
        position.lastCaptureType = petir_header::whitePawn;
    }
    else {
        if ((position.enPassantFile != petir_header::noEnPassant)) {
            position.hashKey = ((position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]) ^ petir_header::zobristEnPassantKeys[petir_header::noEnPassant]);
            position.enPassantFile = petir_header::noEnPassant;
        }
        capturedPiece = position.board[toSquare];
        if ((capturedPiece != 0)) {
            position.fiftyMoveCounter = 0;
            position.blackPieces = (position.blackPieces & petir_bitboard_mask::squareBitInverted(toSquare));
            if ((capturedPiece == petir_header::blackPawn)) {
                position.blackPawns = (position.blackPawns & petir_bitboard_mask::squareBitInverted(toSquare));
                position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackPawn][toSquare]);
                position.lastCaptureType = petir_header::whitePawn;
                (position.materialScore = pas::increment(position.materialScore, petir_header::pawnValue));
            } else {
                if ((capturedPiece == petir_header::blackKnight)) {
                    (position.materialScore = pas::increment(position.materialScore, petir_header::knightValue));
                    position.blackKnights = (position.blackKnights & petir_bitboard_mask::squareBitInverted(toSquare));
                    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackKnight][toSquare]);
                    position.lastCaptureType = petir_header::whiteKnight;
                    (position.blackNonPawnMaterial = pas::decrement(position.blackNonPawnMaterial, 3));
                } else {
                    if ((capturedPiece == petir_header::blackBishop)) {
                        (position.materialScore = pas::increment(position.materialScore, petir_header::bishopValue));
                        position.blackBishops = (position.blackBishops & petir_bitboard_mask::squareBitInverted(toSquare));
                        position.bishopsAndQueens = (position.bishopsAndQueens & petir_bitboard_mask::squareBitInverted(toSquare));
                        position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackBishop][toSquare]);
                        position.lastCaptureType = petir_header::whiteKnight;
                        (position.blackBishopCount = pas::decrement(position.blackBishopCount, 1));
                        (position.blackNonPawnMaterial = pas::decrement(position.blackNonPawnMaterial, 3));
                    } else {
                        if ((capturedPiece == petir_header::blackRook)) {
                            (position.materialScore = pas::increment(position.materialScore, petir_header::rookValue));
                            position.blackRooks = (position.blackRooks & petir_bitboard_mask::squareBitInverted(toSquare));
                            position.rooksAndQueens = (position.rooksAndQueens & petir_bitboard_mask::squareBitInverted(toSquare));
                            position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackRook][toSquare]);
                            position.lastCaptureType = petir_header::whiteRook;
                            (position.blackNonPawnMaterial = pas::decrement(position.blackNonPawnMaterial, 5));
                        } else {
                            (position.materialScore = pas::increment(position.materialScore, petir_header::queenValue));
                            position.blackQueens = (position.blackQueens & petir_bitboard_mask::squareBitInverted(toSquare));
                            position.rooksAndQueens = (position.rooksAndQueens & petir_bitboard_mask::squareBitInverted(toSquare));
                            position.bishopsAndQueens = (position.bishopsAndQueens & petir_bitboard_mask::squareBitInverted(toSquare));
                            position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackQueen][toSquare]);
                            position.lastCaptureType = petir_header::whiteQueen;
                            (position.blackNonPawnMaterial = pas::decrement(position.blackNonPawnMaterial, 9));
                        }
                    }
                }
            }
        } else {
            position.lastCaptureType = 0;
            if ((((position.board[fromSquare] == petir_header::whitePawn) && (pas::sub(toSquare, fromSquare) == 16)) && (((position.board[pas::sub(toSquare, 1)] == petir_header::blackPawn) && ((toSquare & 7) >= 1)) || ((position.board[pas::add(toSquare, 1)] == petir_header::blackPawn) && ((toSquare & 7) <= 6))))) {
                position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]);
                position.enPassantFile = (fromSquare & 7);
                position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]);
            }
        }
        position.board[toSquare] = position.board[fromSquare];
        position.board[fromSquare] = 0;
        position.whitePieces = (position.whitePieces & petir_bitboard_mask::squareBitInverted(fromSquare));
        position.whitePieces = (position.whitePieces | petir_bitboard_mask::squareBit(toSquare));
        position.allPieces = (position.allPieces & petir_bitboard_mask::squareBitInverted(fromSquare));
        position.allPieces = (position.allPieces | petir_bitboard_mask::squareBit(toSquare));
        position.allPiecesRotated90 = (position.allPiecesRotated90 & (~petir_bitboard_mask::rotated90SquareMask[fromSquare]));
        position.allPiecesRotated90 = (position.allPiecesRotated90 | petir_bitboard_mask::rotated90SquareMask[toSquare]);
        position.allPiecesA8H1 = (position.allPiecesA8H1 & (~petir_bitboard_mask::rotatedA8H1SquareMask[fromSquare]));
        position.allPiecesA8H1 = (position.allPiecesA8H1 | petir_bitboard_mask::rotatedA8H1SquareMask[toSquare]);
        position.allPiecesH8A1 = (position.allPiecesH8A1 & (~petir_bitboard_mask::rotatedH8A1SquareMask[fromSquare]));
        position.allPiecesH8A1 = (position.allPiecesH8A1 | petir_bitboard_mask::rotatedH8A1SquareMask[toSquare]);
        if ((position.board[toSquare] == petir_header::whitePawn)) {
            position.fiftyMoveCounter = 0;
            position.whitePawns = (position.whitePawns | petir_bitboard_mask::squareBit(toSquare));
            position.whitePawns = (position.whitePawns & petir_bitboard_mask::squareBitInverted(fromSquare));
            position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whitePawn][fromSquare]);
            position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whitePawn][toSquare]);
            promotion = (pas::shiftRight(move, 14) & 7);
            if (((toSquare >= petir_header::a8) && (promotion == 0))) {
                promotion = petir_header::queenPromotion;
            }
            if ((promotion != 0)) {
                petir_makemove::promoteWhitePawn(position, toSquare, promotion);
                position.lastCaptureType = 10;
            }
        } else {
            if ((position.board[toSquare] == petir_header::whiteKnight)) {
                position.whiteKnights = (position.whiteKnights | petir_bitboard_mask::squareBit(toSquare));
                position.whiteKnights = (position.whiteKnights & petir_bitboard_mask::squareBitInverted(fromSquare));
                position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteKnight][fromSquare]);
                position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteKnight][toSquare]);
            } else {
                if ((position.board[toSquare] == petir_header::whiteBishop)) {
                    position.whiteBishops = (position.whiteBishops | petir_bitboard_mask::squareBit(toSquare));
                    position.bishopsAndQueens = (position.bishopsAndQueens | petir_bitboard_mask::squareBit(toSquare));
                    position.whiteBishops = (position.whiteBishops & petir_bitboard_mask::squareBitInverted(fromSquare));
                    position.bishopsAndQueens = (position.bishopsAndQueens & petir_bitboard_mask::squareBitInverted(fromSquare));
                    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteBishop][fromSquare]);
                    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteBishop][toSquare]);
                } else {
                    if ((position.board[toSquare] == petir_header::whiteRook)) {
                        position.whiteRooks = (position.whiteRooks | petir_bitboard_mask::squareBit(toSquare));
                        position.rooksAndQueens = (position.rooksAndQueens | petir_bitboard_mask::squareBit(toSquare));
                        position.whiteRooks = (position.whiteRooks & petir_bitboard_mask::squareBitInverted(fromSquare));
                        position.rooksAndQueens = (position.rooksAndQueens & petir_bitboard_mask::squareBitInverted(fromSquare));
                        position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteRook][fromSquare]);
                        position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteRook][toSquare]);
                        if (((position.castleRights & petir_header::whiteCastleRights) != 0)) {
                            if ((fromSquare == petir_header::h1)) {
                                position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
                                position.castleRights = (position.castleRights & (~petir_header::whiteShortCastleRight));
                                position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
                            } else {
                                if ((fromSquare == petir_header::a1)) {
                                    position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
                                    position.castleRights = (position.castleRights & (~petir_header::whiteLongCastleRight));
                                    position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
                                }
                            }
                        }
                    } else {
                        if ((position.board[toSquare] == petir_header::whiteQueen)) {
                            position.whiteQueens = (position.whiteQueens | petir_bitboard_mask::squareBit(toSquare));
                            position.bishopsAndQueens = (position.bishopsAndQueens | petir_bitboard_mask::squareBit(toSquare));
                            position.rooksAndQueens = (position.rooksAndQueens | petir_bitboard_mask::squareBit(toSquare));
                            position.whiteQueens = (position.whiteQueens & petir_bitboard_mask::squareBitInverted(fromSquare));
                            position.rooksAndQueens = (position.rooksAndQueens & petir_bitboard_mask::squareBitInverted(fromSquare));
                            position.bishopsAndQueens = (position.bishopsAndQueens & petir_bitboard_mask::squareBitInverted(fromSquare));
                            position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteQueen][fromSquare]);
                            position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteQueen][toSquare]);
                        } else {
                            if ((position.board[toSquare] == petir_header::whiteKing)) {
                                position.whiteKingBoard = (position.whiteKingBoard | petir_bitboard_mask::squareBit(toSquare));
                                position.whiteKingBoard = (position.whiteKingBoard & petir_bitboard_mask::squareBitInverted(fromSquare));
                                position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteKing][fromSquare]);
                                position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteKing][toSquare]);
                                if (((position.castleRights & petir_header::whiteCastleRights) != 0)) {
                                    position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
                                    position.castleRights = (position.castleRights & (~petir_header::whiteCastleRights));
                                    position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// makemove.pas:627
void makeBlackMove(std::int32_t move, petir_header::tdata& position) {
    std::int32_t capturedPiece{};
    std::int32_t piece{};
    std::int32_t fromSquare{};
    std::int32_t toSquare{};
    std::int32_t promotion{};
    std::int32_t specialTarget{};
    (petir_header::nodeCount = pas::increment(petir_header::nodeCount, 1));
    fromSquare = (move & 127);
    toSquare = (pas::shiftRight(move, 7) & 127);
    (position.fiftyMoveCounter = pas::increment(position.fiftyMoveCounter, 1));
    position.lastCaptureType = 0;
    specialTarget = toSquare;
    if (((specialTarget == petir_header::shortCastleTarget))) {
        petir_makemove::castleBlackShort(position);
    }
    else if (((specialTarget == petir_header::longCastleTarget))) {
        petir_makemove::castleBlackLong(position);
    }
    else if (((specialTarget == petir_header::enPassantTarget))) {
        petir_makemove::makeBlackEnPassant(position, fromSquare, toSquare);
        position.lastCaptureType = petir_header::whitePawn;
    }
    else {
        piece = position.board[fromSquare];
        if ((position.enPassantFile != petir_header::noEnPassant)) {
            position.hashKey = ((position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]) ^ petir_header::zobristEnPassantKeys[petir_header::noEnPassant]);
            position.enPassantFile = petir_header::noEnPassant;
        }
        capturedPiece = position.board[toSquare];
        position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[piece][fromSquare]);
        position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[piece][toSquare]);
        if ((capturedPiece != 0)) {
            position.fiftyMoveCounter = 0;
            position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[capturedPiece][toSquare]);
            position.whitePieces = (position.whitePieces & petir_bitboard_mask::squareBitInverted(toSquare));
            if ((capturedPiece == petir_header::whitePawn)) {
                position.whitePawns = (position.whitePawns & petir_bitboard_mask::squareBitInverted(toSquare));
                (position.materialScore = pas::decrement(position.materialScore, petir_header::pawnValue));
                position.lastCaptureType = petir_header::whitePawn;
            } else {
                if ((capturedPiece == petir_header::whiteKnight)) {
                    (position.materialScore = pas::decrement(position.materialScore, petir_header::knightValue));
                    position.whiteKnights = (position.whiteKnights & petir_bitboard_mask::squareBitInverted(toSquare));
                    (position.whiteNonPawnMaterial = pas::decrement(position.whiteNonPawnMaterial, 3));
                    position.lastCaptureType = petir_header::whiteKnight;
                } else {
                    if ((capturedPiece == petir_header::whiteBishop)) {
                        (position.materialScore = pas::decrement(position.materialScore, petir_header::bishopValue));
                        position.whiteBishops = (position.whiteBishops & petir_bitboard_mask::squareBitInverted(toSquare));
                        position.bishopsAndQueens = (position.bishopsAndQueens & petir_bitboard_mask::squareBitInverted(toSquare));
                        (position.whiteNonPawnMaterial = pas::decrement(position.whiteNonPawnMaterial, 3));
                        position.lastCaptureType = petir_header::whiteKnight;
                        (position.whiteBishopCount = pas::decrement(position.whiteBishopCount, 1));
                    } else {
                        if ((capturedPiece == petir_header::whiteRook)) {
                            (position.materialScore = pas::decrement(position.materialScore, petir_header::rookValue));
                            position.whiteRooks = (position.whiteRooks & petir_bitboard_mask::squareBitInverted(toSquare));
                            position.rooksAndQueens = (position.rooksAndQueens & petir_bitboard_mask::squareBitInverted(toSquare));
                            (position.whiteNonPawnMaterial = pas::decrement(position.whiteNonPawnMaterial, 5));
                            position.lastCaptureType = petir_header::whiteRook;
                        } else {
                            (position.materialScore = pas::decrement(position.materialScore, petir_header::queenValue));
                            position.whiteQueens = (position.whiteQueens & petir_bitboard_mask::squareBitInverted(toSquare));
                            position.rooksAndQueens = (position.rooksAndQueens & petir_bitboard_mask::squareBitInverted(toSquare));
                            position.bishopsAndQueens = (position.bishopsAndQueens & petir_bitboard_mask::squareBitInverted(toSquare));
                            (position.whiteNonPawnMaterial = pas::decrement(position.whiteNonPawnMaterial, 9));
                            position.lastCaptureType = petir_header::whiteQueen;
                        }
                    }
                }
            }
        } else {
            if ((((position.board[fromSquare] == petir_header::blackPawn) && (pas::sub(fromSquare, toSquare) == 16)) && (((position.board[pas::sub(toSquare, 1)] == petir_header::whitePawn) && ((toSquare & 7) >= 1)) || ((position.board[pas::add(toSquare, 1)] == petir_header::whitePawn) && ((toSquare & 7) <= 6))))) {
                position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]);
                position.enPassantFile = (fromSquare & 7);
                position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]);
            }
        }
        position.board[toSquare] = piece;
        position.board[fromSquare] = 0;
        position.blackPieces = (position.blackPieces & petir_bitboard_mask::squareBitInverted(fromSquare));
        position.blackPieces = (position.blackPieces | petir_bitboard_mask::squareBit(toSquare));
        position.allPieces = (position.allPieces & petir_bitboard_mask::squareBitInverted(fromSquare));
        position.allPieces = (position.allPieces | petir_bitboard_mask::squareBit(toSquare));
        position.allPiecesRotated90 = (position.allPiecesRotated90 & (~petir_bitboard_mask::rotated90SquareMask[fromSquare]));
        position.allPiecesRotated90 = (position.allPiecesRotated90 | petir_bitboard_mask::rotated90SquareMask[toSquare]);
        position.allPiecesA8H1 = (position.allPiecesA8H1 & (~petir_bitboard_mask::rotatedA8H1SquareMask[fromSquare]));
        position.allPiecesA8H1 = (position.allPiecesA8H1 | petir_bitboard_mask::rotatedA8H1SquareMask[toSquare]);
        position.allPiecesH8A1 = (position.allPiecesH8A1 & (~petir_bitboard_mask::rotatedH8A1SquareMask[fromSquare]));
        position.allPiecesH8A1 = (position.allPiecesH8A1 | petir_bitboard_mask::rotatedH8A1SquareMask[toSquare]);
        if ((piece == petir_header::blackPawn)) {
            position.fiftyMoveCounter = 0;
            position.blackPawns = (position.blackPawns | petir_bitboard_mask::squareBit(toSquare));
            position.blackPawns = (position.blackPawns & petir_bitboard_mask::squareBitInverted(fromSquare));
            promotion = (pas::shiftRight(move, 14) & 7);
            if (((toSquare <= petir_header::h1) && (promotion == 0))) {
                promotion = petir_header::queenPromotion;
            }
            if ((promotion != 0)) {
                petir_makemove::promoteBlackPawn(position, toSquare, promotion);
                position.lastCaptureType = 11;
            }
        } else {
            if ((piece == petir_header::blackKnight)) {
                position.blackKnights = (position.blackKnights | petir_bitboard_mask::squareBit(toSquare));
                position.blackKnights = (position.blackKnights & petir_bitboard_mask::squareBitInverted(fromSquare));
            } else {
                if ((piece == petir_header::blackBishop)) {
                    position.blackBishops = (position.blackBishops | petir_bitboard_mask::squareBit(toSquare));
                    position.bishopsAndQueens = (position.bishopsAndQueens | petir_bitboard_mask::squareBit(toSquare));
                    position.blackBishops = (position.blackBishops & petir_bitboard_mask::squareBitInverted(fromSquare));
                    position.bishopsAndQueens = (position.bishopsAndQueens & petir_bitboard_mask::squareBitInverted(fromSquare));
                } else {
                    if ((piece == petir_header::blackRook)) {
                        position.blackRooks = (position.blackRooks | petir_bitboard_mask::squareBit(toSquare));
                        position.rooksAndQueens = (position.rooksAndQueens | petir_bitboard_mask::squareBit(toSquare));
                        position.blackRooks = (position.blackRooks & petir_bitboard_mask::squareBitInverted(fromSquare));
                        position.rooksAndQueens = (position.rooksAndQueens & petir_bitboard_mask::squareBitInverted(fromSquare));
                        if (((position.castleRights & petir_header::blackCastleRights) != 0)) {
                            if ((fromSquare == petir_header::h8)) {
                                position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
                                position.castleRights = (position.castleRights & (~petir_header::blackShortCastleRight));
                                position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
                            } else {
                                if ((fromSquare == petir_header::a8)) {
                                    position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
                                    position.castleRights = (position.castleRights & (~petir_header::blackLongCastleRight));
                                    position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
                                }
                            }
                        }
                    } else {
                        if ((piece == petir_header::blackQueen)) {
                            position.blackQueens = (position.blackQueens | petir_bitboard_mask::squareBit(toSquare));
                            position.rooksAndQueens = (position.rooksAndQueens | petir_bitboard_mask::squareBit(toSquare));
                            position.bishopsAndQueens = (position.bishopsAndQueens | petir_bitboard_mask::squareBit(toSquare));
                            position.blackQueens = (position.blackQueens & petir_bitboard_mask::squareBitInverted(fromSquare));
                            position.rooksAndQueens = (position.rooksAndQueens & petir_bitboard_mask::squareBitInverted(fromSquare));
                            position.bishopsAndQueens = (position.bishopsAndQueens & petir_bitboard_mask::squareBitInverted(fromSquare));
                        } else {
                            if ((piece == petir_header::blackKing)) {
                                position.blackKingBoard = (position.blackKingBoard | petir_bitboard_mask::squareBit(toSquare));
                                position.blackKingBoard = (position.blackKingBoard & petir_bitboard_mask::squareBitInverted(fromSquare));
                                if (((position.castleRights & petir_header::blackCastleRights) != 0)) {
                                    position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
                                    position.castleRights = (position.castleRights & (~petir_header::blackCastleRights));
                                    position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// makemove.pas:817
void checkPositionConsistency(petir_header::tdata position) {
    std::uint64_t remainingPieces{};
    std::int32_t square{};
    std::int32_t pieceSquare{};
    std::int32_t lastSquare{};
    lastSquare = 63;
    for (square = 0; square <= lastSquare; ++square) {
        if (((position.board[square] == petir_header::whitePawn) && ((petir_bitboard_mask::squareBit(square) & position.whitePawns) == 0))) {
            pas::mainForm.caption = pas::String("error 1");
        }
        if (((position.board[square] == petir_header::blackPawn) && ((petir_bitboard_mask::squareBit(square) & position.blackPawns) == 0))) {
            pas::mainForm.caption = pas::String("error 2");
        }
        if (((position.board[square] == petir_header::whiteKnight) && ((petir_bitboard_mask::squareBit(square) & position.whiteKnights) == 0))) {
            pas::mainForm.caption = pas::String("error 30");
        }
        if (((position.board[square] == petir_header::whiteBishop) && ((petir_bitboard_mask::squareBit(square) & position.whiteBishops) == 0))) {
            pas::mainForm.caption = pas::String("error 4");
        }
        if (((position.board[square] == petir_header::whiteRook) && ((petir_bitboard_mask::squareBit(square) & position.whiteRooks) == 0))) {
            pas::mainForm.caption = pas::String("error 5");
        }
        if (((position.board[square] == petir_header::whiteQueen) && ((petir_bitboard_mask::squareBit(square) & position.whiteQueens) == 0))) {
            pas::mainForm.caption = pas::String("error 6");
        }
        if (((position.board[square] == petir_header::whiteKing) && ((petir_bitboard_mask::squareBit(square) & position.whiteKingBoard) == 0))) {
            pas::mainForm.caption = pas::String("error 7");
        }
        if (((position.board[square] == petir_header::blackKing) && ((petir_bitboard_mask::squareBit(square) & position.blackKingBoard) == 0))) {
            pas::mainForm.caption = pas::String("error 8");
        }
        if (((position.board[square] == petir_header::blackRook) && ((petir_bitboard_mask::squareBit(square) & position.blackRooks) == 0))) {
            pas::mainForm.caption = pas::String("error 8");
        }
        if (((position.board[square] == petir_header::blackQueen) && ((petir_bitboard_mask::squareBit(square) & position.blackQueens) == 0))) {
            pas::mainForm.caption = pas::String("error 8");
        }
        if (((position.board[square] == petir_header::blackBishop) && ((petir_bitboard_mask::squareBit(square) & position.blackBishops) == 0))) {
            pas::mainForm.caption = pas::String("error 8");
        }
        if (((position.board[square] == petir_header::blackKnight) && ((petir_bitboard_mask::squareBit(square) & position.blackKnights) == 0))) {
            pas::mainForm.caption = pas::String("error 8");
        }
        if (((position.board[square] != 0) && ((petir_bitboard_mask::rotatedA8H1SquareMask[square] & position.allPiecesA8H1) == 0))) {
            pas::mainForm.caption = pas::String("error 5");
            break;
        }
        if (((position.board[square] != 0) && ((petir_bitboard_mask::rotated90SquareMask[square] & position.allPiecesRotated90) == 0))) {
            pas::mainForm.caption = pas::String("error 5");
            break;
        }
        if (((position.board[square] != 0) && ((petir_bitboard_mask::rotatedH8A1SquareMask[square] & position.allPiecesH8A1) == 0))) {
            pas::mainForm.caption = pas::String("error 5");
            break;
        }
    }
    if (((position.whitePieces | position.blackPieces) != position.allPieces)) {
        pas::mainForm.caption = pas::String("error 1");
    }
    if (((((((position.whitePawns | position.whiteKnights) | position.whiteBishops) | position.whiteRooks) | position.whiteQueens) | position.whiteKingBoard) != position.whitePieces)) {
        pas::mainForm.caption = pas::String("error 2");
    }
    if (((((((position.blackPawns | position.blackKnights) | position.blackBishops) | position.blackRooks) | position.blackQueens) | position.blackKingBoard) != position.blackPieces)) {
        pas::mainForm.caption = pas::String("error 3000");
    }
    remainingPieces = position.allPieces;
    while ((remainingPieces != 0)) {
        pieceSquare = petir_bitboard_mask::lastBit(pas::address(remainingPieces));
        remainingPieces = (remainingPieces & petir_bitboard_mask::squareBitInverted(pieceSquare));
        if ((position.board[pieceSquare] == 0)) {
            pas::mainForm.caption = pas::String("error4");
        }
        if (((position.allPiecesRotated90 & petir_bitboard_mask::rotated90SquareMask[pieceSquare]) == 0)) {
            pas::mainForm.caption = pas::String("error4");
        }
        if (((position.allPiecesA8H1 & petir_bitboard_mask::rotatedA8H1SquareMask[pieceSquare]) == 0)) {
            pas::mainForm.caption = pas::String("error4");
        }
        if (((position.allPiecesH8A1 & petir_bitboard_mask::rotatedH8A1SquareMask[pieceSquare]) == 0)) {
            pas::mainForm.caption = pas::String("error4");
        }
    }
    remainingPieces = position.whitePieces;
    while ((remainingPieces != 0)) {
        pieceSquare = petir_bitboard_mask::lastBit(pas::address(remainingPieces));
        remainingPieces = (remainingPieces & petir_bitboard_mask::squareBitInverted(pieceSquare));
        if ((position.board[pieceSquare] <= 0)) {
            pas::mainForm.caption = pas::String("error4");
        }
        if (((position.allPiecesRotated90 & petir_bitboard_mask::rotated90SquareMask[pieceSquare]) == 0)) {
            pas::mainForm.caption = pas::String("error4");
        }
        if (((position.allPiecesA8H1 & petir_bitboard_mask::rotatedA8H1SquareMask[pieceSquare]) == 0)) {
            pas::mainForm.caption = pas::String("error4");
        }
        if (((position.allPiecesH8A1 & petir_bitboard_mask::rotatedH8A1SquareMask[pieceSquare]) == 0)) {
            pas::mainForm.caption = pas::String("error4");
        }
    }
    remainingPieces = position.blackPieces;
    while ((remainingPieces != 0)) {
        pieceSquare = petir_bitboard_mask::lastBit(pas::address(remainingPieces));
        remainingPieces = (remainingPieces & petir_bitboard_mask::squareBitInverted(pieceSquare));
        if ((position.board[pieceSquare] >= 0)) {
            pas::mainForm.caption = pas::String("error5");
        }
        if (((position.allPiecesRotated90 & petir_bitboard_mask::rotated90SquareMask[pieceSquare]) == 0)) {
            pas::mainForm.caption = pas::String("error4");
        }
        if (((position.allPiecesA8H1 & petir_bitboard_mask::rotatedA8H1SquareMask[pieceSquare]) == 0)) {
            pas::mainForm.caption = pas::String("error4");
        }
        if (((position.allPiecesH8A1 & petir_bitboard_mask::rotatedH8A1SquareMask[pieceSquare]) == 0)) {
            pas::mainForm.caption = pas::String("error4");
        }
    }
    remainingPieces = position.blackPawns;
    while ((remainingPieces != 0)) {
        pieceSquare = petir_bitboard_mask::lastBit(pas::address(remainingPieces));
        remainingPieces = (remainingPieces & petir_bitboard_mask::squareBitInverted(pieceSquare));
        if ((position.board[pieceSquare] != petir_header::blackPawn)) {
            pas::mainForm.caption = pas::String("error6");
        }
    }
    remainingPieces = position.blackKnights;
    while ((remainingPieces != 0)) {
        pieceSquare = petir_bitboard_mask::lastBit(pas::address(remainingPieces));
        remainingPieces = (remainingPieces & petir_bitboard_mask::squareBitInverted(pieceSquare));
        if ((position.board[pieceSquare] != petir_header::blackKnight)) {
            pas::mainForm.caption = pas::String("error7");
        }
    }
    remainingPieces = position.blackBishops;
    while ((remainingPieces != 0)) {
        pieceSquare = petir_bitboard_mask::lastBit(pas::address(remainingPieces));
        remainingPieces = (remainingPieces & petir_bitboard_mask::squareBitInverted(pieceSquare));
        if ((position.board[pieceSquare] != petir_header::blackBishop)) {
            pas::mainForm.caption = pas::String("error8");
        }
    }
    remainingPieces = position.blackRooks;
    while ((remainingPieces != 0)) {
        pieceSquare = petir_bitboard_mask::lastBit(pas::address(remainingPieces));
        remainingPieces = (remainingPieces & petir_bitboard_mask::squareBitInverted(pieceSquare));
        if ((position.board[pieceSquare] != petir_header::blackRook)) {
            pas::mainForm.caption = pas::String("error9");
        }
    }
    remainingPieces = position.blackQueens;
    while ((remainingPieces != 0)) {
        pieceSquare = petir_bitboard_mask::lastBit(pas::address(remainingPieces));
        remainingPieces = (remainingPieces & petir_bitboard_mask::squareBitInverted(pieceSquare));
        if ((position.board[pieceSquare] != petir_header::blackQueen)) {
            pas::mainForm.caption = pas::String("error100");
            return;
        }
    }
    remainingPieces = position.blackKingBoard;
    while ((remainingPieces != 0)) {
        pieceSquare = petir_bitboard_mask::lastBit(pas::address(remainingPieces));
        remainingPieces = (remainingPieces & petir_bitboard_mask::squareBitInverted(pieceSquare));
        if ((position.board[pieceSquare] != petir_header::blackKing)) {
            pas::mainForm.caption = pas::String("error 11");
        }
    }
    remainingPieces = position.whitePawns;
    while ((remainingPieces != 0)) {
        pieceSquare = petir_bitboard_mask::firstBit(pas::address(remainingPieces));
        remainingPieces = (remainingPieces & petir_bitboard_mask::squareBitInverted(pieceSquare));
        if ((position.board[pieceSquare] != petir_header::whitePawn)) {
            pas::mainForm.caption = pas::String("error 12");
        }
    }
    remainingPieces = position.whiteKnights;
    while ((remainingPieces != 0)) {
        pieceSquare = petir_bitboard_mask::firstBit(pas::address(remainingPieces));
        remainingPieces = (remainingPieces & petir_bitboard_mask::squareBitInverted(pieceSquare));
        if ((position.board[pieceSquare] != petir_header::whiteKnight)) {
            pas::mainForm.caption = pas::String("error 13");
        }
    }
    remainingPieces = position.whiteBishops;
    while ((remainingPieces != 0)) {
        pieceSquare = petir_bitboard_mask::firstBit(pas::address(remainingPieces));
        remainingPieces = (remainingPieces & petir_bitboard_mask::squareBitInverted(pieceSquare));
        if ((position.board[pieceSquare] != petir_header::whiteBishop)) {
            pas::mainForm.caption = pas::String("error 14");
        }
    }
    remainingPieces = position.whiteRooks;
    while ((remainingPieces != 0)) {
        pieceSquare = petir_bitboard_mask::firstBit(pas::address(remainingPieces));
        remainingPieces = (remainingPieces & petir_bitboard_mask::squareBitInverted(pieceSquare));
        if ((position.board[pieceSquare] != petir_header::whiteRook)) {
            pas::mainForm.caption = pas::String("error 15");
        }
    }
    remainingPieces = position.whiteQueens;
    while ((remainingPieces != 0)) {
        pieceSquare = petir_bitboard_mask::firstBit(pas::address(remainingPieces));
        remainingPieces = (remainingPieces & petir_bitboard_mask::squareBitInverted(pieceSquare));
        if ((position.board[pieceSquare] != petir_header::whiteQueen)) {
            pas::mainForm.caption = pas::String("error 16");
        }
    }
    remainingPieces = position.whiteKingBoard;
    while ((remainingPieces != 0)) {
        pieceSquare = petir_bitboard_mask::lastBit(pas::address(remainingPieces));
        remainingPieces = (remainingPieces & petir_bitboard_mask::squareBitInverted(pieceSquare));
        if ((position.board[pieceSquare] != petir_header::whiteKing)) {
            pas::mainForm.caption = pas::String("error 17");
        }
    }
}

// makemove.pas:14
void castleWhiteShort(petir_header::tdata& position) {
    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteRook][petir_header::h1]);
    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteKing][petir_header::e1]);
    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteRook][petir_header::f1]);
    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteKing][petir_header::g1]);
    position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
    position.whiteCastleType = petir_header::castledShort;
    position.castleRights = (position.castleRights & (~petir_header::whiteCastleRights));
    position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
    position.board[petir_header::e1] = 0;
    position.board[petir_header::h1] = 0;
    position.board[petir_header::f1] = petir_header::whiteRook;
    position.board[petir_header::g1] = petir_header::whiteKing;
    position.whiteKingBoard = (position.whiteKingBoard & petir_bitboard_mask::squareBitInverted(petir_header::e1));
    position.whiteRooks = (position.whiteRooks & petir_bitboard_mask::squareBitInverted(petir_header::h1));
    position.rooksAndQueens = (position.rooksAndQueens & petir_bitboard_mask::squareBitInverted(petir_header::h1));
    position.whiteRooks = (position.whiteRooks | petir_bitboard_mask::squareBit(petir_header::f1));
    position.rooksAndQueens = (position.rooksAndQueens | petir_bitboard_mask::squareBit(petir_header::f1));
    position.whiteKingBoard = (position.whiteKingBoard | petir_bitboard_mask::squareBit(petir_header::g1));
    position.whitePieces = (position.whitePieces & petir_bitboard_mask::squareBitInverted(petir_header::e1));
    position.whitePieces = (position.whitePieces & petir_bitboard_mask::squareBitInverted(petir_header::h1));
    position.whitePieces = (position.whitePieces | petir_bitboard_mask::squareBit(petir_header::f1));
    position.whitePieces = (position.whitePieces | petir_bitboard_mask::squareBit(petir_header::g1));
    position.allPiecesRotated90 = (position.allPiecesRotated90 & (~petir_bitboard_mask::rotated90SquareMask[petir_header::e1]));
    position.allPiecesRotated90 = (position.allPiecesRotated90 & (~petir_bitboard_mask::rotated90SquareMask[petir_header::h1]));
    position.allPiecesRotated90 = (position.allPiecesRotated90 | petir_bitboard_mask::rotated90SquareMask[petir_header::f1]);
    position.allPiecesRotated90 = (position.allPiecesRotated90 | petir_bitboard_mask::rotated90SquareMask[petir_header::g1]);
    position.allPiecesA8H1 = (position.allPiecesA8H1 & (~petir_bitboard_mask::rotatedA8H1SquareMask[petir_header::e1]));
    position.allPiecesA8H1 = (position.allPiecesA8H1 & (~petir_bitboard_mask::rotatedA8H1SquareMask[petir_header::h1]));
    position.allPiecesA8H1 = (position.allPiecesA8H1 | petir_bitboard_mask::rotatedA8H1SquareMask[petir_header::f1]);
    position.allPiecesA8H1 = (position.allPiecesA8H1 | petir_bitboard_mask::rotatedA8H1SquareMask[petir_header::g1]);
    position.allPiecesH8A1 = (position.allPiecesH8A1 & (~petir_bitboard_mask::rotatedH8A1SquareMask[petir_header::e1]));
    position.allPiecesH8A1 = (position.allPiecesH8A1 & (~petir_bitboard_mask::rotatedH8A1SquareMask[petir_header::h1]));
    position.allPiecesH8A1 = (position.allPiecesH8A1 | petir_bitboard_mask::rotatedH8A1SquareMask[petir_header::f1]);
    position.allPiecesH8A1 = (position.allPiecesH8A1 | petir_bitboard_mask::rotatedH8A1SquareMask[petir_header::g1]);
    position.allPieces = (position.blackPieces | position.whitePieces);
    position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]);
    position.enPassantFile = petir_header::noEnPassant;
    position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[petir_header::noEnPassant]);
}

// makemove.pas:68
void castleWhiteLong(petir_header::tdata& position) {
    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteRook][petir_header::a1]);
    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteKing][petir_header::e1]);
    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteRook][petir_header::d1]);
    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteKing][petir_header::c1]);
    position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
    position.castleRights = (position.castleRights & (~petir_header::whiteCastleRights));
    position.whiteCastleType = petir_header::castledLong;
    position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
    position.board[petir_header::e1] = 0;
    position.board[petir_header::a1] = 0;
    position.board[petir_header::d1] = petir_header::whiteRook;
    position.board[petir_header::c1] = petir_header::whiteKing;
    position.whiteKingBoard = (position.whiteKingBoard & petir_bitboard_mask::squareBitInverted(petir_header::e1));
    position.whiteRooks = (position.whiteRooks & petir_bitboard_mask::squareBitInverted(petir_header::a1));
    position.rooksAndQueens = (position.rooksAndQueens & petir_bitboard_mask::squareBitInverted(petir_header::a1));
    position.whiteRooks = (position.whiteRooks | petir_bitboard_mask::squareBit(petir_header::d1));
    position.rooksAndQueens = (position.rooksAndQueens | petir_bitboard_mask::squareBit(petir_header::d1));
    position.whiteKingBoard = (position.whiteKingBoard | petir_bitboard_mask::squareBit(petir_header::c1));
    position.whitePieces = (position.whitePieces & petir_bitboard_mask::squareBitInverted(petir_header::e1));
    position.whitePieces = (position.whitePieces & petir_bitboard_mask::squareBitInverted(petir_header::a1));
    position.whitePieces = (position.whitePieces | petir_bitboard_mask::squareBit(petir_header::d1));
    position.whitePieces = (position.whitePieces | petir_bitboard_mask::squareBit(petir_header::c1));
    position.allPiecesRotated90 = (position.allPiecesRotated90 & (~petir_bitboard_mask::rotated90SquareMask[petir_header::e1]));
    position.allPiecesRotated90 = (position.allPiecesRotated90 & (~petir_bitboard_mask::rotated90SquareMask[petir_header::a1]));
    position.allPiecesRotated90 = (position.allPiecesRotated90 | petir_bitboard_mask::rotated90SquareMask[petir_header::d1]);
    position.allPiecesRotated90 = (position.allPiecesRotated90 | petir_bitboard_mask::rotated90SquareMask[petir_header::c1]);
    position.allPiecesA8H1 = (position.allPiecesA8H1 & (~petir_bitboard_mask::rotatedA8H1SquareMask[petir_header::e1]));
    position.allPiecesA8H1 = (position.allPiecesA8H1 & (~petir_bitboard_mask::rotatedA8H1SquareMask[petir_header::a1]));
    position.allPiecesA8H1 = (position.allPiecesA8H1 | petir_bitboard_mask::rotatedA8H1SquareMask[petir_header::d1]);
    position.allPiecesA8H1 = (position.allPiecesA8H1 | petir_bitboard_mask::rotatedA8H1SquareMask[petir_header::c1]);
    position.allPiecesH8A1 = (position.allPiecesH8A1 & (~petir_bitboard_mask::rotatedH8A1SquareMask[petir_header::e1]));
    position.allPiecesH8A1 = (position.allPiecesH8A1 & (~petir_bitboard_mask::rotatedH8A1SquareMask[petir_header::a1]));
    position.allPiecesH8A1 = (position.allPiecesH8A1 | petir_bitboard_mask::rotatedH8A1SquareMask[petir_header::d1]);
    position.allPiecesH8A1 = (position.allPiecesH8A1 | petir_bitboard_mask::rotatedH8A1SquareMask[petir_header::c1]);
    position.allPieces = (position.blackPieces | position.whitePieces);
    position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]);
    position.enPassantFile = petir_header::noEnPassant;
    position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]);
}

// makemove.pas:125
void castleBlackShort(petir_header::tdata& position) {
    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackRook][petir_header::h8]);
    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackKing][petir_header::e8]);
    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackRook][petir_header::f8]);
    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackKing][petir_header::g8]);
    position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
    position.castleRights = (position.castleRights & (~petir_header::blackCastleRights));
    position.blackCastleType = petir_header::castledShort;
    position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
    position.board[petir_header::e8] = 0;
    position.board[petir_header::h8] = 0;
    position.board[petir_header::f8] = petir_header::blackRook;
    position.board[petir_header::g8] = petir_header::blackKing;
    position.blackPieces = (position.blackPieces & petir_bitboard_mask::squareBitInverted(petir_header::e8));
    position.blackPieces = (position.blackPieces & petir_bitboard_mask::squareBitInverted(petir_header::h8));
    position.blackPieces = (position.blackPieces | petir_bitboard_mask::squareBit(petir_header::f8));
    position.blackPieces = (position.blackPieces | petir_bitboard_mask::squareBit(petir_header::g8));
    position.blackKingBoard = (position.blackKingBoard & petir_bitboard_mask::squareBitInverted(petir_header::e8));
    position.blackRooks = (position.blackRooks & petir_bitboard_mask::squareBitInverted(petir_header::h8));
    position.rooksAndQueens = (position.rooksAndQueens & petir_bitboard_mask::squareBitInverted(petir_header::h8));
    position.blackRooks = (position.blackRooks | petir_bitboard_mask::squareBit(petir_header::f8));
    position.rooksAndQueens = (position.rooksAndQueens | petir_bitboard_mask::squareBit(petir_header::f8));
    position.blackKingBoard = (position.blackKingBoard | petir_bitboard_mask::squareBit(petir_header::g8));
    position.allPiecesRotated90 = (position.allPiecesRotated90 & (~petir_bitboard_mask::rotated90SquareMask[petir_header::e8]));
    position.allPiecesRotated90 = (position.allPiecesRotated90 & (~petir_bitboard_mask::rotated90SquareMask[petir_header::h8]));
    position.allPiecesRotated90 = (position.allPiecesRotated90 | petir_bitboard_mask::rotated90SquareMask[petir_header::f8]);
    position.allPiecesRotated90 = (position.allPiecesRotated90 | petir_bitboard_mask::rotated90SquareMask[petir_header::g8]);
    position.allPiecesA8H1 = (position.allPiecesA8H1 & (~petir_bitboard_mask::rotatedA8H1SquareMask[petir_header::e8]));
    position.allPiecesA8H1 = (position.allPiecesA8H1 & (~petir_bitboard_mask::rotatedA8H1SquareMask[petir_header::h8]));
    position.allPiecesA8H1 = (position.allPiecesA8H1 | petir_bitboard_mask::rotatedA8H1SquareMask[petir_header::f8]);
    position.allPiecesA8H1 = (position.allPiecesA8H1 | petir_bitboard_mask::rotatedA8H1SquareMask[petir_header::g8]);
    position.allPiecesH8A1 = (position.allPiecesH8A1 & (~petir_bitboard_mask::rotatedH8A1SquareMask[petir_header::e8]));
    position.allPiecesH8A1 = (position.allPiecesH8A1 & (~petir_bitboard_mask::rotatedH8A1SquareMask[petir_header::h8]));
    position.allPiecesH8A1 = (position.allPiecesH8A1 | petir_bitboard_mask::rotatedH8A1SquareMask[petir_header::f8]);
    position.allPiecesH8A1 = (position.allPiecesH8A1 | petir_bitboard_mask::rotatedH8A1SquareMask[petir_header::g8]);
    position.allPieces = (position.blackPieces | position.whitePieces);
    position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]);
    position.enPassantFile = petir_header::noEnPassant;
    position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]);
}

// makemove.pas:177
void castleBlackLong(petir_header::tdata& position) {
    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackRook][petir_header::a8]);
    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackKing][petir_header::e8]);
    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackRook][petir_header::d8]);
    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackKing][petir_header::c8]);
    position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
    position.castleRights = (position.castleRights & (~petir_header::blackCastleRights));
    position.blackCastleType = petir_header::castledLong;
    position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
    position.board[petir_header::e8] = 0;
    position.board[petir_header::a8] = 0;
    position.board[petir_header::d8] = petir_header::blackRook;
    position.board[petir_header::c8] = petir_header::blackKing;
    position.blackPieces = (position.blackPieces & petir_bitboard_mask::squareBitInverted(petir_header::e8));
    position.blackPieces = (position.blackPieces & petir_bitboard_mask::squareBitInverted(petir_header::a8));
    position.blackPieces = (position.blackPieces | petir_bitboard_mask::squareBit(petir_header::d8));
    position.blackPieces = (position.blackPieces | petir_bitboard_mask::squareBit(petir_header::c8));
    position.blackKingBoard = (position.blackKingBoard & petir_bitboard_mask::squareBitInverted(petir_header::e8));
    position.blackRooks = (position.blackRooks & petir_bitboard_mask::squareBitInverted(petir_header::a8));
    position.rooksAndQueens = (position.rooksAndQueens & petir_bitboard_mask::squareBitInverted(petir_header::a8));
    position.blackRooks = (position.blackRooks | petir_bitboard_mask::squareBit(petir_header::d8));
    position.rooksAndQueens = (position.rooksAndQueens | petir_bitboard_mask::squareBit(petir_header::d8));
    position.blackKingBoard = (position.blackKingBoard | petir_bitboard_mask::squareBit(petir_header::c8));
    position.allPiecesRotated90 = (position.allPiecesRotated90 & (~petir_bitboard_mask::rotated90SquareMask[petir_header::e8]));
    position.allPiecesRotated90 = (position.allPiecesRotated90 & (~petir_bitboard_mask::rotated90SquareMask[petir_header::a8]));
    position.allPiecesRotated90 = (position.allPiecesRotated90 | petir_bitboard_mask::rotated90SquareMask[petir_header::d8]);
    position.allPiecesRotated90 = (position.allPiecesRotated90 | petir_bitboard_mask::rotated90SquareMask[petir_header::c8]);
    position.allPiecesA8H1 = (position.allPiecesA8H1 & (~petir_bitboard_mask::rotatedA8H1SquareMask[petir_header::e8]));
    position.allPiecesA8H1 = (position.allPiecesA8H1 & (~petir_bitboard_mask::rotatedA8H1SquareMask[petir_header::a8]));
    position.allPiecesA8H1 = (position.allPiecesA8H1 | petir_bitboard_mask::rotatedA8H1SquareMask[petir_header::d8]);
    position.allPiecesA8H1 = (position.allPiecesA8H1 | petir_bitboard_mask::rotatedA8H1SquareMask[petir_header::c8]);
    position.allPiecesH8A1 = (position.allPiecesH8A1 & (~petir_bitboard_mask::rotatedH8A1SquareMask[petir_header::e8]));
    position.allPiecesH8A1 = (position.allPiecesH8A1 & (~petir_bitboard_mask::rotatedH8A1SquareMask[petir_header::a8]));
    position.allPiecesH8A1 = (position.allPiecesH8A1 | petir_bitboard_mask::rotatedH8A1SquareMask[petir_header::d8]);
    position.allPiecesH8A1 = (position.allPiecesH8A1 | petir_bitboard_mask::rotatedH8A1SquareMask[petir_header::c8]);
    position.allPieces = (position.blackPieces | position.whitePieces);
    position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]);
    position.enPassantFile = petir_header::noEnPassant;
    position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]);
}

// makemove.pas:230
void makeBlackEnPassant(petir_header::tdata& position, std::int32_t fromSquare, std::int32_t toSquare) {
    position.fiftyMoveCounter = 0;
    (position.materialScore = pas::decrement(position.materialScore, petir_header::pawnValue));
    if ((position.enPassantFile > (fromSquare & 7))) {
        toSquare = pas::sub(fromSquare, 7);
    } else {
        toSquare = pas::sub(fromSquare, 9);
    }
    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackPawn][fromSquare]);
    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackPawn][toSquare]);
    position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]);
    position.enPassantFile = petir_header::noEnPassant;
    position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[petir_header::noEnPassant]);
    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whitePawn][pas::add(toSquare, 8)]);
    position.board[toSquare] = petir_header::blackPawn;
    position.board[fromSquare] = 0;
    position.board[pas::add(toSquare, 8)] = 0;
    position.whitePieces = (position.whitePieces & petir_bitboard_mask::squareBitInverted(pas::add(toSquare, 8)));
    position.blackPieces = (position.blackPieces & petir_bitboard_mask::squareBitInverted(fromSquare));
    position.blackPieces = (position.blackPieces | petir_bitboard_mask::squareBit(toSquare));
    position.allPieces = (position.allPieces & petir_bitboard_mask::squareBitInverted(pas::add(toSquare, 8)));
    position.allPieces = (position.allPieces & petir_bitboard_mask::squareBitInverted(fromSquare));
    position.allPieces = (position.allPieces | petir_bitboard_mask::squareBit(toSquare));
    position.allPiecesRotated90 = (position.allPiecesRotated90 & (~petir_bitboard_mask::rotated90SquareMask[pas::add(toSquare, 8)]));
    position.allPiecesRotated90 = (position.allPiecesRotated90 & (~petir_bitboard_mask::rotated90SquareMask[fromSquare]));
    position.allPiecesRotated90 = (position.allPiecesRotated90 | petir_bitboard_mask::rotated90SquareMask[toSquare]);
    position.allPiecesA8H1 = (position.allPiecesA8H1 & (~petir_bitboard_mask::rotatedA8H1SquareMask[pas::add(toSquare, 8)]));
    position.allPiecesA8H1 = (position.allPiecesA8H1 & (~petir_bitboard_mask::rotatedA8H1SquareMask[fromSquare]));
    position.allPiecesA8H1 = (position.allPiecesA8H1 | petir_bitboard_mask::rotatedA8H1SquareMask[toSquare]);
    position.allPiecesH8A1 = (position.allPiecesH8A1 & (~petir_bitboard_mask::rotatedH8A1SquareMask[pas::add(toSquare, 8)]));
    position.allPiecesH8A1 = (position.allPiecesH8A1 & (~petir_bitboard_mask::rotatedH8A1SquareMask[fromSquare]));
    position.allPiecesH8A1 = (position.allPiecesH8A1 | petir_bitboard_mask::rotatedH8A1SquareMask[toSquare]);
    position.blackPawns = (position.blackPawns & petir_bitboard_mask::squareBitInverted(fromSquare));
    position.whitePawns = (position.whitePawns & petir_bitboard_mask::squareBitInverted(pas::add(toSquare, 8)));
    position.blackPawns = (position.blackPawns | petir_bitboard_mask::squareBit(toSquare));
}

// makemove.pas:279
void makeWhiteEnPassant(petir_header::tdata& position, std::int32_t fromSquare, std::int32_t toSquare) {
    position.fiftyMoveCounter = 0;
    (position.materialScore = pas::increment(position.materialScore, petir_header::pawnValue));
    if ((position.enPassantFile > (fromSquare & 7))) {
        toSquare = pas::add(fromSquare, 9);
    } else {
        toSquare = pas::add(fromSquare, 7);
    }
    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whitePawn][fromSquare]);
    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whitePawn][toSquare]);
    position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]);
    position.enPassantFile = petir_header::noEnPassant;
    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackPawn][pas::sub(toSquare, 8)]);
    position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[petir_header::noEnPassant]);
    position.board[toSquare] = petir_header::whitePawn;
    position.board[fromSquare] = 0;
    position.board[pas::sub(toSquare, 8)] = 0;
    position.blackPieces = (position.blackPieces & petir_bitboard_mask::squareBitInverted(pas::sub(toSquare, 8)));
    position.whitePieces = (position.whitePieces & petir_bitboard_mask::squareBitInverted(fromSquare));
    position.whitePieces = (position.whitePieces | petir_bitboard_mask::squareBit(toSquare));
    position.allPieces = (position.allPieces & petir_bitboard_mask::squareBitInverted(pas::sub(toSquare, 8)));
    position.allPieces = (position.allPieces & petir_bitboard_mask::squareBitInverted(fromSquare));
    position.allPieces = (position.allPieces | petir_bitboard_mask::squareBit(toSquare));
    position.allPiecesRotated90 = (position.allPiecesRotated90 & (~petir_bitboard_mask::rotated90SquareMask[pas::sub(toSquare, 8)]));
    position.allPiecesRotated90 = (position.allPiecesRotated90 & (~petir_bitboard_mask::rotated90SquareMask[fromSquare]));
    position.allPiecesRotated90 = (position.allPiecesRotated90 | petir_bitboard_mask::rotated90SquareMask[toSquare]);
    position.allPiecesA8H1 = (position.allPiecesA8H1 & (~petir_bitboard_mask::rotatedA8H1SquareMask[pas::sub(toSquare, 8)]));
    position.allPiecesA8H1 = (position.allPiecesA8H1 & (~petir_bitboard_mask::rotatedA8H1SquareMask[fromSquare]));
    position.allPiecesA8H1 = (position.allPiecesA8H1 | petir_bitboard_mask::rotatedA8H1SquareMask[toSquare]);
    position.allPiecesH8A1 = (position.allPiecesH8A1 & (~petir_bitboard_mask::rotatedH8A1SquareMask[pas::sub(toSquare, 8)]));
    position.allPiecesH8A1 = (position.allPiecesH8A1 & (~petir_bitboard_mask::rotatedH8A1SquareMask[fromSquare]));
    position.allPiecesH8A1 = (position.allPiecesH8A1 | petir_bitboard_mask::rotatedH8A1SquareMask[toSquare]);
    position.whitePawns = (position.whitePawns & petir_bitboard_mask::squareBitInverted(fromSquare));
    position.blackPawns = (position.blackPawns & petir_bitboard_mask::squareBitInverted(pas::sub(toSquare, 8)));
    position.whitePawns = (position.whitePawns | petir_bitboard_mask::squareBit(toSquare));
}

// makemove.pas:326
void promoteWhitePawn(petir_header::tdata& position, std::int32_t square, std::int32_t promotion) {
    position.whitePawns = (position.whitePawns & (~petir_bitboard_mask::squareBit(square)));
    (position.materialScore = pas::decrement(position.materialScore, petir_header::pawnValue));
    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whitePawn][square]);
    if ((promotion == petir_header::queenPromotion)) {
        position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteQueen][square]);
        position.board[square] = petir_header::whiteQueen;
        position.whiteQueens = (position.whiteQueens | petir_bitboard_mask::squareBit(square));
        position.bishopsAndQueens = (position.bishopsAndQueens | petir_bitboard_mask::squareBit(square));
        position.rooksAndQueens = (position.rooksAndQueens | petir_bitboard_mask::squareBit(square));
        (position.materialScore = pas::increment(position.materialScore, petir_header::queenValue));
        (position.whiteNonPawnMaterial = pas::increment(position.whiteNonPawnMaterial, 9));
    } else {
        if ((promotion == petir_header::knightPromotion)) {
            position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteKnight][square]);
            position.board[square] = petir_header::whiteKnight;
            position.whiteKnights = (position.whiteKnights | petir_bitboard_mask::squareBit(square));
            (position.materialScore = pas::increment(position.materialScore, petir_header::knightValue));
            (position.whiteNonPawnMaterial = pas::increment(position.whiteNonPawnMaterial, 3));
        } else {
            if ((promotion == petir_header::rookPromotion)) {
                position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteRook][square]);
                position.board[square] = petir_header::whiteRook;
                position.whiteRooks = (position.whiteRooks | petir_bitboard_mask::squareBit(square));
                position.rooksAndQueens = (position.rooksAndQueens | petir_bitboard_mask::squareBit(square));
                (position.materialScore = pas::increment(position.materialScore, petir_header::rookValue));
                (position.whiteNonPawnMaterial = pas::increment(position.whiteNonPawnMaterial, 5));
            } else {
                if ((promotion == petir_header::bishopPromotion)) {
                    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteBishop][square]);
                    position.board[square] = petir_header::whiteBishop;
                    position.whiteBishops = (position.whiteBishops | petir_bitboard_mask::squareBit(square));
                    position.bishopsAndQueens = (position.bishopsAndQueens | petir_bitboard_mask::squareBit(square));
                    (position.whiteBishopCount = pas::increment(position.whiteBishopCount, 1));
                    (position.materialScore = pas::increment(position.materialScore, petir_header::bishopValue));
                    (position.whiteNonPawnMaterial = pas::increment(position.whiteNonPawnMaterial, 3));
                }
            }
        }
    }
}

// makemove.pas:369
void promoteBlackPawn(petir_header::tdata& position, std::int32_t square, std::int32_t promotion) {
    position.blackPawns = (position.blackPawns & (~petir_bitboard_mask::squareBit(square)));
    (position.materialScore = pas::increment(position.materialScore, petir_header::pawnValue));
    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackPawn][square]);
    if ((promotion == petir_header::queenPromotion)) {
        position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackQueen][square]);
        position.board[square] = petir_header::blackQueen;
        position.blackQueens = (position.blackQueens | petir_bitboard_mask::squareBit(square));
        position.rooksAndQueens = (position.rooksAndQueens | petir_bitboard_mask::squareBit(square));
        position.bishopsAndQueens = (position.bishopsAndQueens | petir_bitboard_mask::squareBit(square));
        (position.materialScore = pas::decrement(position.materialScore, petir_header::queenValue));
        (position.blackNonPawnMaterial = pas::increment(position.blackNonPawnMaterial, 9));
    } else {
        if ((promotion == petir_header::knightPromotion)) {
            position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackKnight][square]);
            position.board[square] = petir_header::blackKnight;
            position.blackKnights = (position.blackKnights | petir_bitboard_mask::squareBit(square));
            (position.materialScore = pas::decrement(position.materialScore, petir_header::knightValue));
            (position.blackNonPawnMaterial = pas::increment(position.blackNonPawnMaterial, 3));
        } else {
            if ((promotion == petir_header::rookPromotion)) {
                position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackRook][square]);
                position.board[square] = petir_header::blackRook;
                position.blackRooks = (position.blackRooks | petir_bitboard_mask::squareBit(square));
                position.rooksAndQueens = (position.rooksAndQueens | petir_bitboard_mask::squareBit(square));
                (position.materialScore = pas::decrement(position.materialScore, petir_header::rookValue));
                (position.blackNonPawnMaterial = pas::increment(position.blackNonPawnMaterial, 5));
            } else {
                if ((promotion == petir_header::bishopPromotion)) {
                    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackBishop][square]);
                    position.board[square] = petir_header::blackBishop;
                    position.blackBishops = (position.blackBishops | petir_bitboard_mask::squareBit(square));
                    position.bishopsAndQueens = (position.bishopsAndQueens | petir_bitboard_mask::squareBit(square));
                    (position.blackBishopCount = pas::increment(position.blackBishopCount, 1));
                    (position.materialScore = pas::decrement(position.materialScore, petir_header::bishopValue));
                    (position.blackNonPawnMaterial = pas::increment(position.blackNonPawnMaterial, 3));
                }
            }
        }
    }
}

}
