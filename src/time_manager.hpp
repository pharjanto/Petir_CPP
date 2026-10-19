#pragma once
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>

namespace petir {
struct ClockLimits {
    bool timed=false;
    std::int64_t remainingMs=0,incrementMs=0,moveTimeMs=-1;
    int movesToGo=0,overheadMs=15;
};
struct TimeBudget {
    bool timed=false,emergency=false;
    std::int64_t softMs=0,hardMs=0;
};
class TimeManager {
    std::atomic<std::int64_t> originMs{0},softDeadlineMs{0},hardDeadlineMs{0},baseSoftMs{0};
    unsigned pollCount=0; // Search thread only; protocol updates use atomics.
public:
    static std::int64_t nowMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }
    static TimeBudget allocate(const ClockLimits& limits);
    void reset() {originMs=nowMs();softDeadlineMs=0;hardDeadlineMs=0;baseSoftMs=0;pollCount=0;}
    void start(const TimeBudget& budget,std::int64_t receivedAt);
    void tighten(std::int64_t receivedAt,std::int64_t remainingMs,int overheadMs);
    std::int64_t elapsedMs() const {return std::max<std::int64_t>(0,nowMs()-originMs.load());}
    bool isTimed() const {return hardDeadlineMs.load(std::memory_order_relaxed)!=0;}
    bool expired(bool force=false) {
        const auto endMs=hardDeadlineMs.load(std::memory_order_relaxed);
        if(!endMs)return false;
        const unsigned pollInterval=baseSoftMs.load(std::memory_order_relaxed)<1000?16:256;
        if(!force && (++pollCount%pollInterval)!=0)return false;
        return nowMs()>=endMs;
    }
    bool canStartIteration(std::int64_t lastIterationMs) const;
    void adjustSoft(double fraction);
};
extern TimeManager searchTime;
bool searchShouldStop(bool force=false);
}
