#include "engine.hpp"
#include <algorithm>
#include <sstream>
namespace petir {
int self_test() {
    int failures=0,checks=0;
    auto check=[&](bool ok,const std::string& message) {++checks;if(!ok) {++failures;std::cerr<<"FAIL: "<<message<<'\n';}};
    using namespace petir_header;
    // Compare the native scan with the legacy 16-bit lookup for every word
    // value in each block, including boards containing higher occupied blocks.
    std::uint64_t scan_bits=0;
    check(petir_bitboard_mask::firstbitp(pas::address(scan_bits))==0,"empty bit scan retains legacy zero result");
    bool scan_matches=true;
    for(int block=0;block<4;++block) {
        for(std::uint32_t word=1;word<=65535;++word) {
            scan_bits=std::uint64_t(word)<<(block*16);
            if(block<3)scan_bits|=UINT64_C(0xffff000000000000);
            if(petir_bitboard_mask::firstbitp(pas::address(scan_bits))!=block*16+petir_bitboard_mask::firstbitmask[word])scan_matches=false;
        }
    }
    check(scan_matches,"native bit scan matches legacy lookup across all 16-bit blocks");
    Position start{};petir_udata::initstate(start);
    check(start.hashkey==3423812783342919194ULL,"Delphi starting hash and Random evaluation order");
    check(consistent(start),"starting board, rotated occupancy, material, and hash agree");
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
    auto ep=from_fen("7k/8/8/3pP3/8/8/8/K7 w - d6 0 1",stm);
    int m=parse_move("e5d6",ep,stm);check(m!=0,"en passant parsed");make_move(ep,stm,m);
    check(ep.papan[35]==0 && ep.papan[43]==1 && consistent(ep),"en passant updates occupancy, material, and hash");
    auto castling=from_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",stm);
    m=parse_move("e1g1",castling,stm);check(m!=0,"short castling parsed");make_move(castling,stm,m);
    check(castling.papan[6]==6 && castling.papan[5]==4 && consistent(castling),"castling updates rook, king, and hash");
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
    int evasion_positions=0;
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
            }
            if(audit_moves.empty())break;
            sequence=sequence*1664525u+1013904223u;
            make_move(audit,audit_side,audit_moves[sequence%audit_moves.size()]);audit_side=3-audit_side;
        }
    }
    check(evasion_positions>0,"checked positions exercised by independent evasion audit");
    std::cout<<evasion_positions<<" checked positions audited for legal evasions\n";
    if(std::filesystem::exists(pas::native_path(pas::form1.defaultdir+"/white.book"))) {
        m=petir_opening_book::searchbook(start,1);check(m!=0,"original white.book is compatible");
        auto moves=legal_moves(start,1);check(std::find(moves.begin(),moves.end(),m)!=moves.end(),"book move is legal");
    }
    petir_repetition::initrepdata();
    check(petir_repetition::addrephash(start.hashkey,1)==0,"first repetition insertion");
    check(petir_repetition::addrephash(start.hashkey,1)==1,"second repetition insertion");
    petir_repetition::delrephash(start.hashkey,1);
    check(petir_repetition::addrephash(start.hashkey,1)==1,"repetition delete and reinsert");
    prepare_search(1,false,0);usewinboard=false;
    (void)petir_uiterative::iterative(5,1,_nodemax,start);
    auto moves=legal_moves(start,1);
    check(std::find(moves.begin(),moves.end(),mmoves)!=moves.end(),"iterative search returns a legal starting move");
    check(consistent(start),"search does not change the caller's board");
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
