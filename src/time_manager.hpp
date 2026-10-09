#pragma once
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>

namespace petir {
struct ClockLimits {
    bool timed=false;
    std::int64_t remaining_ms=0,increment_ms=0,movetime_ms=-1;
    int moves_to_go=0,overhead_ms=50;
};
struct TimeBudget {
    bool timed=false,emergency=false;
    std::int64_t soft_ms=0,hard_ms=0;
};
class TimeManager {
    std::atomic<std::int64_t> origin_{0},soft_deadline_{0},hard_deadline_{0},base_soft_{0};
    unsigned polls_=0; // Search thread only; protocol updates use atomics.
public:
    static std::int64_t now_ms() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }
    static TimeBudget allocate(const ClockLimits& limits);
    void reset() {origin_=now_ms();soft_deadline_=0;hard_deadline_=0;base_soft_=0;polls_=0;}
    void start(const TimeBudget& budget,std::int64_t received_at);
    void tighten(std::int64_t received_at,std::int64_t remaining_ms,int overhead_ms);
    std::int64_t elapsed_ms() const {return std::max<std::int64_t>(0,now_ms()-origin_.load());}
    bool timed() const {return hard_deadline_.load(std::memory_order_relaxed)!=0;}
    bool expired(bool force=false) {
        const auto end=hard_deadline_.load(std::memory_order_relaxed);
        if(!end)return false;
        const unsigned interval=base_soft_.load(std::memory_order_relaxed)<1000?16:256;
        if(!force && (++polls_%interval)!=0)return false;
        return now_ms()>=end;
    }
    bool can_start_iteration(std::int64_t last_iteration_ms) const;
    void adjust_soft(double fraction);
};
extern TimeManager search_time;
bool search_should_stop(bool force=false);
}
