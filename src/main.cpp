#include "engine.hpp"
#include <algorithm>
#include <mutex>
#include <sstream>
#include <thread>

namespace {
using namespace petir_header;
std::mutex output_mutex;
void emit(const std::string& s) {std::lock_guard lock(output_mutex);std::cout<<s<<std::endl;}
std::int64_t now_ms() {return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();}
std::vector<std::string> words(const std::string& line) {std::istringstream s(line);std::vector<std::string> v;for(std::string x;s>>x;)v.push_back(x);return v;}
std::string join(const std::vector<std::string>& v,std::size_t a,std::size_t b) {std::string s;for(;a<b;++a) {if(!s.empty())s+=' ';s+=v[a];}return s;}
class Console {
    enum class Mode { UCI, WinBoard } mode=Mode::UCI;
    petir::Position position=data;
    int stm=1,fullmove=1,depth=10,engine_side=2;
    bool force=false,post=true;
    std::vector<std::pair<petir::Position,int>> history;
    std::thread worker;
    std::atomic_bool pondering=false,hold_result=false;
    std::atomic<std::int64_t> deadline=0,clock_origin=0;
    int ponder_budget_ms=1000;
    void finish(bool stop=true) {
        if(stop) {stop_process=true;hold_result=false;pondering=false;}
        if(worker.joinable())worker.join();
    }
    void reset_history() {
        history.clear();petir_repetition::initrepdata();
    }
    bool apply(const std::string& text) {
        int move=petir::parse_move(text,position,stm);
        if(!move)return false;
        history.emplace_back(position,stm);petir::make_move(position,stm,move);
        if(stm==2)++fullmove;
        stm=3-stm;++jumlahlangkah;
        petir_repetition::addrephash(position.hashkey,stm-1);return true;
    }
    void begin_search(int requested_depth,bool timed,int budget_ms,bool ponder=false,bool infinite=false) {
        finish();auto root=position;int root_side=stm;
        auto moves=petir::legal_moves(root,root_side);
        petir::prepare_search(root_side,timed,std::max(2,budget_ms/250));
        if(ponder || infinite) {usetimer=true;timelimit=999999999;}
        pondering=ponder;hold_result=ponder || infinite;ponder_budget_ms=budget_ms;
        clock_origin=now_ms();deadline=timed && !ponder && !infinite?clock_origin+budget_ms:0;
        usewinboard=true;
        pas::output=[this](const pas::String& text) {
            if(!post)return;
            if(mode==Mode::WinBoard) {emit(text.value);return;}
            std::istringstream s(text.value);int d=0,score=0,cs=0,nodes=0;std::string pv;
            if(s>>d>>score>>cs>>nodes) {std::getline(s,pv);emit("info depth "+std::to_string(d)+" score cp "+std::to_string(score)+" time "+std::to_string(cs*10)+" nodes "+std::to_string(nodes)+" pv"+pv);}
        };
        worker=std::thread([this,root,root_side,moves,requested_depth,ponder,infinite]() mutable {
            std::atomic_bool active=true;
            std::thread ticker([&] {
                while(active.load()) {
                    const auto current=now_ms();
                    timepassed=static_cast<int>(pondering.load()?0:(current-clock_origin.load())/250);
                    auto end=deadline.load();
                    if(end && current>=end) {stop_process=true;hold_result=false;}
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                }
            });
            int best=0;
            try {
                if(!moves.empty()) {
                    if(use_book && !ponder && !infinite)best=petir_opening_book::searchbook(root,root_side);
                    if(!best) {(void)petir_uiterative::iterative(std::clamp(requested_depth,1,49),root_side,_nodemax,root);best=mmoves;}
                    if(std::find(moves.begin(),moves.end(),best)==moves.end())best=moves.front();
                }
            } catch(const std::exception& e) {
                emit(mode==Mode::UCI?"info string search stopped: "+std::string(e.what()):"Error (search): "+std::string(e.what()));
                best=moves.empty()?0:moves.front();
            }
            while(hold_result.load() && !stop_process.load())std::this_thread::sleep_for(std::chrono::milliseconds(5));
            active=false;ticker.join();computer_thinking=false;
            if(mode==Mode::UCI) {
                emit("info nodes "+std::to_string(total_node)+" time "+std::to_string(now_ms()-clock_origin.load()));
                emit("bestmove "+petir::move_text(best,root,root_side));
            } else {
                if(best) {emit("move "+petir::move_text(best,root,root_side));apply(petir::move_text(best,root,root_side));}
                else emit((root_side==1?petir_tools::white_checked(root):petir_tools::black_checked(root))?(root_side==1?"0-1 {Checkmate}":"1-0 {Checkmate}"):"1/2-1/2 {Stalemate}");
            }
        });
    }
    void uci_position(const std::vector<std::string>& w) {
        finish();std::size_t moves=std::find(w.begin(),w.end(),"moves")-w.begin();
        if(w.size()<2)throw std::invalid_argument("position needs startpos or fen");
        if(w[1]=="startpos") {petir_udata::initstate(position);stm=1;fullmove=1;}
        else if(w[1]=="fen")position=petir::from_fen(join(w,2,moves),stm,&fullmove);
        else throw std::invalid_argument("position needs startpos or fen");
        jumlahlangkah=(fullmove-1)*2+(stm==2);reset_history();
        for(std::size_t i=moves+1;i<w.size();++i)if(!apply(w[i]))throw std::invalid_argument("Illegal move "+w[i]);
    }
    void uci_go(const std::vector<std::string>& w) {
        finish();int d=49,movetime=0,wtime=0,btime=0,moves_to_go=0;bool ponder=false,infinite=false,depth_given=false;
        for(std::size_t i=1;i<w.size();++i) {
            if(w[i]=="ponder")ponder=true;
            else if(w[i]=="infinite")infinite=true;
            else if(i+1<w.size() && (w[i]=="depth" || w[i]=="movetime" || w[i]=="wtime" || w[i]=="btime" || w[i]=="winc" || w[i]=="binc" || w[i]=="movestogo")) {
                int v=std::max(0,std::stoi(w[++i]));const auto& key=w[i-1];
                if(key=="depth") {d=v;depth_given=true;}
                if(key=="movetime")movetime=v;
                if(key=="wtime")wtime=v;
                if(key=="btime")btime=v;
                if(key=="movestogo")moves_to_go=v;
                // Original Petir does not add increments to its per-move budget.
            }
        }
        bool timed=movetime>0 || wtime>0 || btime>0;
        clockwhite=wtime/250;clockblack=btime/250;modetime=minutes_game;
        int budget=movetime>0?movetime:std::max(1,petir::original_time_budget(stm))*250;
        if(moves_to_go>0 && movetime==0)budget=std::max(1,(stm==1?wtime:btime)/moves_to_go);
        if(!timed && !depth_given)infinite=true;
        begin_search(d,timed,std::max(1,budget),ponder,infinite);
    }
    void option(const std::vector<std::string>& w) {
        finish();auto v=std::find(w.begin(),w.end(),"value");std::size_t n=v-w.begin();
        const auto name=join(w,2,n);auto value=n+1<w.size()?join(w,n+1,w.size()):"";
        if(name=="Hash") {
            int mb=std::clamp(std::stoi(value),1,1024),entries=1;
            while(std::int64_t(entries)*2*sizeof(petir_hashing_header::thashrecord)<=std::int64_t(mb)*1048576)entries*=2;
            maxhash=entries;petir_hashing_header::inittable();petir_hashing_header::cleartable();
        } else if(name=="OwnBook")use_book=value=="true";
        else if(name=="Clear Hash")petir_hashing_header::cleartable();
        else if(name=="Contempt")contemp_draw=std::clamp(std::stoi(value),-1000,1000);
        else if(name=="MTDf")use_mtdf=value=="true";
    }
    void winboard_command(const std::string& line,const std::vector<std::string>& w) {
        const auto& c=w[0];
        if(c=="protover") {emit("feature myname=\"Petir 1 C++ faithful port\" setboard=1 usermove=1 san=0 ping=1 colors=0 time=1 sigint=0 sigterm=0 reuse=1 analyze=0 done=1");return;}
        if(c=="ping" && w.size()>1) {emit("pong "+w[1]);return;}
        if(c=="?") {finish();return;}
        finish();
        if(c=="new") {petir::new_game();position=data;stm=1;fullmove=1;engine_side=2;force=false;reset_history();}
        else if(c=="force")force=true;
        else if(c=="go") {force=false;engine_side=stm;begin_search(depth,usetimer,std::max(1,petir::original_time_budget(stm))*250);}
        else if(c=="setboard") {position=petir::from_fen(line.substr(9),stm,&fullmove);jumlahlangkah=(fullmove-1)*2+(stm==2);reset_history();}
        else if(c=="sd" && w.size()>1) {depth=std::clamp(std::stoi(w[1]),1,49);usetimer=false;modetime=fixed_depth;}
        else if(c=="st" && w.size()>1) {realclock=std::max(1,static_cast<int>(std::stod(w[1])*4));usetimer=true;modetime=seconds_move;}
        else if(c=="time" && w.size()>1) {if(engine_side==1)clockwhite=std::stoi(w[1])/25;else clockblack=std::stoi(w[1])/25;}
        else if(c=="otim" && w.size()>1) {if(engine_side==1)clockblack=std::stoi(w[1])/25;else clockwhite=std::stoi(w[1])/25;}
        else if(c=="level" && w.size()>3) {
            rmoves=std::stoi(w[1]);auto colon=w[2].find(':');int secs=std::stoi(w[2])*60;if(colon!=std::string::npos)secs+=std::stoi(w[2].substr(colon+1));
            clockwhite=clockblack=secs*4;rincrement=static_cast<int>(std::stod(w[3])*4);usetimer=true;modetime=rmoves?moves_minutes:minutes_game_increment;
        }
        else if(c=="post")post=true;
        else if(c=="nopost")post=false;
        else if(c=="hard" || c=="easy") { /* Accepted; use UCI go ponder for background search. */ }
        else if(c=="result")force=true;
        else if(c=="undo" || c=="remove") {
            int n=c=="remove"?2:1;
            while(n-- && !history.empty()) {position=history.back().first;stm=history.back().second;history.pop_back();--jumlahlangkah;}
            fullmove=jumlahlangkah/2+1;petir_repetition::initrepdata();
            for(std::size_t i=1;i<history.size();++i)petir_repetition::addrephash(history[i].first.hashkey,history[i].second-1);
            if(!history.empty())petir_repetition::addrephash(position.hashkey,stm-1);
        } else if(c=="usermove" || (c.size()>=4 && c[0]>='a' && c[0]<='h' && c[1]>='1' && c[1]<='8')) {
            std::string m=c=="usermove"?(w.size()>1?w[1]:""):c;
            if(!apply(m))emit("Illegal move: "+m);
            else if(!force && stm==engine_side)begin_search(depth,usetimer,std::max(1,petir::original_time_budget(stm))*250);
        }
    }
public:
    ~Console() {finish();pas::output={};}
    void run() {
        reset_history();std::string line;
        while(std::getline(std::cin,line)) {
            auto w=words(line);if(w.empty())continue;
            try {
                const auto& c=w[0];
                if(c=="quit") {finish();break;}
                if(c=="xboard") {finish();mode=Mode::WinBoard;continue;}
                if(c=="uci") {finish();mode=Mode::UCI;emit("id name Petir 1 C++ faithful port");emit("id author Original Petir author; C++ port");emit("option name Hash type spin default 64 min 1 max 1024");emit("option name OwnBook type check default true");emit("option name Contempt type spin default 0 min -1000 max 1000");emit("option name MTDf type check default false");emit("option name Clear Hash type button");emit("uciok");}
                else if(mode==Mode::WinBoard)winboard_command(line,w);
                else if(c=="isready")emit("readyok");
                else if(c=="setoption")option(w);
                else if(c=="ucinewgame") {finish();petir::new_game();position=data;stm=1;fullmove=1;reset_history();}
                else if(c=="position")uci_position(w);
                else if(c=="go")uci_go(w);
                else if(c=="stop")finish();
                else if(c=="ponderhit") {pondering=false;hold_result=false;clock_origin=now_ms();deadline=clock_origin+ponder_budget_ms;}
                else if(c=="d") {finish();emit(petir_tools::getfen(position,stm).value);}
            } catch(const std::exception& e) {emit(mode==Mode::UCI?"info string "+std::string(e.what()):"Error: "+std::string(e.what()));}
        }
        finish();
    }
};
}
int main(int argc,char** argv) {
    try {
        std::filesystem::path executable=std::filesystem::absolute(argv[0]).parent_path();
        std::filesystem::path books=executable/"books";
        if(!std::filesystem::exists(books))books=executable.parent_path()/"books";
        if(!std::filesystem::exists(books))books="books";
        for(int i=1;i+1<argc;++i)if(std::string(argv[i])=="--book-dir")books=argv[i+1];
        petir::initialize(books);
        if(argc>1 && std::string(argv[1])=="--selftest")return petir::self_test();
        if(argc>1 && std::string(argv[1])=="--perft") {
            int d=argc>2?std::stoi(argv[2]):4,stm=1;
            if(d<0 || d>12)throw std::invalid_argument("perft depth must be 0..12");
            auto p=petir::from_fen(argc>3?argv[3]:petir::start_fen,stm);
            const auto start=now_ms();auto nodes=petir::perft(p,stm,d);
            std::cout<<nodes<<" nodes, "<<now_ms()-start<<" ms\n";return 0;
        }
        if(argc>1 && std::string(argv[1])=="--eval") {
            int stm=1;auto p=petir::from_fen(argc>2?argv[2]:petir::start_fen,stm);
            std::cout<<petir_ueval::eval(p,stm,-20500,20500)<<" cp; hash "<<p.hashkey<<"; book move "<<petir::move_text(petir_opening_book::searchbook(p,stm),p,stm)<<'\n';return 0;
        }
        if(argc>2 && std::string(argv[1])=="--learn-pgn") {petir_opening_book::learn_pgn(argv[2]);return 0;}
        if(argc>1 && std::string(argv[1])=="--help") {std::cout<<"Petir C++20 64-bit engine. Default: UCI; send xboard for WinBoard.\n--selftest | --perft DEPTH [FEN] | --eval [FEN] | --learn-pgn FILE\n--book-dir DIRECTORY selects the legacy white.book and black.book files.\n";return 0;}
        Console{}.run();
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
