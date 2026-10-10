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
    std::uint64_t bits_;
public:
    explicit move_bits(std::uint64_t& bits):bits_(bits) {
        // Each loop consumes its source mask; its body uses returned squares.
        bits=0;
        if constexpr(Legacy) {
            bits_=std::rotl(bits_,32);
            bits_=((bits_ & UINT64_C(0x0000ffff0000ffff))<<16)
                 |((bits_>>16) & UINT64_C(0x0000ffff0000ffff));
        }
    }
    explicit operator bool() const {return bits_!=0;}
    int pop() {
        // The surrounding loop guarantees a nonempty bitboard.
#if defined(_MSC_VER) && defined(_M_X64)
        unsigned long square;
        _BitScanForward64(&square,bits_);
#else
        const int square=std::countr_zero(bits_);
#endif
        bits_ &= bits_-1;
        return static_cast<int>(square) ^ (Legacy?48:0);
    }
};
void append_legal_ep_evasions(petir_header::tdata& data, petir_header::tmovelist& moves, std::int32_t& count, int side) {
    if(data.ep>7)return;
    const bool white=side==petir_header::_sisiputih;
    const int captured=(white?32:24)+data.ep;
    const int target=captured+(white?8:-8);
    if(data.papan[captured]!=(white?-1:1) || data.papan[target]!=0)return;
    for(int offset:{-1,1}) {
        const int file=data.ep+offset;
        if(file<0 || file>7 || data.papan[captured+offset]!=(white?1:-1))continue;
        const int move=(captured+offset)|(petir_header::_en_passant<<7);
        auto child=data;
        const auto nodes=petir_header::total_node;
        if(white)petir_makemove::makewhitemove(move,child);else petir_makemove::makeblackmove(move,child);
        petir_header::total_node=nodes; // Legality probes are not search nodes.
        if(white?petir_tools::white_checked(child):petir_tools::black_checked(child))continue;
        ++count;moves[count].moves=move;moves[count].score=0;
    }
}
}
// movgen.pas:687
void black_movgen_noncaps(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml_out) {
    // Count locally: MSVC would otherwise reload the caller's count after
    // every move-list store. The count is published once on return.
    std::int32_t jml = jml_out;
    std::uint64_t piecepos{};
    std::uint64_t attack{};
    std::int32_t pos{};
    std::int32_t post{};
    std::int32_t move_index=0;
    jml = 0;
    piecepos = (data.pawnblack & (~petir_bitboard_mask::rank_mask[1]));
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        if ((data.papan[pas::sub(pos, 8)] == 0)) {
            (jml = pas::increment(jml, 1));
            auto& entry = movelist[++move_index];
            entry.moves = pas::add(pos, pas::shl(pas::sub(pos, 8), 7));
            entry.score = petir_header::historyb[petir_header::_pionhitam][pas::sub(pos, 8)];
            if (((pos >= petir_header::a7) && (data.papan[pas::sub(pos, 16)] == 0))) {
                (jml = pas::increment(jml, 1));
                auto& entry = movelist[++move_index];
                entry.moves = pas::add(pos, pas::shl(pas::sub(pos, 16), 7));
                entry.score = petir_header::historyb[petir_header::_pionhitam][pas::sub(pos, 16)];
            }
        }
    }
    piecepos = data.knightblack;
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = (petir_bitboard_mask::knightmask[pos] & (~data.allpieces));
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            auto& entry = movelist[++move_index];
            entry.moves = pas::add(pos, pas::shl(post, 7));
            entry.score = petir_header::historyb[petir_header::_kudahitam][post];
        }
    }
    piecepos = data.queenblack;
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack | petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)]);
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        attack = (attack & (~data.allpieces));
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            auto& entry = movelist[++move_index];
            entry.moves = pas::add(pos, pas::shl(post, 7));
            entry.score = petir_header::historyb[petir_header::_mentrihitam][post];
            (jml = pas::increment(jml, 1));
        }
    }
    piecepos = data.bishopblack;
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        attack = (attack & (~data.allpieces));
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            auto& entry = movelist[++move_index];
            entry.moves = pas::add(pos, pas::shl(post, 7));
            entry.score = petir_header::historyb[petir_header::_gajahhitam][post];
        }
    }
    piecepos = data.rookblack;
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack & (~data.allpieces));
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            auto& entry = movelist[++move_index];
            entry.moves = pas::add(pos, pas::shl(post, 7));
            entry.score = petir_header::historyb[petir_header::_bentenghitam][post];
        }
    }
    pos = petir_bitboard_mask::king_square(data.kingblack);
    attack = (petir_bitboard_mask::kingmask[pos] & (~data.allpieces));
    for(move_bits<true> remaining(attack); remaining; ) {
        post = remaining.pop();
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
        movelist[jml].score = petir_header::historyb[petir_header::_rajahitam][post];
    }
    if (((((data.flagrokade & petir_header::bithitamnoc) != 0) && (data.papan[petir_header::e8] == petir_header::_rajahitam)) && (!petir_tools::black_attacked(data, petir_header::e8)))) {
        if (((((((data.papan[petir_header::f8] == 0) && (data.papan[petir_header::g8] == 0)) && (data.papan[petir_header::h8] == petir_header::_bentenghitam)) && (!petir_tools::black_attacked(data, petir_header::f8))) && (!petir_tools::black_attacked(data, petir_header::g8))) && ((data.flagrokade & petir_header::bithitamsc) != 0))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(petir_header::_rokadependek, 7));
            movelist[jml].score = petir_header::historyb[petir_header::_rajahitam][petir_header::_rokadependek];
        }
        if ((((((((data.papan[petir_header::d8] == 0) && (data.papan[petir_header::c8] == 0)) && (data.papan[petir_header::b8] == 0)) && ((data.flagrokade & petir_header::bithitamlc) != 0)) && (data.papan[petir_header::a8] == petir_header::_bentenghitam)) && (!petir_tools::black_attacked(data, petir_header::d8))) && (!petir_tools::black_attacked(data, petir_header::c8)))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(petir_header::_rokadepanjang, 7));
            movelist[jml].score = petir_header::historyb[petir_header::_rajahitam][petir_header::_rokadepanjang];
        }
    }
    jml_out = jml;
}

// movgen.pas:868
void black_movgen_caps(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml_out) {
    // Count locally: MSVC would otherwise reload the caller's count after
    // every move-list store. The count is published once on return.
    std::int32_t jml = jml_out;
    std::uint64_t piecepos{};
    std::uint64_t attack{};
    std::int32_t pos{};
    std::int32_t post{};
    std::int32_t a{};
    std::int32_t* p{};
    jml = 0;
    p = pas::address(movelist);
    piecepos = (data.pawnblack & petir_bitboard_mask::rank_mask[1]);
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        if ((((pos & 7) > 0) && (data.papan[pas::sub(pos, 9)] > 0))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::sub(pos, 9), 7)), pas::shl(petir_header::promosi_mentri, 14));
            movelist[jml].score = pas::add(pas::sub(petir_movgen::nilai_piece[data.papan[pas::sub(pos, 9)]], petir_movgen::_nilai_pion), 5);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::sub(pos, 9), 7)), pas::shl(petir_header::promosi_kuda, 14));
            movelist[jml].score = pas::add(pas::sub(petir_movgen::nilai_piece[data.papan[pas::sub(pos, 9)]], petir_movgen::_nilai_pion), 5);
        }
        if ((((pos & 7) < 7) && (data.papan[pas::sub(pos, 7)] > 0))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::sub(pos, 7), 7)), pas::shl(petir_header::promosi_mentri, 14));
            movelist[jml].score = pas::add(pas::sub(petir_movgen::nilai_piece[data.papan[pas::sub(pos, 7)]], petir_movgen::_nilai_pion), 5);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::sub(pos, 7), 7)), pas::shl(petir_header::promosi_kuda, 14));
            movelist[jml].score = pas::add(pas::sub(petir_movgen::nilai_piece[data.papan[pas::sub(pos, 7)]], petir_movgen::_nilai_pion), 5);
        }
        if ((data.papan[pas::sub(pos, 8)] == 0)) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::sub(pos, 8), 7)), pas::shl(petir_header::promosi_mentri, 14));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, pas::sub(pos, 8));
            if ((movelist[jml].score == 0)) {
                movelist[jml].score = 3;
            }
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::sub(pos, 8), 7)), pas::shl(petir_header::promosi_kuda, 14));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, pas::sub(pos, 8));
            if ((movelist[jml].score == 0)) {
                movelist[jml].score = 3;
            }
        }
    }
    piecepos = (data.pawnblack & (~petir_bitboard_mask::rank_mask[1]));
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        a = (pos & 7);
        if (((data.papan[pas::sub(pos, 9)] > 0) && (a > 0))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(pas::sub(pos, 9), 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[pas::sub(pos, 9)]], petir_movgen::_nilai_pion);
        }
        if (((data.papan[pas::sub(pos, 7)] > 0) && (a < 7))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(pas::sub(pos, 7), 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[pas::sub(pos, 7)]], petir_movgen::_nilai_pion);
        }
        if (((pas::shr(pos, 3) == 3) && (pas::abs(pas::sub((pos & 7), data.ep)) == 1))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(petir_header::_en_passant, 7));
            movelist[jml].score = petir_movgen::_nilai_pion;
        }
    }
    piecepos = data.knightblack;
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = (petir_bitboard_mask::knightmask[pos] & data.whitepieces);
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_kuda);
            if ((movelist[jml].score <= 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
            }
        }
    }
    piecepos = data.bishopblack;
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        attack = (attack & data.whitepieces);
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_gajah);
            if ((movelist[jml].score <= 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
            }
        }
    }
    piecepos = data.rookblack;
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack & data.whitepieces);
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_benteng);
            if ((movelist[jml].score <= 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
            }
        }
    }
    piecepos = data.queenblack;
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack | petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)]);
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        attack = (attack & data.whitepieces);
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
        }
    }
    pos = petir_bitboard_mask::king_square(data.kingblack);
    attack = (petir_bitboard_mask::kingmask[pos] & data.whitepieces);
    for(move_bits<true> remaining(attack); remaining; ) {
        post = remaining.pop();
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
        movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
    }
    jml_out = jml;
}

// movgen.pas:1041
void black_movgen_caps2(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml_out) {
    // Count locally: MSVC would otherwise reload the caller's count after
    // every move-list store. The count is published once on return.
    std::int32_t jml = jml_out;
    std::uint64_t piecepos{};
    std::uint64_t attack{};
    std::int32_t pos{};
    std::int32_t post{};
    std::int32_t a{};
    jml = 0;
    piecepos = (data.pawnblack & petir_bitboard_mask::rank_mask[1]);
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        if ((((pos & 7) > 0) && (data.papan[pas::sub(pos, 9)] > 0))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::sub(pos, 9), 7)), pas::shl(petir_header::promosi_mentri, 14));
            movelist[jml].score = pas::add(pas::sub(petir_movgen::nilai_piece[data.papan[pas::sub(pos, 9)]], petir_movgen::_nilai_pion), 5);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::sub(pos, 9), 7)), pas::shl(petir_header::promosi_kuda, 14));
            movelist[jml].score = pas::add(pas::sub(petir_movgen::nilai_piece[data.papan[pas::sub(pos, 9)]], petir_movgen::_nilai_pion), 5);
        }
        if ((((pos & 7) < 7) && (data.papan[pas::sub(pos, 7)] > 0))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::sub(pos, 7), 7)), pas::shl(petir_header::promosi_mentri, 14));
            movelist[jml].score = pas::add(pas::sub(petir_movgen::nilai_piece[data.papan[pas::sub(pos, 7)]], petir_movgen::_nilai_pion), 5);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::sub(pos, 7), 7)), pas::shl(petir_header::promosi_kuda, 14));
            movelist[jml].score = pas::add(pas::sub(petir_movgen::nilai_piece[data.papan[pas::sub(pos, 7)]], petir_movgen::_nilai_pion), 5);
        }
        if ((data.papan[pas::sub(pos, 8)] == 0)) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::sub(pos, 8), 7)), pas::shl(petir_header::promosi_mentri, 14));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, pas::sub(pos, 8));
            if ((movelist[jml].score == 0)) {
                movelist[jml].score = 3;
            }
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::sub(pos, 8), 7)), pas::shl(petir_header::promosi_kuda, 14));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, pas::sub(pos, 8));
            if ((movelist[jml].score == 0)) {
                movelist[jml].score = 3;
            }
        }
    }
    piecepos = (data.pawnblack & (petir_bitboard_mask::rank_mask[2] | petir_bitboard_mask::rank_mask[3]));
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        if ((data.papan[pas::sub(pos, 8)] == 0)) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(pas::sub(pos, 8), 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, pas::sub(pos, 8));
            if ((movelist[jml].score == 0)) {
                movelist[jml].score = 1;
            }
        }
    }
    piecepos = (data.pawnblack & (~petir_bitboard_mask::rank_mask[1]));
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        a = (pos & 7);
        if (((data.papan[pas::sub(pos, 9)] > 0) && (a > 0))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(pas::sub(pos, 9), 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[pas::sub(pos, 9)]], petir_movgen::_nilai_pion);
        }
        if (((data.papan[pas::sub(pos, 7)] > 0) && (a < 7))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(pas::sub(pos, 7), 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[pas::sub(pos, 7)]], petir_movgen::_nilai_pion);
        }
        if (((pas::shr(pos, 3) == 3) && (pas::abs(pas::sub((pos & 7), data.ep)) == 1))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(petir_header::_en_passant, 7));
            movelist[jml].score = petir_movgen::_nilai_pion;
        }
    }
    piecepos = data.knightblack;
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = (petir_bitboard_mask::knightmask[pos] & data.whitepieces);
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_kuda);
            if ((movelist[jml].score <= 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
            }
        }
    }
    piecepos = data.bishopblack;
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        attack = (attack & data.whitepieces);
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_gajah);
            if ((movelist[jml].score <= 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
            }
        }
    }
    piecepos = data.rookblack;
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack & data.whitepieces);
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_benteng);
            if ((movelist[jml].score <= 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
            }
        }
    }
    piecepos = data.queenblack;
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack | petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)]);
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        attack = (attack & data.whitepieces);
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
        }
    }
    pos = petir_bitboard_mask::king_square(data.kingblack);
    attack = (petir_bitboard_mask::kingmask[pos] & data.whitepieces);
    for(move_bits<true> remaining(attack); remaining; ) {
        post = remaining.pop();
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
        movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
    }
    attack = (petir_bitboard_mask::kingmask[pos] & pas::shl(data.pawnwhite, 8));
    for(move_bits<false> remaining(attack); remaining; ) {
        post = remaining.pop();
        if (((data.papan[post] == 0) && ((petir_bitboard_mask::w_pion_bebas_mask[pas::sub(post, 8)] & data.pawnblack) == 0))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = 0;
        }
    }
    jml_out = jml;
}

// movgen.pas:1238
void q_black_movgen(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml_out) {
    // Count locally: MSVC would otherwise reload the caller's count after
    // every move-list store. The count is published once on return.
    std::int32_t jml = jml_out;
    std::uint64_t piecepos{};
    std::uint64_t attack{};
    std::uint64_t t64{};
    std::int32_t pos{};
    std::int32_t post{};
    std::int32_t a{};
    std::int32_t posraja{};
    std::uint64_t attackb{};
    std::uint64_t attackr{};
    std::uint64_t attackn{};
    jml = 0;
    posraja = petir_bitboard_mask::king_square(data.kingwhite);
    attackb = petir_bitboard_mask::diaga8h1mask[posraja][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[posraja]) & 255)];
    attackb = (attackb | petir_bitboard_mask::diagh8a1mask[posraja][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[posraja]) & 255)]);
    attackr = petir_bitboard_mask::horzmask2[posraja][(pas::shr(data.allpieces, pas::add((posraja & 56), 1)) & 63)];
    attackr = (attackr | petir_bitboard_mask::vertmask2[posraja][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[posraja]) & 63)]);
    attackn = petir_bitboard_mask::knightmask[posraja];
    piecepos = (pas::shr((data.pawnblack & petir_bitboard_mask::mask_left), 17) & data.kingwhite);
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        (pos = pas::increment(pos, 17));
        if ((data.papan[pas::sub(pos, 8)] == 0)) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(pas::sub(pos, 8), 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, pas::sub(pos, 8));
        }
    }
    piecepos = (pas::shr((data.pawnblack & petir_bitboard_mask::mask_right), 15) & data.kingwhite);
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        (pos = pas::increment(pos, 15));
        if ((data.papan[pas::sub(pos, 8)] == 0)) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(pas::sub(pos, 8), 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, pas::sub(pos, 8));
        }
    }
    piecepos = (pas::shr((data.pawnblack & petir_bitboard_mask::mask_left), 9) & data.whitepieces);
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        pos = pas::add(pos, 9);
        if ((pas::shr(pos, 3) == 1)) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::sub(pos, 9), 7)), pas::shl(petir_header::promosi_mentri, 14));
            movelist[jml].score = pas::add(pas::sub(petir_movgen::nilai_piece[data.papan[pas::sub(pos, 9)]], petir_movgen::_nilai_pion), 3);
        } else {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(pas::sub(pos, 9), 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[pas::sub(pos, 9)]], petir_movgen::_nilai_pion);
        }
    }
    piecepos = (pas::shr((data.pawnblack & petir_bitboard_mask::mask_right), 7) & data.whitepieces);
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        pos = pas::add(pos, 7);
        if ((pas::shr(pos, 3) == 1)) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::sub(pos, 7), 7)), pas::shl(petir_header::promosi_mentri, 14));
            movelist[jml].score = pas::add(pas::sub(petir_movgen::nilai_piece[data.papan[pas::sub(pos, 7)]], petir_movgen::_nilai_pion), 3);
        } else {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(pas::sub(pos, 7), 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[pas::sub(pos, 7)]], petir_movgen::_nilai_pion);
        }
    }
    piecepos = (data.pawnblack & petir_bitboard_mask::rank_mask[1]);
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        if ((data.papan[pas::sub(pos, 8)] == 0)) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::sub(pos, 8), 7)), pas::shl(petir_header::promosi_mentri, 14));
            movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, pas::sub(pos, 8));
            if ((movelist[jml].score == 0)) {
                movelist[jml].score = 3;
            }
        }
    }
    piecepos = (data.pawnblack & (petir_bitboard_mask::rank_mask[2] | petir_bitboard_mask::rank_mask[3]));
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        if ((data.papan[pas::sub(pos, 8)] == 0)) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(pas::sub(pos, 8), 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, pas::sub(pos, 8));
            if ((movelist[jml].score == 0)) {
                movelist[jml].score = 1;
            }
        }
    }
    if ((data.ep != petir_header::_no_ep)) {
        piecepos = (data.pawnblack & petir_bitboard_mask::rank_mask[3]);
        for(move_bits<false> remaining(piecepos); remaining; ) {
            pos = remaining.pop();
            if ((pas::abs(pas::sub((pos & 7), data.ep)) == 1)) {
                (jml = pas::increment(jml, 1));
                movelist[jml].moves = pas::add(pos, pas::shl(petir_header::_en_passant, 7));
                movelist[jml].score = petir_movgen::_nilai_pion;
            }
        }
    }
    piecepos = data.knightblack;
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = (petir_bitboard_mask::knightmask[pos] & data.whitepieces);
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_kuda);
            if ((movelist[jml].score < 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
            }
        }
        attack = ((petir_bitboard_mask::knightmask[pos] & attackn) & (~data.allpieces));
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
        }
    }
    piecepos = data.bishopblack;
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        t64 = attack;
        attack = (attack & data.whitepieces);
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_gajah);
            if ((movelist[jml].score < 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
            }
        }
        attack = ((t64 & attackb) & (~data.allpieces));
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
        }
    }
    piecepos = data.rookblack;
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        t64 = attack;
        attack = (attack & data.whitepieces);
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_benteng);
            if ((movelist[jml].score < 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
            }
        }
        attack = ((t64 & attackr) & (~data.allpieces));
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
        }
    }
    piecepos = data.queenblack;
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack | petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)]);
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        t64 = attack;
        attack = (attack & data.whitepieces);
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
        }
        attack = ((t64 & (attackr | attackb)) & (~data.allpieces));
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
        }
    }
    pos = petir_bitboard_mask::king_square(data.kingblack);
    attack = (petir_bitboard_mask::kingmask[pos] & data.whitepieces);
    for(move_bits<true> remaining(attack); remaining; ) {
        post = remaining.pop();
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
        movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
    }
    attack = (petir_bitboard_mask::kingmask[pos] & pas::shl(data.pawnwhite, 8));
    for(move_bits<false> remaining(attack); remaining; ) {
        post = remaining.pop();
        if (((data.papan[post] == 0) && ((petir_bitboard_mask::w_pion_bebas_mask[pas::sub(post, 8)] & data.pawnblack) == 0))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = 0;
        }
    }
    jml_out = jml;
}

// movgen.pas:2080
void q_white_movgen(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml_out) {
    // Count locally: MSVC would otherwise reload the caller's count after
    // every move-list store. The count is published once on return.
    std::int32_t jml = jml_out;
    std::uint64_t piecepos{};
    std::uint64_t attack{};
    std::uint64_t t64{};
    std::int32_t pos{};
    std::int32_t post{};
    std::int32_t a{};
    std::int32_t posraja{};
    std::uint64_t attackb{};
    std::uint64_t attackr{};
    std::uint64_t attackn{};
    std::uint64_t target{};
    jml = 0;
    posraja = petir_bitboard_mask::king_square(data.kingblack);
    attackb = petir_bitboard_mask::diaga8h1mask[posraja][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[posraja]) & 255)];
    attackb = (attackb | petir_bitboard_mask::diagh8a1mask[posraja][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[posraja]) & 255)]);
    attackr = petir_bitboard_mask::horzmask2[posraja][(pas::shr(data.allpieces, pas::add((posraja & 56), 1)) & 63)];
    attackr = (attackr | petir_bitboard_mask::vertmask2[posraja][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[posraja]) & 63)]);
    attackn = petir_bitboard_mask::knightmask[posraja];
    piecepos = (pas::shl((data.pawnwhite & petir_bitboard_mask::mask_left), 15) & data.kingblack);
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        (pos = pas::decrement(pos, 15));
        if ((data.papan[pas::add(pos, 8)] == 0)) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(pas::add(pos, 8), 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, pas::add(pos, 8));
        }
    }
    piecepos = (pas::shl((data.pawnwhite & petir_bitboard_mask::mask_right), 17) & data.kingblack);
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        (pos = pas::decrement(pos, 17));
        if ((data.papan[pas::add(pos, 8)] == 0)) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(pas::add(pos, 8), 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, pas::add(pos, 8));
        }
    }
    piecepos = (pas::shl((data.pawnwhite & petir_bitboard_mask::mask_left), 7) & data.blackpieces);
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        pos = pas::sub(pos, 7);
        if ((pas::shr(pos, 3) == 6)) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::add(pos, 7), 7)), pas::shl(petir_header::promosi_mentri, 14));
            movelist[jml].score = pas::add(pas::sub(petir_movgen::nilai_piece[data.papan[pas::add(pos, 7)]], petir_movgen::_nilai_pion), 3);
        } else {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(pas::add(pos, 7), 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[pas::add(pos, 7)]], petir_movgen::_nilai_pion);
        }
    }
    piecepos = (pas::shl((data.pawnwhite & petir_bitboard_mask::mask_right), 9) & data.blackpieces);
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        pos = pas::sub(pos, 9);
        if ((pas::shr(pos, 3) == 6)) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::add(pos, 9), 7)), pas::shl(petir_header::promosi_mentri, 14));
            movelist[jml].score = pas::add(pas::sub(petir_movgen::nilai_piece[data.papan[pas::add(pos, 9)]], petir_movgen::_nilai_pion), 3);
        } else {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(pas::add(pos, 9), 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[pas::add(pos, 9)]], petir_movgen::_nilai_pion);
        }
    }
    piecepos = (data.pawnwhite & petir_bitboard_mask::rank_mask[6]);
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        if ((data.papan[pas::add(pos, 8)] == 0)) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::add(pos, 8), 7)), pas::shl(petir_header::promosi_mentri, 14));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, pas::add(pos, 8));
            if ((movelist[jml].score == 0)) {
                movelist[jml].score = 3;
            }
        }
    }
    piecepos = (data.pawnwhite & (petir_bitboard_mask::rank_mask[5] | petir_bitboard_mask::rank_mask[4]));
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        if ((data.papan[pas::add(pos, 8)] == 0)) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(pas::add(pos, 8), 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, pas::add(pos, 8));
            if ((movelist[jml].score == 0)) {
                movelist[jml].score = 1;
            }
        }
    }
    if ((data.ep != petir_header::_no_ep)) {
        piecepos = (data.pawnwhite & petir_bitboard_mask::rank_mask[4]);
        for(move_bits<false> remaining(piecepos); remaining; ) {
            pos = remaining.pop();
            if ((pas::abs(pas::sub((pos & 7), data.ep)) == 1)) {
                (jml = pas::increment(jml, 1));
                movelist[jml].moves = pas::add(pos, pas::shl(petir_header::_en_passant, 7));
                movelist[jml].score = petir_movgen::_nilai_pion;
            }
        }
    }
    piecepos = data.knightwhite;
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = (petir_bitboard_mask::knightmask[pos] & data.blackpieces);
        for(move_bits<false> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_kuda);
            if ((movelist[jml].score < 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
            }
        }
        attack = ((petir_bitboard_mask::knightmask[pos] & attackn) & (~data.allpieces));
        for(move_bits<false> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
        }
    }
    piecepos = data.bishopwhite;
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        t64 = attack;
        attack = (attack & data.blackpieces);
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_gajah);
            if ((movelist[jml].score < 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
            }
        }
        attack = ((t64 & attackb) & (~data.allpieces));
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
        }
    }
    piecepos = data.rookwhite;
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        t64 = attack;
        attack = (attack & data.blackpieces);
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_benteng);
            if ((movelist[jml].score < 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
            }
        }
        attack = ((t64 & attackr) & (~data.allpieces));
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
        }
    }
    piecepos = data.queenwhite;
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack | petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)]);
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        t64 = attack;
        attack = (attack & data.blackpieces);
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
        }
        attack = ((t64 & (attackb | attackr)) & (~data.allpieces));
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
        }
    }
    pos = petir_bitboard_mask::king_square(data.kingwhite);
    attack = (petir_bitboard_mask::kingmask[pos] & data.blackpieces);
    for(move_bits<false> remaining(attack); remaining; ) {
        post = remaining.pop();
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
        movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
    }
    attack = (petir_bitboard_mask::kingmask[pos] & pas::shr(data.pawnblack, 8));
    for(move_bits<false> remaining(attack); remaining; ) {
        post = remaining.pop();
        if (((data.papan[post] == 0) && ((petir_bitboard_mask::b_pion_bebas_mask[pas::add(post, 8)] & data.pawnwhite) == 0))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = 0;
        }
    }
    jml_out = jml;
}

// movgen.pas:1681
void white_movgen_caps(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml_out) {
    // Count locally: MSVC would otherwise reload the caller's count after
    // every move-list store. The count is published once on return.
    std::int32_t jml = jml_out;
    std::uint64_t piecepos{};
    std::uint64_t attack{};
    std::uint64_t temp{};
    std::int32_t pos{};
    std::int32_t post{};
    std::int32_t a{};
    jml = 0;
    piecepos = (data.pawnwhite & petir_bitboard_mask::rank_mask[6]);
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        if ((((pos & 7) < 7) && (data.papan[pas::add(pos, 9)] < 0))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::add(pos, 9), 7)), pas::shl(petir_header::promosi_mentri, 14));
            movelist[jml].score = pas::add(pas::sub(petir_movgen::nilai_piece[data.papan[pas::add(pos, 9)]], petir_movgen::_nilai_pion), 5);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::add(pos, 9), 7)), pas::shl(petir_header::promosi_kuda, 14));
            movelist[jml].score = pas::add(pas::sub(petir_movgen::nilai_piece[data.papan[pas::add(pos, 9)]], petir_movgen::_nilai_pion), 5);
        }
        if ((((pos & 7) > 0) && (data.papan[pas::add(pos, 7)] < 0))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::add(pos, 7), 7)), pas::shl(petir_header::promosi_mentri, 14));
            movelist[jml].score = pas::add(pas::sub(petir_movgen::nilai_piece[data.papan[pas::add(pos, 7)]], petir_movgen::_nilai_pion), 5);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::add(pos, 7), 7)), pas::shl(petir_header::promosi_kuda, 14));
            movelist[jml].score = pas::add(pas::sub(petir_movgen::nilai_piece[data.papan[pas::add(pos, 7)]], petir_movgen::_nilai_pion), 5);
        }
        if ((data.papan[pas::add(pos, 8)] == 0)) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::add(pos, 8), 7)), pas::shl(petir_header::promosi_mentri, 14));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, pas::add(pos, 8));
            if ((movelist[jml].score == 0)) {
                movelist[jml].score = 3;
            }
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::add(pos, 8), 7)), pas::shl(petir_header::promosi_kuda, 14));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, pas::add(pos, 8));
            if ((movelist[jml].score == 0)) {
                movelist[jml].score = 3;
            }
        }
    }
    temp = (data.pawnwhite & (~petir_bitboard_mask::rank_mask[6]));
    piecepos = (pas::shl((temp & petir_bitboard_mask::mask_left), 7) & data.blackpieces);
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        pos = pas::sub(pos, 7);
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(pos, pas::shl(pas::add(pos, 7), 7));
        movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[pas::add(pos, 7)]], petir_movgen::_nilai_pion);
    }
    piecepos = (pas::shl((temp & petir_bitboard_mask::mask_right), 9) & data.blackpieces);
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        pos = pas::sub(pos, 9);
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(pos, pas::shl(pas::add(pos, 9), 7));
        movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[pas::add(pos, 9)]], petir_movgen::_nilai_pion);
    }
    if ((data.ep != petir_header::_no_ep)) {
        piecepos = (data.pawnwhite & petir_bitboard_mask::rank_mask[4]);
        for(move_bits<false> remaining(piecepos); remaining; ) {
            pos = remaining.pop();
            if ((pas::abs(pas::sub((pos & 7), data.ep)) == 1)) {
                (jml = pas::increment(jml, 1));
                movelist[jml].moves = pas::add(pos, pas::shl(petir_header::_en_passant, 7));
                movelist[jml].score = petir_movgen::_nilai_pion;
            }
        }
    }
    piecepos = data.knightwhite;
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = (petir_bitboard_mask::knightmask[pos] & data.blackpieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::firstbitp(pas::address(attack));
            attack = (attack & (~petir_bitboard_mask::bit(post)));
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_kuda);
            if ((movelist[jml].score <= 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
            }
        }
    }
    piecepos = data.bishopwhite;
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        attack = (attack & data.blackpieces);
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_gajah);
            if ((movelist[jml].score <= 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
            }
        }
    }
    piecepos = data.rookwhite;
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack & data.blackpieces);
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_benteng);
            if ((movelist[jml].score <= 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
            }
        }
    }
    piecepos = data.queenwhite;
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack | petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)]);
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        attack = (attack & data.blackpieces);
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
        }
    }
    pos = petir_bitboard_mask::king_square(data.kingwhite);
    attack = (petir_bitboard_mask::kingmask[pos] & data.blackpieces);
    for(move_bits<false> remaining(attack); remaining; ) {
        post = remaining.pop();
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
        movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
    }
    jml_out = jml;
}

// movgen.pas:1864
void white_movgen_caps2(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml_out) {
    // Count locally: MSVC would otherwise reload the caller's count after
    // every move-list store. The count is published once on return.
    std::int32_t jml = jml_out;
    std::uint64_t piecepos{};
    std::uint64_t attack{};
    std::uint64_t temp{};
    std::int32_t pos{};
    std::int32_t post{};
    std::int32_t a{};
    jml = 0;
    piecepos = (data.pawnwhite & petir_bitboard_mask::rank_mask[6]);
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        if ((((pos & 7) < 7) && (data.papan[pas::add(pos, 9)] < 0))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::add(pos, 9), 7)), pas::shl(petir_header::promosi_mentri, 14));
            movelist[jml].score = pas::add(pas::sub(petir_movgen::nilai_piece[data.papan[pas::add(pos, 9)]], petir_movgen::_nilai_pion), 5);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::add(pos, 9), 7)), pas::shl(petir_header::promosi_kuda, 14));
            movelist[jml].score = pas::add(pas::sub(petir_movgen::nilai_piece[data.papan[pas::add(pos, 9)]], petir_movgen::_nilai_pion), 5);
        }
        if ((((pos & 7) > 0) && (data.papan[pas::add(pos, 7)] < 0))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::add(pos, 7), 7)), pas::shl(petir_header::promosi_mentri, 14));
            movelist[jml].score = pas::add(pas::sub(petir_movgen::nilai_piece[data.papan[pas::add(pos, 7)]], petir_movgen::_nilai_pion), 5);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::add(pos, 7), 7)), pas::shl(petir_header::promosi_kuda, 14));
            movelist[jml].score = pas::add(pas::sub(petir_movgen::nilai_piece[data.papan[pas::add(pos, 7)]], petir_movgen::_nilai_pion), 5);
        }
        if ((data.papan[pas::add(pos, 8)] == 0)) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::add(pos, 8), 7)), pas::shl(petir_header::promosi_mentri, 14));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, pas::add(pos, 8));
            if ((movelist[jml].score == 0)) {
                movelist[jml].score = 3;
            }
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(pas::add(pos, 8), 7)), pas::shl(petir_header::promosi_kuda, 14));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, pas::add(pos, 8));
            if ((movelist[jml].score == 0)) {
                movelist[jml].score = 3;
            }
        }
    }
    piecepos = (data.pawnwhite & (petir_bitboard_mask::rank_mask[5] | petir_bitboard_mask::rank_mask[4]));
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        if ((data.papan[pas::add(pos, 8)] == 0)) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(pas::add(pos, 8), 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, pas::add(pos, 8));
            if ((movelist[jml].score == 0)) {
                movelist[jml].score = 1;
            }
        }
    }
    temp = (data.pawnwhite & (~petir_bitboard_mask::rank_mask[6]));
    piecepos = (pas::shl((temp & petir_bitboard_mask::mask_left), 7) & data.blackpieces);
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        pos = pas::sub(pos, 7);
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(pos, pas::shl(pas::add(pos, 7), 7));
        movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[pas::add(pos, 7)]], petir_movgen::_nilai_pion);
    }
    piecepos = (pas::shl((temp & petir_bitboard_mask::mask_right), 9) & data.blackpieces);
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        pos = pas::sub(pos, 9);
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(pos, pas::shl(pas::add(pos, 9), 7));
        movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[pas::add(pos, 9)]], petir_movgen::_nilai_pion);
    }
    if ((data.ep != petir_header::_no_ep)) {
        piecepos = (data.pawnwhite & petir_bitboard_mask::rank_mask[4]);
        for(move_bits<false> remaining(piecepos); remaining; ) {
            pos = remaining.pop();
            if ((pas::abs(pas::sub((pos & 7), data.ep)) == 1)) {
                (jml = pas::increment(jml, 1));
                movelist[jml].moves = pas::add(pos, pas::shl(petir_header::_en_passant, 7));
                movelist[jml].score = petir_movgen::_nilai_pion;
            }
        }
    }
    piecepos = data.knightwhite;
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = (petir_bitboard_mask::knightmask[pos] & data.blackpieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::firstbitp(pas::address(attack));
            attack = (attack & (~petir_bitboard_mask::bit(post)));
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_kuda);
            if ((movelist[jml].score <= 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
            }
        }
    }
    piecepos = data.bishopwhite;
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        attack = (attack & data.blackpieces);
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_gajah);
            if ((movelist[jml].score <= 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
            }
        }
    }
    piecepos = data.rookwhite;
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack & data.blackpieces);
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_benteng);
            if ((movelist[jml].score <= 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
            }
        }
    }
    piecepos = data.queenwhite;
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack | petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)]);
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        attack = (attack & data.blackpieces);
        for(move_bits<true> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
        }
    }
    pos = petir_bitboard_mask::king_square(data.kingwhite);
    attack = (petir_bitboard_mask::kingmask[pos] & data.blackpieces);
    for(move_bits<false> remaining(attack); remaining; ) {
        post = remaining.pop();
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
        movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
    }
    attack = (petir_bitboard_mask::kingmask[pos] & pas::shr(data.pawnblack, 8));
    for(move_bits<false> remaining(attack); remaining; ) {
        post = remaining.pop();
        if (((data.papan[post] == 0) && ((petir_bitboard_mask::b_pion_bebas_mask[pas::add(post, 8)] & data.pawnwhite) == 0))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = 0;
        }
    }
    jml_out = jml;
}

// movgen.pas:1520
void white_movgen_noncaps(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml_out, std::int32_t depth) {
    // Count locally: MSVC would otherwise reload the caller's count after
    // every move-list store. The count is published once on return.
    std::int32_t jml = jml_out;
    std::uint64_t piecepos{};
    std::uint64_t attack{};
    std::int32_t pos{};
    std::int32_t post{};
    jml = 0;
    piecepos = (data.pawnwhite & (~petir_bitboard_mask::rank_mask[6]));
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        if ((data.papan[pas::add(pos, 8)] == 0)) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(pas::add(pos, 8), 7));
            movelist[jml].score = petir_header::historyw[petir_header::_pionputih][pas::add(pos, 8)];
            if (((pos <= petir_header::h2) && (data.papan[pas::add(pos, 16)] == 0))) {
                (jml = pas::increment(jml, 1));
                movelist[jml].moves = pas::add(pos, pas::shl(pas::add(pos, 16), 7));
                movelist[jml].score = petir_header::historyw[petir_header::_pionputih][pas::add(pos, 16)];
            }
        }
    }
    piecepos = data.knightwhite;
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = (petir_bitboard_mask::knightmask[pos] & (~data.allpieces));
        for(move_bits<false> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_header::historyw[petir_header::_kudaputih][post];
            if (((depth <= 3) && (petir_usee::see(petir_header::_sisiputih, data, pos, post) < 0))) {
                movelist[jml].score = 0;
            }
        }
    }
    piecepos = data.queenwhite;
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack | petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)]);
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        attack = (attack & (~data.allpieces));
        for(move_bits<false> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_header::historyw[petir_header::_mentriputih][post];
            if (((depth <= 3) && (petir_usee::see(petir_header::_sisiputih, data, pos, post) < 0))) {
                movelist[jml].score = 0;
            }
        }
    }
    piecepos = data.bishopwhite;
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        attack = (attack & (~data.allpieces));
        for(move_bits<false> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_header::historyw[petir_header::_gajahputih][post];
            if (((depth <= 3) && (petir_usee::see(petir_header::_sisiputih, data, pos, post) < 0))) {
                movelist[jml].score = 0;
            }
        }
    }
    piecepos = data.rookwhite;
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack & (~data.allpieces));
        for(move_bits<false> remaining(attack); remaining; ) {
            post = remaining.pop();
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_header::historyw[petir_header::_bentengputih][post];
            if (((depth <= 3) && (petir_usee::see(petir_header::_sisiputih, data, pos, post) < 0))) {
                movelist[jml].score = 0;
            }
        }
    }
    pos = petir_bitboard_mask::king_square(data.kingwhite);
    attack = (petir_bitboard_mask::kingmask[pos] & (~data.allpieces));
    for(move_bits<false> remaining(attack); remaining; ) {
        post = remaining.pop();
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
        movelist[jml].score = petir_header::historyw[petir_header::_rajaputih][post];
    }
    if (((((data.flagrokade & petir_header::bitputihnoc) != 0) && (data.papan[petir_header::e1] == petir_header::_rajaputih)) && (!petir_tools::white_attacked(data, petir_header::e1)))) {
        if (((((((data.papan[petir_header::f1] == 0) && (data.papan[petir_header::g1] == 0)) && ((data.flagrokade & petir_header::bitputihsc) != 0)) && (data.papan[petir_header::h1] == petir_header::_bentengputih)) && (!petir_tools::white_attacked(data, petir_header::f1))) && (!petir_tools::white_attacked(data, petir_header::g1)))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(petir_header::_rokadependek, 7));
            movelist[jml].score = petir_header::historyw[petir_header::_rajaputih][petir_header::_rokadependek];
        }
        if ((((((((data.papan[petir_header::d1] == 0) && (data.papan[petir_header::c1] == 0)) && (data.papan[petir_header::b1] == 0)) && ((data.flagrokade & petir_header::bitputihlc) != 0)) && (data.papan[petir_header::a1] == petir_header::_bentengputih)) && (!petir_tools::white_attacked(data, petir_header::d1))) && (!petir_tools::white_attacked(data, petir_header::c1)))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(petir_header::_rokadepanjang, 7));
            movelist[jml].score = petir_header::historyw[petir_header::_rajaputih][petir_header::_rokadepanjang];
        }
    }
    jml_out = jml;
}

// movgen.pas:30
void white_evasion(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml_out, std::int32_t& skakcount) {
    // Count locally: MSVC would otherwise reload the caller's count after
    // every move-list store. The count is published once on return.
    std::int32_t jml = jml_out;
    std::uint64_t piecepos{};
    std::uint64_t attack{};
    std::uint64_t battack{};
    std::uint64_t attack2{};
    std::int32_t pos{};
    std::int32_t post{};
    std::int32_t posraja{};
    std::int32_t dn{};
    std::int32_t a{};
    std::uint64_t skak64{};
    std::int32_t skakpos{};
    std::uint64_t path{};
    std::uint64_t allp{};
    std::uint64_t allp90{};
    std::uint64_t allpa8{};
    std::uint64_t allph8{};
    std::int32_t pas_temp_1{};
    jml = 0;
    skakcount = 0;
    battack = 0;
    posraja = petir_bitboard_mask::king_square(data.kingwhite);
    skakpos = 64;
    piecepos = data.knightblack;
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        battack = (battack | petir_bitboard_mask::knightmask[pos]);
        if (((petir_bitboard_mask::knightmask[pos] & data.kingwhite) != 0)) {
            (skakcount = pas::increment(skakcount, 1));
            skakpos = pos;
        }
    }
    piecepos = data.pawnblack;
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::b_pawn_attack[pos];
        battack = (battack | attack);
        if (((attack & data.kingwhite) != 0)) {
            (skakcount = pas::increment(skakcount, 1));
            skakpos = pos;
        }
    }
    allp = (data.allpieces & (~data.kingwhite));
    allp90 = (data.allpiecesr90 & (~petir_bitboard_mask::bit2nmask90[posraja]));
    allpa8 = (data.allpiecesa8h1 & (~petir_bitboard_mask::bit2nmaska8h1[posraja]));
    allph8 = (data.allpiecesh8a1 & (~petir_bitboard_mask::bit2nmaskh8a1[posraja]));
    piecepos = data.bishopblack;
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(allpa8, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(allph8, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        battack = (battack | attack);
        if (((attack & data.kingwhite) != 0)) {
            (skakcount = pas::increment(skakcount, 1));
            skakpos = pos;
        }
    }
    piecepos = data.rookblack;
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(allp, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(allp90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        battack = (battack | attack);
        if (((attack & data.kingwhite) != 0)) {
            (skakcount = pas::increment(skakcount, 1));
            skakpos = pos;
        }
    }
    piecepos = data.queenblack;
    for(move_bits<true> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(allp, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(allp90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack | petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(allpa8, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)]);
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(allph8, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        battack = (battack | attack);
        if (((attack & data.kingwhite) != 0)) {
            (skakcount = pas::increment(skakcount, 1));
            skakpos = pos;
        }
    }
    pos = petir_bitboard_mask::king_square(data.kingblack);
    battack = (battack | petir_bitboard_mask::kingmask[pos]);
    piecepos = ((petir_bitboard_mask::kingmask[posraja] & (~battack)) & (~data.whitepieces));
    for(move_bits<true> remaining(piecepos); remaining; ) {
        post = remaining.pop();
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(posraja, pas::shl(post, 7));
        if ((data.papan[post] == 0)) {
            movelist[jml].score = petir_header::historyw[petir_header::_rajaputih][post];
        } else {
            movelist[jml].score = pas::shl(petir_movgen::nilai_piece[data.papan[post]], 14);
        }
    }
    // Removing both pawns can uncover an attack; making the move also handles
    // en passant interpositions against a sliding-piece check.
    append_legal_ep_evasions(data,movelist,jml,petir_header::_sisiputih);
    if ((skakcount == 1)) {
        skak64 = petir_bitboard_mask::bit(skakpos);
        path = petir_bitboard_mask::pathmask[posraja][skakpos];
        piecepos = data.knightwhite;
        dn = petir_bitboard_mask::direction[posraja][skakpos];
        for(move_bits<false> remaining(piecepos); remaining; ) {
            pos = remaining.pop();
            attack = petir_bitboard_mask::knightmask[pos];
            attack2 = (attack & skak64);
            if ((attack2 != 0)) {
                post = petir_bitboard_mask::firstbitp(pas::address(attack2));
                if ((!petir_tools::white_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    (jml = pas::increment(jml, 1));
                    movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                    movelist[jml].score = pas::shl(pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_kuda), 15);
                    if ((movelist[jml].score < 0)) {
                        movelist[jml].score = pas::shl(petir_usee::see(petir_header::_sisiputih, data, pos, post), 15);
                    }
                }
            }
            attack2 = (attack & path);
            for(move_bits<false> remaining(attack2); remaining; ) {
                post = remaining.pop();
                if ((!petir_tools::white_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    (jml = pas::increment(jml, 1));
                    movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                    movelist[jml].score = petir_header::historyw[petir_header::_kudaputih][post];
                }
            }
        }
        piecepos = data.pawnwhite;
        for(move_bits<false> remaining(piecepos); remaining; ) {
            pos = remaining.pop();
            attack = petir_bitboard_mask::w_pawn_attack[pos];
            attack2 = (attack & skak64);
            if ((attack2 != 0)) {
                post = petir_bitboard_mask::firstbitp(pas::address(attack2));
                if ((!petir_tools::white_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    if ((pas::shr(pos, 3) == 6)) {
                        (jml = pas::increment(jml, 1));
                        movelist[jml].moves = pas::add(pas::add(pos, pas::shl(post, 7)), pas::shl(petir_header::promosi_mentri, 14));
                        movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_pion);
                        (jml = pas::increment(jml, 1));
                        movelist[jml].moves = pas::add(pas::add(pos, pas::shl(post, 7)), pas::shl(petir_header::promosi_kuda, 14));
                        movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_pion);
                    } else {
                        (jml = pas::increment(jml, 1));
                        movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                        movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_pion);
                    }
                }
            }
            attack2 = 0;
            if ((data.papan[pas::add(pos, 8)] == 0)) {
                attack2 = (attack2 | petir_bitboard_mask::bit(pas::add(pos, 8)));
                if (((pas::shr(pos, 3) == 1) && (data.papan[pas::add(pos, 16)] == 0))) {
                    attack2 = (attack2 | petir_bitboard_mask::bit(pas::add(pos, 16)));
                }
            }
            attack2 = (attack2 & path);
            if ((attack2 != 0)) {
                post = petir_bitboard_mask::firstbitp(pas::address(attack2));
                if ((!petir_tools::white_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    if ((pas::shr(pos, 3) == 6)) {
                        pas_temp_1 = petir_header::promosi_kuda;
                        for (a = petir_header::promosi_mentri; a <= pas_temp_1; ++a) {
                            (jml = pas::increment(jml, 1));
                            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(post, 7)), pas::shl(a, 14));
                            movelist[jml].score = pas::shl(petir_usee::see(petir_header::_sisiputih, data, pos, post), 15);
                            if ((movelist[jml].score == 0)) {
                                movelist[jml].score = pas::shl(3, 15);
                            }
                        }
                    } else {
                        (jml = pas::increment(jml, 1));
                        movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                        movelist[jml].score = petir_header::historyw[petir_header::_pionputih][post];
                    }
                }
            }
        }
        piecepos = data.bishopwhite;
        for(move_bits<false> remaining(piecepos); remaining; ) {
            pos = remaining.pop();
            attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
            attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
            attack2 = (attack & skak64);
            if ((attack2 != 0)) {
                post = petir_bitboard_mask::firstbitp(pas::address(attack2));
                if ((!petir_tools::white_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    (jml = pas::increment(jml, 1));
                    movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                    movelist[jml].score = pas::shl(pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_gajah), 15);
                    if ((movelist[jml].score < 0)) {
                        movelist[jml].score = pas::shl(petir_usee::see(petir_header::_sisiputih, data, pos, post), 15);
                    }
                }
            }
            attack2 = (attack & path);
            for(move_bits<false> remaining(attack2); remaining; ) {
                post = remaining.pop();
                if ((!petir_tools::white_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    (jml = pas::increment(jml, 1));
                    movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                    movelist[jml].score = petir_header::historyw[petir_header::_gajahputih][post];
                }
            }
        }
        piecepos = data.rookwhite;
        for(move_bits<false> remaining(piecepos); remaining; ) {
            pos = remaining.pop();
            attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
            attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
            attack2 = (attack & skak64);
            if ((attack2 != 0)) {
                post = petir_bitboard_mask::firstbitp(pas::address(attack2));
                if ((!petir_tools::white_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    (jml = pas::increment(jml, 1));
                    movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                    movelist[jml].score = pas::shl(pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_benteng), 15);
                    if ((movelist[jml].score < 0)) {
                        movelist[jml].score = pas::shl(petir_usee::see(petir_header::_sisiputih, data, pos, post), 15);
                    }
                }
            }
            attack2 = (attack & path);
            for(move_bits<false> remaining(attack2); remaining; ) {
                post = remaining.pop();
                if ((!petir_tools::white_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    (jml = pas::increment(jml, 1));
                    movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                    movelist[jml].score = petir_header::historyw[petir_header::_bentengputih][post];
                }
            }
        }
        piecepos = data.queenwhite;
        for(move_bits<false> remaining(piecepos); remaining; ) {
            pos = remaining.pop();
            attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
            attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
            attack = (attack | petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)]);
            attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
            attack2 = (attack & skak64);
            if ((attack2 != 0)) {
                post = petir_bitboard_mask::firstbitp(pas::address(attack2));
                if ((!petir_tools::white_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    (jml = pas::increment(jml, 1));
                    movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                    movelist[jml].score = pas::shl(pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_mentri), 15);
                    if ((movelist[jml].score < 0)) {
                        movelist[jml].score = pas::shl(petir_usee::see(petir_header::_sisiputih, data, pos, post), 15);
                    }
                }
            }
            attack2 = (attack & path);
            for(move_bits<false> remaining(attack2); remaining; ) {
                post = remaining.pop();
                if ((!petir_tools::white_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    (jml = pas::increment(jml, 1));
                    movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                    movelist[jml].score = petir_header::historyw[petir_header::_mentriputih][post];
                }
            }
        }
    }
    jml_out = jml;
}

// movgen.pas:361
void black_evasion(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml_out, std::int32_t& skakcount) {
    // Count locally: MSVC would otherwise reload the caller's count after
    // every move-list store. The count is published once on return.
    std::int32_t jml = jml_out;
    std::uint64_t piecepos{};
    std::uint64_t attack{};
    std::uint64_t battack{};
    std::uint64_t attack2{};
    std::int32_t pos{};
    std::int32_t post{};
    std::int32_t posraja{};
    std::uint64_t skak64{};
    std::int32_t skakpos{};
    std::int32_t a{};
    std::uint64_t path{};
    std::uint64_t allp{};
    std::uint64_t allp90{};
    std::uint64_t allpa8{};
    std::uint64_t allph8{};
    std::int32_t pas_temp_1{};
    std::int32_t pas_temp_2{};
    jml = 0;
    skakcount = 0;
    battack = 0;
    posraja = petir_bitboard_mask::king_square(data.kingblack);
    skakpos = 64;
    piecepos = data.knightwhite;
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        battack = (battack | petir_bitboard_mask::knightmask[pos]);
        if (((petir_bitboard_mask::knightmask[pos] & data.kingblack) != 0)) {
            (skakcount = pas::increment(skakcount, 1));
            skakpos = pos;
        }
    }
    piecepos = data.pawnwhite;
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::w_pawn_attack[pos];
        battack = (battack | attack);
        if (((attack & data.kingblack) != 0)) {
            (skakcount = pas::increment(skakcount, 1));
            skakpos = pos;
        }
    }
    allp = (data.allpieces & (~data.kingblack));
    allp90 = (data.allpiecesr90 & (~petir_bitboard_mask::bit2nmask90[posraja]));
    allpa8 = (data.allpiecesa8h1 & (~petir_bitboard_mask::bit2nmaska8h1[posraja]));
    allph8 = (data.allpiecesh8a1 & (~petir_bitboard_mask::bit2nmaskh8a1[posraja]));
    piecepos = data.bishopwhite;
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(allpa8, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(allph8, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        battack = (battack | attack);
        if (((attack & data.kingblack) != 0)) {
            (skakcount = pas::increment(skakcount, 1));
            skakpos = pos;
        }
    }
    piecepos = data.rookwhite;
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(allp, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(allp90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        battack = (battack | attack);
        if (((attack & data.kingblack) != 0)) {
            (skakcount = pas::increment(skakcount, 1));
            skakpos = pos;
        }
    }
    piecepos = data.queenwhite;
    for(move_bits<false> remaining(piecepos); remaining; ) {
        pos = remaining.pop();
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(allp, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(allp90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack | petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(allpa8, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)]);
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(allph8, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        battack = (battack | attack);
        if (((attack & data.kingblack) != 0)) {
            (skakcount = pas::increment(skakcount, 1));
            skakpos = pos;
        }
    }
    pos = petir_bitboard_mask::king_square(data.kingwhite);
    battack = (battack | petir_bitboard_mask::kingmask[pos]);
    piecepos = ((petir_bitboard_mask::kingmask[posraja] & (~battack)) & (~data.blackpieces));
    for(move_bits<false> remaining(piecepos); remaining; ) {
        post = remaining.pop();
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(posraja, pas::shl(post, 7));
        if ((data.papan[post] == 0)) {
            movelist[jml].score = petir_header::historyb[petir_header::_rajahitam][post];
        } else {
            movelist[jml].score = pas::shl(petir_movgen::nilai_piece[data.papan[post]], 15);
        }
    }
    append_legal_ep_evasions(data,movelist,jml,petir_header::_sisihitam);
    if ((skakcount == 1)) {
        skak64 = petir_bitboard_mask::bit(skakpos);
        path = petir_bitboard_mask::pathmask[posraja][skakpos];
        piecepos = data.knightblack;
        for(move_bits<true> remaining(piecepos); remaining; ) {
            pos = remaining.pop();
            attack = petir_bitboard_mask::knightmask[pos];
            attack2 = (attack & skak64);
            if ((attack2 != 0)) {
                post = petir_bitboard_mask::lastbitp(pas::address(attack2));
                if ((!petir_tools::black_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    (jml = pas::increment(jml, 1));
                    movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                    movelist[jml].score = pas::shl(pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_kuda), 15);
                    if ((movelist[jml].score < 0)) {
                        movelist[jml].score = pas::shl(petir_usee::see(petir_header::_sisihitam, data, pos, post), 15);
                    }
                }
            }
            attack2 = (attack & path);
            for(move_bits<true> remaining(attack2); remaining; ) {
                post = remaining.pop();
                if ((!petir_tools::black_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    (jml = pas::increment(jml, 1));
                    movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                    movelist[jml].score = petir_header::historyb[petir_header::_kudahitam][post];
                }
            }
        }
        piecepos = data.pawnblack;
        for(move_bits<true> remaining(piecepos); remaining; ) {
            pos = remaining.pop();
            attack = petir_bitboard_mask::b_pawn_attack[pos];
            attack2 = (attack & skak64);
            if ((attack2 != 0)) {
                post = petir_bitboard_mask::lastbitp(pas::address(attack2));
                if ((!petir_tools::black_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    if ((pas::shr(pos, 3) == 1)) {
                        pas_temp_1 = petir_header::promosi_kuda;
                        for (a = petir_header::promosi_mentri; a <= pas_temp_1; ++a) {
                            (jml = pas::increment(jml, 1));
                            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(post, 7)), pas::shl(a, 14));
                            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_pion);
                        }
                    } else {
                        (jml = pas::increment(jml, 1));
                        movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                        movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_pion);
                    }
                }
            }
            attack2 = 0;
            if ((data.papan[pas::sub(pos, 8)] == 0)) {
                attack2 = (attack2 | petir_bitboard_mask::bit(pas::sub(pos, 8)));
                if (((pas::shr(pos, 3) == 6) && (data.papan[pas::sub(pos, 16)] == 0))) {
                    attack2 = (attack2 | petir_bitboard_mask::bit(pas::sub(pos, 16)));
                }
            }
            attack2 = (attack2 & path);
            if ((attack2 != 0)) {
                post = petir_bitboard_mask::lastbitp(pas::address(attack2));
                if ((!petir_tools::black_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    if ((pas::shr(pos, 3) == 1)) {
                        pas_temp_2 = petir_header::promosi_kuda;
                        for (a = petir_header::promosi_mentri; a <= pas_temp_2; ++a) {
                            (jml = pas::increment(jml, 1));
                            movelist[jml].moves = pas::add(pas::add(pos, pas::shl(post, 7)), pas::shl(a, 14));
                            movelist[jml].score = pas::shl(petir_usee::see(petir_header::_sisihitam, data, pos, post), 15);
                            if ((movelist[jml].score == 0)) {
                                movelist[jml].score = pas::shl(3, 15);
                            }
                        }
                    } else {
                        (jml = pas::increment(jml, 1));
                        movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                        movelist[jml].score = petir_header::historyb[petir_header::_pionhitam][post];
                    }
                }
            }
        }
        piecepos = data.bishopblack;
        for(move_bits<true> remaining(piecepos); remaining; ) {
            pos = remaining.pop();
            attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
            attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
            attack2 = (attack & skak64);
            if ((attack2 != 0)) {
                post = petir_bitboard_mask::lastbitp(pas::address(attack2));
                if ((!petir_tools::black_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    (jml = pas::increment(jml, 1));
                    movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                    movelist[jml].score = pas::shl(pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_gajah), 15);
                    if ((movelist[jml].score < 0)) {
                        movelist[jml].score = pas::shl(petir_usee::see(petir_header::_sisihitam, data, pos, post), 15);
                    }
                }
            }
            attack2 = (attack & path);
            for(move_bits<true> remaining(attack2); remaining; ) {
                post = remaining.pop();
                if ((!petir_tools::black_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    (jml = pas::increment(jml, 1));
                    movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                    movelist[jml].score = petir_header::historyb[petir_header::_gajahhitam][post];
                }
            }
        }
        piecepos = data.rookblack;
        for(move_bits<true> remaining(piecepos); remaining; ) {
            pos = remaining.pop();
            attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
            attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
            attack2 = (attack & skak64);
            if ((attack2 != 0)) {
                post = petir_bitboard_mask::lastbitp(pas::address(attack2));
                if ((!petir_tools::black_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    (jml = pas::increment(jml, 1));
                    movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                    movelist[jml].score = pas::shl(pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_benteng), 15);
                    if ((movelist[jml].score < 0)) {
                        movelist[jml].score = pas::shl(petir_usee::see(petir_header::_sisihitam, data, pos, post), 15);
                    }
                }
            }
            attack2 = (attack & path);
            for(move_bits<true> remaining(attack2); remaining; ) {
                post = remaining.pop();
                if ((!petir_tools::black_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    (jml = pas::increment(jml, 1));
                    movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                    movelist[jml].score = petir_header::historyb[petir_header::_bentenghitam][post];
                }
            }
        }
        piecepos = data.queenblack;
        for(move_bits<true> remaining(piecepos); remaining; ) {
            pos = remaining.pop();
            attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
            attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
            attack = (attack | petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)]);
            attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
            attack2 = (attack & skak64);
            if ((attack2 != 0)) {
                post = petir_bitboard_mask::lastbitp(pas::address(attack2));
                if ((!petir_tools::black_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    (jml = pas::increment(jml, 1));
                    movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                    movelist[jml].score = pas::shl(pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_mentri), 15);
                    if ((movelist[jml].score < 0)) {
                        movelist[jml].score = pas::shl(petir_usee::see(petir_header::_sisihitam, data, pos, post), 15);
                    }
                }
            }
            attack2 = (attack & path);
            for(move_bits<true> remaining(attack2); remaining; ) {
                post = remaining.pop();
                if ((!petir_tools::black_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    (jml = pas::increment(jml, 1));
                    movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                    movelist[jml].score = petir_header::historyb[petir_header::_mentrihitam][post];
                }
            }
        }
    }
    jml_out = jml;
}

}
