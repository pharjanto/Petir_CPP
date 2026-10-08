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
    int stm=1,fullmove=1,depth=49,engine_side=2;
    bool force=false;
    std::atomic_bool post=true,publish_result=true;
    std::mutex result_mutex;
    std::vector<std::pair<petir::Position,int>> history;
    std::thread worker;
    std::atomic_bool pondering=false,hold_result=false;
    std::atomic<std::int64_t> deadline=0,clock_origin=0;
    int ponder_budget_ms=1000;
    int wb_white_ms=0,wb_black_ms=0;
    int winboard_budget_ms() const {
        if(!usetimer)return 0;
        const int budget=std::max(1,petir::original_time_budget(stm))*250;
        if(modetime==seconds_move)return budget;
        const int remaining=stm==1?wb_white_ms:wb_black_ms;
        return std::min(budget,std::max(1,remaining-std::min(20,remaining/10)));
    }
    void finish(bool return_move=false) {
        {
            std::lock_guard lock(result_mutex);
            if(!return_move)publish_result=false;
            stop_process=true;hold_result=false;pondering=false;
        }
        if(worker.joinable())worker.join();
    }
    void reset_history() {
        history.clear();petir_repetition::initrepdata();
    }
    void rebuild_repetition() {
        petir_repetition::initrepdata();
        for(std::size_t i=1;i<history.size();++i)
            petir_repetition::addrephash(history[i].first.hashkey,history[i].second-1);
        if(!history.empty())petir_repetition::addrephash(position.hashkey,stm-1);
    }
    std::uint64_t repetition_key(petir::Position p,int turn) const {
        // An en passant square changes repetition identity only when capture is legal.
        if(p.ep!=_no_ep) {
            bool capturable=false;
            for(int move:petir::legal_moves(p,turn))
                if(((move>>7)&127)==_en_passant) {capturable=true;break;}
            if(!capturable)p.ep=_no_ep;
        }
        return petir_hashing_header::hashvalue(p,turn);
    }
    bool report_result(const std::vector<int>& moves) {
        std::string result;
        if(moves.empty()) {
            const bool checked=stm==1?petir_tools::white_checked(position):petir_tools::black_checked(position);
            result=checked?(stm==1?"0-1 {Checkmate}":"1-0 {Checkmate}"):"1/2-1/2 {Stalemate}";
        } else if(petir::insufficient_material(position))result="1/2-1/2 {Insufficient material}";
        else if(position.move50count>=100)result="1/2-1/2 {50-move rule}";
        else {
            const auto key=repetition_key(position,stm);
            int occurrences=1;
            for(auto i=history.rbegin();i!=history.rend();++i) {
                if(i->second==stm && repetition_key(i->first,stm)==key && ++occurrences>=3) {
                    result="1/2-1/2 {Threefold repetition}";break;
                }
            }
        }
        if(result.empty())return false;
        force=true;emit(result);return true;
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
        if(mode==Mode::WinBoard && report_result(moves))return;
        petir::prepare_search(root_side,timed,std::max(2,budget_ms/250));
        publish_result=true;
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
            int best=0;bool searched=false;
            try {
                if(!moves.empty()) {
                    if(use_book && !ponder && !infinite)best=petir_opening_book::searchbook(root,root_side);
                    if(!best) {searched=true;(void)petir_uiterative::iterative(std::clamp(requested_depth,1,49),root_side,_nodemax,root);best=mmoves;}
                    if(std::find(moves.begin(),moves.end(),best)==moves.end())best=moves.front();
                }
            } catch(const std::exception& e) {
                emit(mode==Mode::UCI?"info string search stopped: "+std::string(e.what()):"Error (search): "+std::string(e.what()));
                best=petir_uiterative::completed_move;
                if(std::find(moves.begin(),moves.end(),best)==moves.end())best=moves.empty()?0:moves.front();
            }
            while(hold_result.load() && !stop_process.load())std::this_thread::sleep_for(std::chrono::milliseconds(5));
            active=false;ticker.join();computer_thinking=false;
            std::lock_guard lock(result_mutex);
            if(!publish_result.load())return;
            if(searched && post.load() && petir_uiterative::completed_depth>0) {
                // Finish with the completed result, rather than an interrupted PV.
                const auto pv=petir_uiterative::completed_pv;
                const auto d=std::to_string(petir_uiterative::completed_depth);
                const auto score=std::to_string(petir_uiterative::completed_score);
                const auto elapsed=now_ms()-clock_origin.load();
                if(mode==Mode::UCI)emit("info depth "+d+" score cp "+score+" time "+std::to_string(elapsed)+" nodes "+std::to_string(total_node)+" pv "+pv);
                else emit(d+" "+score+" "+std::to_string(elapsed/10)+" "+std::to_string(total_node)+" "+pv);
            }
            if(mode==Mode::UCI) {
                emit("info nodes "+std::to_string(total_node)+" time "+std::to_string(now_ms()-clock_origin.load()));
                emit("bestmove "+petir::move_text(best,root,root_side));
            } else {
                if(best) {
                    const auto text=petir::move_text(best,root,root_side);
                    apply(text);emit("move "+text);
                    report_result(petir::legal_moves(position,stm));
                }
                else emit((root_side==1?petir_tools::white_checked(root):petir_tools::black_checked(root))?(root_side==1?"0-1 {Checkmate}":"1-0 {Checkmate}"):"1/2-1/2 {Stalemate}");
            }
        });
    }
    void uci_position(const std::vector<std::string>& w) {
        finish();std::size_t moves=std::find(w.begin(),w.end(),"moves")-w.begin();
        if(w.size()<2)throw std::invalid_argument("position needs startpos or fen");
        petir::Position next;int next_side=1,next_fullmove=1;
        std::vector<std::pair<petir::Position,int>> next_history;
        if(w[1]=="startpos")petir_udata::initstate(next);
        else if(w[1]=="fen")next=petir::from_fen(join(w,2,moves),next_side,&next_fullmove);
        else throw std::invalid_argument("position needs startpos or fen");
        for(std::size_t i=moves+1;i<w.size();++i) {
            const int move=petir::parse_move(w[i],next,next_side);
            if(!move)throw std::invalid_argument("Illegal move "+w[i]);
            next_history.emplace_back(next,next_side);petir::make_move(next,next_side,move);
            if(next_side==2)++next_fullmove;
            next_side=3-next_side;
        }
        position=next;stm=next_side;fullmove=next_fullmove;history=std::move(next_history);
        jumlahlangkah=(fullmove-1)*2+(stm==2);rebuild_repetition();
    }
    void uci_go(const std::vector<std::string>& w) {
        finish();int d=49,movetime=0,wtime=0,btime=0,moves_to_go=0;bool ponder=false,infinite=false,depth_given=false,clock_given=false,movetime_given=false;
        for(std::size_t i=1;i<w.size();++i) {
            if(w[i]=="ponder")ponder=true;
            else if(w[i]=="infinite")infinite=true;
            else if(i+1<w.size() && (w[i]=="depth" || w[i]=="movetime" || w[i]=="wtime" || w[i]=="btime" || w[i]=="winc" || w[i]=="binc" || w[i]=="movestogo")) {
                int v=std::max(0,std::stoi(w[++i]));const auto& key=w[i-1];
                if(key=="depth") {d=v;depth_given=true;}
                if(key=="movetime") {movetime=v;movetime_given=true;}
                if(key=="wtime") {wtime=v;clock_given=true;}
                if(key=="btime") {btime=v;clock_given=true;}
                if(key=="movestogo")moves_to_go=v;
                // Original Petir does not add increments to its per-move budget.
            }
        }
        bool timed=movetime_given || clock_given;
        clockwhite=wtime/250;clockblack=btime/250;modetime=minutes_game;
        int budget=movetime_given?std::max(1,movetime):std::max(1,petir::original_time_budget(stm))*250;
        const int remaining=stm==1?wtime:btime;
        if(moves_to_go>0 && !movetime_given)budget=std::max(1,remaining/moves_to_go);
        if(clock_given)budget=std::min(budget,std::max(1,remaining-std::min(20,remaining/10)));
        if(!timed && !depth_given)infinite=true;
        begin_search(d,timed,std::max(1,budget),ponder,infinite);
    }
    void option(const std::vector<std::string>& w) {
        finish();auto v=std::find(w.begin(),w.end(),"value");std::size_t n=v-w.begin();
        const auto name=join(w,2,n);auto value=n+1<w.size()?join(w,n+1,w.size()):"";
        if(name=="Hash") {
            int mb=std::clamp(std::stoi(value),1,1024),entries=1;
            while(std::int64_t(entries)*2*static_cast<std::int64_t>(sizeof(petir_hashing_header::thashrecord))<=std::int64_t(mb)*1048576)entries*=2;
            // vector::resize preserves the old table if allocation fails.
            petir_hashing_header::hashtable.resize(entries);
            maxhash=entries;petir_hashing_header::cleartable();
        } else if(name=="OwnBook")use_book=value=="true";
        else if(name=="Clear Hash")petir_hashing_header::cleartable();
        else if(name=="Contempt")contemp_draw=std::clamp(std::stoi(value),-1000,1000);
        else if(name=="MTDf")use_mtdf=value=="true";
    }
    void winboard_command(const std::string& line,const std::vector<std::string>& w) {
        const auto& c=w[0];
        if(c=="protover") {emit("feature myname=\"Petir 1 C++ faithful port\" setboard=1 usermove=1 san=0 ping=1 colors=0 time=1 sigint=0 sigterm=0 reuse=1 analyze=0 done=1");return;}
        if(c=="ping" && w.size()>1) {emit("pong "+w[1]);return;}
        if(c=="?") {finish(true);return;}
        if(c=="time" || c=="otim") {
            if(w.size()>1) {
                const int ms=static_cast<int>(std::clamp<std::int64_t>(std::stoll(w[1]),0,INT32_MAX/10))*10;
                if((engine_side==1)==(c=="time")) {clockwhite=ms/250;wb_white_ms=ms;}
                else {clockblack=ms/250;wb_black_ms=ms;}
                if(c=="time" && usetimer && modetime!=seconds_move) {
                    const auto cap=now_ms()+std::max(1,ms-std::min(20,ms/10));
                    auto end=deadline.load();
                    while(end>cap && !deadline.compare_exchange_weak(end,cap)) {}
                }
            }
            return;
        }
        if(c=="post") {post=true;return;}
        if(c=="nopost") {post=false;return;}
        if(c=="hard" || c=="easy")return;
        const bool move_command=c=="usermove" || (c.size()>=4 && c[0]>='a' && c[0]<='h' && c[1]>='1' && c[1]<='8');
        if(c!="new" && c!="force" && c!="go" && c!="setboard" && c!="sd" && c!="st" && c!="level" && c!="result" && c!="undo" && c!="remove" && !move_command)return;
        finish();
        if(c=="new") {
            petir::new_game();position=data;stm=1;fullmove=1;engine_side=2;force=false;reset_history();
            if(usetimer) {clockwhite=clockblack=realclock;wb_white_ms=wb_black_ms=realclock*250;}
        }
        else if(c=="force")force=true;
        else if(c=="go") {force=false;engine_side=stm;begin_search(depth,usetimer,winboard_budget_ms());}
        else if(c=="setboard") {
            int next_side=1,next_fullmove=1;
            auto next=petir::from_fen(join(w,1,w.size()),next_side,&next_fullmove);
            position=next;stm=next_side;fullmove=next_fullmove;
            jumlahlangkah=(fullmove-1)*2+(stm==2);reset_history();
        }
        else if(c=="sd" && w.size()>1) {depth=std::clamp(std::stoi(w[1]),1,49);usetimer=false;modetime=fixed_depth;}
        else if(c=="st" && w.size()>1) {depth=49;realclock=std::max(1,static_cast<int>(std::stod(w[1])*4));usetimer=true;modetime=seconds_move;}
        else if(c=="level" && w.size()>3) {
            depth=49;
            rmoves=std::stoi(w[1]);auto colon=w[2].find(':');int secs=std::stoi(w[2])*60;if(colon!=std::string::npos)secs+=std::stoi(w[2].substr(colon+1));
            realclock=secs*4;clockwhite=clockblack=realclock;wb_white_ms=wb_black_ms=secs*1000;
            rincrement=static_cast<int>(std::stod(w[3])*4);usetimer=true;modetime=rmoves?moves_minutes:minutes_game_increment;
        }
        else if(c=="result")force=true;
        else if(c=="undo" || c=="remove") {
            int n=c=="remove"?2:1;
            while(n-- && !history.empty()) {position=history.back().first;stm=history.back().second;history.pop_back();--jumlahlangkah;}
            fullmove=jumlahlangkah/2+1;rebuild_repetition();
        } else if(move_command) {
            std::string m=c=="usermove"?(w.size()>1?w[1]:""):c;
            if(!apply(m))emit("Illegal move: "+m);
            else if(!force && stm==engine_side)begin_search(depth,usetimer,winboard_budget_ms());
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
                else if(c=="stop")finish(true);
                else if(c=="ponderhit") {pondering=false;hold_result=false;clock_origin=now_ms();deadline=clock_origin+ponder_budget_ms;}
                else if(c=="d") {finish();emit(petir::to_fen(position,stm,fullmove));}
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
