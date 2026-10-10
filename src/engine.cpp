#include "engine.hpp"
#include "kpk.hpp"
#include <algorithm>
#include <sstream>

namespace petir {
using namespace petir_header;
void initialize(const std::filesystem::path& books) {
    static_assert(sizeof(void*)==8,"Build Petir for a 64-bit target");
    static_assert(sizeof(petir_opening_book::tbook)==10);
    static_assert(sizeof(petir_hashing_header::thashrecord)==16);
    if constexpr(std::endian::native!=std::endian::little)throw std::runtime_error("Petir's legacy books require little endian");
    timefactor=4;
    nullok=lazyok=futpruningok=qpruningok=true;
    petir_bitboard::init_mask();
    petir_hashing_header::inithashkey();
    petir_hashing_header::inittable();
    petir_evalmask::initevalmask();
    petir_kpk::initialize();
    pas::form1.defaultdir=books.string();
    petir_opening_book::initbook();
    petir_opening_book::loadbook();
    pas::randseed=static_cast<std::uint32_t>(pas::gettickcount()); // Original Randomize after hash initialization.
    new_game();
}
void new_game() {
    petir_udata::initstate(data);
    side=_sisiputih;
    outofbook=resign_count=draw_count=jumlahlangkah=0;
    pas::fillchar(historyw,sizeof(historyw),0);
    pas::fillchar(historyb,sizeof(historyb),0);
    pas::fillchar(conthist,sizeof(conthist),0);
    petir_tools::clearkiller();
    petir_hashing_header::cleartable();
    petir_repetition::initrepdata();
}
Position from_fen(const std::string& fen,int& stm,int* fullmove) {
    std::istringstream input(fen);
    std::string board,turn,rights,ep;
    int half=0,full=1;
    if(!(input>>board>>turn>>rights>>ep))throw std::invalid_argument("FEN needs board, side, castling, and en passant fields");
    if(input>>half) {if(!(input>>full))throw std::invalid_argument("FEN missing fullmove number");}
    if(half<0 || full<1 || (turn!="w" && turn!="b"))throw std::invalid_argument("Invalid FEN counters or side");
    Position p{};int rank=7,file=0,kings[2]={};
    const std::string names="PNBRQK";
    for(char c:board) {
        if(c=='/') {if(file!=8 || rank==0)throw std::invalid_argument("Invalid FEN rank");--rank;file=0;continue;}
        if(c>='1' && c<='8') {file+=c-'0';if(file>8)throw std::invalid_argument("FEN rank too long");continue;}
        auto index=names.find(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
        if(index==std::string::npos || file>=8)throw std::invalid_argument("Invalid FEN piece");
        int piece=static_cast<int>(index)+1;
        if(piece==1 && (rank==0 || rank==7))throw std::invalid_argument("FEN pawn on promotion rank");
        if(piece==6)++kings[std::islower(static_cast<unsigned char>(c))?1:0];
        p.papan[rank*8+file++]=static_cast<std::int8_t>(std::islower(static_cast<unsigned char>(c))?-piece:piece);
    }
    if(rank!=0 || file!=8 || kings[0]!=1 || kings[1]!=1)throw std::invalid_argument("FEN needs eight ranks and one king of each color");
    if(rights!="-")for(char c:rights) {
        switch(c) {case 'K':p.flagrokade|=1;break;case 'Q':p.flagrokade|=2;break;case 'k':p.flagrokade|=4;break;case 'q':p.flagrokade|=8;break;default:throw std::invalid_argument("Invalid FEN castling rights");}
    }
    stm=turn=="w"?1:2;
    petir_customboard::fillbitboard(p);
    if(ep!="-") {
        if(ep.size()!=2 || ep[0]<'a' || ep[0]>'h' || ep[1]!=(stm==1?'6':'3'))throw std::invalid_argument("Invalid FEN en passant square");
        p.ep=static_cast<std::uint8_t>(ep[0]-'a');
    }
    p.move50count=static_cast<std::uint8_t>(std::min(half,255));
    p.hashkey=petir_hashing_header::hashvalue(p,stm);
    if(fullmove)*fullmove=full;
    return p;
}
std::string to_fen(const Position& p,int stm,int fullmove) {
    std::string result;
    for(int rank=7;rank>=0;--rank) {
        int empty=0;
        for(int file=0;file<8;++file) {
            const int piece=p.papan[rank*8+file];
            if(!piece) {++empty;continue;}
            if(empty) {result+=char('0'+empty);empty=0;}
            const char name=" PNBRQK"[std::abs(piece)];
            result+=piece>0?name:static_cast<char>(std::tolower(static_cast<unsigned char>(name)));
        }
        if(empty)result+=char('0'+empty);
        if(rank)result+='/';
    }
    result+=stm==1?" w ":" b ";
    std::string rights;
    for(int bit=0;bit<4;++bit)if(p.flagrokade&(1<<bit))rights+="KQkq"[bit];
    result+=rights.empty()?"-":rights;
    result+=' ';
    if(p.ep==_no_ep)result+='-';
    else {result+=char('a'+p.ep);result+=stm==1?'6':'3';}
    return result+" "+std::to_string(p.move50count)+" "+std::to_string(fullmove);
}
bool insufficient_material(const Position& p) {
    if(p.pawnwhite || p.pawnblack || p.rookwhite || p.rookblack || p.queenwhite || p.queenblack)return false;
    const auto knights=p.knightwhite|p.knightblack,bishops=p.bishopwhite|p.bishopblack;
    if(std::popcount(knights|bishops)<=1)return true;
    if(knights)return false;
    // Any number of bishops on one square color cannot deliver mate.
    constexpr std::uint64_t dark=UINT64_C(0xaa55aa55aa55aa55);
    return !(bishops&dark) || !(bishops&~dark);
}
void make_move(Position& p,int stm,int m) {
    if(stm==1)petir_makemove::makewhitemove(m,p);else petir_makemove::makeblackmove(m,p);
}
std::string move_text(int m,const Position& p,int stm) {
    if(m==0)return "0000";
    return petir_notation::movetonotation(m,p.ep,stm).value;
}
std::vector<int> legal_moves(Position p,int stm) {
    std::vector<int> result;tmovelist ml{};int count=0,checks=0;
    const bool incheck=stm==1?petir_tools::white_checked(p):petir_tools::black_checked(p);
    auto append=[&] {
        for(int i=1;i<=count;++i) {
            Position child=p;make_move(child,stm,ml[i].moves);
            if(!(stm==1?petir_tools::white_checked(child):petir_tools::black_checked(child)))result.push_back(ml[i].moves);
        }
    };
    if(incheck) {if(stm==1)petir_movgen::white_evasion(p,ml,count,checks);else petir_movgen::black_evasion(p,ml,count,checks);append();}
    else {
        if(stm==1)petir_movgen::white_movgen_caps(p,ml,count);else petir_movgen::black_movgen_caps(p,ml,count);append();
        if(stm==1)petir_movgen::white_movgen_noncaps(p,ml,count,0);else petir_movgen::black_movgen_noncaps(p,ml,count);append();
    }
    return result;
}
int parse_move(const std::string& text,Position p,int stm) {
    if((text.size()!=4 && text.size()!=5) || text[0]<'a' || text[0]>'h' || text[2]<'a' || text[2]>'h' || text[1]<'1' || text[1]>'8' || text[3]<'1' || text[3]>'8')return 0;
    for(int move:legal_moves(p,stm)) {
        const std::string candidate=move_text(move,p,stm);
        if(text==candidate)return move;
        // The original maker accepts all four promotions, although its generator
        // only searches queen and knight. Accept opponent bishop/rook promotions.
        if(text.size()==5 && candidate.size()==5 && text.substr(0,4)==candidate.substr(0,4)) {
            int code=text[4]=='q'?1:text[4]=='n'?2:text[4]=='b'?3:text[4]=='r'?4:0;
            if(code)return (move&0x3fff)|(code<<14);
        }
    }
    return 0;
}
std::uint64_t perft(Position p,int stm,int depth) {
    if(depth==0)return 1;
    std::uint64_t nodes=0;tmovelist ml{};int count=0,checks=0;
    bool incheck=stm==1?petir_tools::white_checked(p):petir_tools::black_checked(p);
    auto visit=[&](bool filter) {
        for(int i=1;i<=count;++i) {
            Position child=p;make_move(child,stm,ml[i].moves);
            if(filter && (stm==1?petir_tools::white_checked(child):petir_tools::black_checked(child)))continue;
            nodes+=perft(child,3-stm,depth-1);
        }
    };
    // Same generator selection and legality checks as original/perft_test.pas.
    if(incheck) {if(stm==1)petir_movgen::white_evasion(p,ml,count,checks);else petir_movgen::black_evasion(p,ml,count,checks);visit(false);}
    else {
        if(stm==1)petir_movgen::white_movgen_caps(p,ml,count);else petir_movgen::black_movgen_caps(p,ml,count);visit(true);
        if(stm==1)petir_movgen::white_movgen_noncaps(p,ml,count,0);else petir_movgen::black_movgen_noncaps(p,ml,count);visit(true);
    }
    return nodes;
}
void prepare_search(int stm,bool timed,int ticks) {
    search_time.reset();
    evalms=totaladdhash=collision=lazyts=maxdepth1=evalsee=movgenms=cetc=futfail=futtry=total_node=maxdepth=hash_hit=0;
    stalemate=false;ai_side=stm;timepassed=0;stop_process=false;
    lazy_margin1a=lazy_margin1;lazy_margin2a=lazy_margin2;lazy_margin3a=lazy_margin3;
    testsolved=false;petir_tools::clearkiller();nerror=0;
    // Retain usable results between moves and age their replacement priority.
    if(ponder_type==ponder_off)petir_hashing_header::cleartable2();else petir_hashing_header::clearreversetable();
    co_hash=totalco=hashco=lazyexit=fpruning=efpruning=razor=cofirst=0;
    computer_thinking=true;
    for(int piece=-6;piece<=6;++piece)for(int sq=0;sq<=67;++sq) {historyw[piece][sq]/=256;historyb[piece][sq]/=256;}
    startclock=ref2=pas::gettickcount();qnode=0;usetimer=timed;timelimit=timed?ticks:999999999;
    mmoves=0;
    petir_uiterative::completed_move=petir_uiterative::completed_depth=petir_uiterative::completed_score=0;
    petir_uiterative::completed_pv.clear();
}
int original_time_budget(int stm) {
    const int clock=stm==1?clockwhite:clockblack;
    if(modetime==minutes_game || modetime==minutes_game_increment) {
        int result=jumlahlangkah<=80?clock/(60-jumlahlangkah/2)-2:clock/30;
        return result==0?1:result;
    }
    if(modetime==moves_minutes) {
        const int period=std::max(1,rmoves);
        const int played=(jumlahlangkah/2)%period;
        return clock/(period+2-played);
    }
    if(modetime==seconds_move)return realclock;
    return 999999999;
}
bool consistent(const Position& p) {
    Position r=p;petir_customboard::fillbitboard(r);r.ep=p.ep;
    return p.hashkey==petir_hashing_header::hashvalue(r,1)
        && p.materialscore==r.materialscore && p.nilai_perwira_putih==r.nilai_perwira_putih
        && p.nilai_perwira_hitam==r.nilai_perwira_hitam && p.ngajahputih==r.ngajahputih && p.ngajahhitam==r.ngajahhitam
        && p.whitepieces==r.whitepieces && p.blackpieces==r.blackpieces && p.allpieces==r.allpieces
        && p.allpiecesr90==r.allpiecesr90 && p.allpiecesa8h1==r.allpiecesa8h1 && p.allpiecesh8a1==r.allpiecesh8a1
        && p.pawnwhite==r.pawnwhite && p.pawnblack==r.pawnblack && p.knightwhite==r.knightwhite && p.knightblack==r.knightblack
        && p.bishopwhite==r.bishopwhite && p.bishopblack==r.bishopblack && p.rookwhite==r.rookwhite && p.rookblack==r.rookblack
        && p.queenwhite==r.queenwhite && p.queenblack==r.queenblack && p.kingwhite==r.kingwhite && p.kingblack==r.kingblack
        && p.bishopqueen==r.bishopqueen && p.rookqueen==r.rookqueen;
}
}
