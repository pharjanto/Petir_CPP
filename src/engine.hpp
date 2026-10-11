#pragma once
#include "petir.hpp"

namespace petir {
using Position=petir_header::tdata;
inline constexpr const char* startFen="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
void initialize(const std::filesystem::path& booksPath);
void newGame();
Position fromFen(const std::string& fen,int& sideToMove,int* fullMoveNumber=nullptr);
std::string toFen(const Position& position,int sideToMove,int fullMoveNumber);
bool insufficientMaterial(const Position& position);
std::vector<int> legalMoves(Position position,int sideToMove);
void makeMove(Position& position,int sideToMove,int move);
std::string moveText(int move,const Position& position,int sideToMove);
int parseMove(const std::string& text,Position position,int sideToMove);
std::uint64_t perft(Position position,int sideToMove,int depth);
// Reset search state; callers start the millisecond TimeManager afterwards.
void prepareSearch(int sideToMove,bool timed,int timeLimitTicks);
int originalTimeBudget(int sideToMove); // Legacy formula retained for provenance tests.
bool isConsistent(const Position& position);
int selfTest();
}
