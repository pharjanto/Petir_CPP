#include "engine.hpp"
#include "kpk.hpp"
#include <algorithm>
#include <sstream>

namespace petir {
using namespace petir_header;
void initialize(const std::filesystem::path& booksPath) {
    static_assert(sizeof(void*)==8,"Build Petir for a 64-bit target");
    static_assert(sizeof(petir_opening_book::tbook)==10);
    static_assert(sizeof(petir_hashing_header::thashrecord)==16);
    if constexpr(std::endian::native!=std::endian::little)throw std::runtime_error("Petir's legacy books require little endian");
    timeFactor=4;
    nullMoveEnabled=lazyEvalEnabled=futilityPruningEnabled=quiescencePruningEnabled=true;
    petir_bitboard::initMasks();
    petir_hashing_header::initHashKeys();
    petir_hashing_header::allocateHashTable();
    petir_evalmask::initEvalMasks();
    petir_usee::initXray();
    petir_kpk::initialize();
    pas::mainForm.defaultDirectory=booksPath.string();
    petir_opening_book::clearBook();
    petir_opening_book::loadBook();
    pas::randomSeed=static_cast<std::uint32_t>(pas::tickCount()); // Original Randomize after hash initialization.
    newGame();
}
void newGame() {
    petir_udata::setInitialPosition(gamePosition);
    currentSide=whiteSide;
    movesOutOfBook=resignCounter=drawCounter=gamePlyCount=0;
    pas::fillChar(whiteHistory,sizeof(whiteHistory),0);
    pas::fillChar(blackHistory,sizeof(blackHistory),0);
    pas::fillChar(continuationHistory,sizeof(continuationHistory),0);
    petir_tools::clearKillers();
    petir_hashing_header::clearHashTable();
    petir_repetition::clearRepetitionTable();
}
Position fromFen(const std::string& fen,int& sideToMove,int* fullMoveNumber) {
    std::istringstream input(fen);
    std::string boardText,sideText,castlingText,enPassantText;
    int halfMoveClock=0,fullMoveCount=1;
    if(!(input>>boardText>>sideText>>castlingText>>enPassantText))throw std::invalid_argument("FEN needs board, side, castling, and en passant fields");
    if(input>>halfMoveClock) {if(!(input>>fullMoveCount))throw std::invalid_argument("FEN missing fullmove number");}
    if(halfMoveClock<0 || fullMoveCount<1 || (sideText!="w" && sideText!="b"))throw std::invalid_argument("Invalid FEN counters or side");
    Position position{};int rank=7,file=0,kingCounts[2]={};
    const std::string pieceLetters="PNBRQK";
    for(char character:boardText) {
        if(character=='/') {if(file!=8 || rank==0)throw std::invalid_argument("Invalid FEN rank");--rank;file=0;continue;}
        if(character>='1' && character<='8') {file+=character-'0';if(file>8)throw std::invalid_argument("FEN rank too long");continue;}
        auto letterIndex=pieceLetters.find(static_cast<char>(std::toupper(static_cast<unsigned char>(character))));
        if(letterIndex==std::string::npos || file>=8)throw std::invalid_argument("Invalid FEN piece");
        int piece=static_cast<int>(letterIndex)+1;
        if(piece==1 && (rank==0 || rank==7))throw std::invalid_argument("FEN pawn on promotion rank");
        if(piece==6)++kingCounts[std::islower(static_cast<unsigned char>(character))?1:0];
        position.board[rank*8+file++]=static_cast<std::int8_t>(std::islower(static_cast<unsigned char>(character))?-piece:piece);
    }
    if(rank!=0 || file!=8 || kingCounts[0]!=1 || kingCounts[1]!=1)throw std::invalid_argument("FEN needs eight ranks and one king of each color");
    if(castlingText!="-")for(char character:castlingText) {
        switch(character) {case 'K':position.castleRights|=1;break;case 'Q':position.castleRights|=2;break;case 'k':position.castleRights|=4;break;case 'q':position.castleRights|=8;break;default:throw std::invalid_argument("Invalid FEN castling rights");}
    }
    sideToMove=sideText=="w"?1:2;
    petir_customboard::rebuildBitboards(position);
    if(enPassantText!="-") {
        if(enPassantText.size()!=2 || enPassantText[0]<'a' || enPassantText[0]>'h' || enPassantText[1]!=(sideToMove==1?'6':'3'))throw std::invalid_argument("Invalid FEN en passant square");
        position.enPassantFile=static_cast<std::uint8_t>(enPassantText[0]-'a');
    }
    position.fiftyMoveCounter=static_cast<std::uint8_t>(std::min(halfMoveClock,255));
    position.hashKey=petir_hashing_header::computeHashKey(position,sideToMove);
    if(fullMoveNumber)*fullMoveNumber=fullMoveCount;
    return position;
}
std::string toFen(const Position& position,int sideToMove,int fullMoveNumber) {
    std::string result;
    for(int rank=7;rank>=0;--rank) {
        int emptyCount=0;
        for(int file=0;file<8;++file) {
            const int piece=position.board[rank*8+file];
            if(!piece) {++emptyCount;continue;}
            if(emptyCount) {result+=char('0'+emptyCount);emptyCount=0;}
            const char letter=" PNBRQK"[std::abs(piece)];
            result+=piece>0?letter:static_cast<char>(std::tolower(static_cast<unsigned char>(letter)));
        }
        if(emptyCount)result+=char('0'+emptyCount);
        if(rank)result+='/';
    }
    result+=sideToMove==1?" w ":" b ";
    std::string castlingText;
    for(int rightIndex=0;rightIndex<4;++rightIndex)if(position.castleRights&(1<<rightIndex))castlingText+="KQkq"[rightIndex];
    result+=castlingText.empty()?"-":castlingText;
    result+=' ';
    if(position.enPassantFile==noEnPassant)result+='-';
    else {result+=char('a'+position.enPassantFile);result+=sideToMove==1?'6':'3';}
    return result+" "+std::to_string(position.fiftyMoveCounter)+" "+std::to_string(fullMoveNumber);
}
bool insufficientMaterial(const Position& position) {
    if(position.whitePawns || position.blackPawns || position.whiteRooks || position.blackRooks || position.whiteQueens || position.blackQueens)return false;
    const auto knights=position.whiteKnights|position.blackKnights,bishops=position.whiteBishops|position.blackBishops;
    if(std::popcount(knights|bishops)<=1)return true;
    if(knights)return false;
    // Any number of bishops on one square color cannot deliver mate.
    constexpr std::uint64_t darkSquares=UINT64_C(0xaa55aa55aa55aa55);
    return !(bishops&darkSquares) || !(bishops&~darkSquares);
}
void makeMove(Position& position,int sideToMove,int move) {
    if(sideToMove==1)petir_makemove::makeWhiteMove(move,position);else petir_makemove::makeBlackMove(move,position);
}
std::string moveText(int move,const Position& position,int sideToMove) {
    if(move==0)return "0000";
    return petir_notation::moveToNotation(move,position.enPassantFile,sideToMove).text;
}
std::vector<int> legalMoves(Position position,int sideToMove) {
    std::vector<int> result;tmovelist moveList{};int moveCount=0,checkerCount=0;
    const bool inCheck=sideToMove==1?petir_tools::whiteInCheck(position):petir_tools::blackInCheck(position);
    auto appendLegal=[&] {
        for(int index=1;index<=moveCount;++index) {
            Position childPosition=position;makeMove(childPosition,sideToMove,moveList[index].move);
            if(!(sideToMove==1?petir_tools::whiteInCheck(childPosition):petir_tools::blackInCheck(childPosition)))result.push_back(moveList[index].move);
        }
    };
    if(inCheck) {if(sideToMove==1)petir_movgen::generateWhiteEvasions(position,moveList,moveCount,checkerCount);else petir_movgen::generateBlackEvasions(position,moveList,moveCount,checkerCount);appendLegal();}
    else {
        if(sideToMove==1)petir_movgen::generateWhiteCaptures(position,moveList,moveCount);else petir_movgen::generateBlackCaptures(position,moveList,moveCount);appendLegal();
        if(sideToMove==1)petir_movgen::generateWhiteQuietMoves(position,moveList,moveCount);else petir_movgen::generateBlackQuietMoves(position,moveList,moveCount);appendLegal();
    }
    return result;
}
int parseMove(const std::string& text,Position position,int sideToMove) {
    if((text.size()!=4 && text.size()!=5) || text[0]<'a' || text[0]>'h' || text[2]<'a' || text[2]>'h' || text[1]<'1' || text[1]>'8' || text[3]<'1' || text[3]>'8')return 0;
    for(int move:legalMoves(position,sideToMove)) {
        const std::string candidate=moveText(move,position,sideToMove);
        if(text==candidate)return move;
        // The original maker accepts all four promotions, although its generator
        // only searches queen and knight. Accept opponent bishop/rook promotions.
        if(text.size()==5 && candidate.size()==5 && text.substr(0,4)==candidate.substr(0,4)) {
            int promotionCode=text[4]=='q'?1:text[4]=='n'?2:text[4]=='b'?3:text[4]=='r'?4:0;
            if(promotionCode)return (move&0x3fff)|(promotionCode<<14);
        }
    }
    return 0;
}
std::uint64_t perft(Position position,int sideToMove,int depth) {
    if(depth==0)return 1;
    std::uint64_t nodes=0;tmovelist moveList{};int moveCount=0,checkerCount=0;
    bool inCheck=sideToMove==1?petir_tools::whiteInCheck(position):petir_tools::blackInCheck(position);
    auto visitMoves=[&](bool checkLegality) {
        for(int index=1;index<=moveCount;++index) {
            Position childPosition=position;makeMove(childPosition,sideToMove,moveList[index].move);
            if(checkLegality && (sideToMove==1?petir_tools::whiteInCheck(childPosition):petir_tools::blackInCheck(childPosition)))continue;
            nodes+=perft(childPosition,3-sideToMove,depth-1);
        }
    };
    // Same generator selection and legality checks as original/perft_test.pas.
    if(inCheck) {if(sideToMove==1)petir_movgen::generateWhiteEvasions(position,moveList,moveCount,checkerCount);else petir_movgen::generateBlackEvasions(position,moveList,moveCount,checkerCount);visitMoves(false);}
    else {
        if(sideToMove==1)petir_movgen::generateWhiteCaptures(position,moveList,moveCount);else petir_movgen::generateBlackCaptures(position,moveList,moveCount);visitMoves(true);
        if(sideToMove==1)petir_movgen::generateWhiteQuietMoves(position,moveList,moveCount);else petir_movgen::generateBlackQuietMoves(position,moveList,moveCount);visitMoves(true);
    }
    return nodes;
}
void prepareSearch(int sideToMove,bool timed,int timeLimitTicks) {
    searchTime.reset();
    evalTimeMs=hashStoreCount=hashCollisionCount=lazyMarginNodeStamp=maxDepthReached=seeTimeMs=moveGenTimeMs=etcCutoffCount=futilityFailCount=futilityTryCount=nodeCount=maxSearchPly=hashHitCount=0;
    stalemateFound=false;engineSide=sideToMove;timeElapsed=0;stopSearch=false;
    adjustedLazyMargin1=lazyMargin1;adjustedLazyMargin2=lazyMargin2;adjustedLazyMargin3=lazyMargin3;
    testSolved=false;petir_tools::clearKillers();errorCount=0;
    // Retain usable results between moves and age their replacement priority.
    if(ponderMode==ponderOff)petir_hashing_header::advanceHashGeneration();else petir_hashing_header::clearReverseTable();
    cutoffCount=hashCutoffCount=lazyEvalExitCount=futilityPruneCount=extendedFutilityPruneCount=razorCount=firstMoveCutoffCount=0;
    computerThinking=true;
    for(int piece=-6;piece<=6;++piece)for(int square=0;square<=67;++square) {whiteHistory[piece][square]/=256;blackHistory[piece][square]/=256;}
    searchStartTime=pvReferenceTime=pas::tickCount();quiescenceNodeCount=0;useTimer=timed;timeLimit=timed?timeLimitTicks:999999999;
    rootBestMove=0;
    petir_uiterative::completedMove=petir_uiterative::completedDepth=petir_uiterative::completedScore=0;
    petir_uiterative::completedPv.clear();
}
int originalTimeBudget(int sideToMove) {
    const int sideClock=sideToMove==1?whiteClock:blackClock;
    if(timeMode==minutesPerGame || timeMode==minutesPerGameWithIncrement) {
        int result=gamePlyCount<=80?sideClock/(60-gamePlyCount/2)-2:sideClock/30;
        return result==0?1:result;
    }
    if(timeMode==movesInMinutes) {
        const int period=std::max(1,movesPerTimeControl);
        const int playedInPeriod=(gamePlyCount/2)%period;
        return sideClock/(period+2-playedInPeriod);
    }
    if(timeMode==secondsPerMove)return fixedMoveTime;
    return 999999999;
}
bool isConsistent(const Position& position) {
    Position rebuilt=position;petir_customboard::rebuildBitboards(rebuilt);rebuilt.enPassantFile=position.enPassantFile;
    return position.hashKey==petir_hashing_header::computeHashKey(rebuilt,1)
        && position.materialScore==rebuilt.materialScore && position.whiteNonPawnMaterial==rebuilt.whiteNonPawnMaterial
        && position.blackNonPawnMaterial==rebuilt.blackNonPawnMaterial && position.whiteBishopCount==rebuilt.whiteBishopCount && position.blackBishopCount==rebuilt.blackBishopCount
        && position.whitePieces==rebuilt.whitePieces && position.blackPieces==rebuilt.blackPieces && position.allPieces==rebuilt.allPieces
        && position.allPiecesRotated90==rebuilt.allPiecesRotated90 && position.allPiecesA8H1==rebuilt.allPiecesA8H1 && position.allPiecesH8A1==rebuilt.allPiecesH8A1
        && position.whitePawns==rebuilt.whitePawns && position.blackPawns==rebuilt.blackPawns && position.whiteKnights==rebuilt.whiteKnights && position.blackKnights==rebuilt.blackKnights
        && position.whiteBishops==rebuilt.whiteBishops && position.blackBishops==rebuilt.blackBishops && position.whiteRooks==rebuilt.whiteRooks && position.blackRooks==rebuilt.blackRooks
        && position.whiteQueens==rebuilt.whiteQueens && position.blackQueens==rebuilt.blackQueens && position.whiteKingBoard==rebuilt.whiteKingBoard && position.blackKingBoard==rebuilt.blackKingBoard
        && position.bishopsAndQueens==rebuilt.bishopsAndQueens && position.rooksAndQueens==rebuilt.rooksAndQueens;
}
}
