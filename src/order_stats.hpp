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
enum Phase {hash, good_capture, killer1, killer2, killer3, killer4, quiet, bad_capture, evasion_hash, evasion, phase_count};
inline constexpr int depth_count = 6, index_count = 8, rank_count = 7, q_index_count = 4;
// Remaining depth in whole plies: 0-1, 2, 3, 4-5, 6-8, 9+.
inline int depth_bucket(int rlevel) {return rlevel<=1?0:rlevel==2?1:rlevel==3?2:rlevel<=5?3:rlevel<=8?4:5;}
// Move number: 1, 2, 3, 4, 5, 6-10, 11-20, 21+.
inline int index_bucket(int n) {return n<=5?n-1:n<=10?5:n<=20?6:7;}
// Position in the quiet list: 1, 2, 3, 4-5, 6-10, 11-20, 21+.
inline int rank_bucket(int n) {return n<=3?n-1:n<=5?3:n<=10?4:n<=20?5:6;}
// Exact position in the quiet list, 1..39, with 40 and later together.
inline constexpr int position_count = 40;
inline int position_index(int n) {return (n<position_count?n:position_count)-1;}

struct Counters {
    u64 outcome[3][3]{};                        // [expected pv/cut/all][actual pv/cut/all]
    u64 cut_index[depth_count][index_count]{};  // move number of the cutoff move
    u64 cut_move_sum[depth_count]{};            // exact sum of those move numbers
    u64 cut_by_check[2][2]{};                   // [in check][cut on first move]
    u64 pv_hash[depth_count][2]{};              // expected-PV move loops by hash move available
    u64 cut_phase[depth_count][phase_count]{};
    u64 tried_phase[depth_count][phase_count]{};
    u64 cut_wasted[depth_count]{}, cut_nodes[depth_count]{}; // nodes before the cutoff move / whole loop
    u64 phase_wasted[depth_count][phase_count]{};            // cut_wasted by phase of the cutoff move
    u64 cut_by_hash[depth_count][2][2]{};       // [hash move available][cut on first move]
    u64 hash_wasted[depth_count][2]{}, hash_nodes[depth_count][2]{};
    u64 cut_by_side[2][2]{};                    // [white to move][cut on first move]
    u64 quiet_rank[2][rank_count]{};            // [white to move][quiet list position]
    u64 quiet_tried_rank[2][rank_count]{};
    u64 quiet_zero_score{}, quiet_list_moves{};
    u64 quiet_pos_cut[position_count]{}, quiet_pos_tried[position_count]{}; // by exact list position
    u64 pv_best_index[index_count]{};
    u64 reduced{}, lmr_research{}, lmr_held{}, pvs_research{}, pvs_raised{};
    u64 root_calls{}, root_fail_low{}, root_best_index[index_count]{}, root_best_changes{};
    u64 root_nodes_first{}, root_nodes{}, root_research{}, root_raised{};
    u64 q_cut_index[q_index_count]{}, q_nodes{};
};
inline Counters counters;

// Differences stay right if the 32-bit node counter wraps.
inline std::uint32_t nodes() {return static_cast<std::uint32_t>(petir_header::total_node);}

// One mainsearch move loop.
class SearchNode {
    int depth, expected, best=0, rank=0, list=0, score=0;
    bool white, has_hash, incheck;
    Phase phase=hash;
    std::uint32_t start=nodes(), move_start=start;
public:
    SearchNode(int rlevel, int giliran, int hmoves, int nodetype, bool incheck)
        : depth(depth_bucket(rlevel)),
          expected(nodetype==petir_search::node_pv?0:nodetype==petir_search::node_cut?1:2),
          white(giliran==petir_header::_sisiputih), has_hash(hmoves!=petir_header::_no_move), incheck(incheck) {
        if(expected==0)++counters.pv_hash[depth][has_hash];
    }
    // Called once the move is made and known legal; the picker state still
    // describes the phase that produced it.
    void move(int moves, int hmoves, int fase, int curmovenoncap, int jmlnoncap, const petir_header::tmovelist& ml) {
        move_start = nodes()-1; // makemove has already counted this move
        if(incheck)phase = moves==hmoves?evasion_hash:evasion;
        else if(moves==hmoves)phase = hash;
        else switch(fase) {
            case petir_header::_goodcaps: phase = good_capture; break;
            case petir_header::_killermoves2: phase = killer1; break;
            case petir_header::_killermoves3: phase = killer2; break;
            case petir_header::_killermoves4: phase = killer3; break;
            // The last killer leaves the picker in the quiet phase before any
            // quiet move has been generated.
            case petir_header::_othermoves: phase = curmovenoncap==0?killer4:quiet; break;
            default: phase = bad_capture;
        }
        ++counters.tried_phase[depth][phase];
        if(phase==quiet) {
            rank=curmovenoncap;list=jmlnoncap;score=ml[curmovenoncap].score;
            ++counters.quiet_tried_rank[white][rank_bucket(rank)];
            ++counters.quiet_pos_tried[position_index(rank)];
        }
    }
    void searching(int areduce) {if(areduce!=0)++counters.reduced;}
    void pvs_research() {++counters.pvs_research;}
    void pvs_result(bool raised) {counters.pvs_raised+=raised;}
    void lmr_research() {++counters.lmr_research;}
    void lmr_result(bool held) {counters.lmr_held+=held;}
    void raised(int legalmove) {best=legalmove;}
    void cutoff(int legalmove) {
        const std::uint32_t wasted=move_start-start, total=nodes()-start;
        const bool first=legalmove==1;
        ++counters.cut_index[depth][index_bucket(legalmove)];
        counters.cut_move_sum[depth]+=legalmove;
        ++counters.cut_by_check[incheck][first];
        ++counters.cut_phase[depth][phase];
        counters.cut_wasted[depth]+=wasted;counters.cut_nodes[depth]+=total;
        counters.phase_wasted[depth][phase]+=wasted;
        ++counters.cut_by_hash[depth][has_hash][first];
        counters.hash_wasted[depth][has_hash]+=wasted;counters.hash_nodes[depth][has_hash]+=total;
        ++counters.cut_by_side[white][first];
        if(phase==quiet) {
            ++counters.quiet_rank[white][rank_bucket(rank)];
            ++counters.quiet_pos_cut[position_index(rank)];
            counters.quiet_zero_score+=score==0;counters.quiet_list_moves+=list;
        }
    }
    void finish(int g, int alpha, int beta, int legalmove) {
        if(legalmove==0)return;
        const int actual=g>=beta?1:g<=alpha?2:0;
        ++counters.outcome[expected][actual];
        if(actual==0 && best>0)++counters.pv_best_index[index_bucket(best)];
    }
};

// One searchroot call: an iteration or an aspiration re-search.
class RootNode {
    std::uint32_t start=nodes(), first_end=0;
    int best=0;
public:
    void move(int legalmove) {if(legalmove==2)first_end=nodes()-1;}
    void research() {++counters.root_research;}
    void research_result(bool raised) {counters.root_raised+=raised;}
    void raised(int legalmove) {best=legalmove;counters.root_best_changes+=legalmove>1;}
    void finish() {
        ++counters.root_calls;
        if(best>0)++counters.root_best_index[index_bucket(best)];else ++counters.root_fail_low;
        const std::uint32_t end=nodes();
        counters.root_nodes+=end-start;counters.root_nodes_first+=(first_end?first_end:end)-start;
    }
};

// Quiescence nodes not in check; searched counts moves actually searched.
inline void q_cutoff(bool incheck, int searched) {
    if(incheck)return;
    ++counters.q_nodes;++counters.q_cut_index[searched<4?searched-1:3];
}
inline void q_done(bool incheck, int searched) {if(!incheck && searched>0)++counters.q_nodes;}

inline void put(std::string& s, u64 v) {s+=std::to_string(v);}
template<class T, std::size_t N> void put(std::string& s, const T (&a)[N]) {
    s+='[';
    for(std::size_t i=0;i<N;++i) {if(i)s+=',';put(s,a[i]);}
    s+=']';
}
inline std::string report() {
    const auto& c=counters;
    std::string s="orderstats {";
    bool first=true;
    auto add=[&](const char* name, const auto& value) {
        if(!first)s+=',';
        first=false;s+='"';s+=name;s+="\":";put(s,value);
    };
    add("outcome",c.outcome);add("cut_index",c.cut_index);add("cut_move_sum",c.cut_move_sum);
    add("cut_by_check",c.cut_by_check);add("pv_hash",c.pv_hash);add("cut_phase",c.cut_phase);
    add("tried_phase",c.tried_phase);add("cut_wasted",c.cut_wasted);add("cut_nodes",c.cut_nodes);
    add("phase_wasted",c.phase_wasted);
    add("cut_by_hash",c.cut_by_hash);add("hash_wasted",c.hash_wasted);add("hash_nodes",c.hash_nodes);
    add("cut_by_side",c.cut_by_side);add("quiet_rank",c.quiet_rank);add("quiet_tried_rank",c.quiet_tried_rank);
    add("quiet_zero_score",c.quiet_zero_score);add("quiet_list_moves",c.quiet_list_moves);
    add("quiet_pos_cut",c.quiet_pos_cut);add("quiet_pos_tried",c.quiet_pos_tried);
    add("pv_best_index",c.pv_best_index);add("reduced",c.reduced);
    add("lmr_research",c.lmr_research);add("lmr_held",c.lmr_held);
    add("pvs_research",c.pvs_research);add("pvs_raised",c.pvs_raised);
    add("root_calls",c.root_calls);add("root_fail_low",c.root_fail_low);
    add("root_best_index",c.root_best_index);add("root_best_changes",c.root_best_changes);
    add("root_nodes_first",c.root_nodes_first);add("root_nodes",c.root_nodes);
    add("root_research",c.root_research);add("root_raised",c.root_raised);
    add("q_cut_index",c.q_cut_index);add("q_nodes",c.q_nodes);
    s+='}';
    counters={};
    return s;
}
}
#else
#define PETIR_ORDER_STAT(...)
#endif
