#include "engine.hpp"
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
    Position start{};petir_udata::initstate(start);
    check(start.hashkey==3423812783342919194ULL,"Delphi starting hash and Random evaluation order");
    check(consistent(start),"starting board, rotated occupancy, material, and hash agree");
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
            audit_generator([&](tmovelist& list,int& count) {if(color==1)petir_movgen::white_movgen_noncaps(generated_position,list,count,0);else petir_movgen::black_movgen_noncaps(generated_position,list,count);});
            audit_generator([&](tmovelist& list,int& count) {if(color==1)petir_movgen::white_movgen_caps2(generated_position,list,count);else petir_movgen::black_movgen_caps2(generated_position,list,count);});
            audit_generator([&](tmovelist& list,int& count) {if(color==1)petir_movgen::q_white_movgen(generated_position,list,count);else petir_movgen::q_black_movgen(generated_position,list,count);});
            if(color==1?petir_tools::white_checked(generated_position):petir_tools::black_checked(generated_position))
                audit_generator([&](tmovelist& list,int& count) {int checks=0;if(color==1)petir_movgen::white_evasion(generated_position,list,count,checks);else petir_movgen::black_evasion(generated_position,list,count,checks);});
        }
    }
    check(generation_defines_entries,"generated move/score entries do not depend on initial buffer contents");
    auto bare=from_fen("7k/8/8/8/8/8/8/K7 w - - 0 1",stm);
    check(petir_ueval::eval(bare,stm,-20500,20500)==0,"bare kings evaluate as draw");
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
        if(stm==1)petir_movgen::white_movgen_noncaps(evasion,candidates,count,0);else petir_movgen::black_movgen_noncaps(evasion,candidates,count);append_legal();
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
            check(promotion.ngajahputih==0,"preserve original missing bishop-count increment on promotion");
            ++promotion.ngajahputih; // Validate all other fields, without changing the maker.
        }
        check(consistent(promotion),"promotion material and bitboards "+std::string(1,piece));
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
                if(audit_side==1)petir_movgen::white_movgen_noncaps(audit,candidates,count,0);else petir_movgen::black_movgen_noncaps(audit,candidates,count);
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
    const auto hash_slot=start.hashkey&static_cast<std::uint64_t>(maxhash-1);
    check(petir_hashing_header::is_old(petir_hashing_header::hashtable[hash_slot]),"previous search's hash result is aged for replacement");
    const auto aged_record=petir_hashing_header::hashtable[hash_slot];
    const auto colliding_key=start.hashkey^static_cast<std::uint64_t>(maxhash);
    petir_hashing::addtable(colliding_key,1,8,parse_move("d2d4",start,1),321,pv_node,start.move50count);
    check(petir_hashing_header::hashtable[hash_slot].key==colliding_key && !petir_hashing_header::is_old(petir_hashing_header::hashtable[hash_slot]),"a shallow entry replaces an older generation and is stamped current");
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
