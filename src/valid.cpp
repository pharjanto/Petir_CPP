// Faithful translation of valid.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_valid {
// Not in valid.pas: a pawn move to the last rank must name a promotion the
// generators make (queen or knight), and any other pawn move must name none.
// Otherwise a killer such as a rook's c7c8 passes for a pawn on c7; makemove
// promotes it to a queen, but the bare move is not a legal move, so it went
// out as an illegal UCI PV and the engine dropped its search result.
static bool pawnPromotionMatches(std::int32_t toSquare, std::int32_t move) {
    const std::int32_t promotion = (pas::shiftRight(move, 14) & 7);
    if (((pas::shiftRight(toSquare, 3) == 7) || (pas::shiftRight(toSquare, 3) == 0))) {
        return ((promotion == petir_header::queenPromotion) || (promotion == petir_header::knightPromotion));
    }
    return (promotion == 0);
}
// valid.pas:10
bool isValidMove(petir_header::tdata& position, std::int32_t move, std::int32_t sideToMove) {
    bool result{};
    std::int32_t fromSquare{};
    std::int32_t toSquare{};
    std::int32_t piece{};
    std::int32_t pieceType{};
    std::int32_t squareDelta{};
    std::uint64_t toBit{};
    std::uint64_t attacks{};
    fromSquare = (move & 127);
    toSquare = (pas::shiftRight(move, 7) & 127);
    result = false;
    piece = position.board[fromSquare];
    if ((toSquare <= 63)) {
        if (((piece > 0) && (position.board[toSquare] > 0))) {
            return result;
        } else {
            if (((piece < 0) && (position.board[toSquare] < 0))) {
                return result;
            }
        }
    } else {
        if (((toSquare == 65) || (toSquare == 66))) {
            if (((sideToMove == petir_header::whiteSide) && (position.board[petir_header::e1] != petir_header::whiteKing))) {
                return result;
            }
            if (((sideToMove == petir_header::blackSide) && (position.board[petir_header::e8] != petir_header::blackKing))) {
                return result;
            }
        }
    }
    if (((sideToMove == petir_header::whiteSide) && (position.board[fromSquare] <= 0))) {
        return result;
    }
    if (((sideToMove == petir_header::blackSide) && (position.board[fromSquare] >= 0))) {
        return result;
    }
    pieceType = pas::abs(piece);
    // Not in valid.pas: only a pawn move may name a promotion (the slider
    // checks below already demanded this; the knight and king ones did not).
    if (((pieceType != petir_header::whitePawn) && ((pas::shiftRight(move, 14) & 7) != 0))) {
        return result;
    }
    if ((toSquare <= 63)) {
        toBit = petir_bitboard_mask::squareBit(toSquare);
    }
    if ((piece == petir_header::whitePawn)) {
        squareDelta = pas::sub(toSquare, fromSquare);
        if ((toSquare == petir_header::enPassantTarget)) {
            if (((pas::abs(pas::sub((fromSquare & 7), position.enPassantFile)) == 1) && (pas::shiftRight(fromSquare, 3) == 4))) {
                result = true;
                return result;
            }
        }
        if (((toSquare <= 63) && !pawnPromotionMatches(toSquare, move))) {
            return result;
        }
        // Not in valid.pas: a capture must change file by one, or an edge pawn
        // would "capture" around the board (a3 to h3 is squareDelta == 7).
        if ((((squareDelta == 7) || (squareDelta == 9)) && (pas::abs(pas::sub((toSquare & 7), (fromSquare & 7))) == 1) && (position.board[toSquare] < 0))) {
            result = true;
            return result;
        }
        if (((squareDelta == 8) && (position.board[toSquare] == 0))) {
            result = true;
            return result;
        }
        if (((((squareDelta == 16) && (position.board[toSquare] == 0)) && (position.board[pas::add(fromSquare, 8)] == 0)) && (fromSquare <= petir_header::h2))) {
            result = true;
            return result;
        }
    }
    if ((piece == petir_header::blackPawn)) {
        squareDelta = pas::sub(fromSquare, toSquare);
        if ((toSquare == petir_header::enPassantTarget)) {
            if (((pas::abs(pas::sub((fromSquare & 7), position.enPassantFile)) == 1) && (pas::shiftRight(fromSquare, 3) == 3))) {
                result = true;
                return result;
            }
        }
        if (((toSquare <= 63) && !pawnPromotionMatches(toSquare, move))) {
            return result;
        }
        if ((((squareDelta == 7) || (squareDelta == 9)) && (pas::abs(pas::sub((toSquare & 7), (fromSquare & 7))) == 1) && (position.board[toSquare] > 0))) {
            result = true;
            return result;
        }
        if (((squareDelta == 8) && (position.board[toSquare] == 0))) {
            result = true;
            return result;
        }
        if (((((squareDelta == 16) && (position.board[toSquare] == 0)) && (position.board[pas::sub(fromSquare, 8)] == 0)) && (fromSquare >= petir_header::a7))) {
            result = true;
            return result;
        }
    }
    if ((pieceType == petir_header::whiteKnight)) {
        if (((petir_bitboard_mask::knightAttacks[fromSquare] & toBit) != 0)) {
            result = true;
            return result;
        }
    }
    if (((pieceType == petir_header::whiteRook) || (pieceType == petir_header::whiteQueen))) {
        attacks = petir_bitboard_mask::horizontalAttacks[fromSquare][(pas::shiftRight(position.allPieces, pas::add((fromSquare & 56), 1)) & 63)];
        attacks = (attacks | petir_bitboard_mask::verticalAttacks[fromSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[fromSquare]) & 63)]);
        if ((((attacks & toBit) != 0) && ((pas::shiftRight(move, 14) & 7) == 0))) {
            result = true;
            return result;
        }
    }
    if (((pieceType == petir_header::whiteBishop) || (pieceType == petir_header::whiteQueen))) {
        attacks = petir_bitboard_mask::diagonalA8H1Attacks[fromSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[fromSquare]) & 255)];
        attacks = (attacks | petir_bitboard_mask::diagonalH8A1Attacks[fromSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[fromSquare]) & 255)]);
        if ((((attacks & toBit) != 0) && ((pas::shiftRight(move, 14) & 7) == 0))) {
            result = true;
            return result;
        }
    }
    if ((piece == petir_header::whiteKing)) {
        if ((toSquare == petir_header::shortCastleTarget)) {
            if ((((((((((position.castleRights & petir_header::whiteShortCastleRight) != 0) && (position.board[petir_header::e1] == petir_header::whiteKing)) && (position.board[petir_header::f1] == 0)) && (position.board[petir_header::g1] == 0)) && (!petir_tools::attackedByBlack(position, petir_header::e1))) && (!petir_tools::attackedByBlack(position, petir_header::f1))) && (!petir_tools::attackedByBlack(position, petir_header::g1))) && (position.board[petir_header::h1] == petir_header::whiteRook))) {
                result = true;
                return result;
            }
        } else {
            if ((toSquare == petir_header::longCastleTarget)) {
                if (((((((((((position.castleRights & petir_header::whiteLongCastleRight) != 0) && (position.board[petir_header::e1] == petir_header::whiteKing)) && (position.board[petir_header::d1] == 0)) && (position.board[petir_header::c1] == 0)) && (position.board[petir_header::b1] == 0)) && (!petir_tools::attackedByBlack(position, petir_header::e1))) && (!petir_tools::attackedByBlack(position, petir_header::d1))) && (!petir_tools::attackedByBlack(position, petir_header::c1))) && (position.board[petir_header::a1] == petir_header::whiteRook))) {
                    result = true;
                    return result;
                }
            } else {
                if (((petir_bitboard_mask::kingAttacks[fromSquare] & toBit) != 0)) {
                    result = true;
                    return result;
                }
            }
        }
    }
    if ((piece == petir_header::blackKing)) {
        if ((toSquare == petir_header::shortCastleTarget)) {
            if ((((((((((position.castleRights & petir_header::blackShortCastleRight) != 0) && (!petir_tools::attackedByWhite(position, petir_header::e8))) && (position.board[petir_header::e8] == petir_header::blackKing)) && (position.board[petir_header::f8] == 0)) && (position.board[petir_header::g8] == 0)) && (!petir_tools::attackedByWhite(position, petir_header::f8))) && (!petir_tools::attackedByWhite(position, petir_header::g8))) && (position.board[petir_header::h8] == petir_header::blackRook))) {
                result = true;
                return result;
            }
        } else {
            if ((toSquare == petir_header::longCastleTarget)) {
                if (((((((((((position.castleRights & petir_header::blackLongCastleRight) != 0) && (position.board[petir_header::e8] == petir_header::blackKing)) && (!petir_tools::attackedByWhite(position, petir_header::e8))) && (position.board[petir_header::d8] == 0)) && (position.board[petir_header::c8] == 0)) && (position.board[petir_header::b8] == 0)) && (!petir_tools::attackedByWhite(position, petir_header::d8))) && (!petir_tools::attackedByWhite(position, petir_header::c8))) && (position.board[petir_header::a8] == petir_header::blackRook))) {
                    result = true;
                    return result;
                }
            } else {
                if (((petir_bitboard_mask::kingAttacks[fromSquare] & toBit) != 0)) {
                    result = true;
                    return result;
                }
            }
        }
    }
    return result;
}

}
