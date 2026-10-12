// Faithful translation of etc.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_etc {
// etc.pas:130
void updateBlackHashKey(std::int32_t move, petir_header::tdata& position) {
    std::int32_t capturedPiece{};
    std::int32_t piece{};
    std::int32_t fromSquare{};
    std::int32_t toSquare{};
    std::int32_t promotion{};
    std::int32_t newCastleRights{};
    fromSquare = (move & 127);
    toSquare = (pas::shiftRight(move, 7) & 127);
    promotion = (pas::shiftRight(move, 14) & 7);
    if ((toSquare == petir_header::shortCastleTarget)) {
        position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackRook][petir_header::h8]);
        position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackKing][petir_header::e8]);
        position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackRook][petir_header::f8]);
        position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackKing][petir_header::g8]);
        position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
        position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[(position.castleRights & (~12))]);
        position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]);
        position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[petir_header::noEnPassant]);
    } else {
        if ((toSquare == petir_header::longCastleTarget)) {
            position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackRook][petir_header::a8]);
            position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackKing][petir_header::e8]);
            position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackRook][petir_header::d8]);
            position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackKing][petir_header::c8]);
            position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
            position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[(position.castleRights & (~12))]);
            position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]);
            position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[petir_header::noEnPassant]);
        } else {
            if ((toSquare == petir_header::enPassantTarget)) {
                position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackPawn][fromSquare]);
                position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackPawn][toSquare]);
                position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]);
                position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[petir_header::noEnPassant]);
                position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whitePawn][pas::add(toSquare, 8)]);
            } else {
                piece = position.board[fromSquare];
                position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]);
                position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[petir_header::noEnPassant]);
                capturedPiece = position.board[toSquare];
                position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[piece][fromSquare]);
                position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[piece][toSquare]);
                if ((capturedPiece != 0)) {
                    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[capturedPiece][toSquare]);
                } else {
                    if ((((position.board[fromSquare] == petir_header::blackPawn) && (pas::sub(fromSquare, toSquare) == 16)) && (((position.board[pas::sub(toSquare, 1)] == petir_header::whitePawn) && ((toSquare % 8) >= 1)) || ((position.board[pas::add(toSquare, 1)] == petir_header::whitePawn) && ((toSquare % 8) <= 6))))) {
                        position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]);
                        position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[(fromSquare % 8)]);
                    }
                }
                // Collect every right the move clears, then rehash once, so a
                // move that clears both sides' rights hashes correctly.
                newCastleRights = position.castleRights;
                if ((capturedPiece == petir_header::whiteRook)) {
                    if ((toSquare == petir_header::h1)) {
                        newCastleRights = (newCastleRights & (~petir_header::whiteShortCastleRight));
                    } else {
                        if ((toSquare == petir_header::a1)) {
                            newCastleRights = (newCastleRights & (~petir_header::whiteLongCastleRight));
                        }
                    }
                }
                if ((position.board[fromSquare] == petir_header::blackRook)) {
                    if ((fromSquare == petir_header::h8)) {
                        newCastleRights = (newCastleRights & (~petir_header::blackShortCastleRight));
                    } else {
                        if ((fromSquare == petir_header::a8)) {
                            newCastleRights = (newCastleRights & (~petir_header::blackLongCastleRight));
                        }
                    }
                } else {
                    if ((position.board[fromSquare] == petir_header::blackKing)) {
                        newCastleRights = (newCastleRights & (~12));
                    }
                }
                position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
                position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[newCastleRights]);
                if ((promotion != 0)) {
                    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackPawn][toSquare]);
                    if ((promotion == petir_header::queenPromotion)) {
                        position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackQueen][toSquare]);
                    } else {
                        if ((promotion == petir_header::knightPromotion)) {
                            position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackKnight][toSquare]);
                        } else {
                            if ((promotion == petir_header::rookPromotion)) {
                                position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackRook][toSquare]);
                            } else {
                                if ((promotion == petir_header::bishopPromotion)) {
                                    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackBishop][toSquare]);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// etc.pas:21
void updateWhiteHashKey(std::int32_t move, petir_header::tdata& position) {
    std::int32_t capturedPiece{};
    std::int32_t piece{};
    std::int32_t fromSquare{};
    std::int32_t toSquare{};
    std::int32_t promotion{};
    std::int32_t newCastleRights{};
    fromSquare = (move & 127);
    toSquare = (pas::shiftRight(move, 7) & 127);
    promotion = (pas::shiftRight(move, 14) & 7);
    if ((toSquare == petir_header::shortCastleTarget)) {
        position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteRook][petir_header::h1]);
        position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteKing][petir_header::e1]);
        position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteRook][petir_header::f1]);
        position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteKing][petir_header::g1]);
        position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
        position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[(position.castleRights & (~3))]);
        position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]);
        position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[petir_header::noEnPassant]);
    } else {
        if ((toSquare == petir_header::longCastleTarget)) {
            position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteRook][petir_header::a1]);
            position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteKing][petir_header::e1]);
            position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteRook][petir_header::d1]);
            position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteKing][petir_header::c1]);
            position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
            position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[(position.castleRights & (~3))]);
            position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]);
            position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[petir_header::noEnPassant]);
        } else {
            if ((toSquare == petir_header::enPassantTarget)) {
                position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whitePawn][fromSquare]);
                position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whitePawn][toSquare]);
                position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]);
                position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::blackPawn][pas::sub(toSquare, 8)]);
                position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[petir_header::noEnPassant]);
            } else {
                position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]);
                position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[petir_header::noEnPassant]);
                piece = position.board[fromSquare];
                capturedPiece = position.board[toSquare];
                position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[piece][fromSquare]);
                position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[piece][toSquare]);
                if ((capturedPiece != 0)) {
                    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[capturedPiece][toSquare]);
                } else {
                    if ((((position.board[fromSquare] == petir_header::whitePawn) && (pas::sub(toSquare, fromSquare) == 16)) && (((position.board[pas::sub(toSquare, 1)] == petir_header::blackPawn) && ((toSquare % 8) >= 1)) || ((position.board[pas::add(toSquare, 1)] == petir_header::blackPawn) && ((toSquare % 8) <= 6))))) {
                        position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[position.enPassantFile]);
                        position.hashKey = (position.hashKey ^ petir_header::zobristEnPassantKeys[(fromSquare % 8)]);
                    }
                }
                // Collect every right the move clears, then rehash once, so a
                // move that clears both sides' rights hashes correctly.
                newCastleRights = position.castleRights;
                if ((capturedPiece == petir_header::blackRook)) {
                    if ((toSquare == petir_header::h8)) {
                        newCastleRights = (newCastleRights & (~petir_header::blackShortCastleRight));
                    } else {
                        if ((toSquare == petir_header::a8)) {
                            newCastleRights = (newCastleRights & (~petir_header::blackLongCastleRight));
                        }
                    }
                }
                if ((position.board[fromSquare] == petir_header::whiteRook)) {
                    if ((fromSquare == petir_header::h1)) {
                        newCastleRights = (newCastleRights & (~petir_header::whiteShortCastleRight));
                    } else {
                        if ((fromSquare == petir_header::a1)) {
                            newCastleRights = (newCastleRights & (~petir_header::whiteLongCastleRight));
                        }
                    }
                } else {
                    if ((position.board[fromSquare] == petir_header::whiteKing)) {
                        newCastleRights = (newCastleRights & (~3));
                    }
                }
                position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[position.castleRights]);
                position.hashKey = (position.hashKey ^ petir_header::zobristCastleKeys[newCastleRights]);
                if ((promotion != 0)) {
                    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whitePawn][toSquare]);
                    if ((promotion == petir_header::queenPromotion)) {
                        position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteQueen][toSquare]);
                    } else {
                        if ((promotion == petir_header::knightPromotion)) {
                            position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteKnight][toSquare]);
                        } else {
                            if ((promotion == petir_header::rookPromotion)) {
                                position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteRook][toSquare]);
                            } else {
                                if ((promotion == petir_header::bishopPromotion)) {
                                    position.hashKey = (position.hashKey ^ petir_header::zobristPieceKeys[petir_header::whiteBishop][toSquare]);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// etc.pas:13
void updateHashKey(petir_header::tdata& position, std::int32_t move, std::int32_t sideToMove) {
    if ((sideToMove == petir_header::whiteSide)) {
        petir_etc::updateWhiteHashKey(move, position);
    } else {
        petir_etc::updateBlackHashKey(move, position);
    }
}

}
