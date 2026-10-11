// Faithful translation of movgen.pas; Delphi source is in original/.
#include "petir.hpp"
#if defined(_MSC_VER) && defined(_M_X64)
#include <intrin.h>
#endif
namespace petir_movgen {
namespace {
// Iteration order affects move ordering. Legacy loops visit the highest
// occupied 16-bit word first, least bit first within each word.
template<bool Legacy>
class move_bits {
    std::uint64_t remainingBits;
public:
    explicit move_bits(std::uint64_t& sourceBits):remainingBits(sourceBits) {
        // Each loop consumes its source mask; its body uses returned squares.
        sourceBits=0;
        if constexpr(Legacy) {
            remainingBits=std::rotl(remainingBits,32);
            remainingBits=((remainingBits & UINT64_C(0x0000ffff0000ffff))<<16)
                 |((remainingBits>>16) & UINT64_C(0x0000ffff0000ffff));
        }
    }
    explicit operator bool() const {return remainingBits!=0;}
    int pop() {
        // The surrounding loop guarantees a nonempty bitboard.
#if defined(_MSC_VER) && defined(_M_X64)
        unsigned long square;
        _BitScanForward64(&square,remainingBits);
#else
        const int square=std::countr_zero(remainingBits);
#endif
        remainingBits &= remainingBits-1;
        return static_cast<int>(square) ^ (Legacy?48:0);
    }
};
void appendLegalEnPassantEvasions(petir_header::tdata& position, petir_header::tmovelist& moveList, std::int32_t& moveCount, int side) {
    if(position.enPassantFile>7)return;
    const bool isWhite=side==petir_header::whiteSide;
    const int capturedSquare=(isWhite?32:24)+position.enPassantFile;
    const int targetSquare=capturedSquare+(isWhite?8:-8);
    if(position.board[capturedSquare]!=(isWhite?-1:1) || position.board[targetSquare]!=0)return;
    for(int fileOffset:{-1,1}) {
        const int captureFile=position.enPassantFile+fileOffset;
        if(captureFile<0 || captureFile>7 || position.board[capturedSquare+fileOffset]!=(isWhite?1:-1))continue;
        const int move=(capturedSquare+fileOffset)|(petir_header::enPassantTarget<<7);
        auto childPosition=position;
        const auto savedNodeCount=petir_header::nodeCount;
        if(isWhite)petir_makemove::makeWhiteMove(move,childPosition);else petir_makemove::makeBlackMove(move,childPosition);
        petir_header::nodeCount=savedNodeCount; // Legality probes are not search nodes.
        if(isWhite?petir_tools::whiteInCheck(childPosition):petir_tools::blackInCheck(childPosition))continue;
        ++moveCount;moveList[moveCount].move=move;moveList[moveCount].score=0;
    }
}
}
// movgen.pas:687
void generateBlackQuietMoves(petir_header::tdata& position, petir_header::tmovelist& moveList, std::int32_t& moveCountOut) {
    // Count locally: MSVC would otherwise reload the caller's count after
    // every move-list store. The count is published once on return.
    std::int32_t moveCount = moveCountOut;
    std::uint64_t pieces{};
    std::uint64_t targets{};
    std::int32_t fromSquare{};
    std::int32_t toSquare{};
    std::int32_t moveIndex=0;
    moveCount = 0;
    pieces = (position.blackPawns & (~petir_bitboard_mask::rankMask[1]));
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        if ((position.board[pas::sub(fromSquare, 8)] == 0)) {
            (moveCount = pas::increment(moveCount, 1));
            auto& entry = moveList[++moveIndex];
            entry.move = pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 8), 7));
            entry.score = petir_header::blackHistory[petir_header::blackPawn][pas::sub(fromSquare, 8)];
            if (((fromSquare >= petir_header::a7) && (position.board[pas::sub(fromSquare, 16)] == 0))) {
                (moveCount = pas::increment(moveCount, 1));
                auto& entry = moveList[++moveIndex];
                entry.move = pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 16), 7));
                entry.score = petir_header::blackHistory[petir_header::blackPawn][pas::sub(fromSquare, 16)];
            }
        }
    }
    pieces = position.blackKnights;
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = (petir_bitboard_mask::knightAttacks[fromSquare] & (~position.allPieces));
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            auto& entry = moveList[++moveIndex];
            entry.move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            entry.score = petir_header::blackHistory[petir_header::blackKnight][toSquare];
        }
    }
    pieces = position.blackQueens;
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::horizontalAttacks[fromSquare][(pas::shiftRight(position.allPieces, pas::add((fromSquare & 56), 1)) & 63)];
        targets = (targets | petir_bitboard_mask::verticalAttacks[fromSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[fromSquare]) & 63)]);
        targets = (targets | petir_bitboard_mask::diagonalA8H1Attacks[fromSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[fromSquare]) & 255)]);
        targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[fromSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[fromSquare]) & 255)]);
        targets = (targets & (~position.allPieces));
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            auto& entry = moveList[++moveIndex];
            entry.move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            entry.score = petir_header::blackHistory[petir_header::blackQueen][toSquare];
            (moveCount = pas::increment(moveCount, 1));
        }
    }
    pieces = position.blackBishops;
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::diagonalA8H1Attacks[fromSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[fromSquare]) & 255)];
        targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[fromSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[fromSquare]) & 255)]);
        targets = (targets & (~position.allPieces));
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            auto& entry = moveList[++moveIndex];
            entry.move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            entry.score = petir_header::blackHistory[petir_header::blackBishop][toSquare];
        }
    }
    pieces = position.blackRooks;
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::horizontalAttacks[fromSquare][(pas::shiftRight(position.allPieces, pas::add((fromSquare & 56), 1)) & 63)];
        targets = (targets | petir_bitboard_mask::verticalAttacks[fromSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[fromSquare]) & 63)]);
        targets = (targets & (~position.allPieces));
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            auto& entry = moveList[++moveIndex];
            entry.move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            entry.score = petir_header::blackHistory[petir_header::blackRook][toSquare];
        }
    }
    fromSquare = petir_bitboard_mask::kingSquare(position.blackKingBoard);
    targets = (petir_bitboard_mask::kingAttacks[fromSquare] & (~position.allPieces));
    for(move_bits<true> remaining(targets); remaining; ) {
        toSquare = remaining.pop();
        (moveCount = pas::increment(moveCount, 1));
        moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
        moveList[moveCount].score = petir_header::blackHistory[petir_header::blackKing][toSquare];
    }
    if (((((position.castleRights & petir_header::blackCastleRights) != 0) && (position.board[petir_header::e8] == petir_header::blackKing)) && (!petir_tools::attackedByWhite(position, petir_header::e8)))) {
        if (((((((position.board[petir_header::f8] == 0) && (position.board[petir_header::g8] == 0)) && (position.board[petir_header::h8] == petir_header::blackRook)) && (!petir_tools::attackedByWhite(position, petir_header::f8))) && (!petir_tools::attackedByWhite(position, petir_header::g8))) && ((position.castleRights & petir_header::blackShortCastleRight) != 0))) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(petir_header::shortCastleTarget, 7));
            moveList[moveCount].score = petir_header::blackHistory[petir_header::blackKing][petir_header::shortCastleTarget];
        }
        if ((((((((position.board[petir_header::d8] == 0) && (position.board[petir_header::c8] == 0)) && (position.board[petir_header::b8] == 0)) && ((position.castleRights & petir_header::blackLongCastleRight) != 0)) && (position.board[petir_header::a8] == petir_header::blackRook)) && (!petir_tools::attackedByWhite(position, petir_header::d8))) && (!petir_tools::attackedByWhite(position, petir_header::c8)))) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(petir_header::longCastleTarget, 7));
            moveList[moveCount].score = petir_header::blackHistory[petir_header::blackKing][petir_header::longCastleTarget];
        }
    }
    moveCountOut = moveCount;
}

// movgen.pas:868
void generateBlackCaptures(petir_header::tdata& position, petir_header::tmovelist& moveList, std::int32_t& moveCountOut) {
    // Count locally: MSVC would otherwise reload the caller's count after
    // every move-list store. The count is published once on return.
    std::int32_t moveCount = moveCountOut;
    std::uint64_t pieces{};
    std::uint64_t targets{};
    std::int32_t fromSquare{};
    std::int32_t toSquare{};
    std::int32_t file{};
    std::int32_t* moveListAddress{};
    moveCount = 0;
    moveListAddress = pas::address(moveList);
    pieces = (position.blackPawns & petir_bitboard_mask::rankMask[1]);
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        if ((((fromSquare & 7) > 0) && (position.board[pas::sub(fromSquare, 9)] > 0))) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 9), 7)), pas::shiftLeft(petir_header::queenPromotion, 14));
            moveList[moveCount].score = pas::add(pas::sub(petir_movgen::pieceOrderValue[position.board[pas::sub(fromSquare, 9)]], petir_movgen::pawnOrderValue), 5);
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 9), 7)), pas::shiftLeft(petir_header::knightPromotion, 14));
            moveList[moveCount].score = pas::add(pas::sub(petir_movgen::pieceOrderValue[position.board[pas::sub(fromSquare, 9)]], petir_movgen::pawnOrderValue), 5);
        }
        if ((((fromSquare & 7) < 7) && (position.board[pas::sub(fromSquare, 7)] > 0))) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 7), 7)), pas::shiftLeft(petir_header::queenPromotion, 14));
            moveList[moveCount].score = pas::add(pas::sub(petir_movgen::pieceOrderValue[position.board[pas::sub(fromSquare, 7)]], petir_movgen::pawnOrderValue), 5);
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 7), 7)), pas::shiftLeft(petir_header::knightPromotion, 14));
            moveList[moveCount].score = pas::add(pas::sub(petir_movgen::pieceOrderValue[position.board[pas::sub(fromSquare, 7)]], petir_movgen::pawnOrderValue), 5);
        }
        if ((position.board[pas::sub(fromSquare, 8)] == 0)) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 8), 7)), pas::shiftLeft(petir_header::queenPromotion, 14));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, pas::sub(fromSquare, 8));
            if ((moveList[moveCount].score == 0)) {
                moveList[moveCount].score = 3;
            }
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 8), 7)), pas::shiftLeft(petir_header::knightPromotion, 14));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, pas::sub(fromSquare, 8));
            if ((moveList[moveCount].score == 0)) {
                moveList[moveCount].score = 3;
            }
        }
    }
    pieces = (position.blackPawns & (~petir_bitboard_mask::rankMask[1]));
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        file = (fromSquare & 7);
        if (((position.board[pas::sub(fromSquare, 9)] > 0) && (file > 0))) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 9), 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[pas::sub(fromSquare, 9)]], petir_movgen::pawnOrderValue);
        }
        if (((position.board[pas::sub(fromSquare, 7)] > 0) && (file < 7))) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 7), 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[pas::sub(fromSquare, 7)]], petir_movgen::pawnOrderValue);
        }
        if (((pas::shiftRight(fromSquare, 3) == 3) && (pas::abs(pas::sub((fromSquare & 7), position.enPassantFile)) == 1))) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(petir_header::enPassantTarget, 7));
            moveList[moveCount].score = petir_movgen::pawnOrderValue;
        }
    }
    pieces = position.blackKnights;
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = (petir_bitboard_mask::knightAttacks[fromSquare] & position.whitePieces);
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::knightOrderValue);
            if ((moveList[moveCount].score <= 0)) {
                moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, toSquare);
            }
        }
    }
    pieces = position.blackBishops;
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::diagonalA8H1Attacks[fromSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[fromSquare]) & 255)];
        targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[fromSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[fromSquare]) & 255)]);
        targets = (targets & position.whitePieces);
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::bishopOrderValue);
            if ((moveList[moveCount].score <= 0)) {
                moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, toSquare);
            }
        }
    }
    pieces = position.blackRooks;
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::horizontalAttacks[fromSquare][(pas::shiftRight(position.allPieces, pas::add((fromSquare & 56), 1)) & 63)];
        targets = (targets | petir_bitboard_mask::verticalAttacks[fromSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[fromSquare]) & 63)]);
        targets = (targets & position.whitePieces);
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::rookOrderValue);
            if ((moveList[moveCount].score <= 0)) {
                moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, toSquare);
            }
        }
    }
    pieces = position.blackQueens;
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::horizontalAttacks[fromSquare][(pas::shiftRight(position.allPieces, pas::add((fromSquare & 56), 1)) & 63)];
        targets = (targets | petir_bitboard_mask::verticalAttacks[fromSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[fromSquare]) & 63)]);
        targets = (targets | petir_bitboard_mask::diagonalA8H1Attacks[fromSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[fromSquare]) & 255)]);
        targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[fromSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[fromSquare]) & 255)]);
        targets = (targets & position.whitePieces);
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, toSquare);
        }
    }
    fromSquare = petir_bitboard_mask::kingSquare(position.blackKingBoard);
    targets = (petir_bitboard_mask::kingAttacks[fromSquare] & position.whitePieces);
    for(move_bits<true> remaining(targets); remaining; ) {
        toSquare = remaining.pop();
        (moveCount = pas::increment(moveCount, 1));
        moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
        moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, toSquare);
    }
    moveCountOut = moveCount;
}

// movgen.pas:1041
void generateBlackCapturesAndPushes(petir_header::tdata& position, petir_header::tmovelist& moveList, std::int32_t& moveCountOut) {
    // Count locally: MSVC would otherwise reload the caller's count after
    // every move-list store. The count is published once on return.
    std::int32_t moveCount = moveCountOut;
    std::uint64_t pieces{};
    std::uint64_t targets{};
    std::int32_t fromSquare{};
    std::int32_t toSquare{};
    std::int32_t file{};
    moveCount = 0;
    pieces = (position.blackPawns & petir_bitboard_mask::rankMask[1]);
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        if ((((fromSquare & 7) > 0) && (position.board[pas::sub(fromSquare, 9)] > 0))) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 9), 7)), pas::shiftLeft(petir_header::queenPromotion, 14));
            moveList[moveCount].score = pas::add(pas::sub(petir_movgen::pieceOrderValue[position.board[pas::sub(fromSquare, 9)]], petir_movgen::pawnOrderValue), 5);
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 9), 7)), pas::shiftLeft(petir_header::knightPromotion, 14));
            moveList[moveCount].score = pas::add(pas::sub(petir_movgen::pieceOrderValue[position.board[pas::sub(fromSquare, 9)]], petir_movgen::pawnOrderValue), 5);
        }
        if ((((fromSquare & 7) < 7) && (position.board[pas::sub(fromSquare, 7)] > 0))) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 7), 7)), pas::shiftLeft(petir_header::queenPromotion, 14));
            moveList[moveCount].score = pas::add(pas::sub(petir_movgen::pieceOrderValue[position.board[pas::sub(fromSquare, 7)]], petir_movgen::pawnOrderValue), 5);
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 7), 7)), pas::shiftLeft(petir_header::knightPromotion, 14));
            moveList[moveCount].score = pas::add(pas::sub(petir_movgen::pieceOrderValue[position.board[pas::sub(fromSquare, 7)]], petir_movgen::pawnOrderValue), 5);
        }
        if ((position.board[pas::sub(fromSquare, 8)] == 0)) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 8), 7)), pas::shiftLeft(petir_header::queenPromotion, 14));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, pas::sub(fromSquare, 8));
            if ((moveList[moveCount].score == 0)) {
                moveList[moveCount].score = 3;
            }
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 8), 7)), pas::shiftLeft(petir_header::knightPromotion, 14));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, pas::sub(fromSquare, 8));
            if ((moveList[moveCount].score == 0)) {
                moveList[moveCount].score = 3;
            }
        }
    }
    pieces = (position.blackPawns & (petir_bitboard_mask::rankMask[2] | petir_bitboard_mask::rankMask[3]));
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        if ((position.board[pas::sub(fromSquare, 8)] == 0)) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 8), 7));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, pas::sub(fromSquare, 8));
            if ((moveList[moveCount].score == 0)) {
                moveList[moveCount].score = 1;
            }
        }
    }
    pieces = (position.blackPawns & (~petir_bitboard_mask::rankMask[1]));
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        file = (fromSquare & 7);
        if (((position.board[pas::sub(fromSquare, 9)] > 0) && (file > 0))) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 9), 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[pas::sub(fromSquare, 9)]], petir_movgen::pawnOrderValue);
        }
        if (((position.board[pas::sub(fromSquare, 7)] > 0) && (file < 7))) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 7), 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[pas::sub(fromSquare, 7)]], petir_movgen::pawnOrderValue);
        }
        if (((pas::shiftRight(fromSquare, 3) == 3) && (pas::abs(pas::sub((fromSquare & 7), position.enPassantFile)) == 1))) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(petir_header::enPassantTarget, 7));
            moveList[moveCount].score = petir_movgen::pawnOrderValue;
        }
    }
    pieces = position.blackKnights;
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = (petir_bitboard_mask::knightAttacks[fromSquare] & position.whitePieces);
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::knightOrderValue);
            if ((moveList[moveCount].score <= 0)) {
                moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, toSquare);
            }
        }
    }
    pieces = position.blackBishops;
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::diagonalA8H1Attacks[fromSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[fromSquare]) & 255)];
        targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[fromSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[fromSquare]) & 255)]);
        targets = (targets & position.whitePieces);
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::bishopOrderValue);
            if ((moveList[moveCount].score <= 0)) {
                moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, toSquare);
            }
        }
    }
    pieces = position.blackRooks;
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::horizontalAttacks[fromSquare][(pas::shiftRight(position.allPieces, pas::add((fromSquare & 56), 1)) & 63)];
        targets = (targets | petir_bitboard_mask::verticalAttacks[fromSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[fromSquare]) & 63)]);
        targets = (targets & position.whitePieces);
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::rookOrderValue);
            if ((moveList[moveCount].score <= 0)) {
                moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, toSquare);
            }
        }
    }
    pieces = position.blackQueens;
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::horizontalAttacks[fromSquare][(pas::shiftRight(position.allPieces, pas::add((fromSquare & 56), 1)) & 63)];
        targets = (targets | petir_bitboard_mask::verticalAttacks[fromSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[fromSquare]) & 63)]);
        targets = (targets | petir_bitboard_mask::diagonalA8H1Attacks[fromSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[fromSquare]) & 255)]);
        targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[fromSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[fromSquare]) & 255)]);
        targets = (targets & position.whitePieces);
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, toSquare);
        }
    }
    fromSquare = petir_bitboard_mask::kingSquare(position.blackKingBoard);
    targets = (petir_bitboard_mask::kingAttacks[fromSquare] & position.whitePieces);
    for(move_bits<true> remaining(targets); remaining; ) {
        toSquare = remaining.pop();
        (moveCount = pas::increment(moveCount, 1));
        moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
        moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, toSquare);
    }
    targets = (petir_bitboard_mask::kingAttacks[fromSquare] & pas::shiftLeft(position.whitePawns, 8));
    for(move_bits<false> remaining(targets); remaining; ) {
        toSquare = remaining.pop();
        if (((position.board[toSquare] == 0) && ((petir_bitboard_mask::whitePassedPawnMask[pas::sub(toSquare, 8)] & position.blackPawns) == 0))) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = 0;
        }
    }
    moveCountOut = moveCount;
}

// movgen.pas:1238
void generateBlackCapturesAndChecks(petir_header::tdata& position, petir_header::tmovelist& moveList, std::int32_t& moveCountOut) {
    // Count locally: MSVC would otherwise reload the caller's count after
    // every move-list store. The count is published once on return.
    std::int32_t moveCount = moveCountOut;
    std::uint64_t pieces{};
    std::uint64_t targets{};
    std::uint64_t allTargets{};
    std::int32_t fromSquare{};
    std::int32_t toSquare{};
    std::int32_t enemyKingSquare{};
    std::uint64_t diagonalCheckSquares{};
    std::uint64_t straightCheckSquares{};
    std::uint64_t knightCheckSquares{};
    moveCount = 0;
    enemyKingSquare = petir_bitboard_mask::kingSquare(position.whiteKingBoard);
    diagonalCheckSquares = petir_bitboard_mask::diagonalA8H1Attacks[enemyKingSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[enemyKingSquare]) & 255)];
    diagonalCheckSquares = (diagonalCheckSquares | petir_bitboard_mask::diagonalH8A1Attacks[enemyKingSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[enemyKingSquare]) & 255)]);
    straightCheckSquares = petir_bitboard_mask::horizontalAttacks[enemyKingSquare][(pas::shiftRight(position.allPieces, pas::add((enemyKingSquare & 56), 1)) & 63)];
    straightCheckSquares = (straightCheckSquares | petir_bitboard_mask::verticalAttacks[enemyKingSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[enemyKingSquare]) & 63)]);
    knightCheckSquares = petir_bitboard_mask::knightAttacks[enemyKingSquare];
    pieces = (pas::shiftRight((position.blackPawns & petir_bitboard_mask::notFileAMask), 17) & position.whiteKingBoard);
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        (fromSquare = pas::increment(fromSquare, 17));
        if ((position.board[pas::sub(fromSquare, 8)] == 0)) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 8), 7));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, pas::sub(fromSquare, 8));
        }
    }
    pieces = (pas::shiftRight((position.blackPawns & petir_bitboard_mask::notFileHMask), 15) & position.whiteKingBoard);
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        (fromSquare = pas::increment(fromSquare, 15));
        if ((position.board[pas::sub(fromSquare, 8)] == 0)) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 8), 7));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, pas::sub(fromSquare, 8));
        }
    }
    pieces = (pas::shiftRight((position.blackPawns & petir_bitboard_mask::notFileAMask), 9) & position.whitePieces);
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        fromSquare = pas::add(fromSquare, 9);
        if ((pas::shiftRight(fromSquare, 3) == 1)) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 9), 7)), pas::shiftLeft(petir_header::queenPromotion, 14));
            moveList[moveCount].score = pas::add(pas::sub(petir_movgen::pieceOrderValue[position.board[pas::sub(fromSquare, 9)]], petir_movgen::pawnOrderValue), 3);
        } else {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 9), 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[pas::sub(fromSquare, 9)]], petir_movgen::pawnOrderValue);
        }
    }
    pieces = (pas::shiftRight((position.blackPawns & petir_bitboard_mask::notFileHMask), 7) & position.whitePieces);
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        fromSquare = pas::add(fromSquare, 7);
        if ((pas::shiftRight(fromSquare, 3) == 1)) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 7), 7)), pas::shiftLeft(petir_header::queenPromotion, 14));
            moveList[moveCount].score = pas::add(pas::sub(petir_movgen::pieceOrderValue[position.board[pas::sub(fromSquare, 7)]], petir_movgen::pawnOrderValue), 3);
        } else {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 7), 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[pas::sub(fromSquare, 7)]], petir_movgen::pawnOrderValue);
        }
    }
    pieces = (position.blackPawns & petir_bitboard_mask::rankMask[1]);
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        if ((position.board[pas::sub(fromSquare, 8)] == 0)) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 8), 7)), pas::shiftLeft(petir_header::queenPromotion, 14));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, pas::sub(fromSquare, 8));
            if ((moveList[moveCount].score == 0)) {
                moveList[moveCount].score = 3;
            }
        }
    }
    pieces = (position.blackPawns & (petir_bitboard_mask::rankMask[2] | petir_bitboard_mask::rankMask[3]));
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        if ((position.board[pas::sub(fromSquare, 8)] == 0)) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(pas::sub(fromSquare, 8), 7));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, pas::sub(fromSquare, 8));
            if ((moveList[moveCount].score == 0)) {
                moveList[moveCount].score = 1;
            }
        }
    }
    if ((position.enPassantFile != petir_header::noEnPassant)) {
        pieces = (position.blackPawns & petir_bitboard_mask::rankMask[3]);
        for(move_bits<false> remaining(pieces); remaining; ) {
            fromSquare = remaining.pop();
            if ((pas::abs(pas::sub((fromSquare & 7), position.enPassantFile)) == 1)) {
                (moveCount = pas::increment(moveCount, 1));
                moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(petir_header::enPassantTarget, 7));
                moveList[moveCount].score = petir_movgen::pawnOrderValue;
            }
        }
    }
    pieces = position.blackKnights;
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = (petir_bitboard_mask::knightAttacks[fromSquare] & position.whitePieces);
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::knightOrderValue);
            if ((moveList[moveCount].score < 0)) {
                moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, toSquare);
            }
        }
        targets = ((petir_bitboard_mask::knightAttacks[fromSquare] & knightCheckSquares) & (~position.allPieces));
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, toSquare);
        }
    }
    pieces = position.blackBishops;
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::diagonalA8H1Attacks[fromSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[fromSquare]) & 255)];
        targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[fromSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[fromSquare]) & 255)]);
        allTargets = targets;
        targets = (targets & position.whitePieces);
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::bishopOrderValue);
            if ((moveList[moveCount].score < 0)) {
                moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, toSquare);
            }
        }
        targets = ((allTargets & diagonalCheckSquares) & (~position.allPieces));
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, toSquare);
        }
    }
    pieces = position.blackRooks;
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::horizontalAttacks[fromSquare][(pas::shiftRight(position.allPieces, pas::add((fromSquare & 56), 1)) & 63)];
        targets = (targets | petir_bitboard_mask::verticalAttacks[fromSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[fromSquare]) & 63)]);
        allTargets = targets;
        targets = (targets & position.whitePieces);
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::rookOrderValue);
            if ((moveList[moveCount].score < 0)) {
                moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, toSquare);
            }
        }
        targets = ((allTargets & straightCheckSquares) & (~position.allPieces));
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, toSquare);
        }
    }
    pieces = position.blackQueens;
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::horizontalAttacks[fromSquare][(pas::shiftRight(position.allPieces, pas::add((fromSquare & 56), 1)) & 63)];
        targets = (targets | petir_bitboard_mask::verticalAttacks[fromSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[fromSquare]) & 63)]);
        targets = (targets | petir_bitboard_mask::diagonalA8H1Attacks[fromSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[fromSquare]) & 255)]);
        targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[fromSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[fromSquare]) & 255)]);
        allTargets = targets;
        targets = (targets & position.whitePieces);
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, toSquare);
        }
        targets = ((allTargets & (straightCheckSquares | diagonalCheckSquares)) & (~position.allPieces));
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, toSquare);
        }
    }
    fromSquare = petir_bitboard_mask::kingSquare(position.blackKingBoard);
    targets = (petir_bitboard_mask::kingAttacks[fromSquare] & position.whitePieces);
    for(move_bits<true> remaining(targets); remaining; ) {
        toSquare = remaining.pop();
        (moveCount = pas::increment(moveCount, 1));
        moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
        moveList[moveCount].score = petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, toSquare);
    }
    targets = (petir_bitboard_mask::kingAttacks[fromSquare] & pas::shiftLeft(position.whitePawns, 8));
    for(move_bits<false> remaining(targets); remaining; ) {
        toSquare = remaining.pop();
        if (((position.board[toSquare] == 0) && ((petir_bitboard_mask::whitePassedPawnMask[pas::sub(toSquare, 8)] & position.blackPawns) == 0))) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = 0;
        }
    }
    moveCountOut = moveCount;
}

// movgen.pas:2080
void generateWhiteCapturesAndChecks(petir_header::tdata& position, petir_header::tmovelist& moveList, std::int32_t& moveCountOut) {
    // Count locally: MSVC would otherwise reload the caller's count after
    // every move-list store. The count is published once on return.
    std::int32_t moveCount = moveCountOut;
    std::uint64_t pieces{};
    std::uint64_t targets{};
    std::uint64_t allTargets{};
    std::int32_t fromSquare{};
    std::int32_t toSquare{};
    std::int32_t enemyKingSquare{};
    std::uint64_t diagonalCheckSquares{};
    std::uint64_t straightCheckSquares{};
    std::uint64_t knightCheckSquares{};
    std::uint64_t checkTargets{};
    moveCount = 0;
    enemyKingSquare = petir_bitboard_mask::kingSquare(position.blackKingBoard);
    diagonalCheckSquares = petir_bitboard_mask::diagonalA8H1Attacks[enemyKingSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[enemyKingSquare]) & 255)];
    diagonalCheckSquares = (diagonalCheckSquares | petir_bitboard_mask::diagonalH8A1Attacks[enemyKingSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[enemyKingSquare]) & 255)]);
    straightCheckSquares = petir_bitboard_mask::horizontalAttacks[enemyKingSquare][(pas::shiftRight(position.allPieces, pas::add((enemyKingSquare & 56), 1)) & 63)];
    straightCheckSquares = (straightCheckSquares | petir_bitboard_mask::verticalAttacks[enemyKingSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[enemyKingSquare]) & 63)]);
    knightCheckSquares = petir_bitboard_mask::knightAttacks[enemyKingSquare];
    pieces = (pas::shiftLeft((position.whitePawns & petir_bitboard_mask::notFileAMask), 15) & position.blackKingBoard);
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        (fromSquare = pas::decrement(fromSquare, 15));
        if ((position.board[pas::add(fromSquare, 8)] == 0)) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 8), 7));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, pas::add(fromSquare, 8));
        }
    }
    pieces = (pas::shiftLeft((position.whitePawns & petir_bitboard_mask::notFileHMask), 17) & position.blackKingBoard);
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        (fromSquare = pas::decrement(fromSquare, 17));
        if ((position.board[pas::add(fromSquare, 8)] == 0)) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 8), 7));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, pas::add(fromSquare, 8));
        }
    }
    pieces = (pas::shiftLeft((position.whitePawns & petir_bitboard_mask::notFileAMask), 7) & position.blackPieces);
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        fromSquare = pas::sub(fromSquare, 7);
        if ((pas::shiftRight(fromSquare, 3) == 6)) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 7), 7)), pas::shiftLeft(petir_header::queenPromotion, 14));
            moveList[moveCount].score = pas::add(pas::sub(petir_movgen::pieceOrderValue[position.board[pas::add(fromSquare, 7)]], petir_movgen::pawnOrderValue), 3);
        } else {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 7), 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[pas::add(fromSquare, 7)]], petir_movgen::pawnOrderValue);
        }
    }
    pieces = (pas::shiftLeft((position.whitePawns & petir_bitboard_mask::notFileHMask), 9) & position.blackPieces);
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        fromSquare = pas::sub(fromSquare, 9);
        if ((pas::shiftRight(fromSquare, 3) == 6)) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 9), 7)), pas::shiftLeft(petir_header::queenPromotion, 14));
            moveList[moveCount].score = pas::add(pas::sub(petir_movgen::pieceOrderValue[position.board[pas::add(fromSquare, 9)]], petir_movgen::pawnOrderValue), 3);
        } else {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 9), 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[pas::add(fromSquare, 9)]], petir_movgen::pawnOrderValue);
        }
    }
    pieces = (position.whitePawns & petir_bitboard_mask::rankMask[6]);
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        if ((position.board[pas::add(fromSquare, 8)] == 0)) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 8), 7)), pas::shiftLeft(petir_header::queenPromotion, 14));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, pas::add(fromSquare, 8));
            if ((moveList[moveCount].score == 0)) {
                moveList[moveCount].score = 3;
            }
        }
    }
    pieces = (position.whitePawns & (petir_bitboard_mask::rankMask[5] | petir_bitboard_mask::rankMask[4]));
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        if ((position.board[pas::add(fromSquare, 8)] == 0)) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 8), 7));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, pas::add(fromSquare, 8));
            if ((moveList[moveCount].score == 0)) {
                moveList[moveCount].score = 1;
            }
        }
    }
    if ((position.enPassantFile != petir_header::noEnPassant)) {
        pieces = (position.whitePawns & petir_bitboard_mask::rankMask[4]);
        for(move_bits<false> remaining(pieces); remaining; ) {
            fromSquare = remaining.pop();
            if ((pas::abs(pas::sub((fromSquare & 7), position.enPassantFile)) == 1)) {
                (moveCount = pas::increment(moveCount, 1));
                moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(petir_header::enPassantTarget, 7));
                moveList[moveCount].score = petir_movgen::pawnOrderValue;
            }
        }
    }
    pieces = position.whiteKnights;
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = (petir_bitboard_mask::knightAttacks[fromSquare] & position.blackPieces);
        for(move_bits<false> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::knightOrderValue);
            if ((moveList[moveCount].score < 0)) {
                moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, toSquare);
            }
        }
        targets = ((petir_bitboard_mask::knightAttacks[fromSquare] & knightCheckSquares) & (~position.allPieces));
        for(move_bits<false> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, toSquare);
        }
    }
    pieces = position.whiteBishops;
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::diagonalA8H1Attacks[fromSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[fromSquare]) & 255)];
        targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[fromSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[fromSquare]) & 255)]);
        allTargets = targets;
        targets = (targets & position.blackPieces);
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::bishopOrderValue);
            if ((moveList[moveCount].score < 0)) {
                moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, toSquare);
            }
        }
        targets = ((allTargets & diagonalCheckSquares) & (~position.allPieces));
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, toSquare);
        }
    }
    pieces = position.whiteRooks;
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::horizontalAttacks[fromSquare][(pas::shiftRight(position.allPieces, pas::add((fromSquare & 56), 1)) & 63)];
        targets = (targets | petir_bitboard_mask::verticalAttacks[fromSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[fromSquare]) & 63)]);
        allTargets = targets;
        targets = (targets & position.blackPieces);
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::rookOrderValue);
            if ((moveList[moveCount].score < 0)) {
                moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, toSquare);
            }
        }
        targets = ((allTargets & straightCheckSquares) & (~position.allPieces));
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, toSquare);
        }
    }
    pieces = position.whiteQueens;
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::horizontalAttacks[fromSquare][(pas::shiftRight(position.allPieces, pas::add((fromSquare & 56), 1)) & 63)];
        targets = (targets | petir_bitboard_mask::verticalAttacks[fromSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[fromSquare]) & 63)]);
        targets = (targets | petir_bitboard_mask::diagonalA8H1Attacks[fromSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[fromSquare]) & 255)]);
        targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[fromSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[fromSquare]) & 255)]);
        allTargets = targets;
        targets = (targets & position.blackPieces);
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, toSquare);
        }
        targets = ((allTargets & (diagonalCheckSquares | straightCheckSquares)) & (~position.allPieces));
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, toSquare);
        }
    }
    fromSquare = petir_bitboard_mask::kingSquare(position.whiteKingBoard);
    targets = (petir_bitboard_mask::kingAttacks[fromSquare] & position.blackPieces);
    for(move_bits<false> remaining(targets); remaining; ) {
        toSquare = remaining.pop();
        (moveCount = pas::increment(moveCount, 1));
        moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
        moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, toSquare);
    }
    targets = (petir_bitboard_mask::kingAttacks[fromSquare] & pas::shiftRight(position.blackPawns, 8));
    for(move_bits<false> remaining(targets); remaining; ) {
        toSquare = remaining.pop();
        if (((position.board[toSquare] == 0) && ((petir_bitboard_mask::blackPassedPawnMask[pas::add(toSquare, 8)] & position.whitePawns) == 0))) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = 0;
        }
    }
    moveCountOut = moveCount;
}

// movgen.pas:1681
void generateWhiteCaptures(petir_header::tdata& position, petir_header::tmovelist& moveList, std::int32_t& moveCountOut) {
    // Count locally: MSVC would otherwise reload the caller's count after
    // every move-list store. The count is published once on return.
    std::int32_t moveCount = moveCountOut;
    std::uint64_t pieces{};
    std::uint64_t targets{};
    std::uint64_t unpromotedPawns{};
    std::int32_t fromSquare{};
    std::int32_t toSquare{};
    moveCount = 0;
    pieces = (position.whitePawns & petir_bitboard_mask::rankMask[6]);
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        if ((((fromSquare & 7) < 7) && (position.board[pas::add(fromSquare, 9)] < 0))) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 9), 7)), pas::shiftLeft(petir_header::queenPromotion, 14));
            moveList[moveCount].score = pas::add(pas::sub(petir_movgen::pieceOrderValue[position.board[pas::add(fromSquare, 9)]], petir_movgen::pawnOrderValue), 5);
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 9), 7)), pas::shiftLeft(petir_header::knightPromotion, 14));
            moveList[moveCount].score = pas::add(pas::sub(petir_movgen::pieceOrderValue[position.board[pas::add(fromSquare, 9)]], petir_movgen::pawnOrderValue), 5);
        }
        if ((((fromSquare & 7) > 0) && (position.board[pas::add(fromSquare, 7)] < 0))) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 7), 7)), pas::shiftLeft(petir_header::queenPromotion, 14));
            moveList[moveCount].score = pas::add(pas::sub(petir_movgen::pieceOrderValue[position.board[pas::add(fromSquare, 7)]], petir_movgen::pawnOrderValue), 5);
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 7), 7)), pas::shiftLeft(petir_header::knightPromotion, 14));
            moveList[moveCount].score = pas::add(pas::sub(petir_movgen::pieceOrderValue[position.board[pas::add(fromSquare, 7)]], petir_movgen::pawnOrderValue), 5);
        }
        if ((position.board[pas::add(fromSquare, 8)] == 0)) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 8), 7)), pas::shiftLeft(petir_header::queenPromotion, 14));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, pas::add(fromSquare, 8));
            if ((moveList[moveCount].score == 0)) {
                moveList[moveCount].score = 3;
            }
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 8), 7)), pas::shiftLeft(petir_header::knightPromotion, 14));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, pas::add(fromSquare, 8));
            if ((moveList[moveCount].score == 0)) {
                moveList[moveCount].score = 3;
            }
        }
    }
    unpromotedPawns = (position.whitePawns & (~petir_bitboard_mask::rankMask[6]));
    pieces = (pas::shiftLeft((unpromotedPawns & petir_bitboard_mask::notFileAMask), 7) & position.blackPieces);
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        fromSquare = pas::sub(fromSquare, 7);
        (moveCount = pas::increment(moveCount, 1));
        moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 7), 7));
        moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[pas::add(fromSquare, 7)]], petir_movgen::pawnOrderValue);
    }
    pieces = (pas::shiftLeft((unpromotedPawns & petir_bitboard_mask::notFileHMask), 9) & position.blackPieces);
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        fromSquare = pas::sub(fromSquare, 9);
        (moveCount = pas::increment(moveCount, 1));
        moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 9), 7));
        moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[pas::add(fromSquare, 9)]], petir_movgen::pawnOrderValue);
    }
    if ((position.enPassantFile != petir_header::noEnPassant)) {
        pieces = (position.whitePawns & petir_bitboard_mask::rankMask[4]);
        for(move_bits<false> remaining(pieces); remaining; ) {
            fromSquare = remaining.pop();
            if ((pas::abs(pas::sub((fromSquare & 7), position.enPassantFile)) == 1)) {
                (moveCount = pas::increment(moveCount, 1));
                moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(petir_header::enPassantTarget, 7));
                moveList[moveCount].score = petir_movgen::pawnOrderValue;
            }
        }
    }
    pieces = position.whiteKnights;
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = (petir_bitboard_mask::knightAttacks[fromSquare] & position.blackPieces);
        while ((targets != 0)) {
            toSquare = petir_bitboard_mask::firstBit(pas::address(targets));
            targets = (targets & (~petir_bitboard_mask::squareBit(toSquare)));
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::knightOrderValue);
            if ((moveList[moveCount].score <= 0)) {
                moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, toSquare);
            }
        }
    }
    pieces = position.whiteBishops;
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::diagonalA8H1Attacks[fromSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[fromSquare]) & 255)];
        targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[fromSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[fromSquare]) & 255)]);
        targets = (targets & position.blackPieces);
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::bishopOrderValue);
            if ((moveList[moveCount].score <= 0)) {
                moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, toSquare);
            }
        }
    }
    pieces = position.whiteRooks;
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::horizontalAttacks[fromSquare][(pas::shiftRight(position.allPieces, pas::add((fromSquare & 56), 1)) & 63)];
        targets = (targets | petir_bitboard_mask::verticalAttacks[fromSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[fromSquare]) & 63)]);
        targets = (targets & position.blackPieces);
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::rookOrderValue);
            if ((moveList[moveCount].score <= 0)) {
                moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, toSquare);
            }
        }
    }
    pieces = position.whiteQueens;
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::horizontalAttacks[fromSquare][(pas::shiftRight(position.allPieces, pas::add((fromSquare & 56), 1)) & 63)];
        targets = (targets | petir_bitboard_mask::verticalAttacks[fromSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[fromSquare]) & 63)]);
        targets = (targets | petir_bitboard_mask::diagonalA8H1Attacks[fromSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[fromSquare]) & 255)]);
        targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[fromSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[fromSquare]) & 255)]);
        targets = (targets & position.blackPieces);
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, toSquare);
        }
    }
    fromSquare = petir_bitboard_mask::kingSquare(position.whiteKingBoard);
    targets = (petir_bitboard_mask::kingAttacks[fromSquare] & position.blackPieces);
    for(move_bits<false> remaining(targets); remaining; ) {
        toSquare = remaining.pop();
        (moveCount = pas::increment(moveCount, 1));
        moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
        moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, toSquare);
    }
    moveCountOut = moveCount;
}

// movgen.pas:1864
void generateWhiteCapturesAndPushes(petir_header::tdata& position, petir_header::tmovelist& moveList, std::int32_t& moveCountOut) {
    // Count locally: MSVC would otherwise reload the caller's count after
    // every move-list store. The count is published once on return.
    std::int32_t moveCount = moveCountOut;
    std::uint64_t pieces{};
    std::uint64_t targets{};
    std::uint64_t unpromotedPawns{};
    std::int32_t fromSquare{};
    std::int32_t toSquare{};
    moveCount = 0;
    pieces = (position.whitePawns & petir_bitboard_mask::rankMask[6]);
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        if ((((fromSquare & 7) < 7) && (position.board[pas::add(fromSquare, 9)] < 0))) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 9), 7)), pas::shiftLeft(petir_header::queenPromotion, 14));
            moveList[moveCount].score = pas::add(pas::sub(petir_movgen::pieceOrderValue[position.board[pas::add(fromSquare, 9)]], petir_movgen::pawnOrderValue), 5);
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 9), 7)), pas::shiftLeft(petir_header::knightPromotion, 14));
            moveList[moveCount].score = pas::add(pas::sub(petir_movgen::pieceOrderValue[position.board[pas::add(fromSquare, 9)]], petir_movgen::pawnOrderValue), 5);
        }
        if ((((fromSquare & 7) > 0) && (position.board[pas::add(fromSquare, 7)] < 0))) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 7), 7)), pas::shiftLeft(petir_header::queenPromotion, 14));
            moveList[moveCount].score = pas::add(pas::sub(petir_movgen::pieceOrderValue[position.board[pas::add(fromSquare, 7)]], petir_movgen::pawnOrderValue), 5);
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 7), 7)), pas::shiftLeft(petir_header::knightPromotion, 14));
            moveList[moveCount].score = pas::add(pas::sub(petir_movgen::pieceOrderValue[position.board[pas::add(fromSquare, 7)]], petir_movgen::pawnOrderValue), 5);
        }
        if ((position.board[pas::add(fromSquare, 8)] == 0)) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 8), 7)), pas::shiftLeft(petir_header::queenPromotion, 14));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, pas::add(fromSquare, 8));
            if ((moveList[moveCount].score == 0)) {
                moveList[moveCount].score = 3;
            }
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 8), 7)), pas::shiftLeft(petir_header::knightPromotion, 14));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, pas::add(fromSquare, 8));
            if ((moveList[moveCount].score == 0)) {
                moveList[moveCount].score = 3;
            }
        }
    }
    pieces = (position.whitePawns & (petir_bitboard_mask::rankMask[5] | petir_bitboard_mask::rankMask[4]));
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        if ((position.board[pas::add(fromSquare, 8)] == 0)) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 8), 7));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, pas::add(fromSquare, 8));
            if ((moveList[moveCount].score == 0)) {
                moveList[moveCount].score = 1;
            }
        }
    }
    unpromotedPawns = (position.whitePawns & (~petir_bitboard_mask::rankMask[6]));
    pieces = (pas::shiftLeft((unpromotedPawns & petir_bitboard_mask::notFileAMask), 7) & position.blackPieces);
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        fromSquare = pas::sub(fromSquare, 7);
        (moveCount = pas::increment(moveCount, 1));
        moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 7), 7));
        moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[pas::add(fromSquare, 7)]], petir_movgen::pawnOrderValue);
    }
    pieces = (pas::shiftLeft((unpromotedPawns & petir_bitboard_mask::notFileHMask), 9) & position.blackPieces);
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        fromSquare = pas::sub(fromSquare, 9);
        (moveCount = pas::increment(moveCount, 1));
        moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 9), 7));
        moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[pas::add(fromSquare, 9)]], petir_movgen::pawnOrderValue);
    }
    if ((position.enPassantFile != petir_header::noEnPassant)) {
        pieces = (position.whitePawns & petir_bitboard_mask::rankMask[4]);
        for(move_bits<false> remaining(pieces); remaining; ) {
            fromSquare = remaining.pop();
            if ((pas::abs(pas::sub((fromSquare & 7), position.enPassantFile)) == 1)) {
                (moveCount = pas::increment(moveCount, 1));
                moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(petir_header::enPassantTarget, 7));
                moveList[moveCount].score = petir_movgen::pawnOrderValue;
            }
        }
    }
    pieces = position.whiteKnights;
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = (petir_bitboard_mask::knightAttacks[fromSquare] & position.blackPieces);
        while ((targets != 0)) {
            toSquare = petir_bitboard_mask::firstBit(pas::address(targets));
            targets = (targets & (~petir_bitboard_mask::squareBit(toSquare)));
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::knightOrderValue);
            if ((moveList[moveCount].score <= 0)) {
                moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, toSquare);
            }
        }
    }
    pieces = position.whiteBishops;
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::diagonalA8H1Attacks[fromSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[fromSquare]) & 255)];
        targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[fromSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[fromSquare]) & 255)]);
        targets = (targets & position.blackPieces);
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::bishopOrderValue);
            if ((moveList[moveCount].score <= 0)) {
                moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, toSquare);
            }
        }
    }
    pieces = position.whiteRooks;
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::horizontalAttacks[fromSquare][(pas::shiftRight(position.allPieces, pas::add((fromSquare & 56), 1)) & 63)];
        targets = (targets | petir_bitboard_mask::verticalAttacks[fromSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[fromSquare]) & 63)]);
        targets = (targets & position.blackPieces);
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::rookOrderValue);
            if ((moveList[moveCount].score <= 0)) {
                moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, toSquare);
            }
        }
    }
    pieces = position.whiteQueens;
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::horizontalAttacks[fromSquare][(pas::shiftRight(position.allPieces, pas::add((fromSquare & 56), 1)) & 63)];
        targets = (targets | petir_bitboard_mask::verticalAttacks[fromSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[fromSquare]) & 63)]);
        targets = (targets | petir_bitboard_mask::diagonalA8H1Attacks[fromSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[fromSquare]) & 255)]);
        targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[fromSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[fromSquare]) & 255)]);
        targets = (targets & position.blackPieces);
        for(move_bits<true> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, toSquare);
        }
    }
    fromSquare = petir_bitboard_mask::kingSquare(position.whiteKingBoard);
    targets = (petir_bitboard_mask::kingAttacks[fromSquare] & position.blackPieces);
    for(move_bits<false> remaining(targets); remaining; ) {
        toSquare = remaining.pop();
        (moveCount = pas::increment(moveCount, 1));
        moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
        moveList[moveCount].score = petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, toSquare);
    }
    targets = (petir_bitboard_mask::kingAttacks[fromSquare] & pas::shiftRight(position.blackPawns, 8));
    for(move_bits<false> remaining(targets); remaining; ) {
        toSquare = remaining.pop();
        if (((position.board[toSquare] == 0) && ((petir_bitboard_mask::blackPassedPawnMask[pas::add(toSquare, 8)] & position.whitePawns) == 0))) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = 0;
        }
    }
    moveCountOut = moveCount;
}

// movgen.pas:1520
void generateWhiteQuietMoves(petir_header::tdata& position, petir_header::tmovelist& moveList, std::int32_t& moveCountOut) {
    // Count locally: MSVC would otherwise reload the caller's count after
    // every move-list store. The count is published once on return.
    std::int32_t moveCount = moveCountOut;
    std::uint64_t pieces{};
    std::uint64_t targets{};
    std::int32_t fromSquare{};
    std::int32_t toSquare{};
    moveCount = 0;
    pieces = (position.whitePawns & (~petir_bitboard_mask::rankMask[6]));
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        if ((position.board[pas::add(fromSquare, 8)] == 0)) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 8), 7));
            moveList[moveCount].score = petir_header::whiteHistory[petir_header::whitePawn][pas::add(fromSquare, 8)];
            if (((fromSquare <= petir_header::h2) && (position.board[pas::add(fromSquare, 16)] == 0))) {
                (moveCount = pas::increment(moveCount, 1));
                moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(pas::add(fromSquare, 16), 7));
                moveList[moveCount].score = petir_header::whiteHistory[petir_header::whitePawn][pas::add(fromSquare, 16)];
            }
        }
    }
    pieces = position.whiteKnights;
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = (petir_bitboard_mask::knightAttacks[fromSquare] & (~position.allPieces));
        for(move_bits<false> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = petir_header::whiteHistory[petir_header::whiteKnight][toSquare];
        }
    }
    pieces = position.whiteQueens;
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::horizontalAttacks[fromSquare][(pas::shiftRight(position.allPieces, pas::add((fromSquare & 56), 1)) & 63)];
        targets = (targets | petir_bitboard_mask::verticalAttacks[fromSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[fromSquare]) & 63)]);
        targets = (targets | petir_bitboard_mask::diagonalA8H1Attacks[fromSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[fromSquare]) & 255)]);
        targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[fromSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[fromSquare]) & 255)]);
        targets = (targets & (~position.allPieces));
        for(move_bits<false> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = petir_header::whiteHistory[petir_header::whiteQueen][toSquare];
        }
    }
    pieces = position.whiteBishops;
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::diagonalA8H1Attacks[fromSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[fromSquare]) & 255)];
        targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[fromSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[fromSquare]) & 255)]);
        targets = (targets & (~position.allPieces));
        for(move_bits<false> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = petir_header::whiteHistory[petir_header::whiteBishop][toSquare];
        }
    }
    pieces = position.whiteRooks;
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::horizontalAttacks[fromSquare][(pas::shiftRight(position.allPieces, pas::add((fromSquare & 56), 1)) & 63)];
        targets = (targets | petir_bitboard_mask::verticalAttacks[fromSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[fromSquare]) & 63)]);
        targets = (targets & (~position.allPieces));
        for(move_bits<false> remaining(targets); remaining; ) {
            toSquare = remaining.pop();
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
            moveList[moveCount].score = petir_header::whiteHistory[petir_header::whiteRook][toSquare];
        }
    }
    fromSquare = petir_bitboard_mask::kingSquare(position.whiteKingBoard);
    targets = (petir_bitboard_mask::kingAttacks[fromSquare] & (~position.allPieces));
    for(move_bits<false> remaining(targets); remaining; ) {
        toSquare = remaining.pop();
        (moveCount = pas::increment(moveCount, 1));
        moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
        moveList[moveCount].score = petir_header::whiteHistory[petir_header::whiteKing][toSquare];
    }
    if (((((position.castleRights & petir_header::whiteCastleRights) != 0) && (position.board[petir_header::e1] == petir_header::whiteKing)) && (!petir_tools::attackedByBlack(position, petir_header::e1)))) {
        if (((((((position.board[petir_header::f1] == 0) && (position.board[petir_header::g1] == 0)) && ((position.castleRights & petir_header::whiteShortCastleRight) != 0)) && (position.board[petir_header::h1] == petir_header::whiteRook)) && (!petir_tools::attackedByBlack(position, petir_header::f1))) && (!petir_tools::attackedByBlack(position, petir_header::g1)))) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(petir_header::shortCastleTarget, 7));
            moveList[moveCount].score = petir_header::whiteHistory[petir_header::whiteKing][petir_header::shortCastleTarget];
        }
        if ((((((((position.board[petir_header::d1] == 0) && (position.board[petir_header::c1] == 0)) && (position.board[petir_header::b1] == 0)) && ((position.castleRights & petir_header::whiteLongCastleRight) != 0)) && (position.board[petir_header::a1] == petir_header::whiteRook)) && (!petir_tools::attackedByBlack(position, petir_header::d1))) && (!petir_tools::attackedByBlack(position, petir_header::c1)))) {
            (moveCount = pas::increment(moveCount, 1));
            moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(petir_header::longCastleTarget, 7));
            moveList[moveCount].score = petir_header::whiteHistory[petir_header::whiteKing][petir_header::longCastleTarget];
        }
    }
    moveCountOut = moveCount;
}

// movgen.pas:30
void generateWhiteEvasions(petir_header::tdata& position, petir_header::tmovelist& moveList, std::int32_t& moveCountOut, std::int32_t& checkerCount) {
    // Count locally: MSVC would otherwise reload the caller's count after
    // every move-list store. The count is published once on return.
    std::int32_t moveCount = moveCountOut;
    std::uint64_t pieces{};
    std::uint64_t targets{};
    std::uint64_t enemyAttacks{};
    std::uint64_t answerSquares{};
    std::int32_t fromSquare{};
    std::int32_t toSquare{};
    std::int32_t kingSquare{};
    std::int32_t checkDirection{};
    std::int32_t promotionType{};
    std::uint64_t checkerBit{};
    std::int32_t checkerSquare{};
    std::uint64_t blockingSquares{};
    std::uint64_t occupancyWithoutKing{};
    std::uint64_t rotated90WithoutKing{};
    std::uint64_t rotatedA8H1WithoutKing{};
    std::uint64_t rotatedH8A1WithoutKing{};
    std::int32_t lastPushPromotionType{};
    moveCount = 0;
    checkerCount = 0;
    enemyAttacks = 0;
    kingSquare = petir_bitboard_mask::kingSquare(position.whiteKingBoard);
    checkerSquare = 64;
    pieces = position.blackKnights;
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        enemyAttacks = (enemyAttacks | petir_bitboard_mask::knightAttacks[fromSquare]);
        if (((petir_bitboard_mask::knightAttacks[fromSquare] & position.whiteKingBoard) != 0)) {
            (checkerCount = pas::increment(checkerCount, 1));
            checkerSquare = fromSquare;
        }
    }
    pieces = position.blackPawns;
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::blackPawnAttacks[fromSquare];
        enemyAttacks = (enemyAttacks | targets);
        if (((targets & position.whiteKingBoard) != 0)) {
            (checkerCount = pas::increment(checkerCount, 1));
            checkerSquare = fromSquare;
        }
    }
    occupancyWithoutKing = (position.allPieces & (~position.whiteKingBoard));
    rotated90WithoutKing = (position.allPiecesRotated90 & (~petir_bitboard_mask::rotated90SquareMask[kingSquare]));
    rotatedA8H1WithoutKing = (position.allPiecesA8H1 & (~petir_bitboard_mask::rotatedA8H1SquareMask[kingSquare]));
    rotatedH8A1WithoutKing = (position.allPiecesH8A1 & (~petir_bitboard_mask::rotatedH8A1SquareMask[kingSquare]));
    pieces = position.blackBishops;
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::diagonalA8H1Attacks[fromSquare][(pas::shiftRight(rotatedA8H1WithoutKing, petir_bitboard_mask::diagonalA8H1Shift[fromSquare]) & 255)];
        targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[fromSquare][(pas::shiftRight(rotatedH8A1WithoutKing, petir_bitboard_mask::diagonalH8A1Shift[fromSquare]) & 255)]);
        enemyAttacks = (enemyAttacks | targets);
        if (((targets & position.whiteKingBoard) != 0)) {
            (checkerCount = pas::increment(checkerCount, 1));
            checkerSquare = fromSquare;
        }
    }
    pieces = position.blackRooks;
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::horizontalAttacks[fromSquare][(pas::shiftRight(occupancyWithoutKing, pas::add((fromSquare & 56), 1)) & 63)];
        targets = (targets | petir_bitboard_mask::verticalAttacks[fromSquare][(pas::shiftRight(rotated90WithoutKing, petir_bitboard_mask::verticalShift[fromSquare]) & 63)]);
        enemyAttacks = (enemyAttacks | targets);
        if (((targets & position.whiteKingBoard) != 0)) {
            (checkerCount = pas::increment(checkerCount, 1));
            checkerSquare = fromSquare;
        }
    }
    pieces = position.blackQueens;
    for(move_bits<true> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::horizontalAttacks[fromSquare][(pas::shiftRight(occupancyWithoutKing, pas::add((fromSquare & 56), 1)) & 63)];
        targets = (targets | petir_bitboard_mask::verticalAttacks[fromSquare][(pas::shiftRight(rotated90WithoutKing, petir_bitboard_mask::verticalShift[fromSquare]) & 63)]);
        targets = (targets | petir_bitboard_mask::diagonalA8H1Attacks[fromSquare][(pas::shiftRight(rotatedA8H1WithoutKing, petir_bitboard_mask::diagonalA8H1Shift[fromSquare]) & 255)]);
        targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[fromSquare][(pas::shiftRight(rotatedH8A1WithoutKing, petir_bitboard_mask::diagonalH8A1Shift[fromSquare]) & 255)]);
        enemyAttacks = (enemyAttacks | targets);
        if (((targets & position.whiteKingBoard) != 0)) {
            (checkerCount = pas::increment(checkerCount, 1));
            checkerSquare = fromSquare;
        }
    }
    fromSquare = petir_bitboard_mask::kingSquare(position.blackKingBoard);
    enemyAttacks = (enemyAttacks | petir_bitboard_mask::kingAttacks[fromSquare]);
    pieces = ((petir_bitboard_mask::kingAttacks[kingSquare] & (~enemyAttacks)) & (~position.whitePieces));
    for(move_bits<true> remaining(pieces); remaining; ) {
        toSquare = remaining.pop();
        (moveCount = pas::increment(moveCount, 1));
        moveList[moveCount].move = pas::add(kingSquare, pas::shiftLeft(toSquare, 7));
        if ((position.board[toSquare] == 0)) {
            moveList[moveCount].score = petir_header::whiteHistory[petir_header::whiteKing][toSquare];
        } else {
            moveList[moveCount].score = pas::shiftLeft(petir_movgen::pieceOrderValue[position.board[toSquare]], 14);
        }
    }
    // Removing both pawns can uncover an attack; making the move also handles
    // en passant interpositions against a sliding-piece check.
    appendLegalEnPassantEvasions(position,moveList,moveCount,petir_header::whiteSide);
    if ((checkerCount == 1)) {
        checkerBit = petir_bitboard_mask::squareBit(checkerSquare);
        blockingSquares = petir_bitboard_mask::betweenMask[kingSquare][checkerSquare];
        pieces = position.whiteKnights;
        checkDirection = petir_bitboard_mask::rayDirection[kingSquare][checkerSquare];
        for(move_bits<false> remaining(pieces); remaining; ) {
            fromSquare = remaining.pop();
            targets = petir_bitboard_mask::knightAttacks[fromSquare];
            answerSquares = (targets & checkerBit);
            if ((answerSquares != 0)) {
                toSquare = petir_bitboard_mask::firstBit(pas::address(answerSquares));
                if ((!petir_tools::exposesWhiteKing(position, pas::add(fromSquare, pas::shiftLeft(toSquare, 7))))) {
                    (moveCount = pas::increment(moveCount, 1));
                    moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
                    moveList[moveCount].score = pas::shiftLeft(pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::knightOrderValue), 15);
                    if ((moveList[moveCount].score < 0)) {
                        moveList[moveCount].score = pas::shiftLeft(petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, toSquare), 15);
                    }
                }
            }
            answerSquares = (targets & blockingSquares);
            for(move_bits<false> remaining(answerSquares); remaining; ) {
                toSquare = remaining.pop();
                if ((!petir_tools::exposesWhiteKing(position, pas::add(fromSquare, pas::shiftLeft(toSquare, 7))))) {
                    (moveCount = pas::increment(moveCount, 1));
                    moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
                    moveList[moveCount].score = petir_header::whiteHistory[petir_header::whiteKnight][toSquare];
                }
            }
        }
        pieces = position.whitePawns;
        for(move_bits<false> remaining(pieces); remaining; ) {
            fromSquare = remaining.pop();
            targets = petir_bitboard_mask::whitePawnAttacks[fromSquare];
            answerSquares = (targets & checkerBit);
            if ((answerSquares != 0)) {
                toSquare = petir_bitboard_mask::firstBit(pas::address(answerSquares));
                if ((!petir_tools::exposesWhiteKing(position, pas::add(fromSquare, pas::shiftLeft(toSquare, 7))))) {
                    if ((pas::shiftRight(fromSquare, 3) == 6)) {
                        (moveCount = pas::increment(moveCount, 1));
                        moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(toSquare, 7)), pas::shiftLeft(petir_header::queenPromotion, 14));
                        moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::pawnOrderValue);
                        (moveCount = pas::increment(moveCount, 1));
                        moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(toSquare, 7)), pas::shiftLeft(petir_header::knightPromotion, 14));
                        moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::pawnOrderValue);
                    } else {
                        (moveCount = pas::increment(moveCount, 1));
                        moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
                        moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::pawnOrderValue);
                    }
                }
            }
            answerSquares = 0;
            if ((position.board[pas::add(fromSquare, 8)] == 0)) {
                answerSquares = (answerSquares | petir_bitboard_mask::squareBit(pas::add(fromSquare, 8)));
                if (((pas::shiftRight(fromSquare, 3) == 1) && (position.board[pas::add(fromSquare, 16)] == 0))) {
                    answerSquares = (answerSquares | petir_bitboard_mask::squareBit(pas::add(fromSquare, 16)));
                }
            }
            answerSquares = (answerSquares & blockingSquares);
            if ((answerSquares != 0)) {
                toSquare = petir_bitboard_mask::firstBit(pas::address(answerSquares));
                if ((!petir_tools::exposesWhiteKing(position, pas::add(fromSquare, pas::shiftLeft(toSquare, 7))))) {
                    if ((pas::shiftRight(fromSquare, 3) == 6)) {
                        lastPushPromotionType = petir_header::knightPromotion;
                        for (promotionType = petir_header::queenPromotion; promotionType <= lastPushPromotionType; ++promotionType) {
                            (moveCount = pas::increment(moveCount, 1));
                            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(toSquare, 7)), pas::shiftLeft(promotionType, 14));
                            moveList[moveCount].score = pas::shiftLeft(petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, toSquare), 15);
                            if ((moveList[moveCount].score == 0)) {
                                moveList[moveCount].score = pas::shiftLeft(3, 15);
                            }
                        }
                    } else {
                        (moveCount = pas::increment(moveCount, 1));
                        moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
                        moveList[moveCount].score = petir_header::whiteHistory[petir_header::whitePawn][toSquare];
                    }
                }
            }
        }
        pieces = position.whiteBishops;
        for(move_bits<false> remaining(pieces); remaining; ) {
            fromSquare = remaining.pop();
            targets = petir_bitboard_mask::diagonalA8H1Attacks[fromSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[fromSquare]) & 255)];
            targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[fromSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[fromSquare]) & 255)]);
            answerSquares = (targets & checkerBit);
            if ((answerSquares != 0)) {
                toSquare = petir_bitboard_mask::firstBit(pas::address(answerSquares));
                if ((!petir_tools::exposesWhiteKing(position, pas::add(fromSquare, pas::shiftLeft(toSquare, 7))))) {
                    (moveCount = pas::increment(moveCount, 1));
                    moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
                    moveList[moveCount].score = pas::shiftLeft(pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::bishopOrderValue), 15);
                    if ((moveList[moveCount].score < 0)) {
                        moveList[moveCount].score = pas::shiftLeft(petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, toSquare), 15);
                    }
                }
            }
            answerSquares = (targets & blockingSquares);
            for(move_bits<false> remaining(answerSquares); remaining; ) {
                toSquare = remaining.pop();
                if ((!petir_tools::exposesWhiteKing(position, pas::add(fromSquare, pas::shiftLeft(toSquare, 7))))) {
                    (moveCount = pas::increment(moveCount, 1));
                    moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
                    moveList[moveCount].score = petir_header::whiteHistory[petir_header::whiteBishop][toSquare];
                }
            }
        }
        pieces = position.whiteRooks;
        for(move_bits<false> remaining(pieces); remaining; ) {
            fromSquare = remaining.pop();
            targets = petir_bitboard_mask::horizontalAttacks[fromSquare][(pas::shiftRight(position.allPieces, pas::add((fromSquare & 56), 1)) & 63)];
            targets = (targets | petir_bitboard_mask::verticalAttacks[fromSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[fromSquare]) & 63)]);
            answerSquares = (targets & checkerBit);
            if ((answerSquares != 0)) {
                toSquare = petir_bitboard_mask::firstBit(pas::address(answerSquares));
                if ((!petir_tools::exposesWhiteKing(position, pas::add(fromSquare, pas::shiftLeft(toSquare, 7))))) {
                    (moveCount = pas::increment(moveCount, 1));
                    moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
                    moveList[moveCount].score = pas::shiftLeft(pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::rookOrderValue), 15);
                    if ((moveList[moveCount].score < 0)) {
                        moveList[moveCount].score = pas::shiftLeft(petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, toSquare), 15);
                    }
                }
            }
            answerSquares = (targets & blockingSquares);
            for(move_bits<false> remaining(answerSquares); remaining; ) {
                toSquare = remaining.pop();
                if ((!petir_tools::exposesWhiteKing(position, pas::add(fromSquare, pas::shiftLeft(toSquare, 7))))) {
                    (moveCount = pas::increment(moveCount, 1));
                    moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
                    moveList[moveCount].score = petir_header::whiteHistory[petir_header::whiteRook][toSquare];
                }
            }
        }
        pieces = position.whiteQueens;
        for(move_bits<false> remaining(pieces); remaining; ) {
            fromSquare = remaining.pop();
            targets = petir_bitboard_mask::horizontalAttacks[fromSquare][(pas::shiftRight(position.allPieces, pas::add((fromSquare & 56), 1)) & 63)];
            targets = (targets | petir_bitboard_mask::verticalAttacks[fromSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[fromSquare]) & 63)]);
            targets = (targets | petir_bitboard_mask::diagonalA8H1Attacks[fromSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[fromSquare]) & 255)]);
            targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[fromSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[fromSquare]) & 255)]);
            answerSquares = (targets & checkerBit);
            if ((answerSquares != 0)) {
                toSquare = petir_bitboard_mask::firstBit(pas::address(answerSquares));
                if ((!petir_tools::exposesWhiteKing(position, pas::add(fromSquare, pas::shiftLeft(toSquare, 7))))) {
                    (moveCount = pas::increment(moveCount, 1));
                    moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
                    moveList[moveCount].score = pas::shiftLeft(pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::queenOrderValue), 15);
                    if ((moveList[moveCount].score < 0)) {
                        moveList[moveCount].score = pas::shiftLeft(petir_usee::staticExchange(petir_header::whiteSide, position, fromSquare, toSquare), 15);
                    }
                }
            }
            answerSquares = (targets & blockingSquares);
            for(move_bits<false> remaining(answerSquares); remaining; ) {
                toSquare = remaining.pop();
                if ((!petir_tools::exposesWhiteKing(position, pas::add(fromSquare, pas::shiftLeft(toSquare, 7))))) {
                    (moveCount = pas::increment(moveCount, 1));
                    moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
                    moveList[moveCount].score = petir_header::whiteHistory[petir_header::whiteQueen][toSquare];
                }
            }
        }
    }
    moveCountOut = moveCount;
}

// movgen.pas:361
void generateBlackEvasions(petir_header::tdata& position, petir_header::tmovelist& moveList, std::int32_t& moveCountOut, std::int32_t& checkerCount) {
    // Count locally: MSVC would otherwise reload the caller's count after
    // every move-list store. The count is published once on return.
    std::int32_t moveCount = moveCountOut;
    std::uint64_t pieces{};
    std::uint64_t targets{};
    std::uint64_t enemyAttacks{};
    std::uint64_t answerSquares{};
    std::int32_t fromSquare{};
    std::int32_t toSquare{};
    std::int32_t kingSquare{};
    std::uint64_t checkerBit{};
    std::int32_t checkerSquare{};
    std::int32_t promotionType{};
    std::uint64_t blockingSquares{};
    std::uint64_t occupancyWithoutKing{};
    std::uint64_t rotated90WithoutKing{};
    std::uint64_t rotatedA8H1WithoutKing{};
    std::uint64_t rotatedH8A1WithoutKing{};
    std::int32_t lastCapturePromotionType{};
    std::int32_t lastPushPromotionType{};
    moveCount = 0;
    checkerCount = 0;
    enemyAttacks = 0;
    kingSquare = petir_bitboard_mask::kingSquare(position.blackKingBoard);
    checkerSquare = 64;
    pieces = position.whiteKnights;
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        enemyAttacks = (enemyAttacks | petir_bitboard_mask::knightAttacks[fromSquare]);
        if (((petir_bitboard_mask::knightAttacks[fromSquare] & position.blackKingBoard) != 0)) {
            (checkerCount = pas::increment(checkerCount, 1));
            checkerSquare = fromSquare;
        }
    }
    pieces = position.whitePawns;
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::whitePawnAttacks[fromSquare];
        enemyAttacks = (enemyAttacks | targets);
        if (((targets & position.blackKingBoard) != 0)) {
            (checkerCount = pas::increment(checkerCount, 1));
            checkerSquare = fromSquare;
        }
    }
    occupancyWithoutKing = (position.allPieces & (~position.blackKingBoard));
    rotated90WithoutKing = (position.allPiecesRotated90 & (~petir_bitboard_mask::rotated90SquareMask[kingSquare]));
    rotatedA8H1WithoutKing = (position.allPiecesA8H1 & (~petir_bitboard_mask::rotatedA8H1SquareMask[kingSquare]));
    rotatedH8A1WithoutKing = (position.allPiecesH8A1 & (~petir_bitboard_mask::rotatedH8A1SquareMask[kingSquare]));
    pieces = position.whiteBishops;
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::diagonalA8H1Attacks[fromSquare][(pas::shiftRight(rotatedA8H1WithoutKing, petir_bitboard_mask::diagonalA8H1Shift[fromSquare]) & 255)];
        targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[fromSquare][(pas::shiftRight(rotatedH8A1WithoutKing, petir_bitboard_mask::diagonalH8A1Shift[fromSquare]) & 255)]);
        enemyAttacks = (enemyAttacks | targets);
        if (((targets & position.blackKingBoard) != 0)) {
            (checkerCount = pas::increment(checkerCount, 1));
            checkerSquare = fromSquare;
        }
    }
    pieces = position.whiteRooks;
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::horizontalAttacks[fromSquare][(pas::shiftRight(occupancyWithoutKing, pas::add((fromSquare & 56), 1)) & 63)];
        targets = (targets | petir_bitboard_mask::verticalAttacks[fromSquare][(pas::shiftRight(rotated90WithoutKing, petir_bitboard_mask::verticalShift[fromSquare]) & 63)]);
        enemyAttacks = (enemyAttacks | targets);
        if (((targets & position.blackKingBoard) != 0)) {
            (checkerCount = pas::increment(checkerCount, 1));
            checkerSquare = fromSquare;
        }
    }
    pieces = position.whiteQueens;
    for(move_bits<false> remaining(pieces); remaining; ) {
        fromSquare = remaining.pop();
        targets = petir_bitboard_mask::horizontalAttacks[fromSquare][(pas::shiftRight(occupancyWithoutKing, pas::add((fromSquare & 56), 1)) & 63)];
        targets = (targets | petir_bitboard_mask::verticalAttacks[fromSquare][(pas::shiftRight(rotated90WithoutKing, petir_bitboard_mask::verticalShift[fromSquare]) & 63)]);
        targets = (targets | petir_bitboard_mask::diagonalA8H1Attacks[fromSquare][(pas::shiftRight(rotatedA8H1WithoutKing, petir_bitboard_mask::diagonalA8H1Shift[fromSquare]) & 255)]);
        targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[fromSquare][(pas::shiftRight(rotatedH8A1WithoutKing, petir_bitboard_mask::diagonalH8A1Shift[fromSquare]) & 255)]);
        enemyAttacks = (enemyAttacks | targets);
        if (((targets & position.blackKingBoard) != 0)) {
            (checkerCount = pas::increment(checkerCount, 1));
            checkerSquare = fromSquare;
        }
    }
    fromSquare = petir_bitboard_mask::kingSquare(position.whiteKingBoard);
    enemyAttacks = (enemyAttacks | petir_bitboard_mask::kingAttacks[fromSquare]);
    pieces = ((petir_bitboard_mask::kingAttacks[kingSquare] & (~enemyAttacks)) & (~position.blackPieces));
    for(move_bits<false> remaining(pieces); remaining; ) {
        toSquare = remaining.pop();
        (moveCount = pas::increment(moveCount, 1));
        moveList[moveCount].move = pas::add(kingSquare, pas::shiftLeft(toSquare, 7));
        if ((position.board[toSquare] == 0)) {
            moveList[moveCount].score = petir_header::blackHistory[petir_header::blackKing][toSquare];
        } else {
            moveList[moveCount].score = pas::shiftLeft(petir_movgen::pieceOrderValue[position.board[toSquare]], 15);
        }
    }
    appendLegalEnPassantEvasions(position,moveList,moveCount,petir_header::blackSide);
    if ((checkerCount == 1)) {
        checkerBit = petir_bitboard_mask::squareBit(checkerSquare);
        blockingSquares = petir_bitboard_mask::betweenMask[kingSquare][checkerSquare];
        pieces = position.blackKnights;
        for(move_bits<true> remaining(pieces); remaining; ) {
            fromSquare = remaining.pop();
            targets = petir_bitboard_mask::knightAttacks[fromSquare];
            answerSquares = (targets & checkerBit);
            if ((answerSquares != 0)) {
                toSquare = petir_bitboard_mask::lastBit(pas::address(answerSquares));
                if ((!petir_tools::exposesBlackKing(position, pas::add(fromSquare, pas::shiftLeft(toSquare, 7))))) {
                    (moveCount = pas::increment(moveCount, 1));
                    moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
                    moveList[moveCount].score = pas::shiftLeft(pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::knightOrderValue), 15);
                    if ((moveList[moveCount].score < 0)) {
                        moveList[moveCount].score = pas::shiftLeft(petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, toSquare), 15);
                    }
                }
            }
            answerSquares = (targets & blockingSquares);
            for(move_bits<true> remaining(answerSquares); remaining; ) {
                toSquare = remaining.pop();
                if ((!petir_tools::exposesBlackKing(position, pas::add(fromSquare, pas::shiftLeft(toSquare, 7))))) {
                    (moveCount = pas::increment(moveCount, 1));
                    moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
                    moveList[moveCount].score = petir_header::blackHistory[petir_header::blackKnight][toSquare];
                }
            }
        }
        pieces = position.blackPawns;
        for(move_bits<true> remaining(pieces); remaining; ) {
            fromSquare = remaining.pop();
            targets = petir_bitboard_mask::blackPawnAttacks[fromSquare];
            answerSquares = (targets & checkerBit);
            if ((answerSquares != 0)) {
                toSquare = petir_bitboard_mask::lastBit(pas::address(answerSquares));
                if ((!petir_tools::exposesBlackKing(position, pas::add(fromSquare, pas::shiftLeft(toSquare, 7))))) {
                    if ((pas::shiftRight(fromSquare, 3) == 1)) {
                        lastCapturePromotionType = petir_header::knightPromotion;
                        for (promotionType = petir_header::queenPromotion; promotionType <= lastCapturePromotionType; ++promotionType) {
                            (moveCount = pas::increment(moveCount, 1));
                            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(toSquare, 7)), pas::shiftLeft(promotionType, 14));
                            moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::pawnOrderValue);
                        }
                    } else {
                        (moveCount = pas::increment(moveCount, 1));
                        moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
                        moveList[moveCount].score = pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::pawnOrderValue);
                    }
                }
            }
            answerSquares = 0;
            if ((position.board[pas::sub(fromSquare, 8)] == 0)) {
                answerSquares = (answerSquares | petir_bitboard_mask::squareBit(pas::sub(fromSquare, 8)));
                if (((pas::shiftRight(fromSquare, 3) == 6) && (position.board[pas::sub(fromSquare, 16)] == 0))) {
                    answerSquares = (answerSquares | petir_bitboard_mask::squareBit(pas::sub(fromSquare, 16)));
                }
            }
            answerSquares = (answerSquares & blockingSquares);
            if ((answerSquares != 0)) {
                toSquare = petir_bitboard_mask::lastBit(pas::address(answerSquares));
                if ((!petir_tools::exposesBlackKing(position, pas::add(fromSquare, pas::shiftLeft(toSquare, 7))))) {
                    if ((pas::shiftRight(fromSquare, 3) == 1)) {
                        lastPushPromotionType = petir_header::knightPromotion;
                        for (promotionType = petir_header::queenPromotion; promotionType <= lastPushPromotionType; ++promotionType) {
                            (moveCount = pas::increment(moveCount, 1));
                            moveList[moveCount].move = pas::add(pas::add(fromSquare, pas::shiftLeft(toSquare, 7)), pas::shiftLeft(promotionType, 14));
                            moveList[moveCount].score = pas::shiftLeft(petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, toSquare), 15);
                            if ((moveList[moveCount].score == 0)) {
                                moveList[moveCount].score = pas::shiftLeft(3, 15);
                            }
                        }
                    } else {
                        (moveCount = pas::increment(moveCount, 1));
                        moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
                        moveList[moveCount].score = petir_header::blackHistory[petir_header::blackPawn][toSquare];
                    }
                }
            }
        }
        pieces = position.blackBishops;
        for(move_bits<true> remaining(pieces); remaining; ) {
            fromSquare = remaining.pop();
            targets = petir_bitboard_mask::diagonalA8H1Attacks[fromSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[fromSquare]) & 255)];
            targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[fromSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[fromSquare]) & 255)]);
            answerSquares = (targets & checkerBit);
            if ((answerSquares != 0)) {
                toSquare = petir_bitboard_mask::lastBit(pas::address(answerSquares));
                if ((!petir_tools::exposesBlackKing(position, pas::add(fromSquare, pas::shiftLeft(toSquare, 7))))) {
                    (moveCount = pas::increment(moveCount, 1));
                    moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
                    moveList[moveCount].score = pas::shiftLeft(pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::bishopOrderValue), 15);
                    if ((moveList[moveCount].score < 0)) {
                        moveList[moveCount].score = pas::shiftLeft(petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, toSquare), 15);
                    }
                }
            }
            answerSquares = (targets & blockingSquares);
            for(move_bits<true> remaining(answerSquares); remaining; ) {
                toSquare = remaining.pop();
                if ((!petir_tools::exposesBlackKing(position, pas::add(fromSquare, pas::shiftLeft(toSquare, 7))))) {
                    (moveCount = pas::increment(moveCount, 1));
                    moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
                    moveList[moveCount].score = petir_header::blackHistory[petir_header::blackBishop][toSquare];
                }
            }
        }
        pieces = position.blackRooks;
        for(move_bits<true> remaining(pieces); remaining; ) {
            fromSquare = remaining.pop();
            targets = petir_bitboard_mask::horizontalAttacks[fromSquare][(pas::shiftRight(position.allPieces, pas::add((fromSquare & 56), 1)) & 63)];
            targets = (targets | petir_bitboard_mask::verticalAttacks[fromSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[fromSquare]) & 63)]);
            answerSquares = (targets & checkerBit);
            if ((answerSquares != 0)) {
                toSquare = petir_bitboard_mask::lastBit(pas::address(answerSquares));
                if ((!petir_tools::exposesBlackKing(position, pas::add(fromSquare, pas::shiftLeft(toSquare, 7))))) {
                    (moveCount = pas::increment(moveCount, 1));
                    moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
                    moveList[moveCount].score = pas::shiftLeft(pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::rookOrderValue), 15);
                    if ((moveList[moveCount].score < 0)) {
                        moveList[moveCount].score = pas::shiftLeft(petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, toSquare), 15);
                    }
                }
            }
            answerSquares = (targets & blockingSquares);
            for(move_bits<true> remaining(answerSquares); remaining; ) {
                toSquare = remaining.pop();
                if ((!petir_tools::exposesBlackKing(position, pas::add(fromSquare, pas::shiftLeft(toSquare, 7))))) {
                    (moveCount = pas::increment(moveCount, 1));
                    moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
                    moveList[moveCount].score = petir_header::blackHistory[petir_header::blackRook][toSquare];
                }
            }
        }
        pieces = position.blackQueens;
        for(move_bits<true> remaining(pieces); remaining; ) {
            fromSquare = remaining.pop();
            targets = petir_bitboard_mask::horizontalAttacks[fromSquare][(pas::shiftRight(position.allPieces, pas::add((fromSquare & 56), 1)) & 63)];
            targets = (targets | petir_bitboard_mask::verticalAttacks[fromSquare][(pas::shiftRight(position.allPiecesRotated90, petir_bitboard_mask::verticalShift[fromSquare]) & 63)]);
            targets = (targets | petir_bitboard_mask::diagonalA8H1Attacks[fromSquare][(pas::shiftRight(position.allPiecesA8H1, petir_bitboard_mask::diagonalA8H1Shift[fromSquare]) & 255)]);
            targets = (targets | petir_bitboard_mask::diagonalH8A1Attacks[fromSquare][(pas::shiftRight(position.allPiecesH8A1, petir_bitboard_mask::diagonalH8A1Shift[fromSquare]) & 255)]);
            answerSquares = (targets & checkerBit);
            if ((answerSquares != 0)) {
                toSquare = petir_bitboard_mask::lastBit(pas::address(answerSquares));
                if ((!petir_tools::exposesBlackKing(position, pas::add(fromSquare, pas::shiftLeft(toSquare, 7))))) {
                    (moveCount = pas::increment(moveCount, 1));
                    moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
                    moveList[moveCount].score = pas::shiftLeft(pas::sub(petir_movgen::pieceOrderValue[position.board[toSquare]], petir_movgen::queenOrderValue), 15);
                    if ((moveList[moveCount].score < 0)) {
                        moveList[moveCount].score = pas::shiftLeft(petir_usee::staticExchange(petir_header::blackSide, position, fromSquare, toSquare), 15);
                    }
                }
            }
            answerSquares = (targets & blockingSquares);
            for(move_bits<true> remaining(answerSquares); remaining; ) {
                toSquare = remaining.pop();
                if ((!petir_tools::exposesBlackKing(position, pas::add(fromSquare, pas::shiftLeft(toSquare, 7))))) {
                    (moveCount = pas::increment(moveCount, 1));
                    moveList[moveCount].move = pas::add(fromSquare, pas::shiftLeft(toSquare, 7));
                    moveList[moveCount].score = petir_header::blackHistory[petir_header::blackQueen][toSquare];
                }
            }
        }
    }
    moveCountOut = moveCount;
}

}
