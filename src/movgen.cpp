// Faithful translation of movgen.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_movgen {
// movgen.pas:687
void black_movgen_noncaps(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml) {
    std::uint64_t piecepos{};
    std::uint64_t attack{};
    std::int32_t pos{};
    std::int32_t post{};
    std::int32_t move_index=0;
    jml = 0;
    piecepos = (data.pawnblack & (~petir_bitboard_mask::rank_mask[1]));
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = (petir_bitboard_mask::knightmask[pos] & (~data.allpieces));
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            auto& entry = movelist[++move_index];
            entry.moves = pas::add(pos, pas::shl(post, 7));
            entry.score = petir_header::historyb[petir_header::_kudahitam][post];
        }
    }
    piecepos = data.queenblack;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack | petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)]);
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        attack = (attack & (~data.allpieces));
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            auto& entry = movelist[++move_index];
            entry.moves = pas::add(pos, pas::shl(post, 7));
            entry.score = petir_header::historyb[petir_header::_mentrihitam][post];
            (jml = pas::increment(jml, 1));
        }
    }
    piecepos = data.bishopblack;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        attack = (attack & (~data.allpieces));
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            auto& entry = movelist[++move_index];
            entry.moves = pas::add(pos, pas::shl(post, 7));
            entry.score = petir_header::historyb[petir_header::_gajahhitam][post];
        }
    }
    piecepos = data.rookblack;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack & (~data.allpieces));
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            auto& entry = movelist[++move_index];
            entry.moves = pas::add(pos, pas::shl(post, 7));
            entry.score = petir_header::historyb[petir_header::_bentenghitam][post];
        }
    }
    pos = petir_bitboard_mask::lastbitp(pas::address(data.kingblack));
    attack = (petir_bitboard_mask::kingmask[pos] & (~data.allpieces));
    while ((attack != 0)) {
        post = petir_bitboard_mask::lastbitp(pas::address(attack));
        attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
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
}

// movgen.pas:868
void black_movgen_caps(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml) {
    std::uint64_t piecepos{};
    std::uint64_t attack{};
    std::int32_t pos{};
    std::int32_t post{};
    std::int32_t a{};
    std::int32_t* p{};
    jml = 0;
    p = pas::address(movelist);
    piecepos = (data.pawnblack & petir_bitboard_mask::rank_mask[1]);
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = (petir_bitboard_mask::knightmask[pos] & data.whitepieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_kuda);
            if ((movelist[jml].score <= 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
            }
        }
    }
    piecepos = data.bishopblack;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        attack = (attack & data.whitepieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_gajah);
            if ((movelist[jml].score <= 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
            }
        }
    }
    piecepos = data.rookblack;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack & data.whitepieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_benteng);
            if ((movelist[jml].score <= 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
            }
        }
    }
    piecepos = data.queenblack;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack | petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)]);
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        attack = (attack & data.whitepieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
        }
    }
    pos = petir_bitboard_mask::lastbitp(pas::address(data.kingblack));
    attack = (petir_bitboard_mask::kingmask[pos] & data.whitepieces);
    while ((attack != 0)) {
        post = petir_bitboard_mask::lastbitp(pas::address(attack));
        attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
        movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
    }
}

// movgen.pas:1041
void black_movgen_caps2(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml) {
    std::uint64_t piecepos{};
    std::uint64_t attack{};
    std::int32_t pos{};
    std::int32_t post{};
    std::int32_t a{};
    jml = 0;
    piecepos = (data.pawnblack & petir_bitboard_mask::rank_mask[1]);
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = (petir_bitboard_mask::knightmask[pos] & data.whitepieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_kuda);
            if ((movelist[jml].score <= 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
            }
        }
    }
    piecepos = data.bishopblack;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        attack = (attack & data.whitepieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_gajah);
            if ((movelist[jml].score <= 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
            }
        }
    }
    piecepos = data.rookblack;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack & data.whitepieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_benteng);
            if ((movelist[jml].score <= 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
            }
        }
    }
    piecepos = data.queenblack;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack | petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)]);
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        attack = (attack & data.whitepieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
        }
    }
    pos = petir_bitboard_mask::lastbitp(pas::address(data.kingblack));
    attack = (petir_bitboard_mask::kingmask[pos] & data.whitepieces);
    while ((attack != 0)) {
        post = petir_bitboard_mask::lastbitp(pas::address(attack));
        attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
        movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
    }
    attack = (petir_bitboard_mask::kingmask[pos] & pas::shl(data.pawnwhite, 8));
    while ((attack != 0)) {
        post = petir_bitboard_mask::firstbitp(pas::address(attack));
        attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
        if (((data.papan[post] == 0) && ((petir_bitboard_mask::w_pion_bebas_mask[pas::sub(post, 8)] & data.pawnblack) == 0))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = 0;
        }
    }
}

// movgen.pas:1238
void q_black_movgen(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml) {
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
    posraja = petir_bitboard_mask::firstbitp(pas::address(data.kingwhite));
    attackb = petir_bitboard_mask::diaga8h1mask[posraja][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[posraja]) & 255)];
    attackb = (attackb | petir_bitboard_mask::diagh8a1mask[posraja][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[posraja]) & 255)]);
    attackr = petir_bitboard_mask::horzmask2[posraja][(pas::shr(data.allpieces, pas::add((posraja & 56), 1)) & 63)];
    attackr = (attackr | petir_bitboard_mask::vertmask2[posraja][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[posraja]) & 63)]);
    attackn = petir_bitboard_mask::knightmask[posraja];
    piecepos = (pas::shr((data.pawnblack & petir_bitboard_mask::mask_left), 17) & data.kingwhite);
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        (pos = pas::increment(pos, 17));
        if ((data.papan[pas::sub(pos, 8)] == 0)) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(pas::sub(pos, 8), 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, pas::sub(pos, 8));
        }
    }
    piecepos = (pas::shr((data.pawnblack & petir_bitboard_mask::mask_right), 15) & data.kingwhite);
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        (pos = pas::increment(pos, 15));
        if ((data.papan[pas::sub(pos, 8)] == 0)) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(pas::sub(pos, 8), 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, pas::sub(pos, 8));
        }
    }
    piecepos = (pas::shr((data.pawnblack & petir_bitboard_mask::mask_left), 9) & data.whitepieces);
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
        while ((piecepos != 0)) {
            pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
            piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
            if ((pas::abs(pas::sub((pos & 7), data.ep)) == 1)) {
                (jml = pas::increment(jml, 1));
                movelist[jml].moves = pas::add(pos, pas::shl(petir_header::_en_passant, 7));
                movelist[jml].score = petir_movgen::_nilai_pion;
            }
        }
    }
    piecepos = data.knightblack;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = (petir_bitboard_mask::knightmask[pos] & data.whitepieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_kuda);
            if ((movelist[jml].score < 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
            }
        }
        attack = ((petir_bitboard_mask::knightmask[pos] & attackn) & (~data.allpieces));
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
        }
    }
    piecepos = data.bishopblack;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        t64 = attack;
        attack = (attack & data.whitepieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_gajah);
            if ((movelist[jml].score < 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
            }
        }
        attack = ((t64 & attackb) & (~data.allpieces));
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
        }
    }
    piecepos = data.rookblack;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        t64 = attack;
        attack = (attack & data.whitepieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_benteng);
            if ((movelist[jml].score < 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
            }
        }
        attack = ((t64 & attackr) & (~data.allpieces));
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
        }
    }
    piecepos = data.queenblack;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack | petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)]);
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        t64 = attack;
        attack = (attack & data.whitepieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
        }
        attack = ((t64 & (attackr | attackb)) & (~data.allpieces));
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
        }
    }
    pos = petir_bitboard_mask::lastbitp(pas::address(data.kingblack));
    attack = (petir_bitboard_mask::kingmask[pos] & data.whitepieces);
    while ((attack != 0)) {
        post = petir_bitboard_mask::lastbitp(pas::address(attack));
        attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
        movelist[jml].score = petir_usee::see(petir_header::_sisihitam, data, pos, post);
    }
    attack = (petir_bitboard_mask::kingmask[pos] & pas::shl(data.pawnwhite, 8));
    while ((attack != 0)) {
        post = petir_bitboard_mask::firstbitp(pas::address(attack));
        attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
        if (((data.papan[post] == 0) && ((petir_bitboard_mask::w_pion_bebas_mask[pas::sub(post, 8)] & data.pawnblack) == 0))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = 0;
        }
    }
}

// movgen.pas:2080
void q_white_movgen(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml) {
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
    posraja = petir_bitboard_mask::lastbitp(pas::address(data.kingblack));
    attackb = petir_bitboard_mask::diaga8h1mask[posraja][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[posraja]) & 255)];
    attackb = (attackb | petir_bitboard_mask::diagh8a1mask[posraja][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[posraja]) & 255)]);
    attackr = petir_bitboard_mask::horzmask2[posraja][(pas::shr(data.allpieces, pas::add((posraja & 56), 1)) & 63)];
    attackr = (attackr | petir_bitboard_mask::vertmask2[posraja][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[posraja]) & 63)]);
    attackn = petir_bitboard_mask::knightmask[posraja];
    piecepos = (pas::shl((data.pawnwhite & petir_bitboard_mask::mask_left), 15) & data.kingblack);
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        (pos = pas::decrement(pos, 15));
        if ((data.papan[pas::add(pos, 8)] == 0)) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(pas::add(pos, 8), 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, pas::add(pos, 8));
        }
    }
    piecepos = (pas::shl((data.pawnwhite & petir_bitboard_mask::mask_right), 17) & data.kingblack);
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        (pos = pas::decrement(pos, 17));
        if ((data.papan[pas::add(pos, 8)] == 0)) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(pas::add(pos, 8), 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, pas::add(pos, 8));
        }
    }
    piecepos = (pas::shl((data.pawnwhite & petir_bitboard_mask::mask_left), 7) & data.blackpieces);
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
        while ((piecepos != 0)) {
            pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
            piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
            if ((pas::abs(pas::sub((pos & 7), data.ep)) == 1)) {
                (jml = pas::increment(jml, 1));
                movelist[jml].moves = pas::add(pos, pas::shl(petir_header::_en_passant, 7));
                movelist[jml].score = petir_movgen::_nilai_pion;
            }
        }
    }
    piecepos = data.knightwhite;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = (petir_bitboard_mask::knightmask[pos] & data.blackpieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::firstbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_kuda);
            if ((movelist[jml].score < 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
            }
        }
        attack = ((petir_bitboard_mask::knightmask[pos] & attackn) & (~data.allpieces));
        while ((attack != 0)) {
            post = petir_bitboard_mask::firstbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
        }
    }
    piecepos = data.bishopwhite;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        t64 = attack;
        attack = (attack & data.blackpieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_gajah);
            if ((movelist[jml].score < 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
            }
        }
        attack = ((t64 & attackb) & (~data.allpieces));
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
        }
    }
    piecepos = data.rookwhite;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        t64 = attack;
        attack = (attack & data.blackpieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_benteng);
            if ((movelist[jml].score < 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
            }
        }
        attack = ((t64 & attackr) & (~data.allpieces));
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
        }
    }
    piecepos = data.queenwhite;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack | petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)]);
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        t64 = attack;
        attack = (attack & data.blackpieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
        }
        attack = ((t64 & (attackb | attackr)) & (~data.allpieces));
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
        }
    }
    pos = petir_bitboard_mask::firstbitp(pas::address(data.kingwhite));
    attack = (petir_bitboard_mask::kingmask[pos] & data.blackpieces);
    while ((attack != 0)) {
        post = petir_bitboard_mask::firstbitp(pas::address(attack));
        attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
        movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
    }
    attack = (petir_bitboard_mask::kingmask[pos] & pas::shr(data.pawnblack, 8));
    while ((attack != 0)) {
        post = petir_bitboard_mask::firstbitp(pas::address(attack));
        attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
        if (((data.papan[post] == 0) && ((petir_bitboard_mask::b_pion_bebas_mask[pas::add(post, 8)] & data.pawnwhite) == 0))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = 0;
        }
    }
}

// movgen.pas:1681
void white_movgen_caps(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml) {
    std::uint64_t piecepos{};
    std::uint64_t attack{};
    std::uint64_t temp{};
    std::int32_t pos{};
    std::int32_t post{};
    std::int32_t a{};
    jml = 0;
    piecepos = (data.pawnwhite & petir_bitboard_mask::rank_mask[6]);
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        pos = pas::sub(pos, 7);
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(pos, pas::shl(pas::add(pos, 7), 7));
        movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[pas::add(pos, 7)]], petir_movgen::_nilai_pion);
    }
    piecepos = (pas::shl((temp & petir_bitboard_mask::mask_right), 9) & data.blackpieces);
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        pos = pas::sub(pos, 9);
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(pos, pas::shl(pas::add(pos, 9), 7));
        movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[pas::add(pos, 9)]], petir_movgen::_nilai_pion);
    }
    if ((data.ep != petir_header::_no_ep)) {
        piecepos = (data.pawnwhite & petir_bitboard_mask::rank_mask[4]);
        while ((piecepos != 0)) {
            pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
            piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
            if ((pas::abs(pas::sub((pos & 7), data.ep)) == 1)) {
                (jml = pas::increment(jml, 1));
                movelist[jml].moves = pas::add(pos, pas::shl(petir_header::_en_passant, 7));
                movelist[jml].score = petir_movgen::_nilai_pion;
            }
        }
    }
    piecepos = data.knightwhite;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = (petir_bitboard_mask::knightmask[pos] & data.blackpieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::firstbitp(pas::address(attack));
            attack = (attack & (~petir_bitboard_mask::bit2nmask[post]));
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_kuda);
            if ((movelist[jml].score <= 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
            }
        }
    }
    piecepos = data.bishopwhite;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        attack = (attack & data.blackpieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_gajah);
            if ((movelist[jml].score <= 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
            }
        }
    }
    piecepos = data.rookwhite;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack & data.blackpieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_benteng);
            if ((movelist[jml].score <= 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
            }
        }
    }
    piecepos = data.queenwhite;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack | petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)]);
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        attack = (attack & data.blackpieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
        }
    }
    pos = petir_bitboard_mask::firstbitp(pas::address(data.kingwhite));
    attack = (petir_bitboard_mask::kingmask[pos] & data.blackpieces);
    while ((attack != 0)) {
        post = petir_bitboard_mask::firstbitp(pas::address(attack));
        attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
        movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
    }
}

// movgen.pas:1864
void white_movgen_caps2(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml) {
    std::uint64_t piecepos{};
    std::uint64_t attack{};
    std::uint64_t temp{};
    std::int32_t pos{};
    std::int32_t post{};
    std::int32_t a{};
    jml = 0;
    piecepos = (data.pawnwhite & petir_bitboard_mask::rank_mask[6]);
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        pos = pas::sub(pos, 7);
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(pos, pas::shl(pas::add(pos, 7), 7));
        movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[pas::add(pos, 7)]], petir_movgen::_nilai_pion);
    }
    piecepos = (pas::shl((temp & petir_bitboard_mask::mask_right), 9) & data.blackpieces);
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        pos = pas::sub(pos, 9);
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(pos, pas::shl(pas::add(pos, 9), 7));
        movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[pas::add(pos, 9)]], petir_movgen::_nilai_pion);
    }
    if ((data.ep != petir_header::_no_ep)) {
        piecepos = (data.pawnwhite & petir_bitboard_mask::rank_mask[4]);
        while ((piecepos != 0)) {
            pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
            piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
            if ((pas::abs(pas::sub((pos & 7), data.ep)) == 1)) {
                (jml = pas::increment(jml, 1));
                movelist[jml].moves = pas::add(pos, pas::shl(petir_header::_en_passant, 7));
                movelist[jml].score = petir_movgen::_nilai_pion;
            }
        }
    }
    piecepos = data.knightwhite;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = (petir_bitboard_mask::knightmask[pos] & data.blackpieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::firstbitp(pas::address(attack));
            attack = (attack & (~petir_bitboard_mask::bit2nmask[post]));
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_kuda);
            if ((movelist[jml].score <= 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
            }
        }
    }
    piecepos = data.bishopwhite;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        attack = (attack & data.blackpieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_gajah);
            if ((movelist[jml].score <= 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
            }
        }
    }
    piecepos = data.rookwhite;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack & data.blackpieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = pas::sub(petir_movgen::nilai_piece[data.papan[post]], petir_movgen::_nilai_benteng);
            if ((movelist[jml].score <= 0)) {
                movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
            }
        }
    }
    piecepos = data.queenwhite;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack | petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)]);
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        attack = (attack & data.blackpieces);
        while ((attack != 0)) {
            post = petir_bitboard_mask::lastbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
        }
    }
    pos = petir_bitboard_mask::firstbitp(pas::address(data.kingwhite));
    attack = (petir_bitboard_mask::kingmask[pos] & data.blackpieces);
    while ((attack != 0)) {
        post = petir_bitboard_mask::firstbitp(pas::address(attack));
        attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
        movelist[jml].score = petir_usee::see(petir_header::_sisiputih, data, pos, post);
    }
    attack = (petir_bitboard_mask::kingmask[pos] & pas::shr(data.pawnblack, 8));
    while ((attack != 0)) {
        post = petir_bitboard_mask::firstbitp(pas::address(attack));
        attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
        if (((data.papan[post] == 0) && ((petir_bitboard_mask::b_pion_bebas_mask[pas::add(post, 8)] & data.pawnwhite) == 0))) {
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = 0;
        }
    }
}

// movgen.pas:1520
void white_movgen_noncaps(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml, std::int32_t depth) {
    std::uint64_t piecepos{};
    std::uint64_t attack{};
    std::int32_t pos{};
    std::int32_t post{};
    jml = 0;
    piecepos = (data.pawnwhite & (~petir_bitboard_mask::rank_mask[6]));
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = (petir_bitboard_mask::knightmask[pos] & (~data.allpieces));
        while ((attack != 0)) {
            post = petir_bitboard_mask::firstbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_header::historyw[petir_header::_kudaputih][post];
            if (((depth <= 3) && (petir_usee::see(petir_header::_sisiputih, data, pos, post) < 0))) {
                movelist[jml].score = 0;
            }
        }
    }
    piecepos = data.queenwhite;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack | petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)]);
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        attack = (attack & (~data.allpieces));
        while ((attack != 0)) {
            post = petir_bitboard_mask::firstbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_header::historyw[petir_header::_mentriputih][post];
            if (((depth <= 3) && (petir_usee::see(petir_header::_sisiputih, data, pos, post) < 0))) {
                movelist[jml].score = 0;
            }
        }
    }
    piecepos = data.bishopwhite;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        attack = (attack & (~data.allpieces));
        while ((attack != 0)) {
            post = petir_bitboard_mask::firstbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_header::historyw[petir_header::_gajahputih][post];
            if (((depth <= 3) && (petir_usee::see(petir_header::_sisiputih, data, pos, post) < 0))) {
                movelist[jml].score = 0;
            }
        }
    }
    piecepos = data.rookwhite;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        attack = (attack & (~data.allpieces));
        while ((attack != 0)) {
            post = petir_bitboard_mask::firstbitp(pas::address(attack));
            attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
            (jml = pas::increment(jml, 1));
            movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
            movelist[jml].score = petir_header::historyw[petir_header::_bentengputih][post];
            if (((depth <= 3) && (petir_usee::see(petir_header::_sisiputih, data, pos, post) < 0))) {
                movelist[jml].score = 0;
            }
        }
    }
    pos = petir_bitboard_mask::firstbitp(pas::address(data.kingwhite));
    attack = (petir_bitboard_mask::kingmask[pos] & (~data.allpieces));
    while ((attack != 0)) {
        post = petir_bitboard_mask::firstbitp(pas::address(attack));
        attack = (attack & petir_bitboard_mask::bit2nmasknot[post]);
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
}

// movgen.pas:30
void white_evasion(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml, std::int32_t& skakcount) {
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
    posraja = petir_bitboard_mask::firstbitp(pas::address(data.kingwhite));
    skakpos = 64;
    piecepos = data.knightblack;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        battack = (battack | petir_bitboard_mask::knightmask[pos]);
        if (((petir_bitboard_mask::knightmask[pos] & data.kingwhite) != 0)) {
            (skakcount = pas::increment(skakcount, 1));
            skakpos = pos;
        }
    }
    piecepos = data.pawnblack;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(allpa8, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(allph8, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        battack = (battack | attack);
        if (((attack & data.kingwhite) != 0)) {
            (skakcount = pas::increment(skakcount, 1));
            skakpos = pos;
        }
    }
    piecepos = data.rookblack;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(allp, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(allp90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        battack = (battack | attack);
        if (((attack & data.kingwhite) != 0)) {
            (skakcount = pas::increment(skakcount, 1));
            skakpos = pos;
        }
    }
    piecepos = data.queenblack;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
    pos = petir_bitboard_mask::lastbitp(pas::address(data.kingblack));
    battack = (battack | petir_bitboard_mask::kingmask[pos]);
    piecepos = ((petir_bitboard_mask::kingmask[posraja] & (~battack)) & (~data.whitepieces));
    while ((piecepos != 0)) {
        post = petir_bitboard_mask::lastbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[post]);
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(posraja, pas::shl(post, 7));
        if ((data.papan[post] == 0)) {
            movelist[jml].score = petir_header::historyw[petir_header::_rajaputih][post];
        } else {
            movelist[jml].score = pas::shl(petir_movgen::nilai_piece[data.papan[post]], 14);
        }
    }
    if ((skakcount == 1)) {
        if (((data.ep != petir_header::_no_ep) && (data.papan[skakpos] == petir_header::_pionhitam))) {
            if (((data.papan[pas::sub(skakpos, 1)] == petir_header::_pionputih) && ((skakpos & 7) > 0))) {
                (jml = pas::increment(jml, 1));
                movelist[jml].moves = pas::add(pas::sub(skakpos, 1), pas::shl(petir_header::_en_passant, 7));
                movelist[jml].score = 0;
            }
            if (((data.papan[pas::add(skakpos, 1)] == petir_header::_pionputih) && ((skakpos & 7) < 7))) {
                (jml = pas::increment(jml, 1));
                movelist[jml].moves = pas::add(pas::add(skakpos, 1), pas::shl(petir_header::_en_passant, 7));
                movelist[jml].score = 0;
            }
        }
        skak64 = petir_bitboard_mask::bit2nmask[skakpos];
        path = petir_bitboard_mask::pathmask[posraja][skakpos];
        piecepos = data.knightwhite;
        dn = petir_bitboard_mask::direction[posraja][skakpos];
        while ((piecepos != 0)) {
            pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
            piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
            while ((attack2 != 0)) {
                post = petir_bitboard_mask::firstbitp(pas::address(attack2));
                attack2 = (attack2 & petir_bitboard_mask::bit2nmasknot[post]);
                if ((!petir_tools::white_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    (jml = pas::increment(jml, 1));
                    movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                    movelist[jml].score = petir_header::historyw[petir_header::_kudaputih][post];
                }
            }
        }
        piecepos = data.pawnwhite;
        while ((piecepos != 0)) {
            pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
            piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
                attack2 = (attack2 | petir_bitboard_mask::bit2nmask[pas::add(pos, 8)]);
                if (((pas::shr(pos, 3) == 1) && (data.papan[pas::add(pos, 16)] == 0))) {
                    attack2 = (attack2 | petir_bitboard_mask::bit2nmask[pas::add(pos, 16)]);
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
        while ((piecepos != 0)) {
            pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
            piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
            while ((attack2 != 0)) {
                post = petir_bitboard_mask::firstbitp(pas::address(attack2));
                attack2 = (attack2 & petir_bitboard_mask::bit2nmasknot[post]);
                if ((!petir_tools::white_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    (jml = pas::increment(jml, 1));
                    movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                    movelist[jml].score = petir_header::historyw[petir_header::_gajahputih][post];
                }
            }
        }
        piecepos = data.rookwhite;
        while ((piecepos != 0)) {
            pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
            piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
            while ((attack2 != 0)) {
                post = petir_bitboard_mask::firstbitp(pas::address(attack2));
                attack2 = (attack2 & petir_bitboard_mask::bit2nmasknot[post]);
                if ((!petir_tools::white_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    (jml = pas::increment(jml, 1));
                    movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                    movelist[jml].score = petir_header::historyw[petir_header::_bentengputih][post];
                }
            }
        }
        piecepos = data.queenwhite;
        while ((piecepos != 0)) {
            pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
            piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
            while ((attack2 != 0)) {
                post = petir_bitboard_mask::firstbitp(pas::address(attack2));
                attack2 = (attack2 & petir_bitboard_mask::bit2nmasknot[post]);
                if ((!petir_tools::white_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    (jml = pas::increment(jml, 1));
                    movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                    movelist[jml].score = petir_header::historyw[petir_header::_mentriputih][post];
                }
            }
        }
    }
}

// movgen.pas:361
void black_evasion(petir_header::tdata& data, petir_header::tmovelist& movelist, std::int32_t& jml, std::int32_t& skakcount) {
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
    posraja = petir_bitboard_mask::lastbitp(pas::address(data.kingblack));
    skakpos = 64;
    piecepos = data.knightwhite;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        battack = (battack | petir_bitboard_mask::knightmask[pos]);
        if (((petir_bitboard_mask::knightmask[pos] & data.kingblack) != 0)) {
            (skakcount = pas::increment(skakcount, 1));
            skakpos = pos;
        }
    }
    piecepos = data.pawnwhite;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(allpa8, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(allph8, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        battack = (battack | attack);
        if (((attack & data.kingblack) != 0)) {
            (skakcount = pas::increment(skakcount, 1));
            skakpos = pos;
        }
    }
    piecepos = data.rookwhite;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
        attack = petir_bitboard_mask::horzmask2[pos][(pas::shr(allp, pas::add((pos & 56), 1)) & 63)];
        attack = (attack | petir_bitboard_mask::vertmask2[pos][(pas::shr(allp90, petir_bitboard_mask::vershiftmask[pos]) & 63)]);
        battack = (battack | attack);
        if (((attack & data.kingblack) != 0)) {
            (skakcount = pas::increment(skakcount, 1));
            skakpos = pos;
        }
    }
    piecepos = data.queenwhite;
    while ((piecepos != 0)) {
        pos = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
    pos = petir_bitboard_mask::lastbitp(pas::address(data.kingwhite));
    battack = (battack | petir_bitboard_mask::kingmask[pos]);
    piecepos = ((petir_bitboard_mask::kingmask[posraja] & (~battack)) & (~data.blackpieces));
    while ((piecepos != 0)) {
        post = petir_bitboard_mask::firstbitp(pas::address(piecepos));
        piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[post]);
        (jml = pas::increment(jml, 1));
        movelist[jml].moves = pas::add(posraja, pas::shl(post, 7));
        if ((data.papan[post] == 0)) {
            movelist[jml].score = petir_header::historyb[petir_header::_rajahitam][post];
        } else {
            movelist[jml].score = pas::shl(petir_movgen::nilai_piece[data.papan[post]], 15);
        }
    }
    if ((skakcount == 1)) {
        if (((data.ep != petir_header::_no_ep) && (data.papan[skakpos] == petir_header::_pionputih))) {
            if (((data.papan[pas::sub(skakpos, 1)] == petir_header::_pionhitam) && ((skakpos & 7) > 0))) {
                (jml = pas::increment(jml, 1));
                movelist[jml].moves = pas::add(pas::sub(skakpos, 1), pas::shl(petir_header::_en_passant, 7));
                movelist[jml].score = 0;
            }
            if (((data.papan[pas::add(skakpos, 1)] == petir_header::_pionhitam) && ((skakpos & 7) < 7))) {
                (jml = pas::increment(jml, 1));
                movelist[jml].moves = pas::add(pas::add(skakpos, 1), pas::shl(petir_header::_en_passant, 7));
                movelist[jml].score = 0;
            }
        }
        skak64 = petir_bitboard_mask::bit2nmask[skakpos];
        path = petir_bitboard_mask::pathmask[posraja][skakpos];
        piecepos = data.knightblack;
        while ((piecepos != 0)) {
            pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
            piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
            while ((attack2 != 0)) {
                post = petir_bitboard_mask::lastbitp(pas::address(attack2));
                attack2 = (attack2 & petir_bitboard_mask::bit2nmasknot[post]);
                if ((!petir_tools::black_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    (jml = pas::increment(jml, 1));
                    movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                    movelist[jml].score = petir_header::historyb[petir_header::_kudahitam][post];
                }
            }
        }
        piecepos = data.pawnblack;
        while ((piecepos != 0)) {
            pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
            piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
                attack2 = (attack2 | petir_bitboard_mask::bit2nmask[pas::sub(pos, 8)]);
                if (((pas::shr(pos, 3) == 6) && (data.papan[pas::sub(pos, 16)] == 0))) {
                    attack2 = (attack2 | petir_bitboard_mask::bit2nmask[pas::sub(pos, 16)]);
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
        while ((piecepos != 0)) {
            pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
            piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
            while ((attack2 != 0)) {
                post = petir_bitboard_mask::lastbitp(pas::address(attack2));
                attack2 = (attack2 & petir_bitboard_mask::bit2nmasknot[post]);
                if ((!petir_tools::black_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    (jml = pas::increment(jml, 1));
                    movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                    movelist[jml].score = petir_header::historyb[petir_header::_gajahhitam][post];
                }
            }
        }
        piecepos = data.rookblack;
        while ((piecepos != 0)) {
            pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
            piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
            while ((attack2 != 0)) {
                post = petir_bitboard_mask::lastbitp(pas::address(attack2));
                attack2 = (attack2 & petir_bitboard_mask::bit2nmasknot[post]);
                if ((!petir_tools::black_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    (jml = pas::increment(jml, 1));
                    movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                    movelist[jml].score = petir_header::historyb[petir_header::_bentenghitam][post];
                }
            }
        }
        piecepos = data.queenblack;
        while ((piecepos != 0)) {
            pos = petir_bitboard_mask::lastbitp(pas::address(piecepos));
            piecepos = (piecepos & petir_bitboard_mask::bit2nmasknot[pos]);
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
            while ((attack2 != 0)) {
                post = petir_bitboard_mask::lastbitp(pas::address(attack2));
                attack2 = (attack2 & petir_bitboard_mask::bit2nmasknot[post]);
                if ((!petir_tools::black_open_check(data, pas::add(pos, pas::shl(post, 7))))) {
                    (jml = pas::increment(jml, 1));
                    movelist[jml].moves = pas::add(pos, pas::shl(post, 7));
                    movelist[jml].score = petir_header::historyb[petir_header::_mentrihitam][post];
                }
            }
        }
    }
}

}
