// Generated from the bundled Delphi source; see tools/translate.py.
#pragma once
#include "pascal_runtime.hpp"
#include "time_manager.hpp"
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
#include <intrin.h>
#include <xmmintrin.h>
#include <new>
#endif
namespace petir_header {
inline constexpr auto version = 1;
inline constexpr auto a1 = 0;
inline constexpr auto b1 = 1;
inline constexpr auto c1 = 2;
inline constexpr auto d1 = 3;
inline constexpr auto e1 = 4;
inline constexpr auto f1 = 5;
inline constexpr auto g1 = 6;
inline constexpr auto h1 = 7;
inline constexpr auto a2 = 8;
inline constexpr auto b2 = 9;
inline constexpr auto c2 = 10;
inline constexpr auto d2 = 11;
inline constexpr auto e2 = 12;
inline constexpr auto f2 = 13;
inline constexpr auto g2 = 14;
inline constexpr auto h2 = 15;
inline constexpr auto a3 = 16;
inline constexpr auto b3 = 17;
inline constexpr auto c3 = 18;
inline constexpr auto d3 = 19;
inline constexpr auto e3 = 20;
inline constexpr auto f3 = 21;
inline constexpr auto g3 = 22;
inline constexpr auto h3 = 23;
inline constexpr auto a4 = 24;
inline constexpr auto b4 = 25;
inline constexpr auto c4 = 26;
inline constexpr auto d4 = 27;
inline constexpr auto e4 = 28;
inline constexpr auto f4 = 29;
inline constexpr auto g4 = 30;
inline constexpr auto h4 = 31;
inline constexpr auto a5 = 32;
inline constexpr auto b5 = 33;
inline constexpr auto c5 = 34;
inline constexpr auto d5 = 35;
inline constexpr auto e5 = 36;
inline constexpr auto f5 = 37;
inline constexpr auto g5 = 38;
inline constexpr auto h5 = 39;
inline constexpr auto a6 = 40;
inline constexpr auto b6 = 41;
inline constexpr auto c6 = 42;
inline constexpr auto d6 = 43;
inline constexpr auto e6 = 44;
inline constexpr auto f6 = 45;
inline constexpr auto g6 = 46;
inline constexpr auto h6 = 47;
inline constexpr auto a7 = 48;
inline constexpr auto b7 = 49;
inline constexpr auto c7 = 50;
inline constexpr auto d7 = 51;
inline constexpr auto e7 = 52;
inline constexpr auto f7 = 53;
inline constexpr auto g7 = 54;
inline constexpr auto h7 = 55;
inline constexpr auto a8 = 56;
inline constexpr auto b8 = 57;
inline constexpr auto c8 = 58;
inline constexpr auto d8 = 59;
inline constexpr auto e8 = 60;
inline constexpr auto f8 = 61;
inline constexpr auto g8 = 62;
inline constexpr auto h8 = 63;
inline constexpr auto rank1 = 0;
inline constexpr auto rank2 = 1;
inline constexpr auto rank3 = 2;
inline constexpr auto rank4 = 3;
inline constexpr auto rank5 = 4;
inline constexpr auto rank6 = 5;
inline constexpr auto rank7 = 6;
inline constexpr auto rank8 = 7;
inline constexpr auto _pionputih = 1;
inline constexpr auto _kudaputih = 2;
inline constexpr auto _gajahputih = 3;
inline constexpr auto _bentengputih = 4;
inline constexpr auto _mentriputih = 5;
inline constexpr auto _rajaputih = 6;
inline constexpr auto _pionhitam = pas::negate(1);
inline constexpr auto _kudahitam = pas::negate(2);
inline constexpr auto _gajahhitam = pas::negate(3);
inline constexpr auto _bentenghitam = pas::negate(4);
inline constexpr auto _mentrihitam = pas::negate(5);
inline constexpr auto _rajahitam = pas::negate(6);
inline constexpr auto pv_node = 0;
inline constexpr auto fail_low = 1;
inline constexpr auto fail_high = 2;
inline constexpr auto _sisiputih = 1;
inline constexpr auto _sisihitam = 2;
inline constexpr auto _gameover = 3;
inline constexpr auto _nodemax = 1;
inline constexpr auto _nodemin = 2;
inline constexpr auto _no_ep = 10;
inline constexpr auto _norokade = 64;
inline constexpr auto _rokadependek = 65;
inline constexpr auto _rokadepanjang = 66;
inline constexpr auto _en_passant = 67;
inline constexpr auto _no_move = 0;
inline constexpr auto _twoplayers = 0;
inline constexpr auto _playervscomp = 1;
inline constexpr auto _compvsplayer = 2;
inline constexpr auto _compvscomp = 3;
inline constexpr auto norokade = 0;
inline constexpr auto rokadependek = 1;
inline constexpr auto rokadepanjang = 2;
inline constexpr auto bitputihsc = 1;
inline constexpr auto bitputihlc = 2;
inline constexpr auto bithitamsc = 4;
inline constexpr auto bithitamlc = 8;
inline constexpr auto bitputihnoc = 3;
inline constexpr auto bithitamnoc = 12;
inline constexpr auto result_white_win = 0;
inline constexpr auto result_draw = 1;
inline constexpr auto result_black_win = 2;
inline constexpr auto promosi_mentri = 1;
inline constexpr auto promosi_kuda = 2;
inline constexpr auto promosi_gajah = 3;
inline constexpr auto promosi_benteng = 4;
inline pas::Array<std::uint16_t, 0, 12> bit8mask = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
// bit8mask[i] as the int it promotes to; checked builds keep the table's range check.
inline std::int32_t bit8(std::int64_t i) {
#ifdef PETIR_CHECK_BOUNDS
    (void)bit8mask[i];
#endif
    return std::int32_t(1) << i;
}
inline constexpr auto level_nm = 3;
inline constexpr auto _hashmove = 0;
inline constexpr auto _goodcaps = 1;
inline constexpr auto _killermoves1 = 2;
inline constexpr auto _killermoves2 = 3;
inline constexpr auto _killermoves3 = 4;
inline constexpr auto _killermoves4 = 5;
inline constexpr auto _othermoves = 6;
inline constexpr auto _badcaps = 7;
inline constexpr auto _finish = 8;
inline constexpr auto minutes_game = 0;
inline constexpr auto seconds_move = 1;
inline constexpr auto moves_minutes = 2;
inline constexpr auto minutes_game_increment = 3;
inline constexpr auto fixed_depth = 4;
inline constexpr auto ponder_off = 0;
inline constexpr auto ponder_fill_hash = 1;
inline constexpr auto ponder_guess = 2;
inline constexpr auto ponder_combine = 3;
inline constexpr auto _nilai_pion = 90;
inline constexpr auto _nilai_gajah = 319;
inline constexpr auto _nilai_kuda = 312;
inline constexpr auto _nilai_benteng = 500;
inline constexpr auto _nilai_mentri = 975;
inline constexpr auto _nilai_raja = 20000;
inline pas::Array<std::uint16_t, pas::negate(6), 6> nilai_piece = {petir_header::_nilai_raja, petir_header::_nilai_mentri, petir_header::_nilai_benteng, petir_header::_nilai_gajah, petir_header::_nilai_kuda, petir_header::_nilai_pion, 0, petir_header::_nilai_pion, petir_header::_nilai_kuda, petir_header::_nilai_gajah, petir_header::_nilai_benteng, petir_header::_nilai_mentri, petir_header::_nilai_raja};
inline constexpr auto _infinity = 20000;
inline constexpr auto hist_size = 65536;
using tkillermove = std::int32_t;
struct tmoverecord {
    std::int32_t moves;
    std::int32_t score;
};
using pmoverecord = petir_header::tmoverecord*;
// Move generation can exceed the legacy 100-entry limit after promotions.
// Keep this capacity guard even when general Pascal bounds checks are off.
// The throw lives out of line so each access is one compare and branch.
#if defined(_MSC_VER)
[[noreturn]] __declspec(noinline) void move_list_index_error(std::int64_t i);
#else
[[noreturn]] __attribute__((noinline)) void move_list_index_error(std::int64_t i);
#endif
struct tmovelist : pas::Array<petir_header::tmoverecord, 1, 256> {
    tmoverecord& operator[](std::int64_t i) {
        if(static_cast<std::uint64_t>(i-1)>=256) [[unlikely]] move_list_index_error(i);
        return values[i-1];
    }
    const tmoverecord& operator[](std::int64_t i) const {
        if(static_cast<std::uint64_t>(i-1)>=256) [[unlikely]] move_list_index_error(i);
        return values[i-1];
    }
};
using tarr = pas::Array<std::int8_t, 0, 63>;
struct tdata {
    petir_header::tarr papan;
    std::uint8_t ep;
    std::uint8_t flagrokade;
    std::uint8_t wc;
    std::uint8_t bc;
    std::uint8_t move50count;
    std::uint8_t cp;
    std::uint8_t nilai_perwira_putih;
    std::uint8_t nilai_perwira_hitam;
    std::uint8_t ngajahhitam;
    std::uint8_t ngajahputih;
    bool pawnending;
    std::int32_t materialscore;
    std::uint64_t hashkey;
    std::uint64_t whitepieces;
    std::uint64_t blackpieces;
    std::uint64_t allpieces;
    std::uint64_t allpiecesr90;
    std::uint64_t allpiecesa8h1;
    std::uint64_t allpiecesh8a1;
    std::uint64_t pawnblack;
    std::uint64_t pawnwhite;
    std::uint64_t bishopblack;
    std::uint64_t bishopwhite;
    std::uint64_t knightblack;
    std::uint64_t knightwhite;
    std::uint64_t rookblack;
    std::uint64_t rookwhite;
    std::uint64_t queenblack;
    std::uint64_t queenwhite;
    std::uint64_t kingblack;
    std::uint64_t kingwhite;
    std::uint64_t rookqueen;
    std::uint64_t bishopqueen;
};
extern std::int32_t totaladdhash;
extern std::int32_t collision;
extern std::int32_t futfail;
extern std::int32_t futtry;
extern std::int32_t tx;
extern std::int32_t evalms;
extern std::int32_t evalsee;
extern std::int32_t movgenms;
extern bool futpruningok;
extern bool lazyok;
extern bool qpruningok;
extern bool nullok;
extern bool etcok;
extern std::int32_t lazyts;
extern std::int32_t pawneval;
extern std::int32_t knighteval;
extern std::int32_t bishopeval;
extern std::int32_t rookeval;
extern std::int32_t queeneval;
extern std::int32_t kseval;
extern std::int32_t kpeval;
extern std::int32_t tropismeval;
extern std::int32_t q_margin;
extern std::int32_t futil_margin;
extern std::int32_t thread_priority;
extern std::int32_t maxhash;
extern std::int32_t resign_value;
extern std::int32_t resign_count;
extern std::int32_t draw_count;
extern std::int32_t total_node;
extern std::int32_t qnode;
extern bool stalemate;
extern std::int32_t players;
extern std::int32_t ai_side;
extern std::int32_t toplevel;
extern bool use_mtdf;
// Quiet-move history stays within +/-history_max (see petir_search::update_history).
inline constexpr std::int32_t history_max = 16384;
extern pas::Array<pas::Array<std::int32_t, 0, 67>, pas::negate(6), 6> historyw;
extern pas::Array<pas::Array<std::int32_t, 0, 67>, pas::negate(6), 6> historyb;
// Continuation history: how a quiet move [piece][to] did right after an
// earlier move, keyed by that move's [piece][to]; the piece's sign gives the
// side that made it. Entries stay within +/-history_max.
using piece_to_history = pas::Array<pas::Array<std::int16_t, 0, 67>, pas::negate(6), 6>;
extern pas::Array<pas::Array<piece_to_history, 0, 67>, pas::negate(6), 6> conthist;
extern pas::Array<petir_header::tkillermove, pas::negate(2), 63> killer0;
extern pas::Array<petir_header::tkillermove, pas::negate(2), 63> killer1;
extern std::int32_t co_hash;
extern std::int32_t totalco;
extern std::int32_t hashco;
extern std::int32_t lazyexit;
extern std::int32_t fpruning;
extern std::int32_t efpruning;
extern std::int32_t razor;
extern std::int32_t cofirst;
extern pas::StringList sltemp;
extern std::int32_t lazy_margin1;
extern std::int32_t lazy_margin2;
extern std::int32_t lazy_margin3;
extern std::int32_t lazy_margin1a;
extern std::int32_t lazy_margin2a;
extern std::int32_t lazy_margin3a;
extern std::int32_t contemp_draw;
extern std::int32_t hash_hit;
extern std::int32_t cetc;
extern std::int32_t maxdepth1;
extern bool inpondering;
extern std::atomic_bool stop_process;
extern bool thread_killed;
extern std::int32_t maxdepth;
extern std::int32_t outofbook;
extern std::int32_t clockwhite;
extern std::int32_t clockblack;
extern std::int32_t realclock;
extern bool usetimer;
extern std::int32_t timefactor;
extern std::atomic<std::int32_t> timepassed;
extern std::int32_t timelimit;
extern std::int32_t side;
extern std::int32_t mmoves;
extern pas::Array<pas::Array<std::uint64_t, 0, 127>, pas::negate(6), 6> hashkey;
extern pas::Array<pas::Array<std::uint64_t, 0, 127>, pas::negate(6), 6> hk2;
extern pas::Array<std::uint64_t, 0, 255> rokadehashkey;
extern pas::Array<std::uint64_t, 0, 255> rhk2;
extern pas::Array<std::uint64_t, 0, petir_header::_no_ep> enpassanthashkey;
extern pas::Array<std::uint64_t, 0, petir_header::_no_ep> ephk2;
extern std::int32_t startclock;
extern std::int32_t barisitr;
extern double maxext;
extern std::int32_t jumlahlangkah;
extern bool computer_thinking;
extern bool use_book;
extern std::uint8_t ponder_type;
extern std::int32_t amoves;
extern std::int32_t fractional_ply;
extern std::int32_t skak_extension;
extern std::int32_t mate_threat_extension;
extern std::int32_t recapture_pawn_extension;
extern std::int32_t recapture_knight_extension;
extern std::int32_t recapture_rook_extension;
extern std::int32_t recapture_queen_extension;
extern std::int32_t single_reply_extension;
extern std::int32_t two_reply_extension;
extern std::int32_t double_skak_extension;
extern std::int32_t pushed_pawn_extension_midgame;
extern std::int32_t pushed_pawn_extension_endgame;
extern std::int32_t pushed_pawn_extension_late_endgame;
extern petir_header::tdata data;
extern std::int32_t modetime;
extern std::int32_t rmoves;
extern std::int32_t rincrement;
extern bool runningtest;
extern bool testsolved;
extern pas::Array<std::int32_t, 1, 5> testanswer;
extern bool usewinboard;
extern std::int32_t ref2;
extern std::int32_t nerror;
extern std::uint64_t ephashkey;
extern pas::File ftext;
}
namespace petir_bitboard_mask {
using pword = std::uint16_t*;
inline pas::Array<std::uint8_t, 0, 63> h8a1shiftmask = {28, 21, 15, 10, 6, 3, 1, 0, 36, 28, 21, 15, 10, 6, 3, 1, 43, 36, 28, 21, 15, 10, 6, 3, 49, 43, 36, 28, 21, 15, 10, 6, 54, 49, 43, 36, 28, 21, 15, 10, 58, 54, 49, 43, 36, 28, 21, 15, 61, 58, 54, 49, 43, 36, 28, 21, 63, 61, 58, 54, 49, 43, 36, 28};
inline pas::Array<std::uint8_t, 0, 63> a8h1shiftmask = {0, 1, 3, 6, 10, 15, 21, 28, 1, 3, 6, 10, 15, 21, 28, 36, 3, 6, 10, 15, 21, 28, 36, 43, 6, 10, 15, 21, 28, 36, 43, 49, 10, 15, 21, 28, 36, 43, 49, 54, 15, 21, 28, 36, 43, 49, 54, 58, 21, 28, 36, 43, 49, 54, 58, 61, 28, 36, 43, 49, 54, 58, 61, 63};
inline pas::Array<std::uint8_t, 0, 63> vershiftmask = {57, 49, 41, 33, 25, 17, 9, 1, 57, 49, 41, 33, 25, 17, 9, 1, 57, 49, 41, 33, 25, 17, 9, 1, 57, 49, 41, 33, 25, 17, 9, 1, 57, 49, 41, 33, 25, 17, 9, 1, 57, 49, 41, 33, 25, 17, 9, 1, 57, 49, 41, 33, 25, 17, 9, 1, 57, 49, 41, 33, 25, 17, 9, 1};
extern pas::Array<std::uint64_t, 0, 63> bit2nmask;
extern pas::Array<std::uint64_t, 0, 63> bit2nmasknot;
// bit2nmask[sq] and bit2nmasknot[sq] computed with a shift. Unlike table loads,
// these stay in registers across the position stores in move making; checked
// builds keep the tables' range check.
inline std::uint64_t bit(std::int64_t sq) {
#ifdef PETIR_CHECK_BOUNDS
    (void)bit2nmask[sq];
#endif
    return UINT64_C(1) << sq;
}
inline std::uint64_t bitnot(std::int64_t sq) {
#ifdef PETIR_CHECK_BOUNDS
    (void)bit2nmasknot[sq];
#endif
    return ~(UINT64_C(1) << sq);
}
// Square of a king bitboard. A king board holds exactly one bit, so firstbitp
// and lastbitp agree; one forward scan replaces lastbitp's two scans.
inline std::int32_t king_square(std::uint64_t king) {
#ifdef PETIR_CHECK_BOUNDS
    if (!king || (king & (king - 1))) throw std::out_of_range("King bitboard does not hold exactly one king");
#endif
#if defined(_MSC_VER) && defined(_M_X64)
    unsigned long square;
    _BitScanForward64(&square, king);
    return static_cast<std::int32_t>(square);
#else
    return std::countr_zero(king);
#endif
}
extern pas::Array<std::uint64_t, 0, 63> bit2nmask90;
extern pas::Array<std::uint64_t, 0, 63> bit2nmaska8h1;
extern pas::Array<std::uint64_t, 0, 63> bit2nmaskh8a1;
extern pas::Array<std::uint64_t, 0, 63> w_pion_bebas_mask;
extern pas::Array<std::uint64_t, 0, 63> b_pion_bebas_mask;
extern pas::Array<std::uint64_t, 0, 63> knightmask;
extern pas::Array<std::uint64_t, 0, 63> kingmask;
extern std::uint64_t mask_left;
extern std::uint64_t mask_right;
extern pas::Array<pas::Array<std::uint64_t, 0, 63>, 0, 63> horzmask2;
extern pas::Array<pas::Array<std::uint64_t, 0, 63>, 0, 63> vertmask2;
extern pas::Array<pas::Array<std::uint64_t, 0, 255>, 0, 63> diaga8h1mask;
extern pas::Array<pas::Array<std::uint64_t, 0, 255>, 0, 63> diagh8a1mask;
extern pas::Array<std::uint8_t, 0, 65535> firstbitmask;
extern pas::Array<std::uint64_t, 0, 63> w_outpost_mask;
extern pas::Array<std::uint64_t, 0, 63> b_outpost_mask;
extern pas::Array<std::uint64_t, 0, 63> w_pawn_attack;
extern pas::Array<std::uint64_t, 0, 63> b_pawn_attack;
extern pas::Array<std::uint64_t, 0, 7> file_mask;
extern pas::Array<std::uint64_t, 0, 7> rank_mask;
extern pas::Array<pas::Array<std::uint64_t, 0, 63>, 0, 63> pathmask;
extern pas::Array<pas::Array<std::int8_t, 0, 63>, 0, 63> direction;
extern pas::Array<pas::Array<std::int8_t, 0, 63>, 0, 63> direction2;
extern pas::Array<std::uint64_t, 0, 63> rook_attack_up_mask;
extern pas::Array<std::uint64_t, 0, 63> rook_attack_down_mask;
extern pas::Array<std::uint64_t, 0, 63> rook_attack_left_mask;
extern pas::Array<std::uint64_t, 0, 63> rook_attack_right_mask;
extern pas::Array<std::uint64_t, 0, 63> bishop_attack_plus7_mask;
extern pas::Array<std::uint64_t, 0, 63> bishop_attack_plus9_mask;
extern pas::Array<std::uint64_t, 0, 63> bishop_attack_min7_mask;
extern pas::Array<std::uint64_t, 0, 63> bishop_attack_min9_mask;
extern pas::Array<std::uint64_t, 0, 63> bishop_attack_up_mask;
extern pas::Array<std::uint64_t, 0, 63> bishop_attack_down_mask;
}
namespace petir_bitboard {
inline pas::Array<std::uint8_t, 0, 63> r90mask = {56, 48, 40, 32, 24, 16, 8, 0, 57, 49, 41, 33, 25, 17, 9, 1, 58, 50, 42, 34, 26, 18, 10, 2, 59, 51, 43, 35, 27, 19, 11, 3, 60, 52, 44, 36, 28, 20, 12, 4, 61, 53, 45, 37, 29, 21, 13, 5, 62, 54, 46, 38, 30, 22, 14, 6, 63, 55, 47, 39, 31, 23, 15, 7};
inline pas::Array<std::uint8_t, 0, 63> a8h1mask = {0, 1, 3, 6, 10, 15, 21, 28, 2, 4, 7, 11, 16, 22, 29, 36, 5, 8, 12, 17, 23, 30, 37, 43, 9, 13, 18, 24, 31, 38, 44, 49, 14, 19, 25, 32, 39, 45, 50, 54, 20, 26, 33, 40, 46, 51, 55, 58, 27, 34, 41, 47, 52, 56, 59, 61, 35, 42, 48, 53, 57, 60, 62, 63};
inline pas::Array<std::uint8_t, 0, 63> h8a1mask = {35, 27, 20, 14, 9, 5, 2, 0, 42, 34, 26, 19, 13, 8, 4, 1, 48, 41, 33, 25, 18, 12, 7, 3, 53, 47, 40, 32, 24, 17, 11, 6, 57, 52, 46, 39, 31, 23, 16, 10, 60, 56, 51, 45, 38, 30, 22, 15, 62, 59, 55, 50, 44, 37, 29, 21, 63, 61, 58, 54, 49, 43, 36, 28};
inline pas::Array<std::uint8_t, 0, 63> diag_a8h1_length = {1, 2, 3, 4, 5, 6, 7, 8, 2, 3, 4, 5, 6, 7, 8, 7, 3, 4, 5, 6, 7, 8, 7, 6, 4, 5, 6, 7, 8, 7, 6, 5, 5, 6, 7, 8, 7, 6, 5, 4, 6, 7, 8, 7, 6, 5, 4, 3, 7, 8, 7, 6, 5, 4, 3, 2, 8, 7, 6, 5, 4, 3, 2, 1};
inline pas::Array<std::uint8_t, 0, 63> diag_h8a1_length = {8, 7, 6, 5, 4, 3, 2, 1, 7, 8, 7, 6, 5, 4, 3, 2, 6, 7, 8, 7, 6, 5, 4, 3, 5, 6, 7, 8, 7, 6, 5, 4, 4, 5, 6, 7, 8, 7, 6, 5, 3, 4, 5, 6, 7, 8, 7, 6, 2, 3, 4, 5, 6, 7, 8, 7, 1, 2, 3, 4, 5, 6, 7, 8};
inline pas::Array<std::uint8_t, 0, 63> diag_a8h1_posx = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 2, 2, 2, 2, 2, 2, 1, 0, 3, 3, 3, 3, 3, 2, 1, 0, 4, 4, 4, 4, 3, 2, 1, 0, 5, 5, 5, 4, 3, 2, 1, 0, 6, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0};
inline pas::Array<std::uint8_t, 0, 63> diag_h8a1_posx = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 2, 2, 2, 2, 2, 2, 0, 1, 2, 3, 3, 3, 3, 3, 0, 1, 2, 3, 4, 4, 4, 4, 0, 1, 2, 3, 4, 5, 5, 5, 0, 1, 2, 3, 4, 5, 6, 6, 0, 1, 2, 3, 4, 5, 6, 7};
inline pas::Array<std::int32_t, 1, 8> x_kuda_direction = {1, 2, 2, 1, pas::negate(1), pas::negate(2), pas::negate(2), pas::negate(1)};
inline pas::Array<std::int32_t, 1, 8> y_kuda_direction = {2, 1, pas::negate(1), pas::negate(2), pas::negate(2), pas::negate(1), 1, 2};
inline pas::Array<std::int32_t, 1, 8> x_king_direction = {0, 1, 1, 1, 0, pas::negate(1), pas::negate(1), pas::negate(1)};
inline pas::Array<std::int32_t, 1, 8> y_king_direction = {pas::negate(1), pas::negate(1), 0, 1, 1, 1, 0, pas::negate(1)};
}
namespace petir_hashing_header {
inline constexpr auto bit_used = 64;
inline constexpr auto bit_old = 128;
inline constexpr std::uint8_t generation_mask = 63;
extern std::uint8_t generation;
#pragma pack(push, 1)
struct thashrecord {
    std::uint8_t flag;
    std::uint8_t tipe;
    std::uint64_t key;
    std::uint8_t level;
    std::uint16_t moves;
    std::int16_t nilai;
    std::uint8_t move50count; // Uses the former unused byte; entry stays 16 bytes.
};
#pragma pack(pop)
inline bool is_old(const thashrecord& entry) {return (entry.flag&generation_mask)!=generation;}
// From the search just before the current one.
inline bool is_previous(const thashrecord& entry) {return ((entry.flag+1)&generation_mask)==generation;}
// Cache-line aligned, so a two-entry bucket never straddles two lines.
template<class T> struct line_allocator {
    using value_type = T;
    line_allocator() = default;
    template<class U> line_allocator(const line_allocator<U>&) {}
    T* allocate(std::size_t n) {return static_cast<T*>(::operator new(n*sizeof(T), std::align_val_t{64}));}
    void deallocate(T* p, std::size_t) {::operator delete(p, std::align_val_t{64});}
    template<class U> bool operator==(const line_allocator<U>&) const {return true;}
};
using thashdata = std::vector<petir_hashing_header::thashrecord, line_allocator<petir_hashing_header::thashrecord>>;
extern petir_hashing_header::thashdata hashtable;
// The two-entry bucket a key belongs to (maxhash is a power of two).
inline thashrecord* bucket(std::uint64_t key) {return hashtable.data() + (key & pas::sub(petir_header::maxhash, 2));}
extern std::int32_t anu;
}
namespace petir_udata {
}
namespace petir_evalmask {
inline pas::Array<std::uint8_t, 0, 63> onside = {0, 0, 0, 2, 2, 1, 1, 1, 0, 0, 0, 2, 2, 1, 1, 1, 0, 0, 0, 2, 2, 1, 1, 1, 0, 0, 0, 2, 2, 1, 1, 1, 0, 0, 0, 2, 2, 1, 1, 1, 0, 0, 0, 2, 2, 1, 1, 1, 0, 0, 0, 2, 2, 1, 1, 1, 0, 0, 0, 2, 2, 1, 1, 1};
inline pas::Array<std::uint8_t, 0, 8> nilai_badbishop = {0, 1, 3, 8, 25, 40, 55, 70, 100};
extern pas::Array<std::uint64_t, 0, 63> w_outpost_mask;
extern pas::Array<std::uint64_t, 0, 63> b_outpost_mask;
extern std::uint64_t abcd_mask;
extern std::uint64_t efgh_mask;
extern std::uint64_t abc_mask;
extern std::uint64_t fgh_mask;
extern pas::Array<std::uint64_t, 0, 63> pion_gantung_mask;
extern pas::Array<std::uint8_t, 0, 65535> mobmask;
extern pas::Array<pas::Array<bool, 0, 63>, 0, 63> dekatbaris;
extern pas::Array<pas::Array<std::uint8_t, 0, 63>, 0, 63> tabel_edistance;
extern pas::Array<pas::Array<std::uint8_t, 0, 63>, 0, 63> tabel_edistance2;
extern pas::Array<pas::Array<std::uint8_t, 0, 63>, 0, 63> tabel_distance_gajah;
}
namespace petir_makemove {
}
namespace petir_movgen {
inline constexpr auto _nilai_pion = 1;
inline constexpr auto _nilai_gajah = 3;
inline constexpr auto _nilai_kuda = 3;
inline constexpr auto _nilai_benteng = 5;
inline constexpr auto _nilai_mentri = 9;
inline constexpr auto _nilai_raja = 99;
inline pas::Array<std::uint8_t, pas::negate(6), 6> nilai_piece = {petir_movgen::_nilai_raja, petir_movgen::_nilai_mentri, petir_movgen::_nilai_benteng, petir_movgen::_nilai_gajah, petir_movgen::_nilai_kuda, petir_movgen::_nilai_pion, 0, petir_movgen::_nilai_pion, petir_movgen::_nilai_kuda, petir_movgen::_nilai_gajah, petir_movgen::_nilai_benteng, petir_movgen::_nilai_mentri, petir_movgen::_nilai_raja};
}
namespace petir_tools {
using pinteger = std::int32_t*;
extern std::int32_t anu;
}
namespace petir_usee {
inline constexpr auto _nilai_pion = 1;
inline constexpr auto _nilai_gajah = 3;
inline constexpr auto _nilai_kuda = 3;
inline constexpr auto _nilai_benteng = 5;
inline constexpr auto _nilai_mentri = 9;
inline constexpr auto _nilai_raja = 99;
inline pas::Array<std::uint8_t, pas::negate(6), 6> nilai_piece = {petir_usee::_nilai_raja, petir_usee::_nilai_mentri, petir_usee::_nilai_benteng, petir_usee::_nilai_gajah, petir_usee::_nilai_kuda, petir_usee::_nilai_pion, 0, petir_usee::_nilai_pion, petir_usee::_nilai_kuda, petir_usee::_nilai_gajah, petir_usee::_nilai_benteng, petir_usee::_nilai_mentri, petir_usee::_nilai_raja};
}
namespace petir_valid {
}
namespace petir_hashing {
extern std::int32_t anu;
}
namespace petir_repetition {
// Lookups compare full keys and search adds/removes entries in stack order, so
// counts do not depend on table size while a free slot remains. 16384 slots per
// side exceed the positions of any legal game plus the search path, and keep
// the table cache-resident (originally 65536 slots, 1.25 MiB in total).
inline constexpr auto maxhashdraw = 16384;
#pragma pack(push, 1)
struct treprecord {
    bool used;
    std::uint64_t key;
    std::int8_t jumlah;
};
#pragma pack(pop)
using tarrreprecord = pas::Array<pas::Array<petir_repetition::treprecord, 0, pas::sub(petir_repetition::maxhashdraw, 1)>, 0, 1>;
extern petir_repetition::tarrreprecord repdata;
}
namespace petir_next {
extern std::int32_t t1;
}
namespace petir_ueval {
inline constexpr auto queenside = 1;
inline constexpr auto kingside = 2;
inline constexpr auto no_majorities = 0;
inline pas::Array<std::uint8_t, 0, 63> nilai_gajah_outpost_w = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
inline pas::Array<std::uint8_t, 0, 63> nilai_gajah_outpost_b = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
inline pas::Array<std::uint8_t, 0, 63> nilai_kuda_outpost_w = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 0, 0, 0, 0, 0, 8, 12, 12, 8, 0, 0, 0, 0, 8, 16, 16, 8, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
inline pas::Array<std::uint8_t, 0, 63> nilai_kuda_outpost_b = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 8, 16, 16, 8, 0, 0, 0, 0, 8, 12, 12, 8, 0, 0, 0, 0, 0, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
inline pas::Array<std::int8_t, 0, 63> nilai_pos_kuda_w = {pas::negate(16), pas::negate(8), pas::negate(8), pas::negate(8), pas::negate(8), pas::negate(10), pas::negate(8), pas::negate(16), pas::negate(8), pas::negate(6), 0, 0, 0, 0, pas::negate(6), pas::negate(8), pas::negate(8), 2, 2, 2, 2, 2, 2, pas::negate(8), pas::negate(8), 2, 6, 8, 8, 6, 2, pas::negate(8), pas::negate(8), 2, 6, 10, 10, 6, 2, pas::negate(8), pas::negate(8), 2, 6, 10, 10, 6, 2, pas::negate(8), pas::negate(24), pas::negate(12), 4, 4, 4, 4, pas::negate(12), pas::negate(24), pas::negate(32), pas::negate(16), pas::negate(8), pas::negate(8), pas::negate(8), pas::negate(8), pas::negate(16), pas::negate(32)};
inline pas::Array<std::int8_t, 0, 63> nilai_pos_kuda_b = {pas::negate(32), pas::negate(16), pas::negate(8), pas::negate(8), pas::negate(8), pas::negate(8), pas::negate(16), pas::negate(32), pas::negate(24), pas::negate(12), 4, 4, 4, 4, pas::negate(12), pas::negate(24), pas::negate(8), 2, 6, 10, 10, 6, 2, pas::negate(8), pas::negate(8), 2, 6, 10, 10, 6, 2, pas::negate(8), pas::negate(8), 2, 6, 8, 8, 6, 2, pas::negate(8), pas::negate(8), 2, 2, 2, 2, 2, 2, pas::negate(8), pas::negate(8), pas::negate(6), 0, 0, 0, 0, pas::negate(6), pas::negate(8), pas::negate(16), pas::negate(8), pas::negate(8), pas::negate(8), pas::negate(8), pas::negate(10), pas::negate(8), pas::negate(16)};
inline pas::Array<std::int8_t, 0, 63> nilai_pos_gajah_w = {pas::negate(24), pas::negate(16), pas::negate(8), pas::negate(8), pas::negate(8), pas::negate(10), pas::negate(16), pas::negate(24), pas::negate(4), 4, 0, 0, 0, 0, 4, pas::negate(4), pas::negate(4), 3, 2, 2, 2, 2, 3, pas::negate(4), pas::negate(4), 2, 4, 4, 4, 4, 2, pas::negate(4), pas::negate(4), 2, 6, 6, 6, 6, 2, pas::negate(4), pas::negate(2), 2, 6, 6, 6, 6, 2, pas::negate(2), pas::negate(16), 2, 4, 4, 4, 4, 2, pas::negate(16), pas::negate(16), pas::negate(16), 0, 0, 0, 0, pas::negate(16), pas::negate(16)};
inline pas::Array<std::int8_t, 0, 63> nilai_pos_gajah_b = {pas::negate(16), pas::negate(16), 0, 0, 0, 0, pas::negate(16), pas::negate(16), pas::negate(16), 2, 4, 4, 4, 4, 2, pas::negate(16), pas::negate(2), 2, 6, 6, 6, 6, 2, pas::negate(2), pas::negate(4), 2, 6, 6, 6, 6, 2, pas::negate(4), pas::negate(4), 2, 4, 4, 4, 4, 2, pas::negate(4), pas::negate(4), 3, 2, 2, 2, 2, 3, pas::negate(4), pas::negate(4), 4, 0, 0, 0, 0, 4, pas::negate(4), pas::negate(24), pas::negate(16), pas::negate(8), pas::negate(8), pas::negate(8), pas::negate(10), pas::negate(16), pas::negate(24)};
inline pas::Array<std::int8_t, 0, 63> nilai_pos_benteng = {0, 0, 2, 4, 4, 2, 0, 0, 0, 0, 2, 4, 4, 2, 0, 0, 0, 0, 2, 4, 4, 2, 0, 0, 0, 0, 2, 4, 4, 2, 0, 0, 0, 0, 2, 4, 4, 2, 0, 0, 0, 0, 2, 4, 4, 2, 0, 0, 0, 0, 2, 4, 4, 2, 0, 0, 0, 0, 2, 4, 4, 2, 0, 0};
inline pas::Array<std::uint8_t, 0, 63> nilai_pos_raja_w = {1, 0, 1, 12, 12, 1, 0, 1, 2, 3, 8, 15, 15, 8, 3, 2, 12, 12, 15, 22, 22, 15, 12, 12, 18, 18, 22, 25, 25, 22, 18, 18, 25, 25, 27, 30, 30, 27, 25, 25, 30, 30, 33, 35, 35, 33, 30, 30, 45, 45, 48, 50, 50, 48, 45, 45, 60, 60, 65, 70, 70, 65, 60, 60};
inline pas::Array<std::uint8_t, 0, 63> nilai_pos_raja_b = {60, 60, 65, 70, 70, 65, 60, 60, 45, 45, 48, 50, 50, 48, 45, 45, 30, 30, 33, 35, 35, 33, 30, 30, 25, 25, 27, 30, 30, 27, 25, 25, 18, 18, 22, 25, 25, 22, 18, 18, 12, 12, 15, 22, 22, 15, 12, 12, 2, 3, 8, 15, 15, 8, 3, 2, 1, 0, 1, 12, 12, 1, 0, 1};
inline pas::Array<std::int8_t, 0, 63> nilai_pos_pion_w = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, pas::negate(1), pas::negate(6), pas::negate(6), pas::negate(1), 0, 0, 0, 1, 2, pas::negate(2), pas::negate(2), 2, 1, 0, 3, 4, 5, 7, 7, 5, 4, 3, 4, 5, 6, 9, 9, 6, 5, 4, 7, 8, 9, 11, 11, 9, 8, 7, 8, 10, 12, 15, 15, 12, 10, 8, 0, 0, 0, 0, 0, 0, 0, 0};
inline pas::Array<std::int8_t, 0, 63> nilai_pos_pion_b = {0, 0, 0, 0, 0, 0, 0, 0, 8, 10, 12, 15, 15, 12, 10, 8, 7, 8, 9, 11, 11, 9, 8, 7, 4, 5, 6, 9, 9, 6, 5, 4, 3, 4, 5, 7, 7, 5, 4, 3, 0, 1, 2, pas::negate(2), pas::negate(2), 2, 1, 0, 0, 0, pas::negate(1), pas::negate(6), pas::negate(6), pas::negate(1), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
inline pas::Array<std::int8_t, 0, 63> nilai_pos_raja_endk_w = {pas::negate(35), pas::negate(25), pas::negate(20), pas::negate(20), pas::negate(15), pas::negate(15), pas::negate(10), pas::negate(10), pas::negate(30), pas::negate(15), pas::negate(15), pas::negate(10), pas::negate(10), pas::negate(5), pas::negate(5), pas::negate(5), pas::negate(30), pas::negate(15), pas::negate(10), pas::negate(5), 10, 10, 10, 0, pas::negate(30), pas::negate(15), pas::negate(10), pas::negate(5), 20, 20, 20, 0, pas::negate(30), pas::negate(15), pas::negate(10), pas::negate(5), 20, 20, 20, 0, pas::negate(30), pas::negate(15), pas::negate(10), pas::negate(5), 20, 20, 20, 0, pas::negate(30), pas::negate(15), pas::negate(15), pas::negate(10), 0, 0, 0, pas::negate(5), pas::negate(35), pas::negate(25), pas::negate(20), pas::negate(15), pas::negate(10), pas::negate(5), pas::negate(5), pas::negate(10)};
inline pas::Array<std::int8_t, 0, 63> nilai_pos_raja_endq_w = {pas::negate(10), pas::negate(10), pas::negate(15), pas::negate(15), pas::negate(20), pas::negate(20), pas::negate(25), pas::negate(35), pas::negate(5), pas::negate(5), pas::negate(5), pas::negate(10), pas::negate(10), pas::negate(15), pas::negate(15), pas::negate(30), 0, 10, 10, 10, pas::negate(5), pas::negate(10), pas::negate(15), pas::negate(30), 0, 20, 20, 20, pas::negate(5), pas::negate(10), pas::negate(15), pas::negate(30), 0, 20, 20, 20, pas::negate(5), pas::negate(10), pas::negate(15), pas::negate(30), 0, 20, 20, 20, pas::negate(5), pas::negate(10), pas::negate(15), pas::negate(30), pas::negate(5), 0, 0, 0, pas::negate(10), pas::negate(15), pas::negate(15), pas::negate(30), pas::negate(10), pas::negate(5), pas::negate(5), pas::negate(10), pas::negate(15), pas::negate(20), pas::negate(25), pas::negate(35)};
inline pas::Array<std::int8_t, 0, 63> nilai_pos_raja_endq_b = {pas::negate(10), pas::negate(5), pas::negate(5), pas::negate(10), pas::negate(15), pas::negate(20), pas::negate(25), pas::negate(35), pas::negate(5), 0, 0, 0, pas::negate(10), pas::negate(15), pas::negate(15), pas::negate(30), 0, 20, 20, 20, pas::negate(5), pas::negate(10), pas::negate(15), pas::negate(30), 0, 20, 20, 20, pas::negate(5), pas::negate(10), pas::negate(15), pas::negate(30), 0, 20, 20, 20, pas::negate(5), pas::negate(10), pas::negate(15), pas::negate(30), 0, 10, 10, 10, pas::negate(5), pas::negate(10), pas::negate(15), pas::negate(30), pas::negate(5), pas::negate(5), pas::negate(5), pas::negate(10), pas::negate(10), pas::negate(15), pas::negate(15), pas::negate(30), pas::negate(10), pas::negate(10), pas::negate(15), pas::negate(15), pas::negate(20), pas::negate(20), pas::negate(25), pas::negate(35)};
inline pas::Array<std::int8_t, 0, 63> nilai_pos_raja_endk_b = {pas::negate(35), pas::negate(25), pas::negate(20), pas::negate(15), pas::negate(10), pas::negate(5), pas::negate(5), pas::negate(10), pas::negate(30), pas::negate(15), pas::negate(15), pas::negate(10), 0, 0, 0, pas::negate(5), pas::negate(30), pas::negate(15), pas::negate(10), pas::negate(5), 20, 20, 20, 0, pas::negate(30), pas::negate(15), pas::negate(10), pas::negate(5), 20, 20, 20, 0, pas::negate(30), pas::negate(15), pas::negate(10), pas::negate(5), 20, 20, 20, 0, pas::negate(30), pas::negate(15), pas::negate(10), pas::negate(5), 10, 10, 10, 0, pas::negate(30), pas::negate(15), pas::negate(15), pas::negate(10), pas::negate(10), pas::negate(5), pas::negate(5), pas::negate(5), pas::negate(35), pas::negate(25), pas::negate(20), pas::negate(20), pas::negate(15), pas::negate(15), pas::negate(10), pas::negate(10)};
inline pas::Array<std::int8_t, 0, 63> nilai_pos_raja_endn_w = {pas::negate(25), pas::negate(20), pas::negate(15), pas::negate(20), pas::negate(20), pas::negate(30), pas::negate(20), pas::negate(25), pas::negate(15), pas::negate(10), pas::negate(5), 0, 0, pas::negate(10), pas::negate(10), pas::negate(15), pas::negate(15), pas::negate(5), 0, 5, 5, 0, pas::negate(5), pas::negate(15), pas::negate(15), pas::negate(5), 10, 20, 20, 10, pas::negate(5), pas::negate(15), pas::negate(15), pas::negate(5), 20, 30, 30, 20, pas::negate(5), pas::negate(15), pas::negate(15), pas::negate(5), 20, 30, 30, 20, pas::negate(5), pas::negate(15), pas::negate(15), pas::negate(15), 0, 0, 0, 0, pas::negate(15), pas::negate(15), pas::negate(25), pas::negate(15), pas::negate(15), pas::negate(15), pas::negate(15), pas::negate(15), pas::negate(15), pas::negate(25)};
inline pas::Array<std::int8_t, 0, 63> nilai_pos_raja_endn_b = {pas::negate(25), pas::negate(15), pas::negate(15), pas::negate(15), pas::negate(15), pas::negate(15), pas::negate(15), pas::negate(25), pas::negate(15), pas::negate(15), 0, 0, 0, 0, pas::negate(15), pas::negate(15), pas::negate(15), pas::negate(5), 20, 30, 30, 20, pas::negate(5), pas::negate(15), pas::negate(15), pas::negate(5), 20, 30, 30, 20, pas::negate(5), pas::negate(15), pas::negate(15), pas::negate(5), 10, 20, 20, 10, pas::negate(5), pas::negate(15), pas::negate(15), pas::negate(5), 0, 5, 5, 0, pas::negate(5), pas::negate(15), pas::negate(15), pas::negate(10), pas::negate(5), 0, 0, pas::negate(10), pas::negate(10), pas::negate(15), pas::negate(25), pas::negate(20), pas::negate(15), pas::negate(20), pas::negate(20), pas::negate(30), pas::negate(20), pas::negate(25)};
inline pas::Array<std::int8_t, 0, 63> nilai_pos_mentri_w = {pas::negate(5), pas::negate(8), 0, 0, 0, 0, pas::negate(8), pas::negate(5), pas::negate(3), 2, 4, 4, 4, 4, 2, pas::negate(3), pas::negate(1), 2, 3, 6, 6, 3, 2, pas::negate(1), pas::negate(0), 2, 6, 8, 8, 6, 2, pas::negate(0), 0, 2, 6, 8, 8, 6, 2, 0, 0, 2, 6, 6, 6, 6, 2, 0, 0, 3, 5, 5, 5, 5, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0};
inline pas::Array<std::int8_t, 0, 63> nilai_pos_mentri_b = {0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 5, 5, 5, 5, 3, 0, 0, 2, 6, 6, 6, 6, 2, 0, 0, 2, 6, 8, 8, 6, 2, 0, pas::negate(0), 2, 6, 8, 8, 6, 2, pas::negate(0), pas::negate(1), 2, 3, 6, 6, 3, 2, pas::negate(1), pas::negate(3), 2, 4, 4, 4, 4, 2, pas::negate(3), pas::negate(5), pas::negate(8), 0, 0, 0, 0, pas::negate(8), pas::negate(5)};
inline pas::Array<std::uint8_t, 0, 64> color = {0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 2};
inline constexpr auto nilai_2bishop_attack_king = 10;
inline constexpr auto nilai_nggakbisarokade = 40;
inline constexpr auto nilai_rokade = 10;
inline constexpr auto nilai_rokade_wrong_side = 10;
inline constexpr auto nilai_bishop_trapped = 155;
inline constexpr auto nilai_bishop_trapped2 = 30;
inline constexpr auto nilai_kuda_trapped = 200;
inline constexpr auto nilai_pion_pusat_belum_berkembang = 10;
inline constexpr auto nilai_pion_pusat_blocked = 25;
inline constexpr auto nilai_benteng_semi_open_file_dekat_raja = 6;
inline constexpr auto nilai_benteng_tumpuk_kolom_semi_open_file_dekat_raja = 11;
inline constexpr auto nilai_benteng_open_file_dekat_raja = 8;
inline constexpr auto nilai_benteng_tumpuk_kolom = 8;
inline constexpr auto nilai_benteng_tumpuk_kolom_dekat_raja = 6;
inline constexpr auto nilai_benteng_mentri_open = 3;
inline constexpr auto nilai_benteng_mentri_semi_open = 1;
inline constexpr auto nilai_benteng_mentri_semi_open_dekat_raja = 12;
inline constexpr auto nilai_benteng_tumpuk_kolom_open = 12;
inline pas::Array<std::uint8_t, 0, 6> nilai_pion_tumpuk = {0, 6, 20, 40, 65, 90, 120};
inline constexpr auto nilai_pion_backward = 3;
inline pas::Array<std::uint8_t, 0, 7> nilai_pion_terisolasi = {2, 4, 4, 4, 4, 4, 4, 2};
inline constexpr auto nilai_pion_hilang = 45;
inline pas::Array<std::uint8_t, 0, 7> nilai_potential_pp_white = {0, 0, 3, 4, 5, 6, 6, 6};
inline pas::Array<std::uint8_t, 0, 7> nilai_potential_pp_black = {6, 6, 6, 5, 4, 3, 0, 0};
inline pas::Array<std::uint8_t, 0, 8> nilai_pion_terisolasi_ak = {0, 0, 2, 5, 10, 30, 50, 50, 65};
inline constexpr auto nilai_benteng_baris7 = 18;
inline constexpr auto nilai_benteng_baris8 = 4;
inline constexpr auto nilai_can_advance = 40;
inline constexpr auto nilai_benteng_tumpuk_baris7 = 25;
inline constexpr auto nilai_benteng_mentri_tumpuk_baris7 = 33;
inline constexpr auto nilai_benteng_blocked = 9;
inline constexpr auto nilai_pion_bebas = 16;
inline constexpr auto nilai_mobility = 2;
inline constexpr auto nilai_tropism_mentri = 2;
inline constexpr auto nilai_tropism_gajah = 1;
inline constexpr auto nilai_tropism_kuda = 2;
inline constexpr auto nilai_blocked_pawn = 7;
inline constexpr auto nilai_pion_bebas_connected2 = 7;
inline constexpr auto nilai_pion_bebas_connected3 = 45;
inline constexpr auto nilai_pion_bebas_connected = 120;
inline pas::Array<std::uint8_t, 0, 8> nilai_bishop_pair0 = {10, 80, 75, 70, 60, 50, 30, 15, 5};
inline constexpr auto nilai_no_pawn = 40;
inline constexpr auto nilai_no_fianchetto = 4;
inline constexpr auto nilai_king_blocked = 8;
inline constexpr auto nilai_safe_promote = 225;
inline constexpr auto nilai_safe_promote1 = 125;
inline constexpr auto nilai_another_weak_structure = 6;
inline constexpr auto nilai_bishop_over_knight_endgame = 12;
inline pas::Array<std::int8_t, 0, 8> nilai_knight_lack_mobility = {pas::negate(25), pas::negate(5), 0, 0, 0, 0, 0, 0, 0};
inline pas::Array<std::int8_t, 0, 8> mobil_knight = {pas::negate(20), pas::negate(5), 0, 2, 6, 10, 14, 16, 18};
inline pas::Array<std::int8_t, 0, 15> mobil_bishop = {pas::negate(20), pas::negate(10), 0, 4, 8, 12, 16, 20, 24, 28, 31, 34, 35, 36, 37, 38};
inline pas::Array<std::int8_t, 0, 15> mobil_rook = {pas::negate(12), pas::negate(8), pas::negate(4), 0, 4, 8, 11, 14, 16, 18, 19, 20, 21, 22, 23, 24};
inline pas::Array<std::int8_t, 0, 31> mobil_queen = {pas::negate(10), pas::negate(8), pas::negate(6), pas::negate(4), pas::negate(2), 0, 2, 4, 6, 8, 10, 12, 13, 14, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16};
inline constexpr auto nilai_can_promote = 550;
inline constexpr auto nilai_potential_passer = 45;
inline constexpr auto nilai_blocked_rook = 35;
inline pas::Array<std::uint8_t, 0, 7> nilai_semi_open_file = {2, 2, 3, 4, 4, 3, 2, 2};
inline pas::Array<std::uint8_t, 0, 7> nilai_open_file = {3, 3, 4, 5, 5, 4, 3, 3};
inline pas::Array<std::uint8_t, 1, 7> w_nilai_pion_advance_bebas = {0, 2, 5, 25, 55, 120, 0};
inline pas::Array<std::uint8_t, 0, 6> b_nilai_pion_advance_bebas = {0, 120, 55, 25, 5, 2, 0};
inline constexpr auto nilai_benteng_dibelakang_pion_bebas = 25;
inline constexpr auto nilai_pion_advance_bebas_blocked = 15;
inline pas::Array<std::uint8_t, 1, 7> w_nilai_pion_bebas_didukung_raja = {0, 0, 0, 10, 20, 40, 0};
inline pas::Array<std::uint8_t, 0, 6> b_nilai_pion_bebas_didukung_raja = {0, 40, 20, 10, 0, 0, 0};
inline constexpr auto nilai_bishop_blocked2 = 5;
inline constexpr auto nilai_bishop_offside = 20;
inline pas::Array<std::uint8_t, 0, 8> nilai_bishop_blocked = {0, 2, 5, 12, 30, 50, 70, 100, 100};
inline pas::Array<std::uint8_t, 0, 14> nilai_pawn_shield = {0, 5, 10, 15, 20, 30, 40, 60, 70, 80, 90, 100, 120, 180, 200};
inline pas::Array<std::int32_t, 0, 63> nilai_king_pressure = {0, 1, 5, 15, 35, 70, 125, 145, 150, 160, 170, 170, 175, 200, 225, 250, 300, 325, 350, 400, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500};
inline pas::Array<std::uint8_t, 0, 41> nilai_endgame_scale = {100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 90, 80, 70, 60, 50, 40, 30, 20, 20, 10, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
inline pas::Array<std::uint8_t, 0, 41> nilai_rook_scale = {0, 0, 0, 0, 0, 0, 0, 0, 20, 20, 20, 20, 20, 20, 20, 20, 50, 60, 70, 70, 80, 90, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100};
inline constexpr auto nilai_qb_endgame = 5;
inline constexpr auto nilai_pawn_endgame = 10;
inline constexpr auto nilai_pawn_endgame2 = 5;
}
namespace petir_search {
struct anonymous_record_1 {
    std::int32_t fase;
    std::int32_t legalmove;
    bool mte;
    bool ext;
    bool evaluated;
    std::int32_t evalvalue;
};
extern pas::Array<std::int32_t, 0, 63> curr_move;
// The piece that made curr_move, for the continuation-history key.
extern pas::Array<std::int32_t, 0, 63> curr_piece;
inline constexpr auto min_try = pas::mul(50, 6);
inline constexpr auto min_threshold = 0.10;
inline constexpr auto node_all = pas::negate(1);
inline constexpr auto node_pv = 0;
inline constexpr auto node_cut = 1;
extern pas::Array<pas::Array<std::int32_t, 0, 63>, pas::negate(6), 6> hist_tried;
extern pas::Array<pas::Array<std::int32_t, 0, 63>, pas::negate(6), 6> hist_succ;
extern pas::Array<petir_search::anonymous_record_1, 0, 63> stack;
extern std::int32_t rs;
inline pas::Array<std::int32_t, 2, 8> margin = {16, 22, 30, 37, 55, 92, 142};
inline pas::Array<std::int32_t, 0, 15> margingc = {5000, 90, 50, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
inline constexpr auto noval = pas::negate(200000);
}
namespace petir_root {
extern std::int32_t bmoves;
extern std::int32_t ntimelimit;
extern std::int32_t rtl;
extern bool newpv;
}
namespace petir_uiterative {
extern std::int32_t completed_move,completed_depth,completed_score;
extern std::string completed_pv;
inline pas::Array<std::int32_t, 2, 12> timereducement = {1, 5, 15, 20, 20, 25, 30, 30, 30, 30, 30};
extern std::int32_t retime;
}
namespace petir_customboard {
extern std::int32_t a;
extern std::int32_t b;
extern std::int32_t pos;
extern std::int32_t kar;
}
namespace petir_notation {
}
namespace petir_pv {
}
namespace petir_opening_book {
struct anonymous_record_1 {
    std::uint16_t moves;
    std::int32_t weight;
};
inline constexpr auto maxhashbook = (262144 / 2);
inline constexpr auto minimum_elo = 2540;
inline constexpr auto thr_min = pas::negate(25);
inline constexpr auto depth_check = 5;
#pragma pack(push, 1)
struct tbook {
    std::uint64_t key;
    std::uint16_t bobot;
};
#pragma pack(pop)
#pragma pack(push, 1)
struct thashbook {
    petir_opening_book::tbook book;
    bool used;
};
#pragma pack(pop)
using hashbook = pas::Array<pas::Array<petir_opening_book::thashbook, 0, pas::sub(petir_opening_book::maxhashbook, 1)>, 0, 1>;
extern petir_opening_book::hashbook book;
extern std::int32_t no;
extern std::int32_t a;
}
namespace petir_etc {
}
namespace petir_undoredo {
struct tundomoves {
    petir_header::tdata data;
    std::int32_t whiteclock;
    std::int32_t blackclock;
};
struct tundo {
    std::int32_t count;
    std::int32_t max;
    pas::Array<petir_undoredo::tundomoves, 1, 400> moves;
};
}
namespace petir_header {
}
namespace petir_bitboard_mask {
std::int32_t firstbitp(const std::uint16_t* p);
std::int32_t lastbitp(const std::uint16_t* p);
std::uint64_t attackbishop(const petir_header::tdata& data, std::int32_t pos);
std::uint64_t attackrook(petir_header::tdata& data, std::int32_t pos);
}
namespace petir_bitboard {
void init_mask();
}
namespace petir_udata {
std::uint8_t map(std::uint8_t y, std::uint8_t x);
void initstate(petir_header::tdata& data);
}
namespace petir_evalmask {
void initevalmask();
std::int32_t distancemax(std::int32_t p, std::int32_t q);
std::int32_t distance_gajah(std::int32_t p, std::int32_t q);
std::int32_t distance(std::int32_t p, std::int32_t q);
void initevaltable();
}
namespace petir_makemove {
void makewhitemove(std::int32_t moves, petir_header::tdata& data);
void makeblackmove(std::int32_t moves, petir_header::tdata& data);
void cekerror(petir_header::tdata data);
void whiterokadependek(petir_header::tdata& data);
void whiterokadepanjang(petir_header::tdata& data);
void blackrokadependek(petir_header::tdata& data);
void blackrokadepanjang(petir_header::tdata& data);
void black_ep(petir_header::tdata& data, std::int32_t sfrom, std::int32_t sto);
void white_ep(petir_header::tdata& data, std::int32_t sfrom, std::int32_t sto);
void promosi_putih(petir_header::tdata& data, std::int32_t sto, std::int32_t promosi);
void promosi_hitam(petir_header::tdata& data, std::int32_t sto, std::int32_t promosi);
}
namespace petir_movgen {
void black_movgen_noncaps(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml);
void black_movgen_caps(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml);
void black_movgen_caps2(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml);
void q_black_movgen(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml);
void q_white_movgen(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml);
void white_movgen_caps(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml);
void white_movgen_caps2(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml);
void white_movgen_noncaps(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml);
void white_evasion(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml, std::int32_t& skakcount);
void black_evasion(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml, std::int32_t& skakcount);
}
namespace petir_tools {
std::uint64_t pinned_pieces(const petir_header::tdata& data, int side);
bool white_attacked(petir_header::tdata& data, std::int32_t pos);
bool black_attacked(petir_header::tdata& data, std::int32_t pos);
bool white_checked(petir_header::tdata& data);
bool black_checked(petir_header::tdata& data);
void clearkiller();
bool white_open_check(petir_header::tdata& data, std::uint16_t moves);
bool white_open_check2(petir_header::tdata& data, std::uint16_t pos2, std::uint16_t post, std::uint16_t posraja);
bool black_open_check2(petir_header::tdata& data, std::uint16_t pos2, std::uint16_t post, std::uint16_t posraja);
bool black_open_check(petir_header::tdata& data, std::uint16_t moves);
bool isextension(petir_header::tdata& data, std::int32_t& newlevel, std::int32_t giliran, std::int32_t moves, std::int32_t& cext);
bool winningmaterial(std::int32_t ms, std::uint8_t giliran);
bool losingmaterial(std::int32_t ms, std::uint8_t giliran);
bool menskak(petir_header::tdata& data, std::int32_t giliran);
bool menskak2(petir_header::tdata& data, std::uint16_t giliran, std::uint16_t moves);
bool pawnpush(petir_header::tdata& data, std::uint16_t giliran, std::uint16_t moves);
bool capture(petir_header::tdata& data, std::uint16_t moves);
bool white_forkandpin(std::int32_t move, petir_header::tdata& data);
bool black_forkandpin(std::int32_t move, petir_header::tdata& data);
std::int32_t captured_value(std::int32_t sto, petir_header::tdata& data);
bool white_threat(petir_header::tdata& data, std::uint16_t moves, std::int32_t& threatpiece);
bool black_threat(petir_header::tdata& data, std::uint16_t moves, std::int32_t& threatpiece);
bool pawn_7_rank(std::int32_t giliran, petir_header::tdata& data);
bool pawn_7_rankw(petir_header::tdata& data);
bool pawn_7_rankb(petir_header::tdata& data);
bool pawn_6_rank(std::int32_t giliran, petir_header::tdata& data);
bool good_move_white(petir_header::tdata& data, std::int32_t move);
bool good_move_black(petir_header::tdata& data, std::int32_t move);
bool wb_threat(std::uint8_t giliran, petir_header::tdata& data, std::uint16_t moves, std::int32_t& threatpiece);
std::int32_t material(petir_header::tdata& data, std::int32_t giliran);
pas::String getfen(petir_header::tdata data, std::int32_t giliran);
bool good_move(petir_header::tdata& data, std::int32_t move, std::int32_t giliran);
bool white_menskak(petir_header::tdata& data, std::uint16_t moves);
std::uint8_t ipopcount(std::uint64_t p);
bool black_menskak(petir_header::tdata& data, std::uint16_t moves);
bool black_menskak3(petir_header::tdata& data, std::uint16_t moves, std::int32_t& threatpiece);
}
namespace petir_usee {
std::int32_t see(std::int32_t giliran, const petir_header::tdata& data, std::int32_t sfrom, std::int32_t sto);
void xray(std::uint64_t& attack, std::int32_t square, std::int32_t target, const petir_header::tdata& data);
}
namespace petir_valid {
bool isvalid(petir_header::tdata& data, std::int32_t moves, std::int32_t giliran);
}
namespace petir_hashing_header {
void inithashkey();
std::uint64_t hashvalue(petir_header::tdata& data, std::int32_t giliran);
void cleartable();
void cleartable2();
void inittable();
void clearreversetable();
void reversetable();
void coba();
}
namespace petir_hashing {
void addtable(std::uint64_t key, std::int32_t giliran, std::int32_t level, std::int32_t moves, std::int32_t nilai, std::int32_t tipe, std::uint8_t move50count);
bool searchhash(std::uint64_t key, std::int32_t& alpha, std::int32_t& beta, std::int32_t level, std::int32_t giliran, std::int32_t& nilai, std::int32_t& moves, bool& avoidnm, std::uint8_t move50count);
// Cache hint only: starts loading the entry searchhash will read for this key.
inline void prefetch(std::uint64_t key, std::int32_t giliran) {
    if (giliran == petir_header::_sisihitam) key = ~key;
    const auto* entry = petir_hashing_header::bucket(key);
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
    _mm_prefetch(reinterpret_cast<const char*>(entry), _MM_HINT_T0);
#elif defined(__GNUC__)
    __builtin_prefetch(entry);
#else
    (void)entry;
#endif
}
}
namespace petir_repetition {
void initrepdata();
std::int32_t addrephash(std::uint64_t key, std::int32_t giliran);
void delrephash(std::uint64_t key, std::int32_t giliran);
bool isrep(std::uint64_t key, std::int32_t giliran);
}
namespace petir_next {
void nextbest_max(petir_header::tmovelist& a, std::int32_t cur, std::int32_t tot);
bool nextevasion(std::int32_t giliran, petir_header::tdata& data, std::int32_t& moves, std::int32_t& fase, std::int32_t& hmoves, std::int32_t& curmove, std::int32_t& jml, std::int32_t& skakcount, petir_header::tmovelist& ml);
bool nextmove(std::int32_t giliran, std::int32_t depth, petir_header::tdata& data, std::int32_t& moves, std::int32_t& fase, std::int32_t hmoves, std::int32_t& curmovecap, std::int32_t& curmovenoncap, std::int32_t& jmlcap, std::int32_t& jmlnoncap, petir_header::tmovelist& ml, petir_header::tmovelist& mlcap, std::int32_t& goodcap, petir_header::piece_to_history* const* cont);
bool nextroot(std::int32_t giliran, std::int32_t& moves, std::int32_t& fase, std::int32_t& hmoves, std::int32_t& curmove, std::int32_t& jml, petir_header::tmovelist ml);
bool nextq(std::int32_t giliran, petir_header::tdata& data, std::int32_t& moves, std::int32_t& fase, std::int32_t& curmove, std::int32_t& jml, std::int32_t& score, std::int32_t ply, petir_header::tmovelist& ml);
}
namespace petir_ueval {
std::int32_t eval(const petir_header::tdata& data, std::int32_t giliran, std::int32_t alpha, std::int32_t beta, bool infhr = false);
std::int32_t materialvalue(std::int32_t giliran, std::int32_t ms);
bool isdraw(const petir_header::tdata& data, bool& draw2);
bool anotherdraw(const petir_header::tdata& data, std::uint8_t giliran);
std::uint8_t ipopcount(std::uint64_t p);
std::int32_t popcount_w(petir_bitboard_mask::pword p);
std::int32_t popcount_b(petir_bitboard_mask::pword p);
std::uint8_t popcount(petir_bitboard_mask::pword p);
bool onlyone(std::uint64_t p);
std::int32_t dynamiclazy(const petir_header::tdata& data);
}
namespace petir_search {
void empty_historytable();
std::int32_t mainsearch(std::int32_t level, std::int32_t giliran, std::int32_t alpha, std::int32_t beta, petir_header::tdata& data, bool nullmove, std::int32_t depth, std::int32_t cext, bool nmtree, std::int32_t reduced, std::int32_t nodetype);
std::int32_t q_search(std::int32_t giliran, std::int32_t alpha, std::int32_t beta, petir_header::tdata& data, std::int32_t ply, std::int32_t depth, std::int32_t cext, std::int32_t neval, std::int32_t nodetype);
void addhistory(std::int32_t rlevel, std::int32_t depth, std::int32_t moves, std::int32_t giliran, std::int32_t piece);
void update_history(std::int32_t giliran, std::int32_t piece, std::int32_t sto, std::int32_t bonus);
void update_continuation(petir_header::piece_to_history* const* cont, std::int32_t piece, std::int32_t sto, std::int32_t bonus);
void penalise_quiets(std::int32_t rlevel, std::int32_t giliran, const petir_header::tdata& data, petir_header::piece_to_history* const* cont, const std::int32_t* quiets, std::int32_t count, std::int32_t best);
}
namespace petir_root {
std::int32_t searchroot(std::int32_t level, std::int32_t giliran, std::int32_t tipe, std::int32_t alpha, std::int32_t beta, petir_header::tdata data, bool nullmove, std::int32_t depth, std::int32_t cext);
}
namespace petir_uiterative {
std::int32_t iterative(std::int32_t level, std::int32_t giliran, std::int32_t tipe, petir_header::tdata data);
std::int32_t mtdf(petir_header::tdata data, std::uint8_t level, std::uint8_t giliran, std::int32_t firstguess);
std::int32_t aspiration(std::int32_t level, std::int32_t giliran, std::int32_t tipe, std::int32_t nilai, petir_header::tdata data);
bool timemanage(std::int32_t giliran, petir_header::tdata data);
}
namespace petir_customboard {
void fillbitboard(petir_header::tdata& data);
void readnotation(pas::String n, petir_header::tdata& data, bool& wtm);
}
namespace petir_notation {
pas::String movetonotation(std::int32_t move, std::int32_t ep, std::int32_t giliran);
std::int32_t notationtomove(pas::String notasi, petir_header::tdata data, std::uint8_t giliran);
pas::String shortnotationtolong(pas::String notasi, petir_header::tdata data, std::uint8_t giliran);
pas::String getnotasi(std::int32_t square);
std::int32_t squaretobinary(pas::String s);
}
namespace petir_pv {
pas::String getpv(petir_header::tdata data, std::int32_t giliran, std::int32_t maxdepth);
void printpv(std::int32_t& n, pas::String spv, std::int32_t l, std::int32_t t, std::int32_t nilai3);
pas::String strtime(std::int32_t time);
}
namespace petir_opening_book {
void learn_pgn(pas::String a);
void initbook();
void loadbook();
std::int32_t searchbook(petir_header::tdata data, std::int32_t giliran);
bool searchhashbook(std::uint64_t key, std::uint8_t giliran, std::int32_t& weight);
bool searchhashbook2(std::uint64_t key, std::uint8_t giliran, std::int32_t& weight);
bool edithashbook(std::uint64_t key, std::uint8_t giliran, std::int32_t newbobot);
void savebook();
void learn();
bool addhashbook(std::uint64_t key, std::uint8_t hasil, std::uint8_t giliran, std::int32_t whiteelo, std::int32_t blackelo);
bool storebook(petir_opening_book::tbook b, std::int32_t giliran);
void make(pas::String notasi, petir_header::tdata& data, std::uint8_t giliran);
}
namespace petir_etc {
void updateblackkey(std::int32_t moves, petir_header::tdata& data);
void updatewhitekey(std::int32_t moves, petir_header::tdata& data);
void updatekey(petir_header::tdata& data, std::int32_t moves, std::int32_t giliran);
}
namespace petir_undoredo {
void resetundo(petir_undoredo::tundo& undo);
void addundo(petir_undoredo::tundo& undo, petir_header::tdata data, std::int32_t whiteclock, std::int32_t blackclock);
bool takeback(petir_undoredo::tundo& undo, petir_header::tdata& data, std::int32_t& wc, std::int32_t& bc, std::uint8_t& giliran);
bool moveforward(petir_undoredo::tundo& undo, petir_header::tdata& data, std::int32_t& wc, std::int32_t& bc, std::uint8_t& giliran);
}
