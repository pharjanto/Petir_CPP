#include "engine.hpp"
#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <sstream>
#include <thread>

namespace {
using namespace petir_header;
std::mutex output_mutex;
void emit(const std::string& s) {std::lock_guard lock(output_mutex);std::cout<<s<<std::endl;}
std::int64_t now_ms() {return petir::TimeManager::now_ms();}
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
    std::mutex hold_mutex;
    std::condition_variable hold_cv;
    petir::TimeBudget ponder_budget;
    int move_overhead_ms=50;
    std::int64_t wb_white_ms=0,wb_black_ms=0,wb_base_ms=0,wb_increment_ms=0,wb_move_ms=0;
    petir::ClockLimits winboard_limits() const {
        petir::ClockLimits limits;
        limits.timed=usetimer;limits.overhead_ms=move_overhead_ms;
        if(modetime==seconds_move)limits.movetime_ms=wb_move_ms;
        else {
            limits.remaining_ms=stm==1?wb_white_ms:wb_black_ms;
            limits.increment_ms=wb_increment_ms;
            if(rmoves>0)limits.moves_to_go=rmoves-(jumlahlangkah/2)%rmoves;
        }
        return limits;
    }
    void finish(bool return_move=false) {
        {
            std::lock_guard lock(result_mutex);
            if(!return_move)publish_result=false;
            std::lock_guard hold_lock(hold_mutex);
            stop_process=true;hold_result=false;pondering=false;
        }
        hold_cv.notify_all();
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
    int fallback_move(const petir::Position& root,int side,const std::vector<int>& moves) const {
        if(moves.empty())return 0;
        int alpha=-20500,beta=20500,score=0,hint=0;bool avoid=false;
        (void)petir_hashing::searchhash(root.hashkey,alpha,beta,512,side,score,hint,avoid,root.move50count);
        return std::find(moves.begin(),moves.end(),hint)!=moves.end()?hint:moves.front();
    }
    void publish_move(petir::Position root,int root_side,int best,bool searched) {
        computer_thinking=false;
        std::lock_guard lock(result_mutex);
        if(!publish_result.load())return;
        if(searched && post.load() && petir_uiterative::completed_depth>0) {
            const auto pv=petir_uiterative::completed_pv;
            const auto d=std::to_string(petir_uiterative::completed_depth);
            const auto score=std::to_string(petir_uiterative::completed_score);
            const auto elapsed=petir::search_time.elapsed_ms();
            if(mode==Mode::UCI)emit("info depth "+d+" score cp "+score+" time "+std::to_string(elapsed)+" nodes "+std::to_string(total_node)+" pv "+pv);
            else emit(d+" "+score+" "+std::to_string(elapsed/10)+" "+std::to_string(total_node)+" "+pv);
        }
        if(mode==Mode::UCI) {
            if(searched)emit("info nodes "+std::to_string(total_node)+" time "+std::to_string(petir::search_time.elapsed_ms()));
            emit("bestmove "+petir::move_text(best,root,root_side));
        } else if(best) {
            const auto text=petir::move_text(best,root,root_side);
            // Publish before board/history maintenance; finish() serializes the next command.
            emit("move "+text);
            history.emplace_back(position,stm);petir::make_move(position,stm,best);
            if(stm==2)++fullmove;
            stm=3-stm;++jumlahlangkah;
            petir_repetition::addrephash(position.hashkey,stm-1);
            report_result(petir::legal_moves(position,stm));
        } else emit((root_side==1?petir_tools::white_checked(root):petir_tools::black_checked(root))?(root_side==1?"0-1 {Checkmate}":"1-0 {Checkmate}"):"1/2-1/2 {Stalemate}");
    }
    void begin_search(int requested_depth,const petir::ClockLimits& limits,std::int64_t received_at,bool ponder=false,bool infinite=false) {
        finish();auto root=position;int root_side=stm;
        const auto budget=petir::TimeManager::allocate(limits);
        petir::search_time.reset();
        petir::search_time.start(ponder || infinite?petir::TimeBudget{}:budget,received_at);
        auto moves=petir::legal_moves(root,root_side);
        if(mode==Mode::WinBoard && report_result(moves))return;
        publish_result=true;
        const int fallback=fallback_move(root,root_side,moves);
        if(!ponder && !infinite && (budget.emergency || petir::search_time.expired(true))) {
            // No hash maintenance, book lookup, worker, timer, or optional PV output.
            publish_move(root,root_side,fallback,false);return;
        }
        petir::prepare_search(root_side,limits.timed,INT32_MAX);
        petir::search_time.start(ponder || infinite?petir::TimeBudget{}:budget,received_at);
        pondering=ponder;hold_result=ponder || infinite;ponder_budget=budget;
        usewinboard=true;
        pas::output=[this](const pas::String& text) {
            if(!post || petir::search_should_stop(true))return;
            if(mode==Mode::WinBoard) {emit(text.value);return;}
            std::istringstream s(text.value);int d=0,score=0,cs=0,nodes=0;std::string pv;
            if(s>>d>>score>>cs>>nodes) {std::getline(s,pv);emit("info depth "+std::to_string(d)+" score cp "+std::to_string(score)+" time "+std::to_string(cs*10)+" nodes "+std::to_string(nodes)+" pv"+pv);}
        };
        worker=std::thread([this,root,root_side,moves,fallback,requested_depth,ponder,infinite]() mutable {
            int best=fallback;bool searched=false;
            try {
                if(!moves.empty() && !petir::search_should_stop(true)) {
                    int book=0;
                    if(use_book && !ponder && !infinite)book=petir_opening_book::searchbook(root,root_side);
                    if(book)best=book;
                    else if(!petir::search_should_stop(true)) {searched=true;(void)petir_uiterative::iterative(std::clamp(requested_depth,1,49),root_side,_nodemax,root);best=mmoves;}
                    if(std::find(moves.begin(),moves.end(),best)==moves.end())best=fallback;
                }
            } catch(const std::exception& e) {
                emit(mode==Mode::UCI?"info string search stopped: "+std::string(e.what()):"Error (search): "+std::string(e.what()));
                best=petir_uiterative::completed_move;
                if(std::find(moves.begin(),moves.end(),best)==moves.end())best=fallback;
            }
            {
                std::unique_lock hold_lock(hold_mutex);
                hold_cv.wait(hold_lock,[this]{return !hold_result.load() || stop_process.load();});
            }
            publish_move(root,root_side,best,searched);
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
    void uci_go(const std::vector<std::string>& w,std::int64_t received_at) {
        int d=49,moves_to_go=0;std::int64_t movetime=0,wtime=0,btime=0,winc=0,binc=0;
        bool ponder=false,infinite=false,depth_given=false,clock_given=false,movetime_given=false;
        for(std::size_t i=1;i<w.size();++i) {
            if(w[i]=="ponder")ponder=true;
            else if(w[i]=="infinite")infinite=true;
            else if(i+1<w.size() && (w[i]=="depth" || w[i]=="movetime" || w[i]=="wtime" || w[i]=="btime" || w[i]=="winc" || w[i]=="binc" || w[i]=="movestogo")) {
                auto v=std::clamp<std::int64_t>(std::stoll(w[++i]),0,INT32_MAX);const auto& key=w[i-1];
                if(key=="depth") {d=static_cast<int>(std::clamp<std::int64_t>(v,1,49));depth_given=true;}
                if(key=="movetime") {movetime=v;movetime_given=true;}
                if(key=="wtime") {wtime=v;clock_given=true;}
                if(key=="btime") {btime=v;clock_given=true;}
                if(key=="winc")winc=v;
                if(key=="binc")binc=v;
                if(key=="movestogo")moves_to_go=static_cast<int>(v);
            }
        }
        petir::ClockLimits limits;
        limits.timed=movetime_given || clock_given;
        limits.remaining_ms=stm==1?wtime:btime;limits.increment_ms=stm==1?winc:binc;
        limits.moves_to_go=moves_to_go;limits.overhead_ms=move_overhead_ms;
        if(movetime_given)limits.movetime_ms=clock_given?std::min(movetime,limits.remaining_ms):movetime;
        if(!limits.timed && !depth_given)infinite=true;
        begin_search(d,limits,received_at,ponder,infinite);
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
        else if(name=="Move Overhead")move_overhead_ms=std::clamp(std::stoi(value),0,5000);
    }
    void winboard_command(const std::string& line,const std::vector<std::string>& w,std::int64_t received_at) {
        const auto& c=w[0];
        if(c=="protover") {emit("feature myname=\"Petir 1 C++ faithful port\" setboard=1 usermove=1 san=0 ping=1 colors=0 time=1 sigint=0 sigterm=0 reuse=1 analyze=0 option=\"Move Overhead -spin 50 0 5000\" done=1");return;}
        if(c=="ping" && w.size()>1) {emit("pong "+w[1]);return;}
        if(c=="?") {finish(true);return;}
        if(c=="time" || c=="otim") {
            if(w.size()>1) {
                const auto ms=std::clamp<std::int64_t>(std::stoll(w[1]),0,INT32_MAX/10)*10;
                if((engine_side==1)==(c=="time"))wb_white_ms=ms;
                else wb_black_ms=ms;
                if(c=="time" && usetimer && modetime!=seconds_move) {
                    petir::search_time.tighten(received_at,ms,move_overhead_ms);
                }
            }
            return;
        }
        if(c=="post") {post=true;return;}
        if(c=="nopost") {post=false;return;}
        if(c=="hard" || c=="easy")return;
        if(c=="option") {
            const auto setting=join(w,1,w.size());const auto equals=setting.find('=');
            if(equals!=std::string::npos && setting.substr(0,equals)=="Move Overhead") {
                finish();move_overhead_ms=std::clamp(std::stoi(setting.substr(equals+1)),0,5000);
            }
            return;
        }
        const bool move_command=c=="usermove" || (c.size()>=4 && c[0]>='a' && c[0]<='h' && c[1]>='1' && c[1]<='8');
        if(c!="new" && c!="force" && c!="go" && c!="setboard" && c!="sd" && c!="st" && c!="level" && c!="result" && c!="undo" && c!="remove" && !move_command)return;
        finish();
        if(c=="new") {
            petir::new_game();position=data;stm=1;fullmove=1;engine_side=2;force=false;reset_history();
            if(usetimer)wb_white_ms=wb_black_ms=wb_base_ms;
        }
        else if(c=="force")force=true;
        else if(c=="go") {force=false;engine_side=stm;begin_search(depth,winboard_limits(),received_at);}
        else if(c=="setboard") {
            int next_side=1,next_fullmove=1;
            auto next=petir::from_fen(join(w,1,w.size()),next_side,&next_fullmove);
            position=next;stm=next_side;fullmove=next_fullmove;
            jumlahlangkah=(fullmove-1)*2+(stm==2);reset_history();
        }
        else if(c=="sd" && w.size()>1) {depth=std::clamp(std::stoi(w[1]),1,49);usetimer=false;modetime=fixed_depth;}
        else if(c=="st" && w.size()>1) {depth=49;wb_move_ms=static_cast<std::int64_t>(std::clamp(std::stod(w[1])*1000,0.0,double(INT32_MAX)));usetimer=true;modetime=seconds_move;}
        else if(c=="level" && w.size()>3) {
            depth=49;
            rmoves=std::max(0,std::stoi(w[1]));auto colon=w[2].find(':');std::int64_t secs=std::stoll(w[2])*60;if(colon!=std::string::npos)secs+=std::stoll(w[2].substr(colon+1));
            wb_base_ms=std::clamp<std::int64_t>(secs,0,INT32_MAX/1000)*1000;wb_white_ms=wb_black_ms=wb_base_ms;
            wb_increment_ms=static_cast<std::int64_t>(std::clamp(std::stod(w[3])*1000,0.0,double(INT32_MAX)));
            usetimer=true;modetime=rmoves?moves_minutes:minutes_game_increment;
        }
        else if(c=="result")force=true;
        else if(c=="undo" || c=="remove") {
            int n=c=="remove"?2:1;
            while(n-- && !history.empty()) {position=history.back().first;stm=history.back().second;history.pop_back();--jumlahlangkah;}
            fullmove=jumlahlangkah/2+1;rebuild_repetition();
        } else if(move_command) {
            std::string m=c=="usermove"?(w.size()>1?w[1]:""):c;
            if(!apply(m))emit("Illegal move: "+m);
            else if(!force && stm==engine_side)begin_search(depth,winboard_limits(),received_at);
        }
    }
public:
    ~Console() {finish();pas::output={};}
    void run() {
        reset_history();std::string line;
        while(std::getline(std::cin,line)) {
            const auto received_at=now_ms();
            auto w=words(line);if(w.empty())continue;
            try {
                const auto& c=w[0];
                if(c=="quit") {finish();break;}
                if(c=="xboard") {finish();mode=Mode::WinBoard;continue;}
                if(c=="uci") {finish();mode=Mode::UCI;emit("id name Petir 1 C++ faithful port");emit("id author Original Petir author; C++ port");emit("option name Hash type spin default 64 min 1 max 1024");emit("option name OwnBook type check default true");emit("option name Contempt type spin default 0 min -1000 max 1000");emit("option name MTDf type check default false");emit("option name Move Overhead type spin default 50 min 0 max 5000");emit("option name Clear Hash type button");emit("uciok");}
                else if(mode==Mode::WinBoard)winboard_command(line,w,received_at);
                else if(c=="isready")emit("readyok");
                else if(c=="setoption")option(w);
                else if(c=="ucinewgame") {finish();petir::new_game();position=data;stm=1;fullmove=1;reset_history();}
                else if(c=="position")uci_position(w);
                else if(c=="go")uci_go(w,received_at);
                else if(c=="stop")finish(true);
                else if(c=="ponderhit" && pondering.load()) {
                    {std::lock_guard hold_lock(hold_mutex);petir::search_time.start(ponder_budget,received_at);pondering=false;hold_result=false;}
                    hold_cv.notify_all();
                }
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
