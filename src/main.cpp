#include "engine.hpp"
#include "order_stats.hpp"
#include "hash_stats.hpp"
#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <sstream>
#include <thread>

namespace {
using namespace petir_header;
#ifdef PETIR_ENGINE_NAME
constexpr const char* engineName=PETIR_ENGINE_NAME;
#else
constexpr const char* engineName="Petir 1 C++ faithful port";
#endif
std::mutex outputMutex;
void emit(const std::string& text) {std::lock_guard lock(outputMutex);std::cout<<text<<std::endl;}
std::int64_t nowMs() {return petir::TimeManager::nowMs();}
std::vector<std::string> splitWords(const std::string& line) {std::istringstream stream(line);std::vector<std::string> tokens;for(std::string token;stream>>token;)tokens.push_back(token);return tokens;}
std::string joinWords(const std::vector<std::string>& tokens,std::size_t first,std::size_t last) {std::string text;for(;first<last;++first) {if(!text.empty())text+=' ';text+=tokens[first];}return text;}
class Console {
    enum class Mode { UCI, WinBoard } mode=Mode::UCI;
    petir::Position position=gamePosition;
    int sideToMove=1,fullMoveNumber=1,depthLimit=49,computerSide=2;
    bool forceMode=false;
    // WinBoard "analyze": search the current position without a clock and without
    // playing a move, restarting whenever the GUI changes the position.
    bool analyzeMode=false;
    std::atomic_bool postThinking=true,publishResult=true;
    std::mutex resultMutex;
    std::vector<std::pair<petir::Position,int>> history;
    std::thread worker;
    std::atomic_bool pondering=false,holdResult=false;
    std::mutex holdMutex;
    std::condition_variable holdCondition;
    petir::TimeBudget ponderBudget;
    int moveOverheadMs=50;
    // winboardEngineMs is the engine's own clock: the search only runs for the
    // engine's side, and WinBoard's "time" command always gives that clock.
    std::int64_t winboardEngineMs=0,winboardBaseMs=0,winboardIncrementMs=0,winboardMoveMs=0;
    petir::ClockLimits winboardLimits() const {
        petir::ClockLimits limits;
        limits.timed=useTimer;limits.overheadMs=moveOverheadMs;
        if(timeMode==secondsPerMove)limits.moveTimeMs=winboardMoveMs;
        else {
            limits.remainingMs=winboardEngineMs;
            limits.incrementMs=winboardIncrementMs;
            if(movesPerTimeControl>0)limits.movesToGo=movesPerTimeControl-(gamePlyCount/2)%movesPerTimeControl;
        }
        return limits;
    }
    void finish(bool returnMove=false) {
        {
            std::lock_guard lock(resultMutex);
            if(!returnMove)publishResult=false;
            std::lock_guard holdLock(holdMutex);
            stopSearch=true;holdResult=false;pondering=false;
        }
        holdCondition.notify_all();
        if(worker.joinable())worker.join();
    }
    void resetHistory() {
        history.clear();petir_repetition::clearRepetitionTable();
    }
    void rebuildRepetition() {
        petir_repetition::clearRepetitionTable();
        for(std::size_t index=1;index<history.size();++index)
            petir_repetition::addRepetition(history[index].first.hashKey,history[index].second-1);
        if(!history.empty())petir_repetition::addRepetition(position.hashKey,sideToMove-1);
    }
    std::uint64_t repetitionKey(petir::Position keyPosition,int side) const {
        // An en passant square changes repetition identity only when capture is legal.
        if(keyPosition.enPassantFile!=noEnPassant) {
            bool canCaptureEnPassant=false;
            for(int move:petir::legalMoves(keyPosition,side))
                if(((move>>7)&127)==enPassantTarget) {canCaptureEnPassant=true;break;}
            if(!canCaptureEnPassant)keyPosition.enPassantFile=noEnPassant;
        }
        return petir_hashing_header::computeHashKey(keyPosition,side);
    }
    bool reportResult(const std::vector<int>& legalMoveList) {
        std::string result;
        if(legalMoveList.empty()) {
            const bool checked=sideToMove==1?petir_tools::whiteInCheck(position):petir_tools::blackInCheck(position);
            result=checked?(sideToMove==1?"0-1 {Checkmate}":"1-0 {Checkmate}"):"1/2-1/2 {Stalemate}";
        } else if(petir::insufficientMaterial(position))result="1/2-1/2 {Insufficient material}";
        else if(position.fiftyMoveCounter>=100)result="1/2-1/2 {50-move rule}";
        else {
            const auto key=repetitionKey(position,sideToMove);
            int occurrences=1;
            for(auto entry=history.rbegin();entry!=history.rend();++entry) {
                if(entry->second==sideToMove && repetitionKey(entry->first,sideToMove)==key && ++occurrences>=3) {
                    result="1/2-1/2 {Threefold repetition}";break;
                }
            }
        }
        if(result.empty())return false;
        forceMode=true;emit(result);return true;
    }
    bool applyMove(const std::string& text) {
        int move=petir::parseMove(text,position,sideToMove);
        if(!move)return false;
        history.emplace_back(position,sideToMove);petir::makeMove(position,sideToMove,move);
        if(sideToMove==2)++fullMoveNumber;
        sideToMove=3-sideToMove;++gamePlyCount;
        petir_repetition::addRepetition(position.hashKey,sideToMove-1);return true;
    }
    int fallbackMove(const petir::Position& rootPosition,int rootSide,const std::vector<int>& legalMoveList) const {
        if(legalMoveList.empty())return 0;
        int alpha=-20500,beta=20500,score=0,hashMove=0;bool avoidNullMove=false;
        (void)petir_hashing::probeHash(rootPosition.hashKey,alpha,beta,512,rootSide,score,hashMove,avoidNullMove,rootPosition.fiftyMoveCounter);
        return std::find(legalMoveList.begin(),legalMoveList.end(),hashMove)!=legalMoveList.end()?hashMove:legalMoveList.front();
    }
    void publishMove(petir::Position rootPosition,int rootSide,int bestMove,bool wasSearched,bool fromBook=false) {
        computerThinking=false;
        std::lock_guard lock(resultMutex);
        if(!publishResult.load())return;
        if(postThinking.load() && bestMove && !fromBook) {
            const bool isCompleted=wasSearched && petir_uiterative::completedDepth>0 && petir_uiterative::completedMove==bestMove;
            // Without a completed iteration, report this root's static score at
            // depth 0 and the legal move being returned. Do not reuse old search data.
            const auto pv=isCompleted?petir_uiterative::completedPv:petir::moveText(bestMove,rootPosition,rootSide);
            const auto depthText=std::to_string(isCompleted?petir_uiterative::completedDepth:0);
            const auto scoreText=std::to_string(isCompleted?petir_uiterative::completedScore:petir_ueval::evaluate(rootPosition,rootSide,-20500,20500));
            const auto elapsed=petir::searchTime.elapsedMs();
            const auto nodesText=std::to_string(wasSearched?nodeCount:0);
            if(mode==Mode::UCI)emit("info depth "+depthText+" score cp "+scoreText+" time "+std::to_string(elapsed)+" nodes "+nodesText+" pv "+pv);
            else emit(depthText+" "+scoreText+" "+std::to_string(elapsed/10)+" "+nodesText+" "+pv);
        }
        if(mode==Mode::UCI) {
            if(wasSearched)emit("info nodes "+std::to_string(nodeCount)+" time "+std::to_string(petir::searchTime.elapsedMs()));
            emit("bestmove "+petir::moveText(bestMove,rootPosition,rootSide));
        } else if(bestMove) {
            const auto moveText=petir::moveText(bestMove,rootPosition,rootSide);
            // Publish before board/history maintenance; finish() serializes the next command.
            emit("move "+moveText);
            history.emplace_back(position,sideToMove);petir::makeMove(position,sideToMove,bestMove);
            if(sideToMove==2)++fullMoveNumber;
            sideToMove=3-sideToMove;++gamePlyCount;
            petir_repetition::addRepetition(position.hashKey,sideToMove-1);
            reportResult(petir::legalMoves(position,sideToMove));
        } else emit((rootSide==1?petir_tools::whiteInCheck(rootPosition):petir_tools::blackInCheck(rootPosition))?(rootSide==1?"0-1 {Checkmate}":"1-0 {Checkmate}"):"1/2-1/2 {Stalemate}");
    }
    void beginSearch(int requestedDepth,const petir::ClockLimits& limits,std::int64_t receivedAt,bool ponder=false,bool infinite=false) {
        finish();auto rootPosition=position;int rootSide=sideToMove;
        const auto budget=petir::TimeManager::allocate(limits);
        petir::searchTime.reset();
        petir::searchTime.start(ponder || infinite?petir::TimeBudget{}:budget,receivedAt);
        auto legalMoveList=petir::legalMoves(rootPosition,rootSide);
        if(mode==Mode::WinBoard && !infinite && reportResult(legalMoveList))return;
        publishResult=true;
        const int fallback=fallbackMove(rootPosition,rootSide,legalMoveList);
        if(!ponder && !infinite && (budget.emergency || petir::searchTime.expired(true))) {
            // Skip search preparation and return a legal move with depth-0 info.
            publishMove(rootPosition,rootSide,fallback,false);return;
        }
        petir::prepareSearch(rootSide,limits.timed,INT32_MAX);
        petir::searchTime.start(ponder || infinite?petir::TimeBudget{}:budget,receivedAt);
        pondering=ponder;holdResult=ponder || infinite;ponderBudget=budget;
        useWinboard=true;
        pas::output=[this](const pas::String& text) {
            if(!postThinking || petir::searchShouldStop(true))return;
            if(mode==Mode::WinBoard) {emit(text.text);return;}
            std::istringstream stream(text.text);int depth=0,score=0,centiseconds=0,nodes=0;std::string pv;
            if(stream>>depth>>score>>centiseconds>>nodes) {std::getline(stream,pv);emit("info depth "+std::to_string(depth)+" score cp "+std::to_string(score)+" time "+std::to_string(centiseconds*10)+" nodes "+std::to_string(nodes)+" pv"+pv);}
        };
        worker=std::thread([this,rootPosition,rootSide,legalMoveList,fallback,requestedDepth,ponder,infinite]() mutable {
            int bestMove=fallback;bool wasSearched=false,fromBook=false;
            try {
                if(!legalMoveList.empty() && !petir::searchShouldStop(true)) {
                    int bookMove=0;
                    if(useBook && !ponder && !infinite)bookMove=petir_opening_book::findBookMove(rootPosition,rootSide);
                    if(bookMove) {bestMove=bookMove;fromBook=true;}
                    else if(!petir::searchShouldStop(true)) {wasSearched=true;(void)petir_uiterative::iterativeDeepening(std::clamp(requestedDepth,1,49),rootSide,maxNode,rootPosition);bestMove=rootBestMove;}
                    if(std::find(legalMoveList.begin(),legalMoveList.end(),bestMove)==legalMoveList.end())bestMove=fallback;
                }
            } catch(const std::exception& error) {
                emit(mode==Mode::UCI?"info string search stopped: "+std::string(error.what()):"Error (search): "+std::string(error.what()));
                bestMove=petir_uiterative::completedMove;
                if(std::find(legalMoveList.begin(),legalMoveList.end(),bestMove)==legalMoveList.end())bestMove=fallback;
            }
            {
                std::unique_lock holdLock(holdMutex);
                holdCondition.wait(holdLock,[this]{return !holdResult.load() || stopSearch.load();});
            }
            publishMove(rootPosition,rootSide,bestMove,wasSearched,fromBook);
        });
    }
    void uciPosition(const std::vector<std::string>& tokens) {
        finish();std::size_t movesTokenIndex=std::find(tokens.begin(),tokens.end(),"moves")-tokens.begin();
        if(tokens.size()<2)throw std::invalid_argument("position needs startpos or fen");
        petir::Position nextPosition;int nextSide=1,nextFullMove=1;
        std::vector<std::pair<petir::Position,int>> nextHistory;
        if(tokens[1]=="startpos")petir_udata::setInitialPosition(nextPosition);
        else if(tokens[1]=="fen")nextPosition=petir::fromFen(joinWords(tokens,2,movesTokenIndex),nextSide,&nextFullMove);
        else throw std::invalid_argument("position needs startpos or fen");
        for(std::size_t index=movesTokenIndex+1;index<tokens.size();++index) {
            const int move=petir::parseMove(tokens[index],nextPosition,nextSide);
            if(!move)throw std::invalid_argument("Illegal move "+tokens[index]);
            nextHistory.emplace_back(nextPosition,nextSide);petir::makeMove(nextPosition,nextSide,move);
            if(nextSide==2)++nextFullMove;
            nextSide=3-nextSide;
        }
        position=nextPosition;sideToMove=nextSide;fullMoveNumber=nextFullMove;history=std::move(nextHistory);
        gamePlyCount=(fullMoveNumber-1)*2+(sideToMove==2);rebuildRepetition();
    }
    void uciGo(const std::vector<std::string>& tokens,std::int64_t receivedAt) {
        int depth=49,movesToGo=0;std::int64_t moveTime=0,whiteTime=0,blackTime=0,whiteIncrement=0,blackIncrement=0;
        bool ponder=false,infinite=false,depthGiven=false,clockGiven=false,moveTimeGiven=false;
        for(std::size_t index=1;index<tokens.size();++index) {
            if(tokens[index]=="ponder")ponder=true;
            else if(tokens[index]=="infinite")infinite=true;
            else if(index+1<tokens.size() && (tokens[index]=="depth" || tokens[index]=="movetime" || tokens[index]=="wtime" || tokens[index]=="btime" || tokens[index]=="winc" || tokens[index]=="binc" || tokens[index]=="movestogo")) {
                auto value=std::clamp<std::int64_t>(std::stoll(tokens[++index]),0,INT32_MAX);const auto& key=tokens[index-1];
                if(key=="depth") {depth=static_cast<int>(std::clamp<std::int64_t>(value,1,49));depthGiven=true;}
                if(key=="movetime") {moveTime=value;moveTimeGiven=true;}
                if(key=="wtime") {whiteTime=value;clockGiven=true;}
                if(key=="btime") {blackTime=value;clockGiven=true;}
                if(key=="winc")whiteIncrement=value;
                if(key=="binc")blackIncrement=value;
                if(key=="movestogo")movesToGo=static_cast<int>(value);
            }
        }
        petir::ClockLimits limits;
        limits.timed=moveTimeGiven || clockGiven;
        limits.remainingMs=sideToMove==1?whiteTime:blackTime;limits.incrementMs=sideToMove==1?whiteIncrement:blackIncrement;
        limits.movesToGo=movesToGo;limits.overheadMs=moveOverheadMs;
        if(moveTimeGiven)limits.moveTimeMs=clockGiven?std::min(moveTime,limits.remainingMs):moveTime;
        if(!limits.timed && !depthGiven)infinite=true;
        beginSearch(depth,limits,receivedAt,ponder,infinite);
    }
    void setOption(const std::vector<std::string>& tokens) {
        finish();auto valueToken=std::find(tokens.begin(),tokens.end(),"value");std::size_t valueIndex=valueToken-tokens.begin();
        const auto name=joinWords(tokens,2,valueIndex);auto value=valueIndex+1<tokens.size()?joinWords(tokens,valueIndex+1,tokens.size()):"";
        if(name=="Hash") {
            int megabytes=std::clamp(std::stoi(value),1,1024),entries=1;
            while(std::int64_t(entries)*2*static_cast<std::int64_t>(sizeof(petir_hashing_header::thashrecord))<=std::int64_t(megabytes)*1048576)entries*=2;
            // vector::resize preserves the old table if allocation fails.
            petir_hashing_header::hashTable.resize(entries);
            hashTableSize=entries;petir_hashing_header::clearHashTable();
        } else if(name=="OwnBook")useBook=value=="true";
        else if(name=="Clear Hash")petir_hashing_header::clearHashTable();
        else if(name=="Contempt")drawScore=std::clamp(std::stoi(value),-1000,1000);
        else if(name=="MTDf")useMtdf=value=="true";
        else if(name=="Move Overhead")moveOverheadMs=std::clamp(std::stoi(value),0,5000);
    }
    void winboardCommand(const std::string& line,const std::vector<std::string>& tokens,std::int64_t receivedAt) {
        const auto& command=tokens[0];
        if(command=="protover") {emit(std::string("feature myname=\"")+engineName+"\" setboard=1 usermove=1 san=0 ping=1 colors=0 time=1 sigint=0 sigterm=0 reuse=1 analyze=1 option=\"Move Overhead -spin 50 0 5000\" done=1");return;}
        if(command=="ping" && tokens.size()>1) {emit("pong "+tokens[1]);return;}
        if(command=="?") {if(!analyzeMode)finish(true);return;}
        if(command=="analyze") {finish();forceMode=true;analyzeMode=true;beginSearch(49,petir::ClockLimits{},receivedAt,false,true);return;}
        if(command=="exit") {finish();analyzeMode=false;return;}
        // "time" is the engine's clock and "otim" the opponent's, whichever colour
        // the engine plays (the colour may not be known yet, before "go").
        // Only the engine's clock is used.
        if(command=="time" || command=="otim") {
            if(command=="time" && tokens.size()>1) {
                const auto milliseconds=std::clamp<std::int64_t>(std::stoll(tokens[1]),0,INT32_MAX/10)*10;
                winboardEngineMs=milliseconds;
                if(useTimer && timeMode!=secondsPerMove && !analyzeMode) {
                    petir::searchTime.tighten(receivedAt,milliseconds,moveOverheadMs);
                }
            }
            return;
        }
        if(command=="post") {postThinking=true;return;}
        if(command=="nopost") {postThinking=false;return;}
        if(command=="hard" || command=="easy")return;
        if(command=="option") {
            const auto setting=joinWords(tokens,1,tokens.size());const auto equalsPosition=setting.find('=');
            if(equalsPosition!=std::string::npos && setting.substr(0,equalsPosition)=="Move Overhead") {
                finish();moveOverheadMs=std::clamp(std::stoi(setting.substr(equalsPosition+1)),0,5000);
            }
            return;
        }
        const bool isMoveCommand=command=="usermove" || (command.size()>=4 && command[0]>='a' && command[0]<='h' && command[1]>='1' && command[1]<='8');
        if(command!="new" && command!="force" && command!="go" && command!="setboard" && command!="sd" && command!="st" && command!="level" && command!="result" && command!="undo" && command!="remove" && !isMoveCommand)return;
        finish();
        if(command=="new" || command=="go" || command=="force" || command=="result")analyzeMode=false;
        if(command=="new") {
            petir::newGame();position=gamePosition;sideToMove=1;fullMoveNumber=1;computerSide=2;forceMode=false;resetHistory();
            if(useTimer)winboardEngineMs=winboardBaseMs;
        }
        else if(command=="force")forceMode=true;
        else if(command=="go") {forceMode=false;computerSide=sideToMove;beginSearch(depthLimit,winboardLimits(),receivedAt);}
        else if(command=="setboard") {
            int nextSide=1,nextFullMove=1;
            auto nextPosition=petir::fromFen(joinWords(tokens,1,tokens.size()),nextSide,&nextFullMove);
            position=nextPosition;sideToMove=nextSide;fullMoveNumber=nextFullMove;
            gamePlyCount=(fullMoveNumber-1)*2+(sideToMove==2);resetHistory();
        }
        else if(command=="sd" && tokens.size()>1) {depthLimit=std::clamp(std::stoi(tokens[1]),1,49);useTimer=false;timeMode=fixedDepth;}
        else if(command=="st" && tokens.size()>1) {depthLimit=49;winboardMoveMs=static_cast<std::int64_t>(std::clamp(std::stod(tokens[1])*1000,0.0,double(INT32_MAX)));useTimer=true;timeMode=secondsPerMove;}
        else if(command=="level" && tokens.size()>3) {
            depthLimit=49;
            movesPerTimeControl=std::max(0,std::stoi(tokens[1]));auto colonPosition=tokens[2].find(':');std::int64_t seconds=std::stoll(tokens[2])*60;if(colonPosition!=std::string::npos)seconds+=std::stoll(tokens[2].substr(colonPosition+1));
            winboardBaseMs=std::clamp<std::int64_t>(seconds,0,INT32_MAX/1000)*1000;winboardEngineMs=winboardBaseMs;
            winboardIncrementMs=static_cast<std::int64_t>(std::clamp(std::stod(tokens[3])*1000,0.0,double(INT32_MAX)));
            useTimer=true;timeMode=movesPerTimeControl?movesInMinutes:minutesPerGameWithIncrement;
        }
        else if(command=="result")forceMode=true;
        else if(command=="undo" || command=="remove") {
            int undoCount=command=="remove"?2:1;
            while(undoCount-- && !history.empty()) {position=history.back().first;sideToMove=history.back().second;history.pop_back();--gamePlyCount;}
            fullMoveNumber=gamePlyCount/2+1;rebuildRepetition();
        } else if(isMoveCommand) {
            std::string moveText=command=="usermove"?(tokens.size()>1?tokens[1]:""):command;
            if(!applyMove(moveText))emit("Illegal move: "+moveText);
            else if(!forceMode && sideToMove==computerSide)beginSearch(depthLimit,winboardLimits(),receivedAt);
        }
        // Every command that reaches here stopped the search; analysis resumes on the new position.
        if(analyzeMode)beginSearch(49,petir::ClockLimits{},receivedAt,false,true);
    }
public:
    ~Console() {finish();pas::output={};}
    void run() {
        resetHistory();std::string line;
        while(std::getline(std::cin,line)) {
            const auto receivedAt=nowMs();
            auto tokens=splitWords(line);if(tokens.empty())continue;
            try {
                const auto& command=tokens[0];
                if(command=="quit") {finish();break;}
                if(command=="xboard") {finish();mode=Mode::WinBoard;continue;}
                if(command=="uci") {finish();mode=Mode::UCI;emit(std::string("id name ")+engineName);emit("id author Original Petir author; C++ port");emit("option name Hash type spin default 64 min 1 max 1024");emit("option name OwnBook type check default true");emit("option name Contempt type spin default 0 min -1000 max 1000");emit("option name MTDf type check default false");emit("option name Move Overhead type spin default 50 min 0 max 5000");emit("option name Clear Hash type button");emit("uciok");}
                else if(mode==Mode::WinBoard)winboardCommand(line,tokens,receivedAt);
                else if(command=="isready")emit("readyok");
                else if(command=="setoption")setOption(tokens);
                else if(command=="ucinewgame") {finish();petir::newGame();position=gamePosition;sideToMove=1;fullMoveNumber=1;resetHistory();}
                else if(command=="position")uciPosition(tokens);
                else if(command=="go")uciGo(tokens,receivedAt);
                else if(command=="stop")finish(true);
                else if(command=="ponderhit" && pondering.load()) {
                    {std::lock_guard holdLock(holdMutex);petir::searchTime.start(ponderBudget,receivedAt);pondering=false;holdResult=false;}
                    holdCondition.notify_all();
                }
                else if(command=="d") {finish();emit(petir::toFen(position,sideToMove,fullMoveNumber));}
                PETIR_ORDER_STAT(else if(command=="orderstats") {finish();emit(petir_order_stats::report());})
                PETIR_HASH_STAT(else if(command=="hashstats") {finish();emit(petir_hash_stats::report());})
            } catch(const std::exception& error) {emit(mode==Mode::UCI?"info string "+std::string(error.what()):"Error: "+std::string(error.what()));}
        }
        finish();
    }
};
}
int main(int argc,char** argv) {
    try {
        std::filesystem::path executable=std::filesystem::absolute(argv[0]).parent_path();
        std::filesystem::path booksPath=executable/"books";
        if(!std::filesystem::exists(booksPath))booksPath=executable.parent_path()/"books";
        if(!std::filesystem::exists(booksPath))booksPath="books";
        for(int index=1;index+1<argc;++index)if(std::string(argv[index])=="--book-dir")booksPath=argv[index+1];
        petir::initialize(booksPath);
        if(argc>1 && std::string(argv[1])=="--selftest")return petir::selfTest();
        if(argc>1 && std::string(argv[1])=="--perft") {
            int depth=argc>2?std::stoi(argv[2]):4,sideToMove=1;
            if(depth<0 || depth>12)throw std::invalid_argument("perft depth must be 0..12");
            auto position=petir::fromFen(argc>3?argv[3]:petir::startFen,sideToMove);
            const auto startMs=nowMs();auto nodes=petir::perft(position,sideToMove,depth);
            std::cout<<nodes<<" nodes, "<<nowMs()-startMs<<" ms\n";return 0;
        }
        if(argc>1 && std::string(argv[1])=="--eval") {
            int sideToMove=1;auto position=petir::fromFen(argc>2?argv[2]:petir::startFen,sideToMove);
            std::cout<<petir_ueval::evaluate(position,sideToMove,-20500,20500)<<" cp; hash "<<position.hashKey<<"; book move "<<petir::moveText(petir_opening_book::findBookMove(position,sideToMove),position,sideToMove)<<'\n';return 0;
        }
        if(argc>2 && std::string(argv[1])=="--learn-pgn") {petir_opening_book::learnFromPgn(argv[2]);return 0;}
        if(argc>1 && std::string(argv[1])=="--help") {std::cout<<"Petir C++20 64-bit engine. Default: UCI; send xboard for WinBoard.\n--selftest | --perft DEPTH [FEN] | --eval [FEN] | --learn-pgn FILE\n--book-dir DIRECTORY selects the legacy white.book and black.book files.\n";return 0;}
        Console{}.run();
    } catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
}
