#include "engine.hpp"
#include "kpk.hpp"
#include <algorithm>
#include <sstream>
namespace petir {
int self_test() {
    int failures=0,checks=0;
    auto check=[&](bool ok,const std::string& message) {++checks;if(!ok) {++failures;std::cerr<<"FAIL: "<<message<<'\n';}};
    using namespace petir_header;
    ClockLimits tm_limits;tm_limits.timed=true;tm_limits.remaining_ms=1000;
    auto tm_budget=TimeManager::allocate(tm_limits);
    check(tm_budget.soft_ms>0 && tm_budget.soft_ms<250 && tm_budget.hard_ms<=950,"low-clock allocation has no 250 ms floor and reserves overhead");
    tm_limits.increment_ms=2000;const auto with_increment=TimeManager::allocate(tm_limits);
    check(with_increment.soft_ms>tm_budget.soft_ms && with_increment.hard_ms<=950,"increment improves the target without spending future increment before the move");
    tm_limits.remaining_ms=50;
    check(TimeManager::allocate(tm_limits).emergency && TimeManager::allocate(tm_limits).hard_ms==0,"clock below overhead selects the emergency path even with increment");
    tm_limits.remaining_ms=60000;tm_limits.increment_ms=0;tm_limits.moves_to_go=1;
    check(TimeManager::allocate(tm_limits).hard_ms<=59950,"last move of a control still retains move overhead");
    tm_limits.movetime_ms=80;tm_budget=TimeManager::allocate(tm_limits);
    check(tm_budget.soft_ms==30 && tm_budget.hard_ms==30,"fixed movetime includes response overhead");
    TimeManager tm;const auto received=TimeManager::now_ms();
    tm.start({true,false,10,20},received-100);
    check(tm.expired(true),"the hard deadline includes time spent preparing the search");
    tm.adjust_soft(100);
    check(tm.expired(true) && !tm.can_start_iteration(0),"soft extensions cannot revive an expired hard deadline");
    tm.tighten(received,60000,0);
    check(tm.expired(true),"clock updates cannot extend the original hard deadline");
    tm.start({true,false,10000,20000},received);
    check(!tm.can_start_iteration(15000),"iteration prediction rejects work that would exceed the hard budget");
    tm.start({true,false,100,1000},received);
    tm.tighten(received,0,50);
    check(tm.expired(true),"a late zero-clock update expires the deadline immediately");
    tm.start({},received);
    check(!tm.expired(true) && tm.can_start_iteration(1000),"depth-only and infinite searches do not acquire a clock limit");
    // Compare the native scan with the legacy 16-bit lookup for every word
    // value in each block, including higher/lower occupied blocks respectively.
    std::uint64_t scan_bits=0;
    check(petir_bitboard_mask::firstbitp(pas::address(scan_bits))==0,"empty bit scan retains legacy zero result");
    check(petir_bitboard_mask::lastbitp(pas::address(scan_bits))==-1,"empty reverse bit scan retains legacy minus-one result");
    bool scan_matches=true,reverse_scan_matches=true;
    for(int block=0;block<4;++block) {
        for(std::uint32_t word=1;word<=65535;++word) {
            scan_bits=std::uint64_t(word)<<(block*16);
            if(block<3)scan_bits|=UINT64_C(0xffff000000000000);
            if(petir_bitboard_mask::firstbitp(pas::address(scan_bits))!=block*16+petir_bitboard_mask::firstbitmask[word])scan_matches=false;
            scan_bits=(std::uint64_t(word)<<(block*16))|((UINT64_C(1)<<(block*16))-1);
            if(petir_bitboard_mask::lastbitp(pas::address(scan_bits))!=block*16+petir_bitboard_mask::firstbitmask[word])reverse_scan_matches=false;
        }
    }
    check(scan_matches,"native bit scan matches legacy lookup across all 16-bit blocks");
    check(reverse_scan_matches,"native reverse scan preserves the highest-word/least-bit order for every word");
    // Compare both count APIs with the old per-bit loop, including dense and
    // sparse boards whose bits span all four 16-bit words.
    bool counts_match=true,single_bit_matches=true;
    auto check_counts=[&](std::uint64_t bits) {
        int expected=0;
        for(auto remaining=bits;remaining;remaining&=remaining-1)++expected;
        if(petir_ueval::ipopcount(bits)!=expected
            || petir_ueval::popcount(pas::address(bits))!=expected)counts_match=false;
        if(petir_ueval::onlyone(bits)!=(expected==1))single_bit_matches=false;
    };
    check_counts(0);check_counts(UINT64_MAX);
    for(int bit=0;bit<64;++bit)check_counts(UINT64_C(1)<<bit);
    for(int block=0;block<4;++block)for(std::uint64_t word=0;word<65536;++word) {
        check_counts(word<<(block*16));
        check_counts((word<<(block*16))|(UINT64_C(0xa5c39e714b286df0)&~(UINT64_C(0xffff)<<(block*16))));
    }
    check(counts_match,"native bit counts match the legacy loop across all word values and mixed boards");
    check(single_bit_matches,"single-bit predicate agrees with an exact count, including zero and bit 63");
    alignas(std::uint64_t) std::uint16_t unaligned_words[5]{};
    const auto unaligned_bits=UINT64_C(0xfedcba9876543210);
    std::memcpy(unaligned_words+1,&unaligned_bits,sizeof(unaligned_bits));
    check(petir_ueval::popcount(unaligned_words+1)==32,"pointer popcount supports a board without 64-bit alignment");
    Position start{};petir_udata::initstate(start);
    check(start.hashkey==3423812783342919194ULL,"Delphi starting hash and Random evaluation order");
    check(consistent(start),"starting board, rotated occupancy, material, and hash agree");
    // Evaluation regression scores. The mixed pawn-file fixtures include the
    // passed-pawn refinement (+12) and opposing rook activity (-10),
    // mirrored for Black after the existing material draw scaling.
    // Include promoted black pieces in different 16-bit blocks.
    struct EvalOptimizationFixture {const char* name;const char* fen;int full[4];int narrow[4];};
    for(const auto& fixture:{
        EvalOptimizationFixture{"multiple rooks",
            "6k1/2r2rr1/1b4n1/3p4/r2N1b2/2P1B3/1R3P2/3Q1RK1 b - - 0 1",
            {-243,-263,263,243},{-243,-263,263,243}},
        EvalOptimizationFixture{"multiple knights",
            "6k1/2n2n2/1b3b2/3p4/n2N1b2/2P1B3/1R3P2/3Q1RK1 b - - 0 1",
            {907,887,-887,-907},{813,793,-793,-813}},
        EvalOptimizationFixture{"multiple bishops",
            "6k1/2b5/1b3b2/3p4/b2N1b2/2P1B3/1R3P2/3Q1RK1 b - - 0 1",
            {1219,1199,-1199,-1219},{1115,1095,-1095,-1115}},
        EvalOptimizationFixture{"multiple queens",
            "6k1/2q5/1q3q2/3p4/q2N1b2/2P1B3/1R3P2/3Q1RK1 b - - 0 1",
            {-1540,-1560,1560,1540},{-1513,-1533,1533,1513}},
        EvalOptimizationFixture{"white pawn files",
            "7k/8/8/P1PP1P1P/P2P1P2/8/8/KB5r w - - 0 1",
            {770,750,-750,-770},{770,750,-750,-770}},
        EvalOptimizationFixture{"black pawn files",
            "kb5R/8/8/p2p1p2/p1pp1p1p/8/8/7K b - - 0 1",
            {-750,-770,770,750},{-750,-770,770,750}}}) {
        int side=1;const auto position=from_fen(fixture.fen,side);const auto saved=position;
        const auto tag=std::string("evaluation optimization: ")+fixture.name;
        check(consistent(position),tag+" fixture is consistent");
        for(int stm:{1,2})for(bool infhr:{false,true}) {
            const int index=(stm-1)*2+int(infhr);
            const auto detail=tag+" side="+std::to_string(stm)+" infhr="+std::to_string(infhr);
            const int actual=petir_ueval::eval(position,stm,-20500,20500,infhr);
            check(actual==fixture.full[index],detail+" full score="+std::to_string(actual));
            check(petir_ueval::eval(position,stm,0,1,infhr)==fixture.narrow[index],detail+" lazy score unchanged");
        }
        check(std::memcmp(&position,&saved,sizeof(position))==0,tag+" leaves the input unchanged");
    }
    check(sizeof(petir_hashing_header::thashrecord)==16,"halfmove-aware hash entries retain the 16-byte layout");
    // Scores depend on the rule-50 counter even when the board key is identical.
    for(int color=1;color<=2;++color)for(int type:{pv_node,fail_low,fail_high})for(int counter:{0,79,99,255}) {
        const int cached_move=parse_move(color==1?"e2e4":"e7e5",start,color);
        const int expected_score=type==fail_low?-101:type==fail_high?101:17;
        const auto count=static_cast<std::uint8_t>(counter);
        petir_hashing::addtable(start.hashkey,color,48,cached_move,expected_score,type,count);
        int alpha=-100,beta=100,score=777,move=0;bool avoid_null=true;
        const bool hit=petir_hashing::searchhash(start.hashkey,alpha,beta,8,color,score,move,avoid_null,count);
        const std::string tag=std::to_string(color)+"/"+std::to_string(type)+"/"+std::to_string(counter);
        check(hit && score==expected_score && move==cached_move,"reuse hash score with matching halfmove counter "+tag);
        score=777;move=0;avoid_null=true;
        const auto other_count=static_cast<std::uint8_t>(counter==255?0:counter+1);
        const bool other_hit=petir_hashing::searchhash(start.hashkey,alpha,beta,8,color,score,move,avoid_null,other_count);
        check(!other_hit && score==777 && move==cached_move && !avoid_null,"different halfmove counter reuses only the ordering move "+tag);
    }
    for(int color=1;color<=2;++color) {
        const int cached_move=parse_move(color==1?"e2e4":"e7e5",start,color);
        petir_hashing::addtable(start.hashkey,color,48,cached_move,200,fail_low,79);
        int alpha=-100,beta=100,score=777,move=0;bool avoid_null=false;
        (void)petir_hashing::searchhash(start.hashkey,alpha,beta,8,color,score,move,avoid_null,79);
        check(avoid_null,"matching halfmove counter permits cached null-move restriction");
        (void)petir_hashing::searchhash(start.hashkey,alpha,beta,8,color,score,move,avoid_null,80);
        check(!avoid_null,"different halfmove counter rejects cached null-move restriction");
        auto old=start;old.move50count=79;
        petir_hashing::addtable(start.hashkey,color,48,cached_move,17,pv_node,79);
        check(petir_pv::getpv(old,color,1).value.starts_with(color==1?"e2e4":"e7e5"),"PV extraction reads matching halfmove counter");
        old.move50count=80;
        check(petir_pv::getpv(old,color,1).value.empty(),"PV extraction rejects a different halfmove counter");
    }
    auto same_board=start;same_board.move50count=99;
    check(petir_hashing_header::hashvalue(same_board,1)==start.hashkey,"halfmove counter does not alter book/repetition hash keys");
    petir_hashing_header::cleartable();
    std::uint64_t counts[]={1,20,400,8902,197281,4865609};
    for(int d=0;d<=5;++d)check(perft(start,1,d)==counts[d],"start position perft "+std::to_string(d));
    int stm=1;auto kiwi=from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",stm);
    std::uint64_t kc[]={1,48,2039,97862};
    for(int d=1;d<=3;++d)check(perft(kiwi,stm,d)==kc[d],"Kiwipete perft "+std::to_string(d));
    auto ending=from_fen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",stm);
    check(perft(ending,stm,4)==43238,"rook ending / en passant perft 4");
    for(const char* fen:{
        "kn6/pp1Q4/1Q4Q1/4Q3/Q5Q1/2Q5/1Q3Q2/K2Q4 w - - 0 1",
        "k2q4/1q3q2/2q5/q5q1/4q3/1q4q1/PP1q4/KN6 b - - 0 1"}) {
        auto promoted=from_fen(fen,stm);
        auto promoted_moves=legal_moves(promoted,stm);
        check(promoted_moves.size()>100,"promoted queens generate beyond the legacy move-list capacity");
        check(consistent(promoted),"large move generation preserves board state");
        const int promoted_eval=petir_ueval::eval(promoted,stm,-20500,20500);
        check(promoted_eval>8000 && promoted_eval<20000,"promoted material evaluates within bounded scale tables");
    }
    tmovelist guarded_moves{};
    bool overflow_rejected=false;
    try {(void)guarded_moves[257];}catch(const std::out_of_range&) {overflow_rejected=true;}
    check(overflow_rejected,"move-list overflow rejected even without general bounds checks");
    // Search buffers need no initial clear only if generation fully defines
    // every counted move and score. Compare outputs from two poison patterns.
    bool generation_defines_entries=true;
    for(const char* fen:{
        start_fen,
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "kn6/pp1Q4/1Q4Q1/4Q3/Q5Q1/2Q5/1Q3Q2/K2Q4 w - - 0 1",
        "k2q4/1q3q2/2q5/q5q1/4q3/1q4q1/PP1q4/KN6 b - - 0 1",
        "7k/8/8/3pP3/8/8/8/K7 w - d6 0 1",
        "7k/1q5p/5r1P/4Np2/R2p1P2/2bP2PK/1p6/1Q6 w - - 4 68",
        "7k/1q5p/7r/4Np2/R2p1P2/2bP2PK/Qp6/8 w - - 0 69",
        "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"}) {
        auto generated_position=from_fen(fen,stm);
        for(int color=1;color<=2;++color) {
            auto audit_generator=[&](auto generate) {
                tmovelist first,second;int first_count=0,second_count=0;
                pas::fillchar(first,sizeof(first),0xa5);pas::fillchar(second,sizeof(second),0x5a);
                generate(first,first_count);generate(second,second_count);
                if(first_count!=second_count)generation_defines_entries=false;
                for(int i=1;i<=std::min(first_count,second_count);++i)
                    if(first[i].moves!=second[i].moves || first[i].score!=second[i].score)generation_defines_entries=false;
            };
            audit_generator([&](tmovelist& list,int& count) {if(color==1)petir_movgen::white_movgen_caps(generated_position,list,count);else petir_movgen::black_movgen_caps(generated_position,list,count);});
            audit_generator([&](tmovelist& list,int& count) {if(color==1)petir_movgen::white_movgen_noncaps(generated_position,list,count);else petir_movgen::black_movgen_noncaps(generated_position,list,count);});
            audit_generator([&](tmovelist& list,int& count) {if(color==1)petir_movgen::white_movgen_caps2(generated_position,list,count);else petir_movgen::black_movgen_caps2(generated_position,list,count);});
            audit_generator([&](tmovelist& list,int& count) {if(color==1)petir_movgen::q_white_movgen(generated_position,list,count);else petir_movgen::q_black_movgen(generated_position,list,count);});
            if(color==1?petir_tools::white_checked(generated_position):petir_tools::black_checked(generated_position))
                audit_generator([&](tmovelist& list,int& count) {int checks=0;if(color==1)petir_movgen::white_evasion(generated_position,list,count,checks);else petir_movgen::black_evasion(generated_position,list,count,checks);});
        }
    }
    check(generation_defines_entries,"generated move/score entries do not depend on initial buffer contents");
    // Freeze first-maximum selection and the swap order used by the old picker.
    tmovelist ordering;
    ordering[1]={11,5};ordering[2]={12,8};ordering[3]={13,8};ordering[4]={14,-1};ordering[5]={15,8};
    const std::array<int,5> selected_moves{12,13,15,11,14};
    for(int current=1;current<=5;++current) {
        petir_next::nextbest_max(ordering,current,5);
        check(ordering[current].moves==selected_moves[current-1],"range-based move picker preserves maximum ties and swap order");
    }
    ordering[256]={23,std::numeric_limits<std::int32_t>::min()};
    petir_next::nextbest_max(ordering,256,256);
    check(ordering[256].moves==23,"move picker accepts the last capacity slot");
    bool picker_overflow_rejected=false;
    try {petir_next::nextbest_max(ordering,1,257);}catch(const std::out_of_range&) {picker_overflow_rejected=true;}
    check(picker_overflow_rejected,"range-based move picker retains the unconditional capacity guard");
    // Baseline SEE values include losing quiet moves, recaptures, and an xray
    // attacker behind the moving rook. Both side conventions are intentional.
    struct SeeFixture {const char* fen;int from,to;std::array<int,2> expected;};
    for(const auto& fixture:std::array<SeeFixture,4>{{
        {"4k3/8/3p4/2p5/3P4/8/8/4K3 w - - 0 1",d4,c5,{0,1}},
        {"4r2k/8/8/4b3/8/8/4R3/K3Q3 w - - 0 1",e2,e5,{3,3}},
        {"7k/3p4/4q3/8/5N2/8/8/K7 w - - 0 1",f4,e6,{6,9}},
        {"7k/8/8/3p4/8/8/4Q3/K7 w - - 0 1",e2,e4,{-9,0}}
    }}) {
        auto exchange=from_fen(fixture.fen,stm);const auto unchanged=exchange;
        for(int side=1;side<=2;++side)
            check(petir_usee::see(side,exchange,fixture.from,fixture.to)==fixture.expected[side-1]
                  && std::memcmp(&exchange,&unchanged,sizeof(exchange))==0,
                  "SEE workspace optimization preserves frozen exchange scores and input");
    }
    {
        // Search can capture a king without clearing its piece board. The
        // phantom king must still participate, valued by papan as a pawn.
        auto exchange=from_fen("8/8/8/8/8/7k/6P1/K6R w - - 0 1",stm);
        petir_makemove::makewhitemove(g2 | (h3 << 7),exchange);
        check(exchange.kingblack==petir_bitboard_mask::bit(h3)
              && !(exchange.blackpieces & exchange.kingblack) && exchange.papan[h3]==_pionputih
              && petir_usee::see(_sisiputih,exchange,h1,h2)==-5,
              "SEE preserves the phantom king recapture after a pseudo-legal king capture");
    }
    {
        // Independent ray walker: exercise all square pairs with deliberately
        // unrelated rotated occupancies and slider boards. It includes sliders
        // absent from occupancy and blockers absent from the slider boards.
        using namespace petir_bitboard_mask;
        auto walked_xray=[](std::uint64_t attack,int square,int target,const Position& p) {
            const int dx=(square & 7)-(target & 7),dy=(square >> 3)-(target >> 3);
            if((dx==0 && dy==0) || (dx!=0 && dy!=0 && std::abs(dx)!=std::abs(dy)))return attack;
            const int sx=(dx>0)-(dx<0),sy=(dy>0)-(dy<0),step=sx+8*sy;
            const auto sliders=(dx==0 || dy==0)?p.rookqueen:p.bishopqueen;
            int x=(square & 7)+sx,y=(square >> 3)+sy;
            while(x>=0 && x<8 && y>=0 && y<8) {
                const int behind=8*y+x;
                attack|=bit(behind) & sliders;
                const bool occupied=dy==0 ? (p.allpieces & bit(behind))!=0
                    : dx==0 ? (p.allpiecesr90 & bit2nmask90[behind])!=0
                    : std::abs(step)==7 ? (p.allpiecesa8h1 & bit2nmaska8h1[behind])!=0
                    : (p.allpiecesh8a1 & bit2nmaskh8a1[behind])!=0;
                if(occupied)break;
                x+=sx;y+=sy;
            }
            return attack;
        };
        std::uint64_t see_seed=UINT64_C(0x951aadfa20261011);
        auto next_board=[&]() {
            see_seed^=see_seed<<13;see_seed^=see_seed>>7;see_seed^=see_seed<<17;
            return see_seed;
        };
        bool xrays_match=true;
        for(int pattern=0;pattern<64;++pattern) {
            Position exchange{};
            exchange.allpieces=next_board();exchange.allpiecesr90=next_board();
            exchange.allpiecesa8h1=next_board();exchange.allpiecesh8a1=next_board();
            exchange.rookqueen=next_board();exchange.bishopqueen=next_board();
            if(pattern<2)exchange.allpieces=exchange.allpiecesr90=exchange.allpiecesa8h1=exchange.allpiecesh8a1
                =pattern==0?0:UINT64_MAX;
            if(pattern<4)exchange.rookqueen=exchange.bishopqueen=(pattern & 1)?UINT64_MAX:0;
            for(int target=0;target<64;++target)for(int square=0;square<64;++square) {
                const auto initial=next_board();auto actual=initial;
                petir_usee::xray(actual,square,target,exchange);
                if(actual!=walked_xray(initial,square,target,exchange))xrays_match=false;
            }
        }
        check(xrays_match,"SEE x-rays match a ray walker for all directions and independent occupancy fields");
    }
    // Frozen pre-optimization order includes targets in different 16-bit words
    // and two en passant sources interleaved with ordinary pawn captures.
    for(const auto& fixture:std::array<std::array<const char*,3>,2>{{
        {{"4k3/2n5/8/6n1/8/1n6/8/7K b - - 0 1", "",
          "c7a8 c7b5 c7d5 c7a6 c7e6 g5f7 g5h7 g5e6 g5f3 g5h3 g5e4 b3a5 b3c5 b3d4 b3a1 b3c1 b3d2 e8d7 e8e7 e8f7 e8d8 e8f8 "}},
        {{"7k/8/8/8/pPp1pPp1/1R1B3R/8/7K b - f3 0 1",
          "a4b3 c4b3 c4d3 e4d3 e4f3 g4h3 g4f3 ",
          "a4a3 c4c3 e4e3 g4g3 h8g7 h8h7 h8g8 "}}
    }}) {
        auto ordered=from_fen(fixture[0],stm);
        for(int kind=0;kind<2;++kind) {
            tmovelist list;int count=0;
            if(kind==0)petir_movgen::black_movgen_caps(ordered,list,count);
            else petir_movgen::black_movgen_noncaps(ordered,list,count);
            std::string sequence;
            for(int i=1;i<=count;++i)sequence+=move_text(list[i].moves,ordered,stm)+" ";
            check(sequence==fixture[kind+1],"optimized generator preserves frozen Black move order");
        }
    }
    struct PinFixture {const char* fen;std::uint64_t expected;};
    for(const auto& fixture:std::array<PinFixture,8>{{
        {"4r2k/8/8/8/8/8/4R3/4K3 w - - 0 1",UINT64_C(1)<<e2},
        {"4k3/4r3/8/8/8/8/8/4R2K b - - 0 1",UINT64_C(1)<<e7},
        {"k6b/8/5B2/8/3K4/8/8/8 w - - 0 1",UINT64_C(1)<<f6},
        {"4r2k/8/8/8/8/8/1R2p3/4K3 w - - 0 1",0},
        {"4r2k/8/8/8/8/4P3/4R3/4K3 w - - 0 1",0},
        {"5r1k/8/8/8/8/8/4R3/4K3 w - - 0 1",0},
        {"4b2k/8/8/8/8/8/4R3/4K3 w - - 0 1",0},
        {"4r2k/8/8/8/8/6b1/4RB2/4K3 w - - 0 1",(UINT64_C(1)<<e2)|(UINT64_C(1)<<f2)}
    }}) {
        auto pinned=from_fen(fixture.fen,stm);const auto unchanged=pinned;
        check(petir_tools::pinned_pieces(pinned,stm)==fixture.expected
              && std::memcmp(&pinned,&unchanged,sizeof(pinned))==0,
              "cached pin mask finds only single friendly blockers and preserves the position");
    }
    auto bare=from_fen("7k/8/8/8/8/8/8/K7 w - - 0 1",stm);
    check(petir_ueval::eval(bare,stm,-20500,20500)==0,"bare kings evaluate as draw");
    // Independent Syzygy WDL/DTZ fixtures. Mirror each across files and colors;
    // exercise full/lazy windows and infhr's reversed score perspective.
    struct KpkFixture { const char* name; const char* fen; int dtz; };
    for (const auto& fixture : {
        KpkFixture{"opposition draw", "8/4k3/8/4K3/4P3/8/8/8 w - - 0 1", 0},
        KpkFixture{"opposition win", "8/4k3/8/4K3/4P3/8/8/8 b - - 0 1", 4},
        KpkFixture{"sixth-rank king", "4k3/8/4K3/4P3/8/8/8/8 w - - 0 1", 3},
        KpkFixture{"rook promotion avoids stalemate", "8/k1P5/2K5/8/8/8/8/8 w - - 0 1", 1},
        KpkFixture{"stalemate", "k7/2K5/1P6/8/8/8/8/8 b - - 0 1", 0},
        KpkFixture{"pawn can be captured", "8/8/8/4k3/4P3/8/8/K7 b - - 0 1", 0},
        KpkFixture{"blocked seventh-rank pawn wins", "4k3/4P3/4K3/8/8/8/8/8 w - - 0 1", 5},
        KpkFixture{"blocked seventh-rank pawn draws", "4k3/4P3/4K3/8/8/8/8/8 b - - 0 1", 0},
        KpkFixture{"distant king draw", "8/8/8/8/5k2/8/P7/K7 w - - 0 1", 0},
        KpkFixture{"rook pawn corner draw", "7k/8/6KP/8/8/8/8/8 w - - 0 1", 0},
        KpkFixture{"seventh-rank rook pawn draw", "k7/P7/1K6/8/8/8/8/8 w - - 0 1", 0},
        KpkFixture{"initial double push", "8/8/8/8/8/8/2P4k/K7 w - - 0 1", 1},
        KpkFixture{"defender delays pawn reset", "8/8/8/8/8/2P5/7k/K7 b - - 0 1", 14},
        KpkFixture{"king must unblock pawn", "7k/8/8/8/8/8/4P3/K7 w - - 0 1", 11}}) {
        int fixture_side;
        const auto base = from_fen(fixture.fen,fixture_side);
        const int pawn_rank = std::countr_zero(base.pawnwhite) / 8;
        for (bool flip_color : {false,true}) for (bool flip_file : {false,true}) {
            Position position{};
            for (int square = 0; square < 64; ++square)
                position.papan[square ^ (flip_color ? 56 : 0) ^ (flip_file ? 7 : 0)]
                    = static_cast<std::int8_t>(base.papan[square] * (flip_color ? -1 : 1));
            petir_customboard::fillbitboard(position);
            const int actual_side = flip_color ? 3-fixture_side : fixture_side;
            const std::string tag = std::string("KPK: ") + fixture.name
                + (flip_color ? " black pawn" : " white pawn") + (flip_file ? " reflected" : "");
            for (int clock : {0,79,80,96,97,98,99,100,255}) {
                position.move50count = static_cast<std::uint8_t>(clock);
                const auto saved = position;
                const bool drawn = !fixture.dtz || clock >= 100 || clock + fixture.dtz > 100;
                int expected = drawn ? 0 : 700 + 8*pawn_rank - fixture.dtz;
                if (fixture_side == _sisihitam) expected = -expected;
                int raw = 12345;
                check(petir_kpk::probe(position,actual_side,raw) && raw == expected,tag+" exact outcome and DTZ score");
                for (const auto& window : {std::array<int,2>{-20500,20500},{0,1},{1000,1001}}) {
                    check(petir_ueval::eval(position,actual_side,window[0],window[1]) == expected,
                        tag+" full/lazy score at halfmove "+std::to_string(clock));
                    check(petir_ueval::eval(position,3-actual_side,window[0],window[1],true) == -expected,
                        tag+" infhr perspective at halfmove "+std::to_string(clock));
                }
                check(std::memcmp(&position,&saved,sizeof(position)) == 0,tag+" leaves position unchanged");
            }
        }
    }
    // Draw contempt is returned as configured in both perspectives; a winning
    // score equal to contempt must still reverse sign under infhr.
    const int saved_contempt = contemp_draw;
    auto kpk_draw = from_fen("8/4k3/8/4K3/4P3/8/8/8 w - - 0 1",stm);
    auto kpk_win = from_fen("8/4k3/8/4K3/4P3/8/8/8 b - - 0 1",stm);
    contemp_draw = -720;
    check(petir_ueval::eval(kpk_draw,1,0,1) == contemp_draw
        && petir_ueval::eval(kpk_draw,2,0,1,true) == contemp_draw,"KPK draws preserve contempt");
    check(petir_ueval::eval(kpk_win,2,0,1) == -720
        && petir_ueval::eval(kpk_win,1,0,1,true) == 720,"KPK winning score reverses even when equal to contempt");
    contemp_draw = saved_contempt;
    int unused_kpk_score;
    auto not_kpk = from_fen("7k/8/8/8/8/8/4P3/KQ6 w - - 0 1",stm);
    check(!petir_kpk::probe(bare,1,unused_kpk_score)
        && !petir_kpk::probe(not_kpk,1,unused_kpk_score),"KPK probe rejects other material");
    not_kpk = from_fen("7k/8/8/8/8/8/3PP3/K7 w - - 0 1",stm);
    check(!petir_kpk::probe(not_kpk,1,unused_kpk_score),"KPK probe rejects two pawns");
    auto check_mirrored_table=[&](const char* name,const auto& white,const auto& black) {
        bool matches=true;
        for(int square=0;square<64;++square)
            if(white[square]!=black[square^56])matches=false;
        check(matches,std::string("piece-square table agrees after color reversal: ")+name);
    };
    check_mirrored_table("pawn",petir_ueval::nilai_pos_pion_w,petir_ueval::nilai_pos_pion_b);
    check_mirrored_table("king kingside endgame",petir_ueval::nilai_pos_raja_endk_w,petir_ueval::nilai_pos_raja_endk_b);
    check_mirrored_table("king general endgame",petir_ueval::nilai_pos_raja_endn_w,petir_ueval::nilai_pos_raja_endn_b);
    auto king_white=from_fen("k5r1/8/8/8/8/8/P6P/1R2K3 w - - 0 1",stm);
    const int king_white_score=petir_ueval::eval(king_white,stm,-20500,20500);
    auto king_black=from_fen("1r2k3/p6p/8/8/8/8/8/K5R1 b - - 0 1",stm);
    check(king_white_score==petir_ueval::eval(king_black,stm,-20500,20500),
        "endgame king placement agrees after color reversal");
    // Regressions for material scaling, king/passers and connected-pawn order.
    // Reflect ranks and colors; side-to-move scores must agree even when lazy
    // evaluation intentionally returns an approximation in a narrow window.
    struct SymmetryFixture {const char* name;const char* fen;};
    for (const auto& fixture : {
        SymmetryFixture{"minor and pawn versus bare king", "7k/8/8/3N4/4P3/8/8/4K3 w - - 0 1"},
        SymmetryFixture{"minor and pawn versus queen", "7k/8/8/3N4/4P3/8/8/q3K3 w - - 0 1"},
        SymmetryFixture{"minor and pawn versus rook", "7k/8/8/3N4/4P3/8/8/r3K3 w - - 0 1"},
        SymmetryFixture{"minor and pawn versus two knights", "6nk/8/8/3N4/4P3/8/8/n3K3 w - - 0 1"},
        SymmetryFixture{"nearby defending king", "8/1k1P4/8/8/8/7N/7K/8 b - - 0 1"},
        SymmetryFixture{"connected passers with promotion shortcut", "8/3b4/3k1p2/p4P2/P7/6pp/1b2K3/8 b - - 9 99"},
        SymmetryFixture{"staggered passers", "7k/6P1/5P2/8/8/8/8/R3K3 w - - 0 1"},
        SymmetryFixture{"doubled passers", "7k/8/5PP1/5P2/8/8/8/R3K3 w - - 0 1"}}) {
        int side=1;const auto original=from_fen(fixture.fen,side);
        for (int clock : {0,80,99}) {
            auto position=original;position.move50count=static_cast<std::uint8_t>(clock);
            Position reversed{};
            for(int square=0;square<64;++square)
                reversed.papan[square^56]=static_cast<std::int8_t>(-position.papan[square]);
            petir_customboard::fillbitboard(reversed);reversed.move50count=position.move50count;
            const auto saved=position,saved_reversed=reversed;
            const auto tag=std::string("evaluation symmetry: ")+fixture.name+" clock="+std::to_string(clock);
            const int full=petir_ueval::eval(position,side,-20500,20500);
            check(full==petir_ueval::eval(reversed,3-side,-20500,20500),tag+" full");
            for(const auto& window:{std::array<int,2>{0,1},{full,full+1}})
                check(petir_ueval::eval(position,side,window[0],window[1])
                    ==petir_ueval::eval(reversed,3-side,window[0],window[1]),tag+" narrow");
            check(petir_ueval::eval(position,3-side,-20500,20500,true)==-full
                &&petir_ueval::eval(reversed,side,-20500,20500,true)==-full,tag+" infhr");
            check(std::memcmp(&position,&saved,sizeof(position))==0
                &&std::memcmp(&reversed,&saved_reversed,sizeof(reversed))==0,tag+" input unchanged");
        }
    }
    struct SymmetryScoreFixture {const char* name;const char* fen;int expected;};
    for(const auto& fixture:{
        SymmetryScoreFixture{"material scaling reference", "7k/8/8/3N4/4P3/8/8/4K3 w - - 0 1",511},
        SymmetryScoreFixture{"king distance reference", "8/1k1P4/8/8/8/7N/7K/8 b - - 0 1",-610},
        SymmetryScoreFixture{"connected shortcut reference", "8/3b4/3k1p2/p4P2/P7/6pp/1b2K3/8 b - - 9 99",1794}}) {
        auto position=from_fen(fixture.fen,stm);
        check(petir_ueval::eval(position,stm,-20500,20500)==fixture.expected,fixture.name);
    }
    struct ConnectedFixture {const char* name;const char* fen;int expected;};
    for(const auto& fixture:{
        ConnectedFixture{"single pawn", "7k/8/5P2/8/8/8/8/R3K3 w - - 0 1",0},
        ConnectedFixture{"sixth rank pair counted once", "7k/8/5PP1/8/8/8/8/R3K3 w - - 0 1",127},
        ConnectedFixture{"staggered sixth and seventh", "7k/6P1/5P2/8/8/8/8/R3K3 w - - 0 1",157},
        ConnectedFixture{"fifth rank pair", "7k/8/8/5PP1/8/8/8/R3K3 w - - 0 1",52},
        ConnectedFixture{"staggered fifth and sixth", "7k/8/6P1/5P2/8/8/8/R3K3 w - - 0 1",72},
        ConnectedFixture{"widely staggered passers", "7k/6P1/8/8/5P2/8/8/R3K3 w - - 0 1",7},
        ConnectedFixture{"three neighboring passers", "7k/8/4PPP1/8/8/8/8/R3K3 w - - 0 1",254},
        ConnectedFixture{"doubled pawn not counted again", "7k/8/5PP1/5P2/8/8/8/R3K3 w - - 0 1",127},
        ConnectedFixture{"enemy pawn prevents passers", "7k/6p1/5PP1/8/8/8/8/R3K3 w - - 0 1",0}}) {
        int side=1;const auto base=from_fen(fixture.fen,side);
        for(bool color_flip:{false,true})for(bool file_flip:{false,true}) {
            Position position{};
            for(int square=0;square<64;++square)
                position.papan[square^(color_flip?56:0)^(file_flip?7:0)]
                    =static_cast<std::int8_t>(base.papan[square]*(color_flip?-1:1));
            petir_customboard::fillbitboard(position);const auto saved=position;
            check(petir_ueval::connected_passed_pawns(position,!color_flip)==fixture.expected,
                std::string("connected passers: ")+fixture.name);
            check(std::memcmp(&position,&saved,sizeof(position))==0,"connected passers leave input unchanged");
        }
    }
    // Exercise the new component independently of material/PST/draw scaling.
    struct PasserActivityFixture {const char* name;const char* fen;int pawn;int expected;};
    for(const auto& fixture:{
        PasserActivityFixture{"clear path, supporting king", "n6k/8/8/3KP3/8/8/8/1R6 w - - 0 1",e5,12},
        PasserActivityFixture{"own king far away", "n6k/8/8/4P3/8/8/8/KR6 w - - 0 1",e5,4},
        PasserActivityFixture{"defender king controls push", "n7/5k2/8/3KP3/8/8/8/1R6 w - - 0 1",e5,-6},
        PasserActivityFixture{"enemy blockade", "7k/8/4n3/3KP3/8/8/8/1R6 w - - 0 1",e5,-4},
        PasserActivityFixture{"friendly blockade", "n6k/8/4B3/3KP3/8/8/8/1R6 w - - 0 1",e5,-4},
        PasserActivityFixture{"knight controls push", "7k/6n1/8/3KP3/8/8/8/1R6 w - - 0 1",e5,-2},
        PasserActivityFixture{"bishop controls push", "7k/8/8/3KP3/6b1/8/8/1R6 w - - 0 1",e5,-2},
        PasserActivityFixture{"rook controls push", "7k/8/7r/3KP3/8/8/8/1R6 w - - 0 1",e5,-2},
        PasserActivityFixture{"queen controls push", "7k/8/7q/3KP3/8/8/8/1R6 w - - 0 1",e5,-2},
        PasserActivityFixture{"rook revealed behind pawn", "7k/8/8/3KP3/8/8/4r3/1R6 w - - 0 1",e5,-2},
        PasserActivityFixture{"queen revealed behind pawn", "7k/8/8/3KP3/8/8/4q3/1R6 w - - 0 1",e5,-2},
        PasserActivityFixture{"later path square attacked", "6rk/8/8/3KP3/8/8/8/1R6 w - - 0 1",e5,8},
        PasserActivityFixture{"later path square occupied", "n6k/4B3/8/3KP3/8/8/8/1R6 w - - 0 1",e5,8},
        PasserActivityFixture{"seventh-rank board edge", "n6k/4P3/3K4/8/8/8/8/1R6 w - - 0 1",e7,30},
        PasserActivityFixture{"fourth-rank pawn excluded", "n6k/8/8/3K4/4P3/8/8/1R6 w - - 0 1",e4,0},
        PasserActivityFixture{"middlegame excluded", "rq5k/8/8/3KP3/8/8/8/1R6 w - - 0 1",e5,0},
        PasserActivityFixture{"KPK excluded", "7k/8/8/3KP3/8/8/8/8 w - - 0 1",e5,0}}) {
        auto position=from_fen(fixture.fen,stm);const auto saved=position;
        const auto tag=std::string("passed-pawn activity: ")+fixture.name;
        check(consistent(position),tag+" consistent position");
        check(petir_ueval::passed_pawn_activity(position,fixture.pawn,true)==fixture.expected,tag);
        // Reverse colors/ranks, including slider occupancy and all piece types.
        std::string placement;int empty=0;
        for(int square=63;square>=0;--square) {
            const int source=(square^7)^56;
            const int piece=-position.papan[source];
            if(!piece)++empty;
            else {if(empty)placement+=char('0'+empty);empty=0;
                const char* symbols=" PNBRQK";char symbol=symbols[std::abs(piece)];
                placement+=piece>0?symbol:char(symbol-'A'+'a');}
            if((square&7)==0) {if(empty)placement+=char('0'+empty);empty=0;if(square)placement+='/';}
        }
        int reversed_side=2;auto reversed=from_fen(placement+" b - - 0 1",reversed_side);
        check(petir_ueval::passed_pawn_activity(reversed,fixture.pawn^56,false)==fixture.expected,
              tag+" color reversal");
        check(std::memcmp(&position,&saved,sizeof(position))==0,tag+" input unchanged");
    }
    // Endgame activity: safety, pawn targets, passive defense and stable blockades.
    struct EndgameActivityFixture {const char* name;const char* fen;int square;int expected;};
    for(const auto& fixture:{
        EndgameActivityFixture{"rook free movement", "7k/8/8/8/8/8/7P/R6K w - - 0 1",a1,6},
        EndgameActivityFixture{"rook attacks weak pawn", "7k/8/8/R3p3/8/8/7P/7K w - - 0 1",a5,10},
        EndgameActivityFixture{"rook attacks defended pawn", "7k/8/5p2/R3p3/8/8/7P/7K w - - 0 1",a5,6},
        EndgameActivityFixture{"rook under attack", "7k/8/2n5/R3p3/8/8/7P/7K w - - 0 1",a5,0},
        EndgameActivityFixture{"restricted rook tied to pawn", "7k/8/8/8/8/8/P1r5/RN5K w - - 0 1",a1,-9},
        EndgameActivityFixture{"restricted rook pawn has another defender", "7k/8/8/8/8/8/P1r5/RB5K w - - 0 1",a1,-6},
        EndgameActivityFixture{"rook pawnless excluded", "7k/8/8/8/8/8/8/R6K w - - 0 1",a1,0},
        EndgameActivityFixture{"rook middlegame excluded", "rq5k/8/8/8/8/8/7P/R6K w - - 0 1",a1,0},
        EndgameActivityFixture{"knight supported useful outpost", "7k/8/1p6/3N4/4P3/8/8/4K3 w - - 0 1",d5,6},
        EndgameActivityFixture{"knight no support", "7k/8/1p6/3N4/8/4P3/8/4K3 w - - 0 1",d5,0},
        EndgameActivityFixture{"knight defended target", "8/k7/1p6/3N4/4P3/8/8/4K3 w - - 0 1",d5,0},
        EndgameActivityFixture{"knight future pawn challenge", "7k/4p3/1p6/3N4/4P3/8/8/4K3 w - - 0 1",d5,0},
        EndgameActivityFixture{"knight supported blockade", "7k/8/3p4/3N4/4P3/8/8/4K3 w - - 0 1",d5,10},
        EndgameActivityFixture{"knight no concrete target", "7k/8/7p/3N4/4P3/8/8/4K3 w - - 0 1",d5,0},
        EndgameActivityFixture{"knight under attack", "7k/2n5/1p6/3N4/4P3/8/8/4K3 w - - 0 1",d5,0},
        EndgameActivityFixture{"knight edge blockade", "7k/8/p7/N7/1P6/8/8/4K3 w - - 0 1",a5,0},
        EndgameActivityFixture{"KPK excluded", "7k/8/8/3KP3/8/8/8/8 w - - 0 1",d5,0}}) {
        auto position=from_fen(fixture.fen,stm);const auto saved=position;
        const auto tag=std::string("endgame piece activity: ")+fixture.name;
        check(consistent(position),tag+" consistent position");
        const int activity=petir_ueval::endgame_piece_activity(position,fixture.square,true);
        check(activity==fixture.expected,tag+" actual="+std::to_string(activity));
        std::string placement;int empty=0;
        for(int square=63;square>=0;--square) {
            const int piece=-position.papan[(square^7)^56];
            if(!piece)++empty;
            else {if(empty)placement+=char('0'+empty);empty=0;
                const char* symbols=" PNBRQK";char symbol=symbols[std::abs(piece)];
                placement+=piece>0?symbol:char(symbol-'A'+'a');}
            if((square&7)==0) {if(empty)placement+=char('0'+empty);empty=0;if(square)placement+='/';}
        }
        int reversed_side=2;auto reversed=from_fen(placement+" b - - 0 1",reversed_side);
        check(petir_ueval::endgame_piece_activity(reversed,fixture.square^56,false)==activity,
              tag+" color reversal");
        check(petir_ueval::eval(position,1,-20500,20500)
              ==petir_ueval::eval(reversed,2,-20500,20500),tag+" full evaluation color reversal");
        check(std::memcmp(&position,&saved,sizeof(position))==0,tag+" input unchanged");
    }
    // KRP-KR soft scaling: guarded defenses, favorable seventh-rank pawns,
    // side-to-move sensitivity and both board symmetries.
    struct RookPawnFixture {const char* name;const char* fen;int scale;};
    for(const auto& fixture:{
        RookPawnFixture{"Philidor", "8/4k3/r7/4K3/4P3/8/8/1R6 w - - 0 1",25},
        RookPawnFixture{"rear checks", "1k6/3K4/1P6/8/1R6/8/8/7r w - - 0 1",25},
        RookPawnFixture{"rook pawn trapped rook", "R7/P6k/8/8/8/r7/5K2/8 w - - 0 1",25},
        RookPawnFixture{"seventh rank", "2K5/k1P5/8/8/8/8/7r/3R4 w - - 0 1",100},
        RookPawnFixture{"defender to move fallback", "2K5/k1P5/8/8/8/8/7r/3R4 b - - 0 1",50},
        RookPawnFixture{"rook capture tactic", "8/4k3/r7/4K3/4P3/8/8/R7 w - - 0 1",50},
        RookPawnFixture{"Philidor winning exception", "3k4/8/1r6/8/3K4/2P5/3R4/8 w - - 0 1",50},
        RookPawnFixture{"unrecognized position", "7k/7r/8/8/8/8/7P/R6K w - - 0 1",50},
        RookPawnFixture{"KPK outside scope", "7k/8/8/3KP3/8/8/8/8 w - - 0 1",100},
        RookPawnFixture{"extra pawn outside scope", "8/4k3/r7/4K3/3PP3/8/8/1R6 w - - 0 1",100},
        RookPawnFixture{"extra minor outside scope", "8/4k3/r7/4K3/4P3/8/8/1RN5 w - - 0 1",100}}) {
        int original_side=1;auto original=from_fen(fixture.fen,original_side);
        for(bool color_flip:{false,true})for(bool file_flip:{false,true}) {
            std::string placement;int empty=0;
            const int flip=(color_flip?56:0)^(file_flip?7:0);
            for(int square=63;square>=0;--square) {
                int piece=original.papan[(square^7)^flip];if(color_flip)piece=-piece;
                if(!piece)++empty;
                else {if(empty)placement+=char('0'+empty);empty=0;
                    const char* symbols=" PNBRQK";char symbol=symbols[std::abs(piece)];
                    placement+=piece>0?symbol:char(symbol-'A'+'a');}
                if((square&7)==0){if(empty)placement+=char('0'+empty);empty=0;if(square)placement+='/';}
            }
            int side=color_flip?3-original_side:original_side;
            auto position=from_fen(placement+(side==1?" w - - 0 1":" b - - 0 1"),side);
            const auto saved=position;
            const auto tag=std::string("rook-pawn scaling: ")+fixture.name
                +" color="+std::to_string(color_flip)+" file="+std::to_string(file_flip);
            check(consistent(position),tag+" consistent position");
            check(petir_ueval::rook_pawn_scale(position,side)==fixture.scale,tag);
            const int score=petir_ueval::eval(position,side,-20500,20500);
            check(score==-petir_ueval::eval(position,3-side,-20500,20500,true),
                  tag+" infhr preserves actual side to move");
            check(score==petir_ueval::eval(position,side,score,score+1),tag+" narrow window");
            check(std::memcmp(&position,&saved,sizeof(position))==0,tag+" input unchanged");
        }
    }
    {
        int side=1;auto philidor=from_fen("8/4k3/r7/4K3/4P3/8/8/1R6 w - - 0 1",side);
        auto seventh=from_fen("2K5/k1P5/8/8/8/8/7r/3R4 w - - 0 1",side);
        check(petir_ueval::eval(philidor,side,-20500,20500)==48,"Philidor full score uses 25 percent scaling");
        check(petir_ueval::eval(seventh,side,-20500,20500)==316,"seventh-rank full score retains full advantage");
    }
    // A king one file from the black passer must get the same evaluation
    // adjustment as in the color-reversed position (d3/e1 becomes d6/e8).
    auto black_passer=from_fen("k7/8/8/8/8/3p4/8/1R2K3 w - - 0 1",stm);
    const int black_passer_score=petir_ueval::eval(black_passer,stm,-20500,20500);
    auto white_passer=from_fen("1r2k3/8/3P4/8/8/8/8/K7 b - - 0 1",stm);
    check(black_passer_score==petir_ueval::eval(white_passer,stm,-20500,20500),
        "passed-pawn defender king file distance agrees after color reversal");
    auto isolated_white=from_fen("7k/8/8/8/8/4P3/4P3/KR5r w - - 0 1",stm);
    const int isolated_white_score=petir_ueval::eval(isolated_white,stm,-20500,20500);
    auto isolated_black=from_fen("kr5R/4p3/4p3/8/8/8/8/7K b - - 0 1",stm);
    check(isolated_white_score==petir_ueval::eval(isolated_black,stm,-20500,20500),
        "isolated doubled pawns receive the same penalty after color reversal");
    // With e5/e6 doubled passers, only the front pawn's advance bonus counts.
    // Probe each rank's bonus independently without freezing the whole score.
    auto doubled_passers=from_fen("7k/8/4P3/4P3/8/8/8/KR6 w - - 0 1",stm);
    const int doubled_passers_score=petir_ueval::eval(doubled_passers,stm,-20500,20500);
    int doubled_mirror_side=2;
    auto doubled_mirror=from_fen("kr6/8/8/8/4p3/4p3/8/7K b - - 0 1",doubled_mirror_side);
    check(doubled_passers_score==petir_ueval::eval(doubled_mirror,doubled_mirror_side,-20500,20500),
        "doubled passer evaluation agrees after color reversal");
    for(int rank:{4,5}) {
        const auto bonus=petir_ueval::w_nilai_pion_advance_bebas[rank];
        petir_ueval::w_nilai_pion_advance_bebas[rank]=static_cast<std::uint8_t>(bonus+1);
        const int adjusted_score=petir_ueval::eval(doubled_passers,stm,-20500,20500);
        petir_ueval::w_nilai_pion_advance_bebas[rank]=bonus;
        check(adjusted_score-doubled_passers_score==(rank==5?1:0),
            std::string("white doubled passers use the front pawn's advance bonus: ")+(rank==5?"e6":"e5"));
    }
    // Each bishop has four own pawns on its color and no opposing bishop.
    // Neutralizing just that penalty isolates its effect from other terms.
    struct BadBishopFixture {const char* name;const char* fen;int sign;};
    for(const auto& fixture:{
        BadBishopFixture{"black", "7k/8/8/3p1p2/2p1p3/8/8/KR3b2 w - - 0 1",1},
        BadBishopFixture{"white", "7k/8/8/3P1P2/2P1P3/8/8/KB5r w - - 0 1",-1}}) {
        auto position=from_fen(fixture.fen,stm);
        const int score=petir_ueval::eval(position,stm,-20500,20500);
        const auto penalty=petir_evalmask::nilai_badbishop[4];
        petir_evalmask::nilai_badbishop[4]=0;
        const int neutral_score=petir_ueval::eval(position,stm,-20500,20500);
        petir_evalmask::nilai_badbishop[4]=penalty;
        check(score-neutral_score==fixture.sign*penalty,
            std::string("bad-bishop penalty applies without an opposing bishop: ")+fixture.name);
    }
    // Isolate king pressure from the mobility/PST effects of moving the rook.
    auto king_pressure_contribution=[&](const char* fen) {
        int side=1;auto position=from_fen(fen,side);
        const int score=petir_ueval::eval(position,side,-20500,20500);
        const auto pressure_table=petir_ueval::nilai_king_pressure;
        for(int i=0;i<64;++i)petir_ueval::nilai_king_pressure[i]=0;
        const int neutral_score=petir_ueval::eval(position,side,-20500,20500);
        petir_ueval::nilai_king_pressure=pressure_table;
        return score-neutral_score;
    };
    struct RookDefenseFixture {const char* name;const char* defended;const char* exposed;};
    for(const auto& fixture:{
        RookDefenseFixture{"black",
            "r1b3rk/4Q3/4N3/8/8/8/7P/R3K2R w - - 0 1",
            "r1b2r1k/4Q3/4N3/8/8/8/7P/R3K2R w - - 0 1"},
        RookDefenseFixture{"white",
            "r3k2r/7p/8/8/8/4n3/4q3/R1B3RK b - - 0 1",
            "r3k2r/7p/8/8/8/4n3/4q3/R1B2R1K b - - 0 1"}}) {
        check(king_pressure_contribution(fixture.defended)<king_pressure_contribution(fixture.exposed),
            std::string("rook defense reduces king danger without its own queen: ")+fixture.name);
    }
    // These windows pass stage 1; the removed later lazy checkpoints used to
    // return before king-danger evaluation or rule-50 scaling completed.
    struct LazyEvalFixture {const char* name;const char* fen;int alpha,beta,score;};
    for(const auto& fixture:{
        LazyEvalFixture{"king danger after former stage 2",
            "8/4n1r1/1p3P2/p1k5/P1B5/1P2bK1N/2PN2Pq/5RR1 w - - 0 1",-68,-67,-68},
        LazyEvalFixture{"rule-50 scaling after former stage 3",
            "7k/8/8/8/8/8/8/KQ6 w - - 99 1",200,201,100}}) {
        auto position=from_fen(fixture.fen,stm);
        check(consistent(position),std::string("lazy evaluation fixture is consistent: ")+fixture.name);
        const int full=petir_ueval::eval(position,stm,-20500,20500);
        const int narrow=petir_ueval::eval(position,stm,fixture.alpha,fixture.beta);
        check(full==fixture.score,std::string("full evaluation reference: ")+fixture.name+" actual="+std::to_string(full));
        check(narrow==full,std::string("narrow evaluation completes remaining terms: ")+fixture.name);
    }
    // Use windows that previously let stage 1 bypass the final draw scaling.
    for(const char* fen:{
        "7k/8/8/8/8/8/8/KQ6 w - - 0 1",
        "kq6/8/8/8/8/8/8/7K b - - 0 1"}) {
        int strong_side=1;auto position=from_fen(fen,strong_side);
        for(int counter:{80,99})for(int side:{strong_side,3-strong_side}) {
            position.move50count=static_cast<std::uint8_t>(counter);
            const int full=petir_ueval::eval(position,side,-20500,20500);
            const int beta=side==strong_side?101:-100;
            const int narrow=petir_ueval::eval(position,side,beta-1,beta);
            check(narrow==full,"stage 1 preserves rule-50 scaling at halfmove "+std::to_string(counter)
                +" / side "+std::to_string(side)+" / stronger side "+std::to_string(strong_side));
        }
    }
    for(const char* fen:{
        "7k/8/8/8/8/8/8/KRB4r w - - 0 1",
        "krb4R/8/8/8/8/8/8/7K b - - 0 1",
        "7k/8/8/8/8/8/8/KRN4r w - - 0 1",
        "krn4R/8/8/8/8/8/8/7K b - - 0 1"}) {
        int strong_side=1;auto position=from_fen(fen,strong_side);
        for(int side:{strong_side,3-strong_side}) {
            const int full=petir_ueval::eval(position,side,-20500,20500);
            const int beta=side==strong_side?-999:1000;
            const int narrow=petir_ueval::eval(position,side,beta-1,beta);
            check(narrow==full,"stage 1 preserves rook-plus-minor draw scaling: "+std::string(fen)
                +" / side "+std::to_string(side));
        }
    }
    check(insufficient_material(bare),"bare kings are insufficient mating material");
    for(const char* fen:{
        "7k/8/8/8/8/8/8/KN6 w - - 0 1",
        "7k/8/8/8/8/8/8/KB6 w - - 0 1",
        "7k/8/8/4b3/8/8/8/K1B5 w - - 0 1"}) {
        auto dead=from_fen(fen,stm);
        check(insufficient_material(dead),"single minor / same-color bishops cannot mate");
    }
    for(const char* fen:{
        "7k/8/8/8/8/8/8/KNN5 w - - 0 1",
        "6nk/8/8/8/8/8/8/KN6 w - - 0 1",
        "7k/8/8/3b4/8/8/8/K1B5 w - - 0 1"}) {
        auto live=from_fen(fen,stm);
        check(!insufficient_material(live),"do not claim insufficient material when mate is possible");
    }
    const std::string fen_state="r3k2r/8/8/3pP3/8/8/8/R3K2R w KQkq d6 42 17";
    int fullmove=0;auto roundtrip=from_fen(fen_state,stm,&fullmove);
    check(to_fen(roundtrip,stm,fullmove)==fen_state,"FEN output preserves castling, en passant, and counters");
    const auto old_mode=modetime,old_moves=jumlahlangkah,old_rmoves=rmoves,old_clock=clockwhite;
    modetime=moves_minutes;rmoves=40;clockwhite=2400;
    jumlahlangkah=0;const int period_budget=original_time_budget(1);
    jumlahlangkah=80;check(original_time_budget(1)==period_budget,"clock budget resets at the second time-control period");
    jumlahlangkah=160;check(original_time_budget(1)==period_budget,"clock budget resets at later time-control periods");
    jumlahlangkah=78;check(original_time_budget(1)==800,"last move of a time-control period gets the legacy budget");
    modetime=old_mode;jumlahlangkah=old_moves;rmoves=old_rmoves;clockwhite=old_clock;
    auto ep=from_fen("7k/8/8/3pP3/8/8/8/K7 w - d6 0 1",stm);
    int m=parse_move("e5d6",ep,stm);check(m!=0,"en passant parsed");make_move(ep,stm,m);
    check(ep.papan[35]==0 && ep.papan[43]==1 && consistent(ep),"en passant updates occupancy, material, and hash");
    // An en passant evasion must remove the check without exposing another
    // attack. Also cover legal captures and interpositions for both colors.
    for(const auto& [fen,ep_legal]:std::initializer_list<std::pair<const char*,bool>>{
        {"4r2k/8/8/3pP3/4K3/8/8/8 w - d6 0 1",false},
        {"8/8/8/4k3/3Pp3/8/8/4R2K b - d3 0 1",false},
        {"1r5k/8/8/pP6/1K6/8/8/8 w - a6 0 1",false},
        {"8/8/8/1k6/Pp6/8/8/1R5K b - a3 0 1",false},
        {"6rk/8/8/6Pp/6K1/8/8/8 w - h6 0 1",false},
        {"8/8/8/6k1/6pP/8/8/6RK b - h3 0 1",false},
        {"7k/8/8/3pP3/4K3/8/8/8 w - d6 0 1",true},
        {"8/8/8/4k3/3Pp3/8/8/7K b - d3 0 1",true},
        {"7k/8/8/2Pp1p2/2K5/8/8/8 w - f6 0 1",false},
        {"8/8/8/2k5/2pP1P2/8/8/7K b - f3 0 1",false},
        {"1b5k/8/8/2Pp4/8/8/7K/8 w - d6 0 1",true},
        {"8/7k/8/8/2pP4/8/8/1B5K b - d3 0 1",true}}) {
        auto evasion=from_fen(fen,stm);const auto original_fen=to_fen(evasion,stm,1);
        check(stm==1?petir_tools::white_checked(evasion):petir_tools::black_checked(evasion),"en passant evasion fixture starts in check");
        tmovelist candidates;int count=0,check_count=0;
        if(stm==1)petir_movgen::white_evasion(evasion,candidates,count,check_count);else petir_movgen::black_evasion(evasion,candidates,count,check_count);
        std::vector<int> actual,expected;bool contains_ep=false;
        for(int i=1;i<=count;++i) {
            actual.push_back(candidates[i].moves);
            if(((candidates[i].moves>>7)&127)==_en_passant)contains_ep=true;
        }
        auto append_legal=[&] {
            for(int i=1;i<=count;++i) {
                auto child=evasion;make_move(child,stm,candidates[i].moves);
                if(!(stm==1?petir_tools::white_checked(child):petir_tools::black_checked(child)))expected.push_back(candidates[i].moves);
            }
        };
        if(stm==1)petir_movgen::white_movgen_caps(evasion,candidates,count);else petir_movgen::black_movgen_caps(evasion,candidates,count);append_legal();
        if(stm==1)petir_movgen::white_movgen_noncaps(evasion,candidates,count);else petir_movgen::black_movgen_noncaps(evasion,candidates,count);append_legal();
        std::sort(actual.begin(),actual.end());std::sort(expected.begin(),expected.end());
        check(contains_ep==ep_legal,"en passant evasion legality "+std::string(fen));
        check(actual==expected,"en passant evasions match independent king-safety filtering "+std::string(fen));
        check(to_fen(evasion,stm,1)==original_fen && consistent(evasion),"en passant evasion generation preserves the position");
    }
    for(const char* fen:{
        "3rr2k/8/8/3pPP2/3PKP2/3PPP2/8/8 w - d6 0 1",
        "8/8/3ppp2/3pkp2/3Ppp2/8/8/3RR2K b - d3 0 1"}) {
        auto ep_mate=from_fen(fen,stm);tmovelist candidates;int count=0,check_count=0;
        if(stm==1)petir_movgen::white_evasion(ep_mate,candidates,count,check_count);else petir_movgen::black_evasion(ep_mate,candidates,count,check_count);
        check(legal_moves(ep_mate,stm).empty(),"illegal en passant is the only pseudo-evasion in a checkmate fixture");
        prepare_search(stm,false,0);usewinboard=false;
        check(count==0 && petir_search::q_search(stm,-20500,20500,ep_mate,1,2,0,petir_search::noval,petir_search::node_pv)==-19998,"quiescence recognizes mate when en passant would expose the king");
    }
    for(const char* fen:{
        "7k/5Q2/6K1/8/8/8/8/8 b - - 0 1",
        "8/8/8/8/8/6k1/5q2/7K w - - 0 1",
        "R5bk/5Q2/6K1/8/8/8/8/8 b - - 0 1",
        "8/8/8/8/8/6k1/5q2/r5BK w - - 0 1",
        "8/8/1b6/8/8/5k2/r7/6nK w - - 0 1",
        "6Nk/R7/5K2/8/8/1B6/8/8 b - - 0 1",
        "7k/5Q2/6K1/8/8/8/p7/N7 b - - 0 1",
        "n7/P7/8/8/8/6k1/5q2/7K w - - 0 1",
        "8/1b6/8/8/5b2/4k3/5qP1/7K w - - 0 1",
        "7k/5Qp1/4K3/5B2/8/8/1B6/8 b - - 0 1"}) {
        auto stale=from_fen(fen,stm);const auto original_fen=to_fen(stale,stm,1);
        check(!(stm==1?petir_tools::white_checked(stale):petir_tools::black_checked(stale)) && legal_moves(stale,stm).empty(),"quiescence fixture is stalemate "+std::string(fen));
        prepare_search(stm,false,0);usewinboard=false;
        check(petir_search::q_search(stm,-20500,20500,stale,1,2,0,petir_search::noval,petir_search::node_pv)==contemp_draw,"quiescence scores stalemate as a draw with checks enabled");
        check(petir_search::q_search(stm,-1,0,stale,0,2,0,petir_search::noval,petir_search::node_cut)==contemp_draw,"capture-only quiescence scores stalemate as a draw");
        check(petir_search::q_search(stm,320,321,stale,0,2,1,321,petir_search::node_cut)==contemp_draw,"stalemate overrides a cached stand-pat beta cutoff");
        check(petir_search::q_search(stm,-20500,20500,stale,1,63,0,petir_search::noval,petir_search::node_pv)==contemp_draw,"stalemate overrides the quiescence depth-limit evaluation");
        check(to_fen(stale,stm,1)==original_fen && consistent(stale) && total_node==0,"stalemate probes preserve the position and search node count");
    }
    for(const char* fen:{
        "7k/4Q3/6K1/8/8/8/8/8 b - - 0 1",
        "8/8/8/8/8/6k1/4q3/7K w - - 0 1",
        "8/8/8/8/8/5k2/r7/6nK w - - 0 1",
        "6Nk/R7/5K2/8/8/8/8/8 b - - 0 1",
        "8/8/8/8/8/6k1/5q2/1N5K w - - 0 1",
        "8/P7/8/8/8/6k1/5q2/7K w - - 0 1",
        "8/8/2p5/2Pp4/8/6k1/5q2/7K w - d6 0 1",
        "7k/5Q2/6K1/8/2pP4/2P5/8/8 b - d3 0 1"}) {
        auto live=from_fen(fen,stm);
        check(!(stm==1?petir_tools::white_checked(live):petir_tools::black_checked(live)) && !legal_moves(live,stm).empty(),"quiet-move/promotion/en passant fixture has a legal move");
        prepare_search(stm,false,0);usewinboard=false;
        check(petir_search::q_search(stm,320,321,live,0,2,0,321,petir_search::node_cut)==321 && total_node==0,"a legal quiet move or en passant prevents a false stalemate draw");
    }
    auto castling=from_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",stm);
    m=parse_move("e1g1",castling,stm);check(m!=0,"short castling parsed");make_move(castling,stm,m);
    check(castling.papan[6]==6 && castling.papan[5]==4 && consistent(castling),"castling updates rook, king, and hash");
    // Put a legal castle first with a shallow hash move, then force a root
    // beta cutoff. Both castle codes exceed the board's last square (63).
    for(int color=1;color<=2;++color)for(bool queenside:{false,true}) {
        new_game();
        auto castle_root=from_fen(std::string("r3k2r/8/8/8/8/8/8/R3K2R ")+(color==1?"w":"b")+" KQkq - 0 1",stm);
        const auto text=color==1?(queenside?"e1c1":"e1g1"):(queenside?"e8c8":"e8g8");
        const int castle_move=parse_move(text,castle_root,color);
        prepare_search(color,false,0);usewinboard=false;
        petir_hashing::addtable(castle_root.hashkey,color,0,castle_move,0,fail_high,castle_root.move50count);
        const int score=petir_root::searchroot(8,color,_nodemax,-19001,-19000,castle_root,false,0,0);
        const auto& ordering=color==1?historyw:historyb;
        const int code=(castle_move>>7)&127;
        check(castle_move!=0 && mmoves==castle_move && score>=-19000,"castling root cutoff returns the legal castle "+std::string(text));
        check(ordering[color==1?6:-6][code]==1 && killer0[0]==castle_move,"castling root cutoff updates king history and killers "+std::string(text));
        check(consistent(castle_root),"castling root search preserves its input board "+std::string(text));
    }
    for(char piece:std::string("qnbr")) {
        auto promotion=from_fen("7k/P7/8/8/8/8/8/7K w - - 0 1",stm);
        m=parse_move("a7a8"+std::string(1,piece),promotion,stm);
        check(m!=0,"accept opponent promotion "+std::string(1,piece));make_move(promotion,stm,m);
        if(piece=='b') {
            check(promotion.ngajahputih==1,"bishop promotion increments the bishop count");
        }
        check(consistent(promotion),"promotion material and bitboards "+std::string(1,piece));
    }
    struct BishopPromotionFixture {const char* name;const char* fen;const char* promotion;const char* capture;};
    for(const auto& fixture:{
        BishopPromotionFixture{"white first bishop",
            "1r5k/P7/8/8/8/8/8/7K w - - 0 1","a7a8b","b8a8"},
        BishopPromotionFixture{"black first bishop",
            "7k/8/8/8/8/8/p7/1R5K b - - 0 1","a2a1b","b1a1"},
        BishopPromotionFixture{"white second bishop by capture",
            "1rr4k/P7/8/8/8/8/8/K4B2 w - - 0 1","a7b8b","c8b8"},
        BishopPromotionFixture{"black second bishop by capture",
            "k4b2/8/8/8/8/8/p7/1RR4K b - - 0 1","a2b1b","c1b1"}}) {
        int side=1;auto position=from_fen(fixture.fen,side);
        const int before=side==1?position.ngajahputih:position.ngajahhitam;
        const int promotion_move=parse_move(fixture.promotion,position,side);
        check(promotion_move!=0,std::string("bishop promotion is legal: ")+fixture.name);
        if(!promotion_move)continue;
        make_move(position,side,promotion_move);
        check((side==1?position.ngajahputih:position.ngajahhitam)==before+1,
            std::string("bishop promotion increases existing count: ")+fixture.name);
        check(consistent(position),std::string("bishop promotion preserves incremental state: ")+fixture.name);
        const int capture_move=parse_move(fixture.capture,position,3-side);
        check(capture_move!=0,std::string("promoted bishop can be captured: ")+fixture.name);
        if(!capture_move)continue;
        make_move(position,3-side,capture_move);
        check((side==1?position.ngajahputih:position.ngajahhitam)==before,
            std::string("capturing promoted bishop restores original count: ")+fixture.name);
        check(consistent(position),std::string("promoted bishop capture preserves incremental state: ")+fixture.name);
    }
    // Replay deterministic legal sequences to check all incremental representations.
    Position walk=start;stm=1;std::uint32_t sequence=12345;
    for(int i=0;i<180;++i) {
        auto moves=legal_moves(walk,stm);if(moves.empty()) {walk=start;stm=1;continue;}
        sequence=sequence*1664525u+1013904223u;make_move(walk,stm,moves[sequence%moves.size()]);stm=3-stm;
        check(consistent(walk),"incremental state after random move "+std::to_string(i));
        (void)petir_ueval::eval(walk,stm,-20500,20500);
    }
    // Verify unchanged packed binary books load and can answer an initial move.
    // Audit check evasions against full move generation followed by making
    // each move and checking the king, without relying on the evasion filter.
    int evasion_positions=0,noncheck_positions=0;
    prepare_search(1,false,0);usewinboard=false;
    for(int game=0;game<32;++game) {
        Position audit=start;int audit_side=1;
        for(int ply_index=0;ply_index<160;++ply_index) {
            auto audit_moves=legal_moves(audit,audit_side);
            const bool checked=audit_side==1?petir_tools::white_checked(audit):petir_tools::black_checked(audit);
            if(checked) {
                ++evasion_positions;
                tmovelist candidates{};int count=0,check_count=0;
                std::vector<int> reference,raw_evasions;
                auto collect=[&] {
                    for(int i=1;i<=count;++i) {
                        int destination=(candidates[i].moves>>7)&127;
                        if(destination==_rokadependek || destination==_rokadepanjang)continue;
                        auto child=audit;make_move(child,audit_side,candidates[i].moves);
                        if(!(audit_side==1?petir_tools::white_checked(child):petir_tools::black_checked(child)))reference.push_back(candidates[i].moves);
                    }
                };
                if(audit_side==1)petir_movgen::white_movgen_caps(audit,candidates,count);else petir_movgen::black_movgen_caps(audit,candidates,count);
                collect();
                if(audit_side==1)petir_movgen::white_movgen_noncaps(audit,candidates,count);else petir_movgen::black_movgen_noncaps(audit,candidates,count);
                collect();
                if(audit_side==1)petir_movgen::white_evasion(audit,candidates,count,check_count);else petir_movgen::black_evasion(audit,candidates,count,check_count);
                for(int i=1;i<=count;++i)raw_evasions.push_back(candidates[i].moves);
                std::sort(reference.begin(),reference.end());std::sort(raw_evasions.begin(),raw_evasions.end());
                if(reference!=raw_evasions) {
                    std::cerr<<"Evasion mismatch: "<<petir_tools::getfen(audit,audit_side).value<<'\n';
                    for(int move:reference)std::cerr<<" reference "<<move_text(move,audit,audit_side);
                    for(int move:raw_evasions)std::cerr<<" evasion "<<move_text(move,audit,audit_side);
                    std::cerr<<'\n';return 1;
                }
            } else {
                ++noncheck_positions;
                const auto nodes=total_node;
                const int score=petir_search::q_search(audit_side,320,321,audit,0,2,0,321,petir_search::node_cut);
                if(score!=(audit_moves.empty()?contemp_draw:321) || total_node!=nodes) {
                    std::cerr<<"Stalemate probe mismatch: "<<to_fen(audit,audit_side,1)<<'\n';return 1;
                }
            }
            if(audit_moves.empty())break;
            sequence=sequence*1664525u+1013904223u;
            make_move(audit,audit_side,audit_moves[sequence%audit_moves.size()]);audit_side=3-audit_side;
        }
    }
    check(evasion_positions>0,"checked positions exercised by independent evasion audit");
    check(noncheck_positions>0,"quiescence stalemate probes match independently filtered legal moves");
    std::cout<<evasion_positions<<" checked positions audited for legal evasions\n";
    std::cout<<noncheck_positions<<" nonchecked positions audited for stalemate probes\n";
    if(std::filesystem::exists(pas::native_path(pas::form1.defaultdir+"/white.book"))) {
        m=petir_opening_book::searchbook(start,1);check(m!=0,"original white.book is compatible");
        auto moves=legal_moves(start,1);check(std::find(moves.begin(),moves.end(),m)!=moves.end(),"book move is legal");
    }
    petir_repetition::initrepdata();
    check(petir_repetition::addrephash(start.hashkey,1)==0,"first repetition insertion");
    check(petir_repetition::addrephash(start.hashkey,1)==1,"second repetition insertion");
    petir_repetition::delrephash(start.hashkey,1);
    check(petir_repetition::addrephash(start.hashkey,1)==1,"repetition delete and reinsert");
    // Keep previous results available; new searches age rather than clear them.
    petir_hashing::addtable(start.hashkey,1,48,parse_move("e2e4",start,1),777,pv_node,start.move50count);
    const auto cached_start=[&] {
        int alpha=-20500,beta=20500,score=0,move=0;bool avoid_null=false;
        return petir_hashing::searchhash(start.hashkey,alpha,beta,8,1,score,move,avoid_null,start.move50count);
    };
    check(cached_start(),"previous search result is available before preparing a new search");
    historyw[1][28]=historyb[-1][36]=1048576;
    killer0[0]=killer1[63]=1;
    prepare_search(1,false,0);usewinboard=false;
    check(cached_start(),"new search retains the previous search's hash result");
    const auto hash_slot=start.hashkey&static_cast<std::uint64_t>(maxhash-2); // the bucket's first entry
    check(petir_hashing_header::is_old(petir_hashing_header::hashtable[hash_slot]),"previous search's hash result is aged for replacement");
    const auto aged_record=petir_hashing_header::hashtable[hash_slot];
    const auto colliding_key=start.hashkey^static_cast<std::uint64_t>(maxhash);
    petir_hashing::addtable(colliding_key,1,8,parse_move("d2d4",start,1),321,pv_node,start.move50count);
    check(petir_hashing_header::hashtable[hash_slot].key==aged_record.key && petir_hashing_header::hashtable[hash_slot+1].key==colliding_key && !petir_hashing_header::is_old(petir_hashing_header::hashtable[hash_slot+1]),"a shallow entry leaves the previous search's deeper entry and is stamped current");
    petir_hashing_header::cleartable2();
    petir_hashing::addtable(colliding_key,1,8,parse_move("d2d4",start,1),321,pv_node,start.move50count);
    check(petir_hashing_header::hashtable[hash_slot].key==colliding_key && !petir_hashing_header::is_old(petir_hashing_header::hashtable[hash_slot]),"a shallow entry replaces a deeper one two searches old");
    petir_hashing_header::hashtable[hash_slot]=aged_record;
    const auto flags_before_age=aged_record.flag;
    petir_hashing_header::cleartable2();
    check(petir_hashing_header::hashtable[hash_slot].flag==flags_before_age && cached_start(),"generation aging leaves cached entries and their bytes untouched");
    const auto generation_before_wrap=petir_hashing_header::generation;
    for(int cycle=0;cycle<64;++cycle)petir_hashing_header::cleartable2();
    check(petir_hashing_header::generation==generation_before_wrap && petir_hashing_header::hashtable[hash_slot].flag==flags_before_age && cached_start(),"generation wrap preserves hash contents and score validity");
    check(historyw[1][28]==4096 && historyb[-1][36]==4096 && killer0[0]==0 && killer1[63]==0,"new search decays move-ordering history and resets killers");
    check(petir_repetition::addrephash(start.hashkey,1)==2,"search reset preserves game repetition history");
    petir_repetition::initrepdata();
    petir_hashing_header::cleartable();
    check(!cached_start(),"explicit Clear Hash discards the retained result");
    // A deep exact root entry must guide ordering without replacing depth 1.
    const int root_hint=parse_move("e2e4",start,1);
    petir_hashing::addtable(start.hashkey,1,48,root_hint,777,pv_node,start.move50count);
    std::string first_root_move;
    usewinboard=true;
    pas::output=[&](const pas::String& output) {
        if(!first_root_move.empty())return;
        std::istringstream line(output.value);int d=0,score=0,time=0,nodes=0;
        line>>d>>score>>time>>nodes>>first_root_move;
    };
    (void)petir_uiterative::iterative(1,1,_nodemax,start);
    pas::output={};usewinboard=false;
    check(total_node>0 && petir_uiterative::completed_depth==1 && petir_uiterative::completed_score!=777,"depth 1 searches root moves instead of accepting a cached root score");
    check(first_root_move=="e2e4","root search tries the retained hash move first");
    petir_hashing_header::cleartable();
    (void)petir_uiterative::iterative(5,1,_nodemax,start);
    auto moves=legal_moves(start,1);
    check(std::find(moves.begin(),moves.end(),mmoves)!=moves.end(),"iterative search returns a legal starting move");
    check(consistent(start),"search does not change the caller's board");
    for(int requested=1;requested<=4;++requested) {
        new_game();prepare_search(1,false,0);usewinboard=false;
        (void)petir_uiterative::iterative(requested,1,_nodemax,start);
        check(petir_uiterative::completed_depth==requested,"honor shallow depth limit "+std::to_string(requested));
    }
    new_game();prepare_search(1,false,0);usewinboard=true;
    int previous_best=0;
    pas::output=[&](const pas::String& output) {
        std::istringstream line(output.value);int reported=0;line>>reported;
        if(reported==6 && !previous_best) {previous_best=petir_uiterative::completed_move;stop_process=true;}
    };
    (void)petir_uiterative::iterative(8,1,_nodemax,start);
    pas::output={};usewinboard=false;
    check(previous_best!=0 && mmoves==previous_best && petir_uiterative::completed_depth==5,"interrupted iteration restores the last completed best move");
    // A quiet rook move up the board permits a back-rank mate; search must
    // choose a defense for either color, rather than just a legal move.
    for(const char* fen:{
        "4r1k1/5ppp/8/8/8/8/5PPP/R5K1 w - - 0 1",
        "r5k1/5ppp/8/8/8/8/5PPP/4R1K1 b - - 0 1",
        "7k/1q5p/5r1P/4Np2/R2p1P2/2bP2PK/1p6/1Q6 w - - 4 68"}) {
        new_game();
        auto threatened=from_fen(fen,stm);
        prepare_search(stm,false,0);usewinboard=false;
        (void)petir_uiterative::iterative(6,stm,_nodemax,threatened);
        auto defenses=legal_moves(threatened,stm);
        const bool valid=std::find(defenses.begin(),defenses.end(),mmoves)!=defenses.end();
        check(valid,"mate-threat defense is a legal move");
        bool permits_mate=false;
        if(valid) {
            auto defended=threatened;make_move(defended,stm,mmoves);
            for(int reply:legal_moves(defended,3-stm)) {
                auto after_reply=defended;make_move(after_reply,3-stm,reply);
                const bool checked=stm==1?petir_tools::white_checked(after_reply):petir_tools::black_checked(after_reply);
                if(checked && legal_moves(after_reply,stm).empty())permits_mate=true;
            }
        }
        check(valid && !permits_mate,"search avoids an immediate checkmate threat");
    }
    // PetirCPP round 1 ended with 68.Qa2 Rxh6#. Checkmate must use
    // header's centipawn king value, not usee/movgen's private value 99.
    new_game();
    auto game_mate=from_fen("7k/1q5p/7r/4Np2/R2p1P2/2bP2PK/Qp6/8 w - - 0 69",stm);
    check(petir_tools::white_checked(game_mate) && legal_moves(game_mate,stm).empty(),"PGN final position is checkmate");
    prepare_search(stm,false,0);usewinboard=false;
    check(petir_search::q_search(stm,-20500,20500,game_mate,1,2,0,petir_search::noval,petir_search::node_pv)==-19998,"quiescence checkmate score uses centipawns");
    check(petir_search::mainsearch(16,stm,-20500,20500,game_mate,false,2,0,false,0,petir_search::node_pv)==-19998,"main search checkmate score uses centipawns");
    check(petir_root::searchroot(16,stm,_nodemax,-20500,20500,game_mate,false,0,0)<-19000,"root checkmate score uses centipawns");
    // Learning must update a packed 16-bit weight even in an odd probe slot.
    petir_opening_book::initbook();
    for(int i=0;i<2;++i) {
        petir_opening_book::addhashbook(2,result_white_win,1,2700,2700);
        petir_opening_book::addhashbook(2+petir_opening_book::maxhashbook,result_white_win,1,2700,2700);
    }
    int weight=0;
    check(petir_opening_book::searchhashbook(2+petir_opening_book::maxhashbook,1,weight) && weight==4,"packed book learning and collision probing");
    std::cout<<checks<<" checks, "<<failures<<" failures\n";
    return failures?1:0;
}
}
