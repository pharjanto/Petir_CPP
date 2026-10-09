#pragma once
#include "petir.hpp"

namespace petir {
using Position=petir_header::tdata;
inline constexpr const char* start_fen="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
void initialize(const std::filesystem::path& books);
void new_game();
Position from_fen(const std::string& fen,int& side,int* fullmove=nullptr);
std::string to_fen(const Position& position,int side,int fullmove);
bool insufficient_material(const Position& position);
std::vector<int> legal_moves(Position position,int side);
void make_move(Position& position,int side,int move);
std::string move_text(int move,const Position& position,int side);
int parse_move(const std::string& text,Position position,int side);
std::uint64_t perft(Position position,int side,int depth);
// Reset search state; callers start the millisecond TimeManager afterwards.
void prepare_search(int side,bool timed,int ticks);
int original_time_budget(int side); // Legacy formula retained for provenance tests.
bool consistent(const Position& position);
int self_test();
}
