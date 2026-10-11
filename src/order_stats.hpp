// Move-ordering statistics, compiled only with -DPETIR_ORDER_STATS=ON. The
// hooks read search state and never write it, so an instrumented search is
// identical to a normal one. The UCI command "orderstats" prints everything
// counted since the previous report as one JSON line, then clears it.
#pragma once
#ifdef PETIR_ORDER_STATS
#define PETIR_ORDER_STAT(...) __VA_ARGS__
#include "petir.hpp"
#include <cstdint>
#include <string>
namespace petir_order_stats {
using u64 = std::uint64_t;
// Where a searched move came from; tools/order_stats.py uses the same order.
enum Phase {hash, goodCapture, killer1, killer2, killer3, killer4, quiet, badCapture, evasionHash, evasion, phaseCount};
inline constexpr int depthCount = 6, indexCount = 8, rankCount = 7, quiescenceIndexCount = 4;
// Remaining depth in whole plies: 0-1, 2, 3, 4-5, 6-8, 9+.
inline int depthBucket(int plies) {return plies<=1?0:plies==2?1:plies==3?2:plies<=5?3:plies<=8?4:5;}
// Move number: 1, 2, 3, 4, 5, 6-10, 11-20, 21+.
inline int indexBucket(int moveNumber) {return moveNumber<=5?moveNumber-1:moveNumber<=10?5:moveNumber<=20?6:7;}
// Position in the quiet list: 1, 2, 3, 4-5, 6-10, 11-20, 21+.
inline int rankBucket(int listPosition) {return listPosition<=3?listPosition-1:listPosition<=5?3:listPosition<=10?4:listPosition<=20?5:6;}
// Exact position in the quiet list, 1..39, with 40 and later together.
inline constexpr int positionCount = 40;
inline int positionIndex(int listPosition) {return (listPosition<positionCount?listPosition:positionCount)-1;}

struct Counters {
    u64 outcome[3][3]{};                        // [expected pv/cut/all][actual pv/cut/all]
    u64 cutIndex[depthCount][indexCount]{};  // move number of the cutoff move
    u64 cutMoveSum[depthCount]{};            // exact sum of those move numbers
    u64 cutByCheck[2][2]{};                   // [in check][cut on first move]
    u64 pvHash[depthCount][2]{};              // expected-PV move loops by hash move available
    u64 cutPhase[depthCount][phaseCount]{};
    u64 triedPhase[depthCount][phaseCount]{};
    u64 cutWasted[depthCount]{}, cutNodes[depthCount]{}; // nodes before the cutoff move / whole loop
    u64 phaseWasted[depthCount][phaseCount]{};            // cutWasted by phase of the cutoff move
    u64 cutByHash[depthCount][2][2]{};       // [hash move available][cut on first move]
    u64 hashWasted[depthCount][2]{}, hashNodes[depthCount][2]{};
    u64 cutBySide[2][2]{};                    // [white to move][cut on first move]
    u64 quietRank[2][rankCount]{};            // [white to move][quiet list position]
    u64 quietTriedRank[2][rankCount]{};
    u64 quietZeroScore{}, quietListMoves{};
    u64 quietPositionCut[positionCount]{}, quietPositionTried[positionCount]{}; // by exact list position
    u64 pvBestIndex[indexCount]{};
    u64 reduced{}, lmrResearch{}, lmrHeld{}, pvsResearch{}, pvsRaised{};
    u64 rootCalls{}, rootFailLow{}, rootBestIndex[indexCount]{}, rootBestChanges{};
    u64 rootNodesFirst{}, rootNodes{}, rootResearch{}, rootRaised{};
    u64 quiescenceCutIndex[quiescenceIndexCount]{}, quiescenceNodes{};
};
inline Counters counters;

// Differences stay right if the 32-bit node counter wraps.
inline std::uint32_t nodeCounter() {return static_cast<std::uint32_t>(petir_header::nodeCount);}

// One mainSearch move loop.
class SearchNode {
    int depth, expected, best=0, rank=0, list=0, score=0;
    bool isWhite, hasHash, inCheck;
    Phase phase=hash;
    std::uint32_t start=nodeCounter(), moveStart=start;
public:
    SearchNode(int plies, int sideToMove, int hashMove, int nodeType, bool isInCheck)
        : depth(depthBucket(plies)),
          expected(nodeType==petir_search::pvNode?0:nodeType==petir_search::cutNode?1:2),
          isWhite(sideToMove==petir_header::whiteSide), hasHash(hashMove!=petir_header::noMove), inCheck(isInCheck) {
        if(expected==0)++counters.pvHash[depth][hasHash];
    }
    // Called once the move is made and known legal; the picker state still
    // describes the phase that produced it.
    void move(int searchedMove, int hashMove, int pickerPhase, int quietIndex, int quietMoveCount, const petir_header::tmovelist& quietList) {
        moveStart = nodeCounter()-1; // makemove has already counted this move
        if(inCheck)phase = searchedMove==hashMove?evasionHash:evasion;
        else if(searchedMove==hashMove)phase = hash;
        else switch(pickerPhase) {
            case petir_header::goodCapturesPhase: phase = goodCapture; break;
            case petir_header::killerMove2Phase: phase = killer1; break;
            case petir_header::killerMove3Phase: phase = killer2; break;
            case petir_header::killerMove4Phase: phase = killer3; break;
            // The last killer leaves the picker in the quiet phase before any
            // quiet move has been generated.
            case petir_header::otherMovesPhase: phase = quietIndex==0?killer4:quiet; break;
            default: phase = badCapture;
        }
        ++counters.triedPhase[depth][phase];
        if(phase==quiet) {
            rank=quietIndex;list=quietMoveCount;score=quietList[quietIndex].score;
            ++counters.quietTriedRank[isWhite][rankBucket(rank)];
            ++counters.quietPositionTried[positionIndex(rank)];
        }
    }
    void searching(int reduction) {if(reduction!=0)++counters.reduced;}
    void pvsResearch() {++counters.pvsResearch;}
    void pvsResult(bool wasRaised) {counters.pvsRaised+=wasRaised;}
    void lmrResearch() {++counters.lmrResearch;}
    void lmrResult(bool wasHeld) {counters.lmrHeld+=wasHeld;}
    void raised(int moveNumber) {best=moveNumber;}
    void cutoff(int moveNumber) {
        const std::uint32_t wasted=moveStart-start, total=nodeCounter()-start;
        const bool isFirst=moveNumber==1;
        ++counters.cutIndex[depth][indexBucket(moveNumber)];
        counters.cutMoveSum[depth]+=moveNumber;
        ++counters.cutByCheck[inCheck][isFirst];
        ++counters.cutPhase[depth][phase];
        counters.cutWasted[depth]+=wasted;counters.cutNodes[depth]+=total;
        counters.phaseWasted[depth][phase]+=wasted;
        ++counters.cutByHash[depth][hasHash][isFirst];
        counters.hashWasted[depth][hasHash]+=wasted;counters.hashNodes[depth][hasHash]+=total;
        ++counters.cutBySide[isWhite][isFirst];
        if(phase==quiet) {
            ++counters.quietRank[isWhite][rankBucket(rank)];
            ++counters.quietPositionCut[positionIndex(rank)];
            counters.quietZeroScore+=score==0;counters.quietListMoves+=list;
        }
    }
    void finish(int bestScore, int alpha, int beta, int legalMoveCount) {
        if(legalMoveCount==0)return;
        const int actualType=bestScore>=beta?1:bestScore<=alpha?2:0;
        ++counters.outcome[expected][actualType];
        if(actualType==0 && best>0)++counters.pvBestIndex[indexBucket(best)];
    }
};

// One searchRoot call: an iteration or an aspiration re-search.
class RootNode {
    std::uint32_t start=nodeCounter(), firstEnd=0;
    int best=0;
public:
    void move(int moveNumber) {if(moveNumber==2)firstEnd=nodeCounter()-1;}
    void research() {++counters.rootResearch;}
    void researchResult(bool wasRaised) {counters.rootRaised+=wasRaised;}
    void raised(int moveNumber) {best=moveNumber;counters.rootBestChanges+=moveNumber>1;}
    void finish() {
        ++counters.rootCalls;
        if(best>0)++counters.rootBestIndex[indexBucket(best)];else ++counters.rootFailLow;
        const std::uint32_t endNodes=nodeCounter();
        counters.rootNodes+=endNodes-start;counters.rootNodesFirst+=(firstEnd?firstEnd:endNodes)-start;
    }
};

// Quiescence nodes not in check; searched counts moves actually searched.
inline void quiescenceCutoff(bool inCheck, int searchedCount) {
    if(inCheck)return;
    ++counters.quiescenceNodes;++counters.quiescenceCutIndex[searchedCount<4?searchedCount-1:3];
}
inline void quiescenceDone(bool inCheck, int searchedCount) {if(!inCheck && searchedCount>0)++counters.quiescenceNodes;}

inline void put(std::string& text, u64 value) {text+=std::to_string(value);}
template<class T, std::size_t N> void put(std::string& text, const T (&values)[N]) {
    text+='[';
    for(std::size_t index=0;index<N;++index) {if(index)text+=',';put(text,values[index]);}
    text+=']';
}
inline std::string report() {
    const auto& current=counters;
    std::string text="orderstats {";
    bool isFirst=true;
    auto addField=[&](const char* fieldName, const auto& value) {
        if(!isFirst)text+=',';
        isFirst=false;text+='"';text+=fieldName;text+="\":";put(text,value);
    };
    addField("outcome",current.outcome);addField("cut_index",current.cutIndex);addField("cut_move_sum",current.cutMoveSum);
    addField("cut_by_check",current.cutByCheck);addField("pv_hash",current.pvHash);addField("cut_phase",current.cutPhase);
    addField("tried_phase",current.triedPhase);addField("cut_wasted",current.cutWasted);addField("cut_nodes",current.cutNodes);
    addField("phase_wasted",current.phaseWasted);
    addField("cut_by_hash",current.cutByHash);addField("hash_wasted",current.hashWasted);addField("hash_nodes",current.hashNodes);
    addField("cut_by_side",current.cutBySide);addField("quiet_rank",current.quietRank);addField("quiet_tried_rank",current.quietTriedRank);
    addField("quiet_zero_score",current.quietZeroScore);addField("quiet_list_moves",current.quietListMoves);
    addField("quiet_pos_cut",current.quietPositionCut);addField("quiet_pos_tried",current.quietPositionTried);
    addField("pv_best_index",current.pvBestIndex);addField("reduced",current.reduced);
    addField("lmr_research",current.lmrResearch);addField("lmr_held",current.lmrHeld);
    addField("pvs_research",current.pvsResearch);addField("pvs_raised",current.pvsRaised);
    addField("root_calls",current.rootCalls);addField("root_fail_low",current.rootFailLow);
    addField("root_best_index",current.rootBestIndex);addField("root_best_changes",current.rootBestChanges);
    addField("root_nodes_first",current.rootNodesFirst);addField("root_nodes",current.rootNodes);
    addField("root_research",current.rootResearch);addField("root_raised",current.rootRaised);
    addField("q_cut_index",current.quiescenceCutIndex);addField("q_nodes",current.quiescenceNodes);
    text+='}';
    counters={};
    return text;
}
}
#else
#define PETIR_ORDER_STAT(...)
#endif
