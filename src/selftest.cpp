#include "engine.hpp"
#include <algorithm>
#include <sstream>
namespace petir {
int self_test() {
    int failures=0,checks=0;
    auto check=[&](bool ok,const std::string& message) {++checks;if(!ok) {++failures;std::cerr<<"FAIL: "<<message<<'\n';}};
    using namespace petir_header;
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
