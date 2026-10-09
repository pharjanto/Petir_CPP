// Faithful translation of ueval.pas; Delphi source is in original/.
#include "petir.hpp"
#include "kpk.hpp"
#include <algorithm>
#include <bit>
#include <cstring>
#if defined(_MSC_VER) && defined(_M_X64)
#include <intrin.h>
#endif
namespace petir_ueval {
namespace {
// These helpers are only used in loops that have already checked bits != 0.
int pop_lsb(std::uint64_t& bits) {
#if defined(_MSC_VER) && defined(_M_X64)
    unsigned long square;
    _BitScanForward64(&square, bits);
#else
    const auto square = std::countr_zero(bits);
#endif
    bits &= bits - 1;
    return static_cast<int>(square);
}

int pop_msb(std::uint64_t& bits) {
#if defined(_MSC_VER) && defined(_M_X64)
    unsigned long square;
    _BitScanReverse64(&square, bits);
#else
    const auto square = 63 - std::countl_zero(bits);
#endif
    bits ^= UINT64_C(1) << square;
    return static_cast<int>(square);
}

std::uint8_t pawn_files(std::uint64_t pawns) {
    pawns |= pawns >> 32;
    pawns |= pawns >> 16;
    pawns |= pawns >> 8;
    return static_cast<std::uint8_t>(pawns);
}

#if defined(_MSC_VER) && defined(_M_X64)
const bool has_native_popcount = [] {
    int cpu_info[4];
    __cpuid(cpu_info, 1);
    return (cpu_info[2] & (1 << 23)) != 0;
}();
#endif

template<bool NativePopcount>
std::uint8_t count_bits(std::uint64_t bits) {
#if defined(_MSC_VER) && defined(_M_X64)
    if constexpr (NativePopcount) {
        return static_cast<std::uint8_t>(__popcnt64(bits));
    } else {
        // Keep a hardware-independent path for CPUs without POPCNT.
        return static_cast<std::uint8_t>(
            petir_evalmask::mobmask[bits & 0xffff]
            + petir_evalmask::mobmask[(bits >> 16) & 0xffff]
            + petir_evalmask::mobmask[(bits >> 32) & 0xffff]
            + petir_evalmask::mobmask[bits >> 48]);
    }
#else
    return static_cast<std::uint8_t>(std::popcount(bits));
#endif
}

// ueval.pas:850
template<bool NativePopcount>
std::int32_t eval_impl(const petir_header::tdata& data, std::int32_t giliran, std::int32_t alpha, std::int32_t beta, bool infhr) {
    std::int32_t return_value{};
    bool kingotherside{};
    bool draw2{};
    std::int32_t attacker{};
    std::int32_t defender{};
    bool tempb{};
    bool tempb2{};
    bool real_endgame{};
    bool early_endgame{};
    bool endgame{};
    std::int32_t nilai{};
    std::uint64_t attack{};
    std::uint64_t piecepos{};
    std::int32_t pos{};
    std::int32_t h{};
    std::int32_t temp{};
    std::int32_t tempx{};
    std::int32_t tempi{};
    std::int32_t temp2{};
    std::int32_t posrajaputih{};
    std::int32_t posrajahitam{};
    std::int32_t xrajaputih{};
    std::int32_t yrajaputih{};
    std::int32_t xrajahitam{};
    std::int32_t yrajahitam{};
    std::int32_t filerajaputih{};
    std::int32_t filerajahitam{};
    std::int32_t pionbebasputih{};
    std::int32_t pionbebashitam{};
    std::int32_t blackkingpressure{};
    std::int32_t whitekingpressure{};
    std::int32_t m{};
    std::int32_t posgajah{};
    std::int32_t m1{};
    std::int32_t m2{};
    std::int32_t m3{};
    std::int32_t temp3{};
    std::int32_t temp4{};
    std::uint64_t horattack{};
    std::uint64_t verattack{};
    std::uint64_t pawnwattack{};
    std::uint64_t pawnbattack{};
    std::uint64_t temp64{};
    std::uint64_t tempk64{};
    std::uint64_t temp264{};
    std::uint64_t whitekingattack{};
    std::uint64_t blackkingattack{};
    bool wevalks{};
    bool bevalks{};
    std::int32_t nppputih{};
    std::int32_t npphitam{};
    std::int32_t npionputih{};
    std::int32_t npionhitam{};
    std::int32_t black_attacker{};
    std::int32_t white_attacker{};
    std::int32_t realgiliran{};
    std::uint64_t rookwattack{};
    std::uint64_t rookbattack{};
    std::uint64_t bishopwattack{};
    std::uint64_t bishopbattack{};
    std::uint64_t queenbattack{};
    std::uint64_t queenwattack{};
    std::uint64_t knightwattack{};
    std::uint64_t knightbattack{};
    std::int32_t pas_temp_1{};
    std::int32_t pas_temp_2{};
    std::int32_t pas_temp_3{};
    std::int32_t pas_temp_4{};
    std::int32_t pas_temp_5{};
    std::int32_t pas_temp_6{};
    draw2 = false;
    if (((petir_ueval::isdraw(data, draw2) || ((infhr == false) && petir_ueval::anotherdraw(data, giliran))) || ((infhr == true) && petir_ueval::anotherdraw(data, pas::sub(3, giliran))))) {
        return_value = petir_header::contemp_draw;
        return return_value;
    }
    npionhitam = 0;
    npionputih = 0;
    nppputih = 0;
    npphitam = 0;
    realgiliran = giliran;
    if (infhr) {
        realgiliran = pas::sub(3, giliran);
    }
    white_attacker = 0;
    black_attacker = 0;
    nilai = data.materialscore;
    if ((((data.papan[petir_header::h6] == petir_header::_gajahputih) && (data.papan[petir_header::g5] == petir_header::_pionhitam)) && ((data.papan[petir_header::f6] == petir_header::_pionhitam) || (petir_usee::see(petir_header::_sisiputih, data, petir_header::h6, petir_header::g5) < 0)))) {
        (nilai = pas::decrement(nilai, petir_ueval::nilai_bishop_trapped2));
    }
    if ((((data.papan[petir_header::a6] == petir_header::_gajahputih) && (data.papan[petir_header::b5] == petir_header::_pionhitam)) && ((data.papan[petir_header::c6] == petir_header::_pionhitam) || (petir_usee::see(petir_header::_sisiputih, data, petir_header::a6, petir_header::b5) < 0)))) {
        (nilai = pas::decrement(nilai, petir_ueval::nilai_bishop_trapped2));
    }
    if ((((data.papan[petir_header::h3] == petir_header::_gajahhitam) && (data.papan[petir_header::g4] == petir_header::_pionputih)) && ((data.papan[petir_header::f3] == petir_header::_pionputih) || (petir_usee::see(petir_header::_sisihitam, data, petir_header::h3, petir_header::g4) < 0)))) {
        (nilai = pas::increment(nilai, petir_ueval::nilai_bishop_trapped2));
    }
    if ((((data.papan[petir_header::a3] == petir_header::_gajahhitam) && (data.papan[petir_header::b4] == petir_header::_pionputih)) && ((data.papan[petir_header::c3] == petir_header::_pionputih) || (petir_usee::see(petir_header::_sisihitam, data, petir_header::a3, petir_header::b4) < 0)))) {
        (nilai = pas::increment(nilai, petir_ueval::nilai_bishop_trapped2));
    }
    if ((((data.papan[petir_header::h7] == petir_header::_gajahputih) && (((data.papan[petir_header::g7] == petir_header::_pionhitam) && (realgiliran == petir_header::_sisihitam)) || (data.papan[petir_header::g6] == petir_header::_pionhitam))) && ((data.papan[petir_header::f7] == petir_header::_pionhitam) || (petir_usee::see(petir_header::_sisiputih, data, petir_header::h7, petir_header::g6) < 0)))) {
        (nilai = pas::decrement(nilai, petir_ueval::nilai_bishop_trapped));
    }
    if ((((data.papan[petir_header::a7] == petir_header::_gajahputih) && (((data.papan[petir_header::b7] == petir_header::_pionhitam) && (realgiliran == petir_header::_sisihitam)) || (data.papan[petir_header::b6] == petir_header::_pionhitam))) && ((data.papan[petir_header::c7] == petir_header::_pionhitam) || (petir_usee::see(petir_header::_sisiputih, data, petir_header::a7, petir_header::b6) < 0)))) {
        (nilai = pas::decrement(nilai, petir_ueval::nilai_bishop_trapped));
    }
    if ((((data.papan[petir_header::h2] == petir_header::_gajahhitam) && (((data.papan[petir_header::g2] == petir_header::_pionputih) && (realgiliran == petir_header::_sisiputih)) || (data.papan[petir_header::g3] == petir_header::_pionputih))) && ((data.papan[petir_header::f2] == petir_header::_pionputih) || (petir_usee::see(petir_header::_sisihitam, data, petir_header::h2, petir_header::g3) < 0)))) {
        (nilai = pas::increment(nilai, petir_ueval::nilai_bishop_trapped));
    }
    if ((((data.papan[petir_header::a2] == petir_header::_gajahhitam) && (((data.papan[petir_header::b2] == petir_header::_pionputih) && (realgiliran == petir_header::_sisiputih)) || (data.papan[petir_header::b3] == petir_header::_pionputih))) && ((data.papan[petir_header::c2] == petir_header::_pionputih) || (petir_usee::see(petir_header::_sisihitam, data, petir_header::a2, petir_header::b3) < 0)))) {
        (nilai = pas::increment(nilai, petir_ueval::nilai_bishop_trapped));
    }
    if ((((data.papan[petir_header::a8] == petir_header::_kudaputih) && (petir_usee::see(petir_header::_sisiputih, data, petir_header::a8, petir_header::c7) < 0)) && (petir_usee::see(petir_header::_sisiputih, data, petir_header::a8, petir_header::b6) < 0))) {
        (nilai = pas::decrement(nilai, petir_ueval::nilai_kuda_trapped));
    }
    if ((((data.papan[petir_header::h8] == petir_header::_kudaputih) && (petir_usee::see(petir_header::_sisiputih, data, petir_header::h8, petir_header::f7) < 0)) && (petir_usee::see(petir_header::_sisiputih, data, petir_header::h8, petir_header::g6) < 0))) {
        (nilai = pas::decrement(nilai, petir_ueval::nilai_kuda_trapped));
    }
    if ((((data.papan[petir_header::a1] == petir_header::_kudahitam) && (petir_usee::see(petir_header::_sisihitam, data, petir_header::a1, petir_header::c2) < 0)) && (petir_usee::see(petir_header::_sisihitam, data, petir_header::a1, petir_header::b3) < 0))) {
        (nilai = pas::increment(nilai, petir_ueval::nilai_kuda_trapped));
    }
    if ((((data.papan[petir_header::h1] == petir_header::_kudahitam) && (petir_usee::see(petir_header::_sisihitam, data, petir_header::h1, petir_header::f2) < 0)) && (petir_usee::see(petir_header::_sisihitam, data, petir_header::h1, petir_header::g3) < 0))) {
        (nilai = pas::increment(nilai, petir_ueval::nilai_kuda_trapped));
    }
    m1 = nilai;
    if (((data.nilai_perwira_putih < 16) || (data.nilai_perwira_hitam < 16))) {
        real_endgame = ((data.nilai_perwira_putih <= 10) && (data.nilai_perwira_hitam <= 10));
        endgame = ((data.nilai_perwira_putih <= 12) && (data.nilai_perwira_hitam <= 12));
        if (real_endgame) {
            (nilai = pas::increment(nilai, pas::shl(count_bits<NativePopcount>(data.pawnwhite), 3)));
            (nilai = pas::decrement(nilai, pas::shl(count_bits<NativePopcount>(data.pawnblack), 3)));
        } else {
            (nilai = pas::increment(nilai, pas::shl(count_bits<NativePopcount>(data.pawnwhite), 2)));
            (nilai = pas::decrement(nilai, pas::shl(count_bits<NativePopcount>(data.pawnblack), 2)));
        }
        if (((((data.nilai_perwira_putih == 12) && (data.nilai_perwira_hitam == 12)) && (data.queenblack != 0)) && (data.queenwhite != 0))) {
            if ((data.knightwhite != 0)) {
                (nilai = pas::increment(nilai, petir_ueval::nilai_qb_endgame));
            }
            if ((data.knightblack != 0)) {
                (nilai = pas::decrement(nilai, petir_ueval::nilai_qb_endgame));
            }
        }
        early_endgame = true;
    } else {
        endgame = false;
        early_endgame = false;
        real_endgame = false;
    }
    if ((realgiliran == petir_header::_sisiputih)) {
        (nilai = pas::increment(nilai, 10));
    } else {
        (nilai = pas::decrement(nilai, 10));
    }
    if (((data.nilai_perwira_putih > 17) && (data.nilai_perwira_hitam > 17))) {
        if ((((data.papan[petir_header::f1] == petir_header::_rajaputih) || (data.papan[petir_header::g1] == petir_header::_rajaputih)) && (((data.papan[petir_header::h1] == petir_header::_bentengputih) || (data.papan[petir_header::g1] == petir_header::_bentengputih)) || (data.papan[petir_header::h2] == petir_header::_bentengputih)))) {
            (nilai = pas::decrement(nilai, petir_ueval::nilai_blocked_rook));
        }
        if ((((data.papan[petir_header::c1] == petir_header::_rajaputih) || (data.papan[petir_header::b1] == petir_header::_rajaputih)) && (((data.papan[petir_header::a1] == petir_header::_bentengputih) || (data.papan[petir_header::a2] == petir_header::_bentengputih)) || (data.papan[petir_header::b1] == petir_header::_bentengputih)))) {
            (nilai = pas::decrement(nilai, petir_ueval::nilai_blocked_rook));
        }
        if ((((data.papan[petir_header::f8] == petir_header::_rajahitam) || (data.papan[petir_header::g8] == petir_header::_rajahitam)) && (((data.papan[petir_header::h8] == petir_header::_bentenghitam) || (data.papan[petir_header::g8] == petir_header::_bentenghitam)) || (data.papan[petir_header::h7] == petir_header::_bentenghitam)))) {
            (nilai = pas::increment(nilai, petir_ueval::nilai_blocked_rook));
        }
        if ((((data.papan[petir_header::c8] == petir_header::_rajahitam) || (data.papan[petir_header::b8] == petir_header::_rajahitam)) && (((data.papan[petir_header::a8] == petir_header::_bentenghitam) || (data.papan[petir_header::b8] == petir_header::_bentenghitam)) || (data.papan[petir_header::a7] == petir_header::_bentenghitam)))) {
            (nilai = pas::increment(nilai, petir_ueval::nilai_blocked_rook));
        }
    }
    // Complete evaluation when its score needs the later draw scaling.
    if (!draw2 && data.move50count < 80 && alpha == pas::sub(beta, 1)) {
        m = petir_ueval::dynamiclazy(data);
        if (endgame) {
            (m = pas::increment(m, 600));
        } else {
            if (early_endgame) {
                (m = pas::increment(m, 200));
            }
        }
        if ((giliran == petir_header::_sisiputih)) {
            temp = nilai;
        } else {
            temp = pas::negate(nilai);
        }
        if ((pas::add(pas::add(temp, 280), m) < alpha)) {
            return_value = temp;
            return return_value;
        }
        if ((pas::sub(temp, pas::add(280, m)) > beta)) {
            return_value = temp;
            return return_value;
        }
    }
    whitekingpressure = 0;
    blackkingpressure = 0;
    wevalks = true;
    bevalks = true;
    if (endgame) {
        wevalks = false;
        bevalks = false;
    } else {
        if ((data.queenblack == 0)) {
            wevalks = false;
        }
        if ((data.queenwhite == 0)) {
            bevalks = false;
        }
    }
    pawnwattack = 0;
    pawnbattack = 0;
    pionbebasputih = 0;
    pionbebashitam = 0;
    posrajahitam = petir_bitboard_mask::lastbitp(pas::address(data.kingblack));
    posrajaputih = petir_bitboard_mask::firstbitp(pas::address(data.kingwhite));
    filerajaputih = (posrajaputih & 7);
    filerajahitam = (posrajahitam & 7);
    if (early_endgame) {
        kingotherside = false;
    } else {
        kingotherside = ((((data.wc == petir_header::rokadependek) && (data.bc == petir_header::rokadepanjang)) || ((data.bc == petir_header::rokadependek) && (data.wc == petir_header::rokadepanjang))) || (pas::abs(pas::sub(filerajaputih, filerajahitam)) >= 3));
    }
    whitekingattack = (petir_bitboard_mask::kingmask[posrajaputih] | data.kingwhite);
    blackkingattack = (petir_bitboard_mask::kingmask[posrajahitam] | data.kingblack);
    rookwattack = 0;
    rookbattack = 0;
    queenwattack = 0;
    queenbattack = 0;
    knightwattack = 0;
    knightbattack = 0;
    bishopwattack = 0;
    bishopbattack = 0;
    if (((data.bc == petir_header::norokade) && ((data.flagrokade & petir_header::bithitamnoc) == 0))) {
        if ((data.queenwhite != 0)) {
            (nilai = pas::increment(nilai, petir_ueval::nilai_nggakbisarokade));
        } else {
            (nilai = pas::increment(nilai, pas::shr(petir_ueval::nilai_nggakbisarokade, 1)));
        }
    } else {
        if ((data.bc != petir_header::norokade)) {
            if (((data.bc == petir_header::rokadepanjang) && (data.wc == petir_header::rokadependek))) {
                (nilai = pas::increment(nilai, petir_ueval::nilai_rokade_wrong_side));
            } else {
                (nilai = pas::decrement(nilai, petir_ueval::nilai_rokade));
            }
        }
    }
    if (((data.wc == petir_header::norokade) && ((data.flagrokade & petir_header::bitputihnoc) == 0))) {
        if ((data.queenblack != 0)) {
            (nilai = pas::decrement(nilai, petir_ueval::nilai_nggakbisarokade));
        } else {
            (nilai = pas::decrement(nilai, pas::shr(petir_ueval::nilai_nggakbisarokade, 1)));
        }
    } else {
        if ((data.wc != petir_header::norokade)) {
            if (((data.wc == petir_header::rokadepanjang) && (data.bc == petir_header::rokadependek))) {
                (nilai = pas::decrement(nilai, petir_ueval::nilai_rokade_wrong_side));
            } else {
                (nilai = pas::increment(nilai, petir_ueval::nilai_rokade));
            }
        }
    }
    if ((((data.queenblack != 0) && (data.nilai_perwira_hitam >= 16)) || (data.nilai_perwira_hitam >= 19))) {
        temp = 0;
        if ((filerajaputih >= 5)) {
            if (((petir_bitboard_mask::file_mask[6] & data.pawnwhite) == 0)) {
                (temp = pas::increment(temp, 4));
                if (((petir_bitboard_mask::file_mask[6] & data.pawnblack) == 0)) {
                    (temp = pas::increment(temp, 1));
                }
            } else {
                if (((data.papan[14] != petir_header::_pionputih) && (data.papan[22] != petir_header::_pionputih))) {
                    (temp = pas::increment(temp, 2));
                }
            }
            if (((petir_bitboard_mask::file_mask[7] & data.pawnwhite) == 0)) {
                (temp = pas::increment(temp, 2));
                if (((petir_bitboard_mask::file_mask[7] & data.pawnblack) == 0)) {
                    (temp = pas::increment(temp, 1));
                }
            } else {
                if (((data.papan[15] != petir_header::_pionputih) && (data.papan[23] != petir_header::_pionputih))) {
                    (temp = pas::increment(temp, 2));
                }
            }
            if (((petir_bitboard_mask::file_mask[5] & data.pawnwhite) == 0)) {
                (temp = pas::increment(temp, 1));
            }
            if (((data.papan[petir_header::g2] != petir_header::_gajahputih) && (data.papan[petir_header::g2] != petir_header::_pionputih))) {
                (nilai = pas::decrement(nilai, petir_ueval::nilai_no_fianchetto));
            }
            if ((data.wc != data.bc)) {
                (temp = pas::increment(temp, 1));
            }
            if ((data.queenblack == 0)) {
                temp = (temp / 2);
            }
            (nilai = pas::decrement(nilai, petir_ueval::nilai_pawn_shield[temp]));
        } else {
            if ((filerajaputih <= 2)) {
                if (((petir_bitboard_mask::file_mask[0] & data.pawnwhite) == 0)) {
                    (temp = pas::increment(temp, 3));
                    if (((petir_bitboard_mask::file_mask[0] & data.pawnblack) == 0)) {
                        (temp = pas::increment(temp, 1));
                    }
                } else {
                    if (((data.papan[8] != petir_header::_pionputih) && (data.papan[16] != petir_header::_pionputih))) {
                        (temp = pas::increment(temp, 2));
                    }
                }
                if (((petir_bitboard_mask::file_mask[1] & data.pawnwhite) == 0)) {
                    (temp = pas::increment(temp, 4));
                    if (((petir_bitboard_mask::file_mask[1] & data.pawnblack) == 0)) {
                        (temp = pas::increment(temp, 1));
                    }
                } else {
                    if (((data.papan[9] != petir_header::_pionputih) && (data.papan[17] != petir_header::_pionputih))) {
                        (temp = pas::increment(temp, 2));
                    }
                }
                if (((petir_bitboard_mask::file_mask[2] & data.pawnwhite) == 0)) {
                    (temp = pas::increment(temp, 1));
                }
                if (((data.papan[petir_header::b2] != petir_header::_gajahputih) && (data.papan[petir_header::b2] != petir_header::_pionputih))) {
                    (nilai = pas::decrement(nilai, petir_ueval::nilai_no_fianchetto));
                }
                if ((data.wc != data.bc)) {
                    (temp = pas::increment(temp, 1));
                }
                if ((data.queenblack == 0)) {
                    temp = (temp / 2);
                }
                (nilai = pas::decrement(nilai, petir_ueval::nilai_pawn_shield[temp]));
            } else {
                tempx = filerajaputih;
                if ((tempx > 0)) {
                    if (((petir_bitboard_mask::file_mask[pas::sub(tempx, 1)] & data.pawnwhite) == 0)) {
                        (temp = pas::increment(temp, 2));
                    }
                }
                if (((petir_bitboard_mask::file_mask[tempx] & data.pawnwhite) == 0)) {
                    (temp = pas::increment(temp, 2));
                }
                if ((tempx < 7)) {
                    if (((petir_bitboard_mask::file_mask[pas::add(tempx, 1)] & data.pawnwhite) == 0)) {
                        (temp = pas::increment(temp, 2));
                    }
                }
                if ((data.queenblack == 0)) {
                    temp = (temp / 2);
                }
                (nilai = pas::decrement(nilai, petir_ueval::nilai_pawn_shield[temp]));
            }
        }
    }
    if ((((data.queenwhite != 0) && (data.nilai_perwira_putih >= 16)) || (data.nilai_perwira_putih >= 19))) {
        temp = 0;
        if ((filerajahitam >= 5)) {
            if (((petir_bitboard_mask::file_mask[6] & data.pawnblack) == 0)) {
                (temp = pas::increment(temp, 4));
                if (((petir_bitboard_mask::file_mask[6] & data.pawnwhite) == 0)) {
                    (temp = pas::increment(temp, 1));
                }
            } else {
                if (((data.papan[54] != petir_header::_pionhitam) && (data.papan[46] != petir_header::_pionhitam))) {
                    (temp = pas::increment(temp, 2));
                }
            }
            if (((petir_bitboard_mask::file_mask[7] & data.pawnblack) == 0)) {
                (temp = pas::increment(temp, 2));
                if (((petir_bitboard_mask::file_mask[7] & data.pawnwhite) == 0)) {
                    (temp = pas::increment(temp, 1));
                }
            } else {
                if (((data.papan[55] != petir_header::_pionhitam) && (data.papan[47] != petir_header::_pionhitam))) {
                    (temp = pas::increment(temp, 2));
                }
            }
            if (((petir_bitboard_mask::file_mask[5] & data.pawnblack) == 0)) {
                (temp = pas::increment(temp, 1));
            }
            if (((data.papan[petir_header::g7] != petir_header::_pionhitam) && (data.papan[petir_header::g7] != petir_header::_gajahhitam))) {
                (nilai = pas::increment(nilai, petir_ueval::nilai_no_fianchetto));
            }
            if ((data.wc != data.bc)) {
                (temp = pas::increment(temp, 1));
            }
            if ((data.queenwhite == 0)) {
                temp = (temp / 2);
            }
            (nilai = pas::increment(nilai, petir_ueval::nilai_pawn_shield[temp]));
        } else {
            if ((filerajahitam <= 2)) {
                if (((petir_bitboard_mask::file_mask[0] & data.pawnblack) == 0)) {
                    (temp = pas::increment(temp, 3));
                    if (((petir_bitboard_mask::file_mask[0] & data.pawnwhite) == 0)) {
                        (temp = pas::increment(temp, 1));
                    }
                } else {
                    if (((data.papan[48] != petir_header::_pionhitam) && (data.papan[40] != petir_header::_pionhitam))) {
                        (temp = pas::increment(temp, 2));
                    }
                }
                if (((petir_bitboard_mask::file_mask[1] & data.pawnblack) == 0)) {
                    (temp = pas::increment(temp, 4));
                    if (((petir_bitboard_mask::file_mask[1] & data.pawnwhite) == 0)) {
                        (temp = pas::increment(temp, 1));
                    }
                } else {
                    if (((data.papan[49] != petir_header::_pionhitam) && (data.papan[41] != petir_header::_pionhitam))) {
                        (temp = pas::increment(temp, 2));
                    }
                }
                if (((petir_bitboard_mask::file_mask[2] & data.pawnblack) == 0)) {
                    (temp = pas::increment(temp, 1));
                }
                if (((data.papan[petir_header::b7] != petir_header::_pionhitam) && (data.papan[petir_header::b7] != petir_header::_gajahhitam))) {
                    (nilai = pas::increment(nilai, petir_ueval::nilai_no_fianchetto));
                }
                if ((data.wc != data.bc)) {
                    (temp = pas::increment(temp, 1));
                }
                if ((data.queenwhite == 0)) {
                    temp = (temp / 2);
                }
                (nilai = pas::increment(nilai, petir_ueval::nilai_pawn_shield[temp]));
            } else {
                if ((filerajahitam > 0)) {
                    if (((petir_bitboard_mask::file_mask[pas::sub(filerajahitam, 1)] & data.pawnblack) == 0)) {
                        (temp = pas::increment(temp, 2));
                    }
                }
                if (((petir_bitboard_mask::file_mask[filerajahitam] & data.pawnblack) == 0)) {
                    (temp = pas::increment(temp, 2));
                }
                if ((filerajahitam < 7)) {
                    if (((petir_bitboard_mask::file_mask[pas::add(filerajahitam, 1)] & data.pawnblack) == 0)) {
                        (temp = pas::increment(temp, 2));
                    }
                }
                if ((data.queenwhite == 0)) {
                    temp = (temp / 2);
                }
                (nilai = pas::increment(nilai, petir_ueval::nilai_pawn_shield[temp]));
            }
        }
    }
    pas_temp_1 = data.ngajahhitam;
    if (((pas_temp_1 == 1))) {
        posgajah = petir_bitboard_mask::lastbitp(pas::address(data.bishopblack));
        if ((endgame && (data.ngajahputih == 0))) {
            temp64 = (data.pawnblack | data.pawnwhite);
            if ((((temp64 & petir_evalmask::abc_mask) != 0) && ((temp64 & petir_evalmask::fgh_mask) != 0))) {
                if (real_endgame) {
                    (nilai = pas::decrement(nilai, petir_ueval::nilai_bishop_over_knight_endgame));
                } else {
                    (nilai = pas::decrement(nilai, pas::shr(petir_ueval::nilai_bishop_over_knight_endgame, 1)));
                }
            }
        }
    }
    else if (((pas_temp_1 == 2))) {
        posgajah = 64;
        if ((data.ngajahputih == 0)) {
            (nilai = pas::decrement(nilai, petir_ueval::nilai_bishop_pair0[pas::shr(count_bits<NativePopcount>((data.pawnblack | data.pawnwhite)), 1)]));
        }
        if ((endgame && (data.ngajahputih == 1))) {
            temp64 = (data.pawnblack | data.pawnwhite);
            if ((((temp64 & petir_evalmask::abc_mask) != 0) && ((temp64 & petir_evalmask::fgh_mask) != 0))) {
                if (real_endgame) {
                    (nilai = pas::decrement(nilai, petir_ueval::nilai_bishop_over_knight_endgame));
                } else {
                    (nilai = pas::decrement(nilai, pas::shr(petir_ueval::nilai_bishop_over_knight_endgame, 1)));
                }
            }
        }
    }
    else {
        posgajah = 64;
    }
    const auto black_pawn_files = pawn_files(data.pawnblack);
    const auto black_isolated_files = black_pawn_files & ~((black_pawn_files << 1) | (black_pawn_files >> 1));
    npionhitam = count_bits<NativePopcount>(data.pawnblack);
    temp2 = posgajah < 64 ? count_bits<NativePopcount>(data.pawnblack & UINT64_C(0x0000ffffffffffff)
        & (petir_ueval::color[posgajah] == 0 ? UINT64_C(0xaa55aa55aa55aa55) : UINT64_C(0x55aa55aa55aa55aa))) : 0;
    // Each occupied file has exactly one foremost pawn; all others are doubled.
    temp3 = npionhitam - count_bits<NativePopcount>(std::uint64_t(black_pawn_files));
    temp4 = 0;
    piecepos = data.pawnblack;
    while ((piecepos != 0)) {
        pos = pop_lsb(piecepos);
        (nilai = pas::decrement(nilai, petir_ueval::nilai_pos_pion_b[pos]));
        if ((kingotherside && (petir_evalmask::onside[pos] == petir_evalmask::onside[posrajaputih]))) {
            (nilai = pas::decrement(nilai, petir_ueval::nilai_pos_pion_b[pos]));
        }
        if (((pos >= petir_header::a7) && (data.papan[pas::sub(pos, 8)] == petir_header::_pionputih))) {
            (nilai = pas::increment(nilai, petir_ueval::nilai_blocked_pawn));
        }
        if ((black_isolated_files & (1 << (pos & 7))) != 0) {
            if ((data.nilai_perwira_putih != 0)) {
                (temp4 = pas::increment(temp4, 1));
                (nilai = pas::increment(nilai, pas::shl(petir_ueval::nilai_pion_terisolasi[(pos & 7)], 1)));
                tempb = false;
                if (((petir_bitboard_mask::rook_attack_up_mask[pos] & data.pawnblack) != 0)) {
                    (nilai = pas::increment(nilai, pas::shl(petir_ueval::nilai_pion_terisolasi[(pos & 7)], 1)));
                    tempb = true;
                }
                if (((petir_bitboard_mask::rook_attack_down_mask[pos] & (data.pawnwhite | data.pawnblack)) == 0)) {
                    (nilai = pas::increment(nilai, petir_ueval::nilai_pion_terisolasi[(pos & 7)]));
                    if (tempb) {
                        (nilai = pas::increment(nilai, petir_ueval::nilai_pion_terisolasi[(pos & 7)]));
                    }
                }
                if (((pos == petir_header::a7) || (pos == petir_header::b7))) {
                    if (((((data.papan[petir_header::a6] == petir_header::_pionputih) && (data.papan[petir_header::b5] == petir_header::_pionputih)) || ((data.papan[petir_header::b6] == petir_header::_pionputih) && (data.papan[petir_header::a5] == petir_header::_pionputih))) || ((data.papan[petir_header::b6] == petir_header::_pionputih) && (data.papan[petir_header::a6] == petir_header::_pionputih)))) {
                        (nilai = pas::increment(nilai, petir_ueval::nilai_potential_passer));
                    } else {
                        if (((data.papan[petir_header::a5] == petir_header::_pionputih) && (data.papan[petir_header::b5] == petir_header::_pionputih))) {
                            (nilai = pas::increment(nilai, pas::shr(petir_ueval::nilai_potential_passer, 1)));
                        }
                    }
                } else {
                    if (((pos == petir_header::g7) || (pos == petir_header::h7))) {
                        if (((((data.papan[petir_header::g6] == petir_header::_pionputih) && (data.papan[petir_header::h5] == petir_header::_pionputih)) || ((data.papan[petir_header::h6] == petir_header::_pionputih) && (data.papan[petir_header::g5] == petir_header::_pionputih))) || ((data.papan[petir_header::h6] == petir_header::_pionputih) && (data.papan[petir_header::g6] == petir_header::_pionputih)))) {
                            (nilai = pas::increment(nilai, petir_ueval::nilai_potential_passer));
                        } else {
                            if (((data.papan[petir_header::g5] == petir_header::_pionputih) && (data.papan[petir_header::h5] == petir_header::_pionputih))) {
                                (nilai = pas::increment(nilai, pas::shr(petir_ueval::nilai_potential_passer, 1)));
                            }
                        }
                    }
                }
            }
        }
        if ((((petir_bitboard_mask::b_pion_bebas_mask[pos] & data.pawnwhite) == 0) && ((pionbebashitam & petir_header::bit8mask[(pos & 7)]) == 0))) {
            (npphitam = pas::increment(npphitam, 1));
            tempx = (pos & 7);
            (nilai = pas::decrement(nilai, petir_ueval::nilai_pion_bebas));
            temp = 0;
            pionbebashitam = (pionbebashitam | petir_header::bit8mask[tempx]);
            if (((tempx > 0) && ((pionbebashitam & petir_header::bit8mask[pas::sub(tempx, 1)]) != 0))) {
                (temp = pas::increment(temp, petir_ueval::nilai_pion_bebas_connected2));
            }
            if (((tempx < 7) && ((pionbebashitam & petir_header::bit8mask[pas::add(tempx, 1)]) != 0))) {
                (temp = pas::increment(temp, petir_ueval::nilai_pion_bebas_connected2));
            }
            if ((pas::shr(pos, 3) <= 2)) {
                if ((tempx > 0)) {
                    if (((pionbebashitam & petir_header::bit8mask[pas::sub(tempx, 1)]) != 0)) {
                        if ((data.papan[pas::sub(pos, 1)] == petir_header::_pionhitam)) {
                            (temp = pas::increment(temp, petir_ueval::nilai_pion_bebas_connected));
                        } else {
                            if ((data.papan[pas::sub(pos, 9)] == petir_header::_pionhitam)) {
                                (temp = pas::increment(temp, pas::add(petir_ueval::nilai_pion_bebas_connected, 30)));
                            } else {
                                (temp = pas::increment(temp, petir_ueval::nilai_pion_bebas_connected3));
                            }
                        }
                    }
                }
                if ((tempx < 7)) {
                    if (((pionbebashitam & petir_header::bit8mask[pas::add(tempx, 1)]) != 0)) {
                        if ((data.papan[pas::add(pos, 1)] == petir_header::_pionhitam)) {
                            (temp = pas::increment(temp, petir_ueval::nilai_pion_bebas_connected));
                        } else {
                            if ((data.papan[pas::sub(pos, 7)] == petir_header::_pionhitam)) {
                                (temp = pas::increment(temp, pas::add(petir_ueval::nilai_pion_bebas_connected, 30)));
                            } else {
                                (temp = pas::increment(temp, petir_ueval::nilai_pion_bebas_connected3));
                            }
                        }
                    }
                }
            } else {
                if ((pas::shr(pos, 3) <= 3)) {
                    if ((tempx > 0)) {
                        if (((pionbebashitam & petir_header::bit8mask[pas::sub(tempx, 1)]) != 0)) {
                            if ((data.papan[pas::sub(pos, 1)] == petir_header::_pionhitam)) {
                                (temp = pas::increment(temp, petir_ueval::nilai_pion_bebas_connected3));
                            } else {
                                if ((data.papan[pas::sub(pos, 9)] == petir_header::_pionhitam)) {
                                    (temp = pas::increment(temp, pas::add(petir_ueval::nilai_pion_bebas_connected3, 20)));
                                }
                            }
                        }
                    }
                    if ((tempx < 7)) {
                        if (((pionbebashitam & petir_header::bit8mask[pas::add(tempx, 1)]) != 0)) {
                            if ((data.papan[pas::add(pos, 1)] == petir_header::_pionhitam)) {
                                (temp = pas::increment(temp, petir_ueval::nilai_pion_bebas_connected3));
                            } else {
                                if ((data.papan[pas::sub(pos, 7)] == petir_header::_pionhitam)) {
                                    (temp = pas::increment(temp, pas::add(petir_ueval::nilai_pion_bebas_connected3, 20)));
                                }
                            }
                        }
                    }
                }
            }
            if ((data.nilai_perwira_putih == 0)) {
                if (((pos <= petir_header::h3) && ((petir_bitboard_mask::b_pawn_attack[pos] & data.kingblack) != 0))) {
                    (nilai = pas::decrement(nilai, petir_ueval::nilai_can_promote));
                    (nilai = pas::decrement(nilai, pas::shl(petir_ueval::b_nilai_pion_advance_bebas[pas::shr(pos, 3)], 1)));
                    continue;
                }
                if (((((pos <= petir_header::g2) && (pos >= petir_header::b2)) && (data.papan[pas::sub(pos, 8)] != petir_header::_rajaputih)) && ((petir_bitboard_mask::kingmask[pos] & data.kingblack) != 0))) {
                    if ((((((posrajahitam >= petir_header::a3) && ((petir_bitboard_mask::kingmask[pos] & data.kingwhite) != 0)) && (realgiliran == petir_header::_sisiputih)) && (data.nilai_perwira_hitam == 0)) && ((posrajaputih == pas::sub(pos, 1)) || (posrajaputih == pas::add(pos, 1))))) {
                    } else {
                        (nilai = pas::decrement(nilai, petir_ueval::nilai_can_promote));
                        (nilai = pas::decrement(nilai, pas::shl(petir_ueval::b_nilai_pion_advance_bebas[pas::shr(pos, 3)], 1)));
                        continue;
                    }
                }
                if (((pas::add(pas::shr(pos, 3), 1) < pas::shr(posrajaputih, 3)) || ((pas::shr(pos, 3) < pas::shr(posrajaputih, 3)) && (realgiliran == petir_header::_sisihitam)))) {
                    (nilai = pas::decrement(nilai, petir_ueval::nilai_can_promote));
                    (nilai = pas::decrement(nilai, petir_ueval::b_nilai_pion_advance_bebas[pas::shr(pos, 3)]));
                    continue;
                }
                if (((pas::shr(pos, 3) < pas::sub(pas::abs(pas::sub((pos & 7), filerajaputih)), 1)) || ((pas::shr(pos, 3) == pas::sub(pas::abs(pas::sub((pos & 7), filerajaputih)), 1)) && (realgiliran == petir_header::_sisihitam)))) {
                    (nilai = pas::decrement(nilai, petir_ueval::nilai_can_promote));
                    (nilai = pas::decrement(nilai, petir_ueval::b_nilai_pion_advance_bebas[pas::shr(pos, 3)]));
                    continue;
                }
                if (((pas::shr(pos, 3) == 2) && ((petir_bitboard_mask::b_pion_bebas_mask[pos] & data.kingblack) != 0))) {
                    (nilai = pas::decrement(nilai, petir_ueval::nilai_can_promote));
                    (nilai = pas::decrement(nilai, petir_ueval::b_nilai_pion_advance_bebas[pas::shr(pos, 3)]));
                    continue;
                }
            }
            (temp = pas::increment(temp, petir_ueval::b_nilai_pion_advance_bebas[pas::shr(pos, 3)]));
            verattack = petir_bitboard_mask::rook_attack_up_mask[pos];
            if (((verattack & data.rookblack) != 0)) {
                if ((((petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)] & data.rookblack) != 0) && (!(data.papan[pas::sub(pos, 8)] > 0)))) {
                    (temp = pas::increment(temp, petir_ueval::nilai_benteng_dibelakang_pion_bebas));
                }
            } else {
                if (((verattack & data.rookwhite) != 0)) {
                    if (((petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)] & data.rookwhite) != 0)) {
                        (temp = pas::decrement(temp, pas::shr(petir_ueval::nilai_benteng_dibelakang_pion_bebas, 1)));
                        if ((((data.nilai_perwira_putih == 5) && (data.nilai_perwira_hitam == 5)) && (petir_evalmask::tabel_edistance2[pos][posrajahitam] <= 5))) {
                            (temp = pas::decrement(temp, pas::shr(petir_ueval::nilai_benteng_dibelakang_pion_bebas, 2)));
                        }
                    }
                }
            }
            if ((data.papan[pas::sub(pos, 8)] > 0)) {
                (temp = pas::decrement(temp, pas::shr(petir_ueval::b_nilai_pion_advance_bebas[pas::shr(pos, 3)], 1)));
                (temp = pas::increment(temp, pas::shr(petir_ueval::b_nilai_pion_advance_bebas[pas::shr(pos, 3)], 3)));
            } else {
                if ((((pos <= petir_header::h2) && (data.papan[pas::sub(pos, 8)] == 0)) && (petir_usee::see(petir_header::_sisihitam, data, pos, pas::sub(pos, 8)) == 0))) {
                    if ((realgiliran == petir_header::_sisihitam)) {
                        (nilai = pas::decrement(nilai, pas::add(petir_ueval::nilai_can_advance, 40)));
                    } else {
                        (nilai = pas::decrement(nilai, petir_ueval::nilai_can_advance));
                    }
                }
            }
            if (((pas::sub(posrajahitam, pos) <= 9) && ((petir_bitboard_mask::kingmask[pos] & data.kingblack) != 0))) {
                (temp = pas::increment(temp, petir_ueval::b_nilai_pion_bebas_didukung_raja[pas::shr(pos, 3)]));
            } else {
                if ((((petir_evalmask::tabel_edistance2[pos][posrajaputih] >= 7) && (pas::shr(posrajaputih, 3) <= pas::shr(pos, 3))) && (pas::shr(pos, 3) >= pas::abs(pas::sub(filerajaputih, (pos & 7)))))) {
                    if ((petir_evalmask::tabel_edistance2[pos][posrajaputih] >= 8)) {
                        (temp = pas::decrement(temp, pas::shr(petir_ueval::b_nilai_pion_advance_bebas[pas::shr(pos, 3)], 1)));
                    } else {
                        (temp = pas::decrement(temp, pas::shr(petir_ueval::b_nilai_pion_advance_bebas[pas::shr(pos, 3)], 2)));
                    }
                }
            }
            if ((temp > 0)) {
                (nilai = pas::decrement(nilai, (pas::mul(temp, petir_ueval::nilai_endgame_scale[std::min<int>(data.nilai_perwira_putih, 41)]) / 100)));
            }
        }
    }
    pawnbattack = pas::shr((data.pawnblack & petir_bitboard_mask::mask_left), 9);
    pawnbattack = (pawnbattack | pas::shr((data.pawnblack & petir_bitboard_mask::mask_right), 7));
    (nilai = pas::increment(nilai, petir_ueval::nilai_pion_tumpuk[temp3]));
    (nilai = pas::increment(nilai, petir_ueval::nilai_pion_terisolasi_ak[temp4]));
    if (petir_ueval::onlyone(data.bishopblack)) {
        (nilai = pas::increment(nilai, petir_evalmask::nilai_badbishop[temp2]));
    }
    tempk64 = (pawnbattack & whitekingattack);
    (whitekingpressure = pas::increment(whitekingpressure, count_bits<NativePopcount>(tempk64)));
    pas_temp_2 = data.ngajahputih;
    if (((pas_temp_2 == 1))) {
        posgajah = petir_bitboard_mask::firstbitp(pas::address(data.bishopwhite));
        if ((endgame && (data.ngajahhitam == 0))) {
            temp64 = (data.pawnblack | data.pawnwhite);
            if ((((temp64 & petir_evalmask::abc_mask) != 0) && ((temp64 & petir_evalmask::fgh_mask) != 0))) {
                if (real_endgame) {
                    (nilai = pas::increment(nilai, petir_ueval::nilai_bishop_over_knight_endgame));
                } else {
                    (nilai = pas::increment(nilai, pas::shr(petir_ueval::nilai_bishop_over_knight_endgame, 1)));
                }
            }
        }
    }
    else if (((pas_temp_2 == 2))) {
        posgajah = 64;
        if ((data.ngajahhitam == 0)) {
            (nilai = pas::increment(nilai, petir_ueval::nilai_bishop_pair0[pas::shr(count_bits<NativePopcount>((data.pawnblack | data.pawnwhite)), 1)]));
        }
        if ((endgame && (data.ngajahhitam == 1))) {
            temp64 = (data.pawnblack | data.pawnwhite);
            if ((((temp64 & petir_evalmask::abc_mask) != 0) && ((temp64 & petir_evalmask::fgh_mask) != 0))) {
                if (real_endgame) {
                    (nilai = pas::increment(nilai, petir_ueval::nilai_bishop_over_knight_endgame));
                } else {
                    (nilai = pas::increment(nilai, pas::shr(petir_ueval::nilai_bishop_over_knight_endgame, 1)));
                }
            }
        }
    }
    else {
        posgajah = 64;
    }
    piecepos = data.pawnwhite;
    const auto white_pawn_files = pawn_files(data.pawnwhite);
    const auto white_isolated_files = white_pawn_files & ~((white_pawn_files << 1) | (white_pawn_files >> 1));
    npionputih = count_bits<NativePopcount>(data.pawnwhite);
    temp2 = posgajah < 64 ? count_bits<NativePopcount>(data.pawnwhite & UINT64_C(0xffffffffffff0000)
        & (petir_ueval::color[posgajah] == 0 ? UINT64_C(0xaa55aa55aa55aa55) : UINT64_C(0x55aa55aa55aa55aa))) : 0;
    temp3 = npionputih - count_bits<NativePopcount>(std::uint64_t(white_pawn_files));
    temp4 = 0;
    while ((piecepos != 0)) {
        // Evaluate the foremost white passer before marking its file handled.
        pos = pop_msb(piecepos);
        (nilai = pas::increment(nilai, petir_ueval::nilai_pos_pion_w[pos]));
        if ((kingotherside && (petir_evalmask::onside[pos] == petir_evalmask::onside[posrajahitam]))) {
            (nilai = pas::increment(nilai, petir_ueval::nilai_pos_pion_w[pos]));
        }
        if ((data.papan[pas::add(pos, 8)] == petir_header::_pionhitam)) {
            if ((pos <= petir_header::h2)) {
                (nilai = pas::decrement(nilai, petir_ueval::nilai_blocked_pawn));
            }
        }
        if ((white_isolated_files & (1 << (pos & 7))) != 0) {
            if ((data.nilai_perwira_hitam != 0)) {
                (temp4 = pas::increment(temp4, 1));
                (nilai = pas::decrement(nilai, pas::shl(petir_ueval::nilai_pion_terisolasi[(pos & 7)], 1)));
                tempb = false;
                if (((petir_bitboard_mask::rook_attack_down_mask[pos] & data.pawnwhite) != 0)) {
                    (nilai = pas::decrement(nilai, pas::shl(petir_ueval::nilai_pion_terisolasi[(pos & 7)], 1)));
                    tempb = true;
                }
                if (((petir_bitboard_mask::rook_attack_up_mask[pos] & (data.pawnblack | data.pawnwhite)) == 0)) {
                    (nilai = pas::decrement(nilai, petir_ueval::nilai_pion_terisolasi[(pos & 7)]));
                    if (tempb) {
                        (nilai = pas::decrement(nilai, petir_ueval::nilai_pion_terisolasi[(pos & 7)]));
                    }
                }
                if (((pos == petir_header::a2) || (pos == petir_header::b2))) {
                    if (((((data.papan[petir_header::a3] == petir_header::_pionhitam) && (data.papan[petir_header::b4] == petir_header::_pionhitam)) || ((data.papan[petir_header::b3] == petir_header::_pionhitam) && (data.papan[petir_header::a4] == petir_header::_pionhitam))) || ((data.papan[petir_header::b3] == petir_header::_pionhitam) && (data.papan[petir_header::a3] == petir_header::_pionhitam)))) {
                        (nilai = pas::decrement(nilai, petir_ueval::nilai_potential_passer));
                    } else {
                        if (((data.papan[petir_header::a4] == petir_header::_pionhitam) && (data.papan[petir_header::b4] == petir_header::_pionhitam))) {
                            (nilai = pas::decrement(nilai, pas::shr(petir_ueval::nilai_potential_passer, 1)));
                        }
                    }
                } else {
                    if (((pos == petir_header::g2) || (pos == petir_header::h2))) {
                        if (((((data.papan[petir_header::g3] == petir_header::_pionhitam) && (data.papan[petir_header::h4] == petir_header::_pionhitam)) || ((data.papan[petir_header::h3] == petir_header::_pionhitam) && (data.papan[petir_header::g4] == petir_header::_pionhitam))) || ((data.papan[petir_header::h3] == petir_header::_pionhitam) && (data.papan[petir_header::g3] == petir_header::_pionhitam)))) {
                            (nilai = pas::decrement(nilai, petir_ueval::nilai_potential_passer));
                        } else {
                            if (((data.papan[petir_header::g4] == petir_header::_pionhitam) && (data.papan[petir_header::h4] == petir_header::_pionhitam))) {
                                (nilai = pas::decrement(nilai, pas::shr(petir_ueval::nilai_potential_passer, 1)));
                            }
                        }
                    }
                }
            }
        }
        if ((((petir_bitboard_mask::w_pion_bebas_mask[pos] & data.pawnblack) == 0) && ((pionbebasputih & petir_header::bit8mask[(pos & 7)]) == 0))) {
            (nppputih = pas::increment(nppputih, 1));
            tempi = 0;
            tempx = (pos & 7);
            (nilai = pas::increment(nilai, petir_ueval::nilai_pion_bebas));
            temp = pas::shr(pos, 3);
            pionbebasputih = (pionbebasputih | petir_header::bit8mask[tempx]);
            if (((tempx > 0) && ((pionbebasputih & petir_header::bit8mask[pas::sub(tempx, 1)]) != 0))) {
                (tempi = pas::increment(tempi, petir_ueval::nilai_pion_bebas_connected2));
            }
            if (((tempx < 7) && ((pionbebasputih & petir_header::bit8mask[pas::add(tempx, 1)]) != 0))) {
                (tempi = pas::increment(tempi, petir_ueval::nilai_pion_bebas_connected2));
            }
            if ((temp >= 5)) {
                if ((tempx > 0)) {
                    if (((pionbebasputih & petir_header::bit8mask[pas::sub(tempx, 1)]) != 0)) {
                        if ((data.papan[pas::sub(pos, 1)] == petir_header::_pionputih)) {
                            (tempi = pas::increment(tempi, petir_ueval::nilai_pion_bebas_connected));
                        } else {
                            if ((data.papan[pas::add(pos, 7)] == petir_header::_pionputih)) {
                                (tempi = pas::increment(tempi, pas::add(petir_ueval::nilai_pion_bebas_connected, 30)));
                            } else {
                                (tempi = pas::increment(tempi, petir_ueval::nilai_pion_bebas_connected3));
                            }
                        }
                    }
                }
                if ((tempx < 7)) {
                    if (((pionbebasputih & petir_header::bit8mask[pas::add(tempx, 1)]) != 0)) {
                        if ((data.papan[pas::add(pos, 1)] == petir_header::_pionputih)) {
                            (tempi = pas::increment(tempi, petir_ueval::nilai_pion_bebas_connected));
                        } else {
                            if ((data.papan[pas::add(pos, 9)] == petir_header::_pionputih)) {
                                (tempi = pas::increment(tempi, pas::add(petir_ueval::nilai_pion_bebas_connected, 30)));
                            } else {
                                (tempi = pas::increment(tempi, petir_ueval::nilai_pion_bebas_connected3));
                            }
                        }
                    }
                }
            } else {
                if ((temp >= 4)) {
                    if ((tempx > 0)) {
                        if (((pionbebasputih & petir_header::bit8mask[pas::sub(tempx, 1)]) != 0)) {
                            if ((data.papan[pas::sub(pos, 1)] == petir_header::_pionputih)) {
                                (tempi = pas::increment(tempi, petir_ueval::nilai_pion_bebas_connected3));
                            }
                            if ((data.papan[pas::add(pos, 7)] == petir_header::_pionputih)) {
                                (tempi = pas::increment(tempi, pas::add(petir_ueval::nilai_pion_bebas_connected3, 20)));
                            }
                        }
                    }
                    if ((tempx < 7)) {
                        if (((pionbebasputih & petir_header::bit8mask[pas::add(tempx, 1)]) != 0)) {
                            if ((data.papan[pas::add(pos, 1)] == petir_header::_pionputih)) {
                                (tempi = pas::increment(tempi, petir_ueval::nilai_pion_bebas_connected3));
                            } else {
                                if ((data.papan[pas::add(pos, 9)] == petir_header::_pionputih)) {
                                    (tempi = pas::increment(tempi, pas::add(petir_ueval::nilai_pion_bebas_connected3, 20)));
                                }
                            }
                        }
                    }
                }
            }
            if ((data.nilai_perwira_hitam == 0)) {
                if (((pos >= petir_header::a6) && ((petir_bitboard_mask::w_pawn_attack[pos] & data.kingwhite) != 0))) {
                    (nilai = pas::increment(nilai, petir_ueval::nilai_can_promote));
                    (nilai = pas::increment(nilai, pas::shl(petir_ueval::w_nilai_pion_advance_bebas[pas::shr(pos, 3)], 1)));
                    continue;
                }
                if (((((pos >= petir_header::b7) && (pos <= petir_header::g7)) && (data.papan[pas::add(pos, 8)] != petir_header::_rajahitam)) && ((petir_bitboard_mask::kingmask[pos] & data.kingwhite) != 0))) {
                    if ((((((posrajahitam <= petir_header::h6) && ((petir_bitboard_mask::kingmask[pos] & data.kingblack) != 0)) && (realgiliran == petir_header::_sisihitam)) && (data.nilai_perwira_putih == 0)) && ((posrajahitam == pas::sub(pos, 1)) || (posrajahitam == pas::add(pos, 1))))) {
                    } else {
                        (nilai = pas::increment(nilai, petir_ueval::nilai_can_promote));
                        (nilai = pas::increment(nilai, pas::shl(petir_ueval::w_nilai_pion_advance_bebas[pas::shr(pos, 3)], 1)));
                        continue;
                    }
                }
                if (((pas::shr(pos, 3) > pas::add(pas::shr(posrajahitam, 3), 1)) || ((pas::shr(pos, 3) > pas::shr(posrajahitam, 3)) && (realgiliran == petir_header::_sisiputih)))) {
                    (nilai = pas::increment(nilai, petir_ueval::nilai_can_promote));
                    (nilai = pas::increment(nilai, petir_ueval::w_nilai_pion_advance_bebas[temp]));
                    continue;
                }
                if (((pas::sub(7, pas::shr(pos, 3)) < pas::sub(pas::abs(pas::sub((pos & 7), filerajahitam)), 1)) || ((pas::sub(7, pas::shr(pos, 3)) == pas::sub(pas::abs(pas::sub((pos & 7), filerajahitam)), 1)) && (realgiliran == petir_header::_sisiputih)))) {
                    (nilai = pas::increment(nilai, petir_ueval::nilai_can_promote));
                    (nilai = pas::increment(nilai, petir_ueval::w_nilai_pion_advance_bebas[temp]));
                    continue;
                }
                if (((pas::shr(pos, 3) == 5) && ((petir_bitboard_mask::w_pion_bebas_mask[pos] & data.kingwhite) != 0))) {
                    (nilai = pas::increment(nilai, petir_ueval::nilai_can_promote));
                    (nilai = pas::increment(nilai, petir_ueval::w_nilai_pion_advance_bebas[pas::shr(pos, 3)]));
                    continue;
                }
            }
            (tempi = pas::increment(tempi, petir_ueval::w_nilai_pion_advance_bebas[temp]));
            verattack = petir_bitboard_mask::rook_attack_down_mask[pos];
            if (((verattack & data.rookwhite) != 0)) {
                if ((((petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)] & data.rookwhite) != 0) && (!(data.papan[pas::add(pos, 8)] < 0)))) {
                    (tempi = pas::increment(tempi, petir_ueval::nilai_benteng_dibelakang_pion_bebas));
                }
            } else {
                if (((verattack & data.rookblack) != 0)) {
                    if (((petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)] & data.rookblack) != 0)) {
                        (tempi = pas::decrement(tempi, pas::shr(petir_ueval::nilai_benteng_dibelakang_pion_bebas, 1)));
                        if ((((data.nilai_perwira_putih == 5) && (data.nilai_perwira_hitam == 5)) && (petir_evalmask::tabel_edistance2[pos][posrajaputih] <= 5))) {
                            (tempi = pas::decrement(tempi, pas::shr(petir_ueval::nilai_benteng_dibelakang_pion_bebas, 2)));
                        }
                    }
                }
            }
            if ((data.papan[pas::add(pos, 8)] < 0)) {
                (tempi = pas::decrement(tempi, pas::shr(petir_ueval::w_nilai_pion_advance_bebas[temp], 1)));
                (tempi = pas::increment(tempi, pas::shr(petir_ueval::w_nilai_pion_advance_bebas[temp], 3)));
            } else {
                if ((((pos >= petir_header::a7) && (data.papan[pas::add(pos, 8)] == 0)) && (petir_usee::see(petir_header::_sisiputih, data, pos, pas::add(pos, 8)) == 0))) {
                    if ((realgiliran == petir_header::_sisiputih)) {
                        (nilai = pas::increment(nilai, pas::add(petir_ueval::nilai_can_advance, 40)));
                    } else {
                        (nilai = pas::increment(nilai, petir_ueval::nilai_can_advance));
                    }
                }
            }
            if (((pas::sub(pos, posrajaputih) <= 9) && ((petir_bitboard_mask::kingmask[pos] & data.kingwhite) != 0))) {
                (tempi = pas::increment(tempi, petir_ueval::w_nilai_pion_bebas_didukung_raja[temp]));
            } else {
                if (((petir_evalmask::tabel_edistance2[pos][posrajahitam] >= 7) && (pas::shr(posrajahitam, 3) >= pas::shr(pos, 3)))) {
                    if ((petir_evalmask::tabel_edistance2[pos][posrajahitam] >= 8)) {
                        (tempi = pas::decrement(tempi, pas::shr(petir_ueval::w_nilai_pion_advance_bebas[temp], 1)));
                    } else {
                        (tempi = pas::decrement(tempi, pas::shr(petir_ueval::w_nilai_pion_advance_bebas[temp], 2)));
                    }
                }
            }
            if ((tempi > 0)) {
                (nilai = pas::increment(nilai, (pas::mul(tempi, petir_ueval::nilai_endgame_scale[std::min<int>(data.nilai_perwira_hitam, 41)]) / 100)));
            }
        }
    }
    pawnwattack = pas::shl((data.pawnwhite & petir_bitboard_mask::mask_left), 7);
    pawnwattack = (pawnwattack | pas::shl((data.pawnwhite & petir_bitboard_mask::mask_right), 9));
    (nilai = pas::decrement(nilai, petir_ueval::nilai_pion_tumpuk[temp3]));
    (nilai = pas::decrement(nilai, petir_ueval::nilai_pion_terisolasi_ak[temp4]));
    if (petir_ueval::onlyone(data.bishopwhite)) {
        (nilai = pas::decrement(nilai, petir_evalmask::nilai_badbishop[temp2]));
    }
    tempk64 = (pawnwattack & blackkingattack);
    (blackkingpressure = pas::increment(blackkingpressure, count_bits<NativePopcount>(tempk64)));
    if ((((data.nilai_perwira_putih == pas::add(data.nilai_perwira_hitam, 3)) && (data.queenwhite != 0)) && ((pas::sub(npionhitam, npionputih) >= 2 && pas::sub(npionhitam, npionputih) <= 3)))) {
        (nilai = pas::increment(nilai, 35));
    } else {
        if ((((data.nilai_perwira_hitam == pas::add(data.nilai_perwira_putih, 3)) && (data.queenblack != 0)) && ((pas::sub(npionputih, npionhitam) >= 2 && pas::sub(npionputih, npionhitam) <= 3)))) {
            (nilai = pas::decrement(nilai, 35));
        }
    }
    temp = 0;
    piecepos = data.rookwhite;
    while ((piecepos != 0)) {
        pos = pop_lsb(piecepos);
        horattack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        verattack = petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)];
        attack = (horattack | verattack);
        temp64 = (attack & (~data.whitepieces));
        (nilai = pas::increment(nilai, petir_ueval::mobil_rook[count_bits<NativePopcount>(temp64)]));
        rookwattack = (rookwattack | attack);
        if (bevalks) {
            tempk64 = (attack & blackkingattack);
            (blackkingpressure = pas::increment(blackkingpressure, count_bits<NativePopcount>(tempk64)));
            if (tempk64 != 0) {
                (black_attacker = pas::increment(black_attacker, 1));
            }
        }
        (nilai = pas::increment(nilai, petir_ueval::nilai_pos_benteng[pos]));
        if (((petir_bitboard_mask::rook_attack_up_mask[pos] & data.pawnwhite) == 0)) {
            (temp = pas::increment(temp, petir_ueval::nilai_semi_open_file[(pos & 7)]));
            tempb2 = ((petir_evalmask::onside[pos] == petir_evalmask::onside[posrajahitam]) && (!endgame));
            if (tempb2) {
                (temp = pas::increment(temp, petir_ueval::nilai_benteng_semi_open_file_dekat_raja));
            }
            tempb = false;
            if (((petir_bitboard_mask::rook_attack_up_mask[pos] & data.rookwhite) != 0)) {
                (temp = pas::increment(temp, petir_ueval::nilai_benteng_tumpuk_kolom));
                if (tempb2) {
                    (temp = pas::increment(temp, petir_ueval::nilai_benteng_tumpuk_kolom_semi_open_file_dekat_raja));
                }
                tempb = true;
            }
            if (((verattack & data.queenwhite) != 0)) {
                (temp = pas::increment(temp, petir_ueval::nilai_benteng_mentri_semi_open));
                tempb = true;
                if (tempb2) {
                    (temp = pas::increment(temp, petir_ueval::nilai_benteng_mentri_semi_open_dekat_raja));
                }
            }
            if (((petir_bitboard_mask::rook_attack_up_mask[pos] & data.pawnblack) == 0)) {
                (temp = pas::increment(temp, petir_ueval::nilai_open_file[(pos & 7)]));
                if (((petir_bitboard_mask::rook_attack_down_mask[pos] & data.queenwhite) != 0)) {
                    (temp = pas::increment(temp, petir_ueval::nilai_benteng_mentri_open));
                }
                if (tempb2) {
                    (temp = pas::increment(temp, petir_ueval::nilai_benteng_open_file_dekat_raja));
                } else {
                    if (((petir_evalmask::onside[pos] == petir_evalmask::onside[posrajahitam]) && (!endgame))) {
                        (temp = pas::increment(temp, pas::shr(petir_ueval::nilai_benteng_open_file_dekat_raja, 1)));
                    }
                }
                if (tempb) {
                    (temp = pas::increment(temp, petir_ueval::nilai_benteng_tumpuk_kolom_open));
                    if (tempb2) {
                        (temp = pas::increment(temp, petir_ueval::nilai_benteng_tumpuk_kolom_dekat_raja));
                    }
                }
            }
        }
        if (((pas::shr(pos, 3) == 6) && (((petir_bitboard_mask::rank_mask[6] & data.pawnblack) != 0) || ((petir_bitboard_mask::rank_mask[7] & data.kingblack) != 0)))) {
            if (((horattack & data.rookwhite) != 0)) {
                (nilai = pas::increment(nilai, petir_ueval::nilai_benteng_tumpuk_baris7));
            }
            if (((horattack & data.queenwhite) != 0)) {
                (nilai = pas::increment(nilai, petir_ueval::nilai_benteng_mentri_tumpuk_baris7));
            }
            (nilai = pas::increment(nilai, petir_ueval::nilai_benteng_baris7));
        } else {
            if (((pas::shr(pos, 3) == 7) && ((petir_bitboard_mask::rank_mask[7] & data.kingblack) != 0))) {
                (nilai = pas::increment(nilai, petir_ueval::nilai_benteng_baris8));
            }
        }
        h = (pos & 7);
        if ((h == 0)) {
            if ((data.papan[pas::add(pos, 1)] > 0)) {
                (nilai = pas::decrement(nilai, petir_ueval::nilai_benteng_blocked));
            }
        } else {
            if ((h == 7)) {
                if ((data.papan[pas::sub(pos, 1)] > 0)) {
                    (nilai = pas::decrement(nilai, petir_ueval::nilai_benteng_blocked));
                }
            } else {
                if (((data.papan[pas::sub(pos, 1)] > 0) && (data.papan[pas::add(pos, 1)] > 0))) {
                    (nilai = pas::decrement(nilai, petir_ueval::nilai_benteng_blocked));
                }
            }
        }
    }
    (nilai = pas::increment(nilai, (pas::mul(temp, petir_ueval::nilai_rook_scale[std::min<int>(data.nilai_perwira_putih, 41)]) / 100)));
    temp = 0;
    piecepos = data.rookblack;
    while ((piecepos != 0)) {
        // Rook scores and attack unions are independent of iteration order.
        pos = pop_lsb(piecepos);
        horattack = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
        verattack = petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)];
        attack = (horattack | verattack);
        temp64 = (attack & (~data.blackpieces));
        (nilai = pas::decrement(nilai, petir_ueval::mobil_rook[count_bits<NativePopcount>(temp64)]));
        rookbattack = (rookbattack | attack);
        if (wevalks) {
            tempk64 = (attack & whitekingattack);
            (whitekingpressure = pas::increment(whitekingpressure, count_bits<NativePopcount>(tempk64)));
            if (tempk64 != 0) {
                (white_attacker = pas::increment(white_attacker, 1));
            }
        }
        (nilai = pas::decrement(nilai, petir_ueval::nilai_pos_benteng[pos]));
        if (((petir_bitboard_mask::rook_attack_down_mask[pos] & data.pawnblack) == 0)) {
            (temp = pas::increment(temp, petir_ueval::nilai_semi_open_file[(pos & 7)]));
            tempb2 = ((petir_evalmask::onside[pos] == petir_evalmask::onside[posrajaputih]) && (!endgame));
            if (tempb2) {
                (temp = pas::increment(temp, petir_ueval::nilai_benteng_semi_open_file_dekat_raja));
            }
            tempb = false;
            if (((verattack & data.queenblack) != 0)) {
                (temp = pas::increment(temp, petir_ueval::nilai_benteng_mentri_semi_open));
                if (tempb2) {
                    (temp = pas::increment(temp, petir_ueval::nilai_benteng_mentri_semi_open_dekat_raja));
                }
                tempb = true;
            }
            if (((petir_bitboard_mask::rook_attack_down_mask[pos] & data.rookblack) != 0)) {
                (temp = pas::increment(temp, petir_ueval::nilai_benteng_tumpuk_kolom));
                if (tempb2) {
                    (temp = pas::increment(temp, petir_ueval::nilai_benteng_tumpuk_kolom_semi_open_file_dekat_raja));
                }
                tempb = true;
            }
            if (((petir_bitboard_mask::rook_attack_down_mask[pos] & data.pawnwhite) == 0)) {
                (temp = pas::increment(temp, petir_ueval::nilai_open_file[(pos & 7)]));
                if (((petir_bitboard_mask::rook_attack_up_mask[pos] & data.queenblack) != 0)) {
                    (temp = pas::increment(temp, petir_ueval::nilai_benteng_mentri_open));
                }
                if (tempb2) {
                    (temp = pas::increment(temp, petir_ueval::nilai_benteng_open_file_dekat_raja));
                } else {
                    if (((petir_evalmask::onside[pos] == petir_evalmask::onside[posrajaputih]) && (!endgame))) {
                        (temp = pas::increment(temp, pas::shr(petir_ueval::nilai_benteng_open_file_dekat_raja, 1)));
                    }
                }
                if (tempb) {
                    (temp = pas::increment(temp, petir_ueval::nilai_benteng_tumpuk_kolom_open));
                }
                if ((tempb && tempb2)) {
                    (temp = pas::increment(temp, petir_ueval::nilai_benteng_tumpuk_kolom_dekat_raja));
                }
            }
        }
        if (((pas::shr(pos, 3) == 1) && (((petir_bitboard_mask::rank_mask[1] & data.pawnwhite) != 0) || ((petir_bitboard_mask::rank_mask[0] & data.kingwhite) != 0)))) {
            (nilai = pas::decrement(nilai, petir_ueval::nilai_benteng_baris7));
            if (((horattack & data.rookblack) != 0)) {
                (nilai = pas::decrement(nilai, petir_ueval::nilai_benteng_tumpuk_baris7));
            }
            if (((horattack & data.queenblack) != 0)) {
                (nilai = pas::decrement(nilai, petir_ueval::nilai_benteng_mentri_tumpuk_baris7));
            }
        } else {
            if (((pas::shr(pos, 3) == 0) && ((petir_bitboard_mask::rank_mask[0] & data.kingwhite) != 0))) {
                (nilai = pas::decrement(nilai, petir_ueval::nilai_benteng_baris8));
            }
        }
        h = (pos & 7);
        if ((h == 0)) {
            if ((data.papan[pas::add(pos, 1)] < 0)) {
                (nilai = pas::increment(nilai, petir_ueval::nilai_benteng_blocked));
            }
        } else {
            if ((h == 7)) {
                if ((data.papan[pas::sub(pos, 1)] < 0)) {
                    (nilai = pas::increment(nilai, petir_ueval::nilai_benteng_blocked));
                }
            } else {
                if (((data.papan[pas::sub(pos, 1)] < 0) && (data.papan[pas::add(pos, 1)] < 0))) {
                    (nilai = pas::increment(nilai, petir_ueval::nilai_benteng_blocked));
                }
            }
        }
    }
    (nilai = pas::decrement(nilai, (pas::mul(temp, petir_ueval::nilai_rook_scale[std::min<int>(data.nilai_perwira_hitam, 41)]) / 100)));
    piecepos = data.knightwhite;
    while ((piecepos != 0)) {
        pos = pop_lsb(piecepos);
        (nilai = pas::increment(nilai, petir_ueval::nilai_pos_kuda_w[pos]));
        if ((!early_endgame)) {
            (nilai = pas::increment(nilai, pas::mul(petir_evalmask::tabel_edistance[pos][posrajahitam], petir_ueval::nilai_tropism_kuda)));
        }
        attack = petir_bitboard_mask::knightmask[pos];
        knightwattack = (knightwattack | attack);
        temp64 = (attack & (~data.whitepieces));
        (nilai = pas::increment(nilai, petir_ueval::mobil_knight[count_bits<NativePopcount>(temp64)]));
        if (bevalks) {
            tempk64 = (attack & blackkingattack);
            (blackkingpressure = pas::increment(blackkingpressure, count_bits<NativePopcount>(tempk64)));
            if (tempk64 != 0) {
                (black_attacker = pas::increment(black_attacker, 1));
            }
        }
        if (((petir_ueval::nilai_kuda_outpost_w[pos] > 0) && (!endgame))) {
            if (((petir_evalmask::w_outpost_mask[pos] & data.pawnblack) == 0)) {
                temp64 = (petir_bitboard_mask::b_pawn_attack[pos] & data.pawnwhite);
                temp3 = count_bits<NativePopcount>(temp64);
                pas_temp_3 = temp3;
                if (((pas_temp_3 == 1))) {
                    (nilai = pas::increment(nilai, pas::shr(petir_ueval::nilai_kuda_outpost_w[pos], 1)));
                }
                else if (((pas_temp_3 == 2))) {
                    (nilai = pas::increment(nilai, petir_ueval::nilai_kuda_outpost_w[pos]));
                }
                if ((((temp3 > 0) && (data.knightblack == 0)) && ((data.ngajahhitam == 0) || ((data.ngajahhitam == 1) && (petir_ueval::color[petir_bitboard_mask::lastbitp(pas::address(data.bishopblack))] != petir_ueval::color[pos]))))) {
                    (nilai = pas::increment(nilai, pas::shr(petir_ueval::nilai_kuda_outpost_w[pos], 1)));
                }
            }
        }
    }
    piecepos = data.knightblack;
    while ((piecepos != 0)) {
        pos = pop_lsb(piecepos);
        (nilai = pas::decrement(nilai, petir_ueval::nilai_pos_kuda_b[pos]));
        if ((!early_endgame)) {
            (nilai = pas::decrement(nilai, pas::mul(petir_evalmask::tabel_edistance[pos][posrajaputih], petir_ueval::nilai_tropism_kuda)));
        }
        attack = petir_bitboard_mask::knightmask[pos];
        knightbattack = (knightbattack | attack);
        temp64 = (attack & (~data.blackpieces));
        (nilai = pas::decrement(nilai, petir_ueval::mobil_knight[count_bits<NativePopcount>(temp64)]));
        if (wevalks) {
            tempk64 = (attack & whitekingattack);
            (whitekingpressure = pas::increment(whitekingpressure, count_bits<NativePopcount>(tempk64)));
            if (tempk64 != 0) {
                (white_attacker = pas::increment(white_attacker, 1));
            }
        }
        if (((petir_ueval::nilai_kuda_outpost_b[pos] > 0) && (!endgame))) {
            if (((petir_evalmask::b_outpost_mask[pos] & data.pawnwhite) == 0)) {
                temp64 = (petir_bitboard_mask::w_pawn_attack[pos] & data.pawnblack);
                temp3 = count_bits<NativePopcount>(temp64);
                pas_temp_4 = temp3;
                if (((pas_temp_4 == 1))) {
                    (nilai = pas::decrement(nilai, pas::shr(petir_ueval::nilai_kuda_outpost_b[pos], 1)));
                }
                else if (((pas_temp_4 == 2))) {
                    (nilai = pas::decrement(nilai, petir_ueval::nilai_kuda_outpost_b[pos]));
                }
                if ((((temp3 > 0) && (data.knightwhite == 0)) && ((data.ngajahputih == 0) || ((data.ngajahputih == 1) && (petir_ueval::color[petir_bitboard_mask::firstbitp(pas::address(data.bishopwhite))] != petir_ueval::color[pos]))))) {
                    (nilai = pas::decrement(nilai, pas::shr(petir_ueval::nilai_kuda_outpost_b[pos], 1)));
                }
            }
        }
    }
    piecepos = data.bishopwhite;
    tempi = 0;
    while ((piecepos != 0)) {
        pos = pop_lsb(piecepos);
        attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        temp64 = (attack & (~data.whitepieces));
        (nilai = pas::increment(nilai, petir_ueval::mobil_bishop[count_bits<NativePopcount>(temp64)]));
        if (((((!endgame) && (petir_ueval::nilai_gajah_outpost_w[pos] > 0)) && ((petir_bitboard_mask::b_pawn_attack[pos] & data.pawnwhite) != 0)) && ((petir_evalmask::w_outpost_mask[pos] & data.pawnblack) == 0))) {
            (nilai = pas::increment(nilai, petir_ueval::nilai_gajah_outpost_w[pos]));
        }
        if (bevalks) {
            tempk64 = (attack & blackkingattack);
            (blackkingpressure = pas::increment(blackkingpressure, count_bits<NativePopcount>(tempk64)));
            if (tempk64 != 0) {
                (black_attacker = pas::increment(black_attacker, 1));
            }
        }
        bishopwattack = (bishopwattack | attack);
        (nilai = pas::increment(nilai, petir_ueval::nilai_pos_gajah_w[pos]));
        if ((!early_endgame)) {
            if ((petir_evalmask::tabel_distance_gajah[pos][posrajahitam] > 0)) {
                (tempi = pas::increment(tempi, 1));
            }
            (nilai = pas::increment(nilai, pas::mul(petir_evalmask::tabel_distance_gajah[pos][posrajahitam], petir_ueval::nilai_tropism_gajah)));
        }
    }
    if ((tempi == 2)) {
        (nilai = pas::increment(nilai, petir_ueval::nilai_2bishop_attack_king));
    }
    piecepos = data.bishopblack;
    tempi = 0;
    while ((piecepos != 0)) {
        pos = pop_lsb(piecepos);
        (nilai = pas::decrement(nilai, petir_ueval::nilai_pos_gajah_b[pos]));
        if ((!early_endgame)) {
            (nilai = pas::decrement(nilai, pas::mul(petir_evalmask::tabel_distance_gajah[pos][posrajaputih], petir_ueval::nilai_tropism_gajah)));
            if ((petir_evalmask::tabel_distance_gajah[pos][posrajaputih] > 0)) {
                (tempi = pas::increment(tempi, 1));
            }
        }
        attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
        attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
        temp64 = (attack & (~data.blackpieces));
        (nilai = pas::decrement(nilai, petir_ueval::mobil_bishop[count_bits<NativePopcount>(temp64)]));
        if (((((!endgame) && (petir_ueval::nilai_gajah_outpost_b[pos] > 0)) && ((petir_bitboard_mask::w_pawn_attack[pos] & data.pawnblack) != 0)) && ((petir_evalmask::b_outpost_mask[pos] & data.pawnwhite) == 0))) {
            (nilai = pas::decrement(nilai, petir_ueval::nilai_gajah_outpost_b[pos]));
        }
        if (wevalks) {
            tempk64 = (attack & whitekingattack);
            (whitekingpressure = pas::increment(whitekingpressure, count_bits<NativePopcount>(tempk64)));
            if (tempk64 != 0) {
                (white_attacker = pas::increment(white_attacker, 1));
            }
        }
        bishopbattack = (bishopbattack | attack);
    }
    if ((tempi == 2)) {
        (nilai = pas::decrement(nilai, petir_ueval::nilai_2bishop_attack_king));
    }
    piecepos = data.queenwhite;
    while ((piecepos != 0)) {
        pos = pop_lsb(piecepos);
        (nilai = pas::increment(nilai, petir_ueval::nilai_pos_mentri_w[pos]));
        if ((!endgame)) {
            attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
            attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
            attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
            attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
            temp64 = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
            tempk64 = petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)];
            attack = ((attack | tempk64) | temp64);
            temp264 = (attack & (~data.whitepieces));
            (nilai = pas::increment(nilai, petir_ueval::mobil_queen[count_bits<NativePopcount>(temp264)]));
            if (((temp64 & data.rookwhite) != 0)) {
                if (((temp64 & blackkingattack) != 0)) {
                    rookwattack = (rookwattack | temp64);
                }
            }
            if (((tempk64 & data.rookwhite) != 0)) {
                if (((tempk64 & blackkingattack) != 0)) {
                    rookwattack = (rookwattack | tempk64);
                }
            }
            queenwattack = (queenwattack | attack);
            (blackkingpressure = pas::increment(blackkingpressure, count_bits<NativePopcount>((attack & blackkingattack))));
            if ((attack & blackkingattack) != 0) {
                (black_attacker = pas::increment(black_attacker, 1));
            }
        }
        if ((!early_endgame)) {
            (nilai = pas::increment(nilai, pas::mul(petir_evalmask::tabel_edistance[pos][posrajahitam], petir_ueval::nilai_tropism_mentri)));
        }
    }
    piecepos = data.queenblack;
    while ((piecepos != 0)) {
        pos = pop_lsb(piecepos);
        (nilai = pas::decrement(nilai, petir_ueval::nilai_pos_mentri_b[pos]));
        if ((!endgame)) {
            attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
            attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
            attack = petir_bitboard_mask::diaga8h1mask[pos][(pas::shr(data.allpiecesa8h1, petir_bitboard_mask::a8h1shiftmask[pos]) & 255)];
            attack = (attack | petir_bitboard_mask::diagh8a1mask[pos][(pas::shr(data.allpiecesh8a1, petir_bitboard_mask::h8a1shiftmask[pos]) & 255)]);
            temp64 = petir_bitboard_mask::horzmask2[pos][(pas::shr(data.allpieces, pas::add((pos & 56), 1)) & 63)];
            tempk64 = petir_bitboard_mask::vertmask2[pos][(pas::shr(data.allpiecesr90, petir_bitboard_mask::vershiftmask[pos]) & 63)];
            attack = ((attack | tempk64) | temp64);
            temp264 = (attack & (~data.blackpieces));
            (nilai = pas::decrement(nilai, petir_ueval::mobil_queen[count_bits<NativePopcount>(temp264)]));
            queenbattack = (queenbattack | attack);
            if (((temp64 & data.rookblack) != 0)) {
                if (((temp64 & whitekingattack) != 0)) {
                    rookbattack = (rookbattack | temp64);
                }
            }
            if (((tempk64 & data.rookblack) != 0)) {
                if (((tempk64 & whitekingattack) != 0)) {
                    rookbattack = (rookbattack | tempk64);
                }
            }
            (whitekingpressure = pas::increment(whitekingpressure, count_bits<NativePopcount>((attack & whitekingattack))));
            if ((attack & whitekingattack) != 0) {
                (white_attacker = pas::increment(white_attacker, 1));
            }
        }
        if ((!early_endgame)) {
            (nilai = pas::decrement(nilai, pas::mul(petir_evalmask::tabel_edistance[pos][posrajaputih], petir_ueval::nilai_tropism_mentri)));
        }
    }
    if ((!endgame)) {
        (nilai = pas::decrement(nilai, petir_ueval::nilai_pos_raja_w[posrajaputih]));
        (nilai = pas::increment(nilai, petir_ueval::nilai_pos_raja_b[posrajahitam]));
        if (infhr) {
            temp4 = pas::sub(3, giliran);
        } else {
            temp4 = giliran;
        }
        if ((data.queenblack != 0)) {
            tempk64 = (((((queenwattack | rookwattack) | bishopwattack) | knightwattack) | pawnwattack) & whitekingattack);
            temp64 = ((whitekingattack & queenbattack) & (~tempk64));
            attack = (temp64 & bishopbattack);
            if (((attack != 0) && (temp4 == petir_header::_sisihitam))) {
                (whitekingpressure = pas::increment(whitekingpressure, 2));
            }
            (whitekingpressure = pas::increment(whitekingpressure, pas::mul(count_bits<NativePopcount>(attack), 2)));
            attack = ((temp64 & knightbattack) & (~tempk64));
            if (((attack != 0) && (temp4 == petir_header::_sisihitam))) {
                (whitekingpressure = pas::increment(whitekingpressure, 2));
            }
            (whitekingpressure = pas::increment(whitekingpressure, pas::mul(count_bits<NativePopcount>(attack), 2)));
            attack = ((temp64 & rookbattack) & (~tempk64));
            if (((attack != 0) && (temp4 == petir_header::_sisihitam))) {
                (whitekingpressure = pas::increment(whitekingpressure, 2));
            }
            (whitekingpressure = pas::increment(whitekingpressure, pas::mul(count_bits<NativePopcount>(attack), 2)));
            attack = ((temp64 & pawnbattack) & (~tempk64));
            (whitekingpressure = pas::increment(whitekingpressure, pas::mul(count_bits<NativePopcount>(attack), 2)));
            attack = ((whitekingattack & rookbattack) & bishopbattack);
            (whitekingpressure = pas::increment(whitekingpressure, count_bits<NativePopcount>(attack)));
            tempi = petir_ueval::nilai_king_pressure[std::clamp(whitekingpressure, 0, 63)];
            pas_temp_5 = white_attacker;
            if (((pas_temp_5 == 1))) {
                tempi = (tempi / 5);
            }
            else if (((pas_temp_5 == 3))) {
                tempi = (pas::mul(tempi, 125) / 100);
            }
            else if (((pas_temp_5 == 4))) {
                tempi = (pas::mul(tempi, 150) / 100);
            }
            else if (((pas_temp_5 == 5))) {
                tempi = (pas::mul(tempi, 200) / 100);
            }
            (nilai = pas::decrement(nilai, tempi));
        }
        if ((data.queenwhite != 0)) {
            tempk64 = (((((queenbattack | rookbattack) | bishopbattack) | knightbattack) | pawnbattack) & blackkingattack);
            temp64 = ((blackkingattack & queenwattack) & (~tempk64));
            attack = (temp64 & bishopwattack);
            (blackkingpressure = pas::increment(blackkingpressure, pas::mul(count_bits<NativePopcount>(attack), 2)));
            if (((attack != 0) && (temp4 == petir_header::_sisiputih))) {
                (blackkingpressure = pas::increment(blackkingpressure, 2));
            }
            attack = ((temp64 & knightwattack) & (~tempk64));
            if (((attack != 0) && (temp4 == petir_header::_sisiputih))) {
                (blackkingpressure = pas::increment(blackkingpressure, 2));
            }
            (blackkingpressure = pas::increment(blackkingpressure, pas::mul(count_bits<NativePopcount>(attack), 2)));
            attack = ((temp64 & rookwattack) & (~tempk64));
            if (((attack != 0) && (temp4 == petir_header::_sisiputih))) {
                (blackkingpressure = pas::increment(blackkingpressure, 2));
            }
            (blackkingpressure = pas::increment(blackkingpressure, pas::mul(count_bits<NativePopcount>(attack), 2)));
            attack = ((temp64 & pawnwattack) & (~tempk64));
            (blackkingpressure = pas::increment(blackkingpressure, pas::mul(count_bits<NativePopcount>(attack), 2)));
            attack = ((blackkingattack & rookwattack) & bishopwattack);
            (blackkingpressure = pas::increment(blackkingpressure, count_bits<NativePopcount>(attack)));
            tempi = petir_ueval::nilai_king_pressure[std::clamp(blackkingpressure, 0, 63)];
            pas_temp_6 = black_attacker;
            if (((pas_temp_6 == 1))) {
                tempi = (tempi / 5);
            }
            else if (((pas_temp_6 == 3))) {
                tempi = (pas::mul(tempi, 125) / 100);
            }
            else if (((pas_temp_6 == 4))) {
                tempi = (pas::mul(tempi, 150) / 100);
            }
            else if (((pas_temp_6 == 5))) {
                tempi = (pas::mul(tempi, 200) / 100);
            }
            (nilai = pas::increment(nilai, tempi));
        }
    } else {
        temp64 = (data.pawnblack | data.pawnwhite);
        if (((petir_evalmask::abcd_mask & temp64) != 0)) {
            if (((petir_evalmask::efgh_mask & temp64) != 0)) {
                (nilai = pas::increment(nilai, petir_ueval::nilai_pos_raja_endn_w[posrajaputih]));
                (nilai = pas::decrement(nilai, petir_ueval::nilai_pos_raja_endn_b[posrajahitam]));
            } else {
                (nilai = pas::increment(nilai, petir_ueval::nilai_pos_raja_endq_w[posrajaputih]));
                (nilai = pas::decrement(nilai, petir_ueval::nilai_pos_raja_endq_b[posrajahitam]));
            }
        } else {
            if (((petir_evalmask::efgh_mask & temp64) != 0)) {
                (nilai = pas::increment(nilai, petir_ueval::nilai_pos_raja_endk_w[posrajaputih]));
                (nilai = pas::decrement(nilai, petir_ueval::nilai_pos_raja_endk_b[posrajahitam]));
            } else {
                (nilai = pas::increment(nilai, petir_ueval::nilai_pos_raja_endn_w[posrajaputih]));
                (nilai = pas::decrement(nilai, petir_ueval::nilai_pos_raja_endn_b[posrajahitam]));
            }
        }
    }
    if ((data.pawnwhite == 0)) {
        (nilai = pas::decrement(nilai, petir_ueval::nilai_no_pawn));
    }
    if ((data.pawnblack == 0)) {
        (nilai = pas::increment(nilai, petir_ueval::nilai_no_pawn));
    }
    if ((data.papan[petir_header::d2] == petir_header::_pionputih)) {
        if (((data.papan[petir_header::d3] != 0) && (data.papan[petir_header::c1] == petir_header::_gajahputih))) {
            (nilai = pas::decrement(nilai, petir_ueval::nilai_pion_pusat_blocked));
        }
    }
    if ((data.papan[petir_header::e2] == petir_header::_pionputih)) {
        if (((data.papan[petir_header::e3] != 0) && (data.papan[petir_header::f1] == petir_header::_gajahputih))) {
            (nilai = pas::decrement(nilai, petir_ueval::nilai_pion_pusat_blocked));
        }
    }
    if ((data.papan[petir_header::d7] == petir_header::_pionhitam)) {
        if (((data.papan[petir_header::d6] != 0) && (data.papan[petir_header::c8] == petir_header::_gajahhitam))) {
            (nilai = pas::increment(nilai, petir_ueval::nilai_pion_pusat_blocked));
        }
    }
    if ((data.papan[petir_header::e7] == petir_header::_pionhitam)) {
        if (((data.papan[petir_header::e6] != 0) && (data.papan[petir_header::f8] == petir_header::_gajahhitam))) {
            (nilai = pas::increment(nilai, petir_ueval::nilai_pion_pusat_blocked));
        }
    }
    if ((((((data.nilai_perwira_putih == 3) && (data.nilai_perwira_hitam == 3)) && (data.ngajahhitam == 1)) && (data.ngajahputih == 1)) && (petir_ueval::color[petir_bitboard_mask::firstbitp(pas::address(data.bishopblack))] != petir_ueval::color[petir_bitboard_mask::firstbitp(pas::address(data.bishopwhite))]))) {
        nilai = (nilai / 2);
        if ((petir_ueval::onlyone(data.pawnblack) && (data.pawnwhite == 0))) {
            pos = petir_bitboard_mask::firstbitp(pas::address(data.pawnblack));
            attack = petir_bitboard_mask::rook_attack_down_mask[pos];
            if (((bishopwattack & attack) != 0)) {
                nilai = 0;
            }
        } else {
            if ((petir_ueval::onlyone(data.pawnwhite) && (data.pawnblack == 0))) {
                pos = petir_bitboard_mask::lastbitp(pas::address(data.pawnwhite));
                attack = petir_bitboard_mask::rook_attack_up_mask[pos];
                if (((bishopbattack & attack) != 0)) {
                    nilai = 0;
                }
            }
        }
    }
    if ((data.move50count >= 80)) {
        temp = pas::sub(101, data.move50count);
        nilai = pas::round((nilai * (static_cast<double>(temp) / (20))));
    }
    if (((data.nilai_perwira_putih == 5) && (data.nilai_perwira_hitam == 6))) {
    }
    if (((npionhitam == 0) && (npionputih == 0))) {
        if ((((data.nilai_perwira_putih == 3) && (data.nilai_perwira_hitam == 5)) || ((data.nilai_perwira_putih == 5) && (data.nilai_perwira_hitam == 3)))) {
            nilai = (nilai / 4);
        } else {
            if ((((data.nilai_perwira_putih == 12) && (data.nilai_perwira_hitam == 9)) || ((data.nilai_perwira_putih == 9) && (data.nilai_perwira_hitam == 12)))) {
                nilai = (nilai / 8);
            }
        }
    } else {
        if (((npionputih == 1) && (npionhitam == 0))) {
            if (((data.nilai_perwira_putih == 5) && (data.nilai_perwira_hitam == 6))) {
                nilai = (nilai / 2);
            } else {
                if (((data.nilai_perwira_putih == 5) && (data.nilai_perwira_hitam == 5))) {
                    nilai = (nilai / 2);
                } else {
                    if (((data.nilai_perwira_putih == 9) && (data.nilai_perwira_hitam == 9))) {
                        nilai = (nilai / 4);
                    } else {
                        if (((data.nilai_perwira_putih == 3) && (data.nilai_perwira_hitam == 3))) {
                            nilai = (nilai / 2);
                        } else {
                            if (((data.nilai_perwira_putih == 3) && ((data.nilai_perwira_hitam == 6) || (data.nilai_perwira_hitam == 5)))) {
                                nilai = (nilai / 4);
                            }
                        }
                    }
                }
            }
        } else {
            if (((npionputih == 0) && (npionhitam == 1))) {
                if (((data.nilai_perwira_putih == 6) && (data.nilai_perwira_hitam == 5))) {
                    nilai = (nilai / 2);
                } else {
                    if (((data.nilai_perwira_putih == 3) && (data.nilai_perwira_hitam == 3))) {
                        nilai = (nilai / 2);
                    } else {
                        if (((data.nilai_perwira_putih == 5) && (data.nilai_perwira_hitam == 5))) {
                            nilai = (nilai / 2);
                        } else {
                            if (((data.nilai_perwira_putih == 9) && (data.nilai_perwira_hitam == 9))) {
                                nilai = (nilai / 4);
                            } else {
                                if ((((data.nilai_perwira_putih == 6) || (data.nilai_perwira_hitam == 3)) && (data.nilai_perwira_hitam == 3))) {
                                    nilai = (nilai / 4);
                                }
                            }
                        }
                    }
                }
            } else {
                if (((npionputih == 2) && (npionhitam == 0))) {
                    if ((((data.nilai_perwira_putih == 0) && (data.nilai_perwira_hitam == 6)) && (data.bishopblack != 0))) {
                        nilai = (nilai / 2);
                    }
                } else {
                    if (((npionputih == 0) && (npionhitam == 2))) {
                        if ((((data.nilai_perwira_putih == 6) && (data.nilai_perwira_hitam == 0)) && (data.bishopwhite != 0))) {
                            nilai = (nilai / 2);
                        }
                    } else {
                        if ((npionputih == npionhitam)) {
                            if (((((nppputih == 0) && (npphitam == 0)) && (data.nilai_perwira_putih == 5)) && (data.nilai_perwira_hitam == 5))) {
                                nilai = (pas::mul(nilai, 75) / 100);
                            }
                        }
                    }
                }
            }
        }
    }
    if (draw2) {
        nilai = (nilai / 8);
    } else {
    }
    if ((giliran == petir_header::_sisiputih)) {
        return_value = nilai;
    } else {
        return_value = pas::negate(nilai);
    }
    return return_value;
}

}

std::int32_t eval(const petir_header::tdata& data, std::int32_t giliran, std::int32_t alpha, std::int32_t beta, bool infhr) {
    int kpk_score;
    if (petir_kpk::probe(data, infhr ? 3 - giliran : giliran, kpk_score)) {
        // infhr supplies the opponent's score perspective after a move.
        if (!kpk_score) return petir_header::contemp_draw;
        return infhr ? -kpk_score : kpk_score;
    }
#if defined(_MSC_VER) && defined(_M_X64)
    // Select once per evaluation instead of dispatching at every bit count.
    if (has_native_popcount) {
        return eval_impl<true>(data, giliran, alpha, beta, infhr);
    }
#endif
    return eval_impl<false>(data, giliran, alpha, beta, infhr);
}

// ueval.pas:2920
std::int32_t materialvalue(std::int32_t giliran, std::int32_t ms) {
    std::int32_t return_value{};
    if ((giliran == petir_header::_sisihitam)) {
        return_value = pas::negate(ms);
    } else {
        return_value = ms;
    }
    return return_value;
}

// ueval.pas:494
bool isdraw(const petir_header::tdata& data, bool& draw2) {
    bool return_value{};
    return_value = false;
    draw2 = false;
    if (((data.pawnblack | data.pawnwhite) == 0)) {
        if ((((data.nilai_perwira_putih == 8) && (data.nilai_perwira_hitam == 5)) || ((data.nilai_perwira_putih == 5) && (data.nilai_perwira_hitam == 8)))) {
            draw2 = true;
            return return_value;
        }
        if ((data.nilai_perwira_putih == 0)) {
            if ((data.nilai_perwira_hitam <= 3)) {
                return_value = true;
                return return_value;
            }
            if (((data.nilai_perwira_hitam == 6) && (data.bishopblack == 0))) {
                return_value = true;
                return return_value;
            }
        }
        if ((data.nilai_perwira_hitam == 0)) {
            if ((data.nilai_perwira_putih <= 3)) {
                return_value = true;
                return return_value;
            }
            if (((data.nilai_perwira_putih == 6) && (data.bishopwhite == 0))) {
                return_value = true;
                return return_value;
            }
        }
        if ((data.nilai_perwira_putih == data.nilai_perwira_hitam)) {
            if ((data.nilai_perwira_putih == 3)) {
                return_value = true;
                return return_value;
            }
            if ((((data.nilai_perwira_putih == 6) && (data.bishopblack == 0)) && (data.bishopwhite == 0))) {
                return_value = true;
                return return_value;
            }
        }
        if ((((pas::sub(data.nilai_perwira_putih, data.nilai_perwira_hitam) == 3) && (data.nilai_perwira_putih == 6)) && (data.bishopwhite == 0))) {
            return_value = true;
            return return_value;
        }
        if ((((pas::sub(data.nilai_perwira_hitam, data.nilai_perwira_putih) == 3) && (data.nilai_perwira_hitam == 6)) && (data.bishopblack == 0))) {
            return_value = true;
            return return_value;
        }
    }
    return return_value;
}

// ueval.pas:595
bool anotherdraw(const petir_header::tdata& data, std::uint8_t giliran) {
    bool return_value{};
    std::int32_t a{};
    std::int32_t pos{};
    std::int32_t pos2{};
    std::int32_t pos3{};
    return_value = false;
    if (((data.nilai_perwira_putih == 0) && (data.nilai_perwira_hitam == 0))) {
        if (((data.pawnblack == 0) && petir_ueval::onlyone(data.pawnwhite))) {
            pos = petir_bitboard_mask::lastbitp(pas::address(data.pawnwhite));
            if (((pos & 7) == 7)) {
                pos2 = petir_bitboard_mask::lastbitp(pas::address(data.kingblack));
                if ((((pos2 & 7) >= 6) && (pas::shr(pos2, 3) >= pas::shr(pos, 3)))) {
                    return_value = true;
                    return return_value;
                }
                pos3 = petir_bitboard_mask::lastbitp(pas::address(data.kingwhite));
                if ((((((pos3 & 7) == 7) && (pas::shr(pos3, 3) > pas::shr(pos, 3))) && ((pos2 & 7) == 5)) && (pas::shr(pos2, 3) > pas::shr(pos, 3)))) {
                    return_value = true;
                    return return_value;
                }
            } else {
                if (((pos & 7) == 0)) {
                    pos2 = petir_bitboard_mask::lastbitp(pas::address(data.kingblack));
                    if ((((pos2 & 7) <= 1) && (pas::shr(pos2, 3) >= pas::shr(pos, 3)))) {
                        return_value = true;
                        return return_value;
                    }
                    pos3 = petir_bitboard_mask::lastbitp(pas::address(data.kingwhite));
                    if ((((((pos3 & 7) == 0) && (pas::shr(pos3, 3) > pas::shr(pos, 3))) && ((pos2 & 7) == 2)) && (pas::shr(pos2, 3) > pas::shr(pos, 3)))) {
                        return_value = true;
                        return return_value;
                    }
                }
            }
            return return_value;
            pos2 = petir_bitboard_mask::lastbitp(pas::address(data.kingblack));
            pos3 = petir_bitboard_mask::lastbitp(pas::address(data.kingwhite));
            if ((pos2 >= petir_header::a8)) {
                return return_value;
            }
            if (((pas::sub(pos2, pos) == 8) && (pas::sub(pos, pos3) == 8))) {
                return_value = true;
                return return_value;
            }
            if (((((pos2 < petir_header::a8) && (pas::sub(pos2, pos3) == 16)) && (pas::sub(pos3, pos) == 8)) && (giliran == petir_header::_sisiputih))) {
                return_value = true;
                return return_value;
            }
            if (((pas::sub(pos2, pos3) == 16) && (giliran == petir_header::_sisiputih))) {
                return_value = true;
                return return_value;
            }
            if ((((pas::sub(pos2, pos3) == 16) && (pas::abs(pas::sub(pos, pos3)) == 1)) && (giliran == petir_header::_sisiputih))) {
                return_value = true;
                return return_value;
            }
            if (((pas::sub(pos2, pos) == 15) || (pas::sub(pos2, pos) == 16) || (pas::sub(pos2, pos) == 17))) {
                if (((pas::sub(pos, pos3) == 8) && (giliran == petir_header::_sisihitam))) {
                    return_value = true;
                    return return_value;
                }
                if ((((pas::sub(pos, pos3) == 7) || (pas::sub(pos, pos3) == 8) || (pas::sub(pos, pos3) == 9)) && (giliran == petir_header::_sisiputih))) {
                    return_value = true;
                    return return_value;
                }
            }
        } else {
            if (((data.pawnwhite == 0) && petir_ueval::onlyone(data.pawnblack))) {
                pos = petir_bitboard_mask::lastbitp(pas::address(data.pawnblack));
                if (((pos & 7) == 7)) {
                    pos2 = petir_bitboard_mask::lastbitp(pas::address(data.kingwhite));
                    if ((((pos2 & 7) >= 6) && (pas::shr(pos2, 3) <= pas::shr(pos, 3)))) {
                        return_value = true;
                        return return_value;
                    }
                    pos3 = petir_bitboard_mask::lastbitp(pas::address(data.kingblack));
                    if ((((((pos3 & 7) == 7) && (pas::shr(pos3, 3) < pas::shr(pos, 3))) && ((pos2 & 7) == 5)) && (pas::shr(pos2, 3) < pas::shr(pos, 3)))) {
                        return_value = true;
                        return return_value;
                    }
                } else {
                    if (((pos & 7) == 0)) {
                        pos2 = petir_bitboard_mask::lastbitp(pas::address(data.kingwhite));
                        if ((((pos2 & 7) <= 1) && (pas::shr(pos2, 3) <= pas::shr(pos, 3)))) {
                            return_value = true;
                            return return_value;
                        }
                        pos3 = petir_bitboard_mask::lastbitp(pas::address(data.kingblack));
                        if ((((((pos3 & 7) == 0) && (pas::shr(pos3, 3) < pas::shr(pos, 3))) && ((pos2 & 7) == 2)) && (pas::shr(pos2, 3) < pas::shr(pos, 3)))) {
                            return_value = true;
                            return return_value;
                        }
                    }
                }
                return return_value;
                pos2 = petir_bitboard_mask::lastbitp(pas::address(data.kingblack));
                pos3 = petir_bitboard_mask::lastbitp(pas::address(data.kingwhite));
                if (((pas::sub(pos, pos3) == 8) && (pas::sub(pos2, pos) == 8))) {
                    return_value = true;
                    return return_value;
                }
                if (((pas::sub(pos2, pos3) == 16) && (giliran == petir_header::_sisihitam))) {
                    return_value = true;
                    return return_value;
                }
            }
        }
    } else {
        if ((((data.pawnwhite == 0) && (data.nilai_perwira_putih == 3)) && (data.nilai_perwira_hitam == 0))) {
            if (petir_ueval::onlyone(data.pawnblack)) {
                if ((data.ngajahputih != 0)) {
                    a = petir_bitboard_mask::firstbitp(pas::address(data.pawnblack));
                    pos2 = petir_bitboard_mask::firstbitp(pas::address(data.kingwhite));
                    pos3 = petir_bitboard_mask::firstbitp(pas::address(data.kingblack));
                    if ((a >= petir_header::a3)) {
                        if ((!(((giliran == petir_header::_sisihitam) && (petir_evalmask::distancemax(a, pos3) == 1)) && (petir_evalmask::distancemax(a, pos2) > 1)))) {
                            return_value = true;
                            return return_value;
                        }
                    }
                    pos = petir_bitboard_mask::firstbitp(pas::address(data.bishopwhite));
                    if ((((petir_bitboard_mask::attackbishop(data, pos) & petir_bitboard_mask::rook_attack_down_mask[a]) != 0) || ((petir_bitboard_mask::rook_attack_down_mask[a] & data.kingwhite) != 0))) {
                        return_value = true;
                        return return_value;
                    }
                } else {
                    a = petir_bitboard_mask::firstbitp(pas::address(data.knightwhite));
                    pos = petir_bitboard_mask::firstbitp(pas::address(data.pawnblack));
                    if (((petir_bitboard_mask::knightmask[a] & petir_bitboard_mask::rook_attack_down_mask[pos]) != 0)) {
                        return_value = true;
                        return return_value;
                    }
                }
            }
        } else {
            if ((((data.pawnblack == 0) && (data.nilai_perwira_hitam == 3)) && (data.nilai_perwira_putih == 0))) {
                if (petir_ueval::onlyone(data.pawnwhite)) {
                    if ((data.ngajahhitam != 0)) {
                        a = petir_bitboard_mask::firstbitp(pas::address(data.pawnwhite));
                        if ((a < petir_header::a7)) {
                            pos2 = petir_bitboard_mask::firstbitp(pas::address(data.kingwhite));
                            pos3 = petir_bitboard_mask::firstbitp(pas::address(data.kingblack));
                            if ((!(((giliran == petir_header::_sisiputih) && (petir_evalmask::distancemax(a, pos2) == 1)) && (petir_evalmask::distancemax(a, pos3) > 1)))) {
                                return_value = true;
                                return return_value;
                            }
                        }
                    } else {
                        a = petir_bitboard_mask::firstbitp(pas::address(data.knightblack));
                        pos = petir_bitboard_mask::firstbitp(pas::address(data.pawnwhite));
                        if (((petir_bitboard_mask::knightmask[a] & petir_bitboard_mask::rook_attack_up_mask[pos]) != 0)) {
                            return_value = true;
                            return return_value;
                        }
                    }
                }
            } else {
                if ((((data.pawnwhite == 0) && (data.nilai_perwira_hitam == 3)) && (data.nilai_perwira_putih == 0))) {
                    if ((petir_ueval::onlyone(data.pawnblack) && (data.bishopblack != 0))) {
                        a = petir_bitboard_mask::firstbitp(pas::address(data.pawnblack));
                        if (((a & 7) == 7)) {
                            pos = petir_bitboard_mask::firstbitp(pas::address(data.bishopblack));
                            if (((petir_ueval::color[pos] != petir_ueval::color[petir_header::h1]) && (((data.papan[petir_header::h1] == petir_header::_rajaputih) || (data.papan[petir_header::g1] == petir_header::_rajaputih)) || (data.papan[petir_header::h2] == petir_header::_rajaputih)))) {
                                return_value = true;
                                return return_value;
                            }
                        } else {
                            if (((a & 7) == 0)) {
                                pos = petir_bitboard_mask::firstbitp(pas::address(data.bishopblack));
                                if (((petir_ueval::color[pos] != petir_ueval::color[petir_header::a1]) && (((data.papan[petir_header::a1] == petir_header::_rajaputih) || (data.papan[petir_header::b1] == petir_header::_rajaputih)) || (data.papan[petir_header::a2] == petir_header::_rajaputih)))) {
                                    return_value = true;
                                    return return_value;
                                }
                            }
                        }
                    }
                } else {
                    if ((((data.pawnblack == 0) && (data.nilai_perwira_putih == 3)) && (data.nilai_perwira_hitam == 0))) {
                        if ((petir_ueval::onlyone(data.pawnwhite) && (data.bishopwhite != 0))) {
                            a = petir_bitboard_mask::firstbitp(pas::address(data.pawnwhite));
                            if (((a & 7) == 7)) {
                                pos = petir_bitboard_mask::firstbitp(pas::address(data.bishopwhite));
                                if (((petir_ueval::color[pos] != petir_ueval::color[petir_header::h8]) && (((data.papan[petir_header::h8] == petir_header::_rajahitam) || (data.papan[petir_header::g8] == petir_header::_rajahitam)) || (data.papan[petir_header::h7] == petir_header::_rajahitam)))) {
                                    return_value = true;
                                    return return_value;
                                }
                            } else {
                                if (((a & 7) == 0)) {
                                    pos = petir_bitboard_mask::firstbitp(pas::address(data.bishopwhite));
                                    if (((petir_ueval::color[pos] != petir_ueval::color[petir_header::a8]) && (((data.papan[petir_header::a8] == petir_header::_rajahitam) || (data.papan[petir_header::b8] == petir_header::_rajahitam)) || (data.papan[petir_header::a7] == petir_header::_rajahitam)))) {
                                        return_value = true;
                                        return return_value;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return return_value;
}

// ueval.pas:483
std::uint8_t ipopcount(std::uint64_t p) {
    return static_cast<std::uint8_t>(std::popcount(p));
}

// ueval.pas:421
std::int32_t popcount_w(petir_bitboard_mask::pword p) {
    std::int32_t return_value{};
    std::int32_t n{};
    (p = pas::increment(p, 1));
    n = petir_evalmask::mobmask[(*p)];
    (p = pas::increment(p, 1));
    n = pas::add(n, petir_evalmask::mobmask[(*p)]);
    (p = pas::increment(p, 1));
    n = pas::add(n, petir_evalmask::mobmask[(*p)]);
    return_value = n;
    return return_value;
}

// ueval.pas:433
std::int32_t popcount_b(petir_bitboard_mask::pword p) {
    std::int32_t return_value{};
    std::int32_t n{};
    n = petir_evalmask::mobmask[(*p)];
    (p = pas::increment(p, 1));
    n = pas::add(n, petir_evalmask::mobmask[(*p)]);
    (p = pas::increment(p, 1));
    n = pas::add(n, petir_evalmask::mobmask[(*p)]);
    return_value = n;
    return return_value;
}

// ueval.pas:444
std::uint8_t popcount(petir_bitboard_mask::pword p) {
    std::uint64_t bits;
    std::memcpy(&bits, p, sizeof(bits));
    return petir_ueval::ipopcount(bits);
}

// ueval.pas:456
bool onlyone(std::uint64_t p) {
    return p != 0 && (p & (p - 1)) == 0;
}

// ueval.pas:832
std::int32_t dynamiclazy(const petir_header::tdata& data) {
    std::int32_t return_value{};
    return_value = 0;
    if (((data.queenblack == 0) && (data.queenwhite == 0))) {
        (return_value = pas::decrement(return_value, 50));
    }
    if (((data.rookblack == 0) && (data.rookwhite == 0))) {
        (return_value = pas::decrement(return_value, 30));
    }
    if (((data.knightblack == 0) && (data.knightwhite == 0))) {
        (return_value = pas::decrement(return_value, 20));
    }
    if (((data.bishopblack == 0) && (data.bishopwhite == 0))) {
        (return_value = pas::decrement(return_value, 20));
    }
    return return_value;
}

}
