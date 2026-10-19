#include "petir.hpp"

namespace petir {
TimeManager searchTime;
TimeBudget TimeManager::allocate(const ClockLimits& limits) {
    if(!limits.timed)return {};
    const auto overheadMs=std::int64_t(std::max(0,limits.overheadMs));
    if(limits.moveTimeMs>=0) {
        const auto availableMs=std::max<std::int64_t>(0,limits.moveTimeMs-overheadMs);
        return {true,availableMs<=2,availableMs,availableMs};
    }
    const auto remainingMs=std::max<std::int64_t>(0,limits.remainingMs);
    const auto availableMs=std::max<std::int64_t>(0,remainingMs-overheadMs);
    const auto movesLeft=std::int64_t(limits.movesToGo>0?std::clamp(limits.movesToGo,1,100):30);
    // With a move-count control, plan for up to two extra moves so overruns
    // come out of a reserve instead of starving the moves just before the
    // control; the reserve tapers away so the last move gets the whole clock.
    const auto horizonMoves=limits.movesToGo>0?movesLeft+std::min<std::int64_t>(2,movesLeft-1):movesLeft;
    // Bank future response overhead, especially in long games without increment.
    const auto bankMs=std::min(remainingMs/2,overheadMs*(movesLeft+1));
    const auto distributableMs=std::max<std::int64_t>(0,remainingMs-bankMs);
    const auto incrementMs=std::max<std::int64_t>(0,limits.incrementMs);
    const auto softMs=std::min(availableMs,std::max<std::int64_t>(1,distributableMs/horizonMoves+incrementMs*4/5));
    // One search may overrun to 3x soft, but never past a third of the time
    // left after its soft share, so a few moves before the control cannot
    // spend the whole clock.
    const auto hardMs=std::min({availableMs,softMs*3,softMs+std::max<std::int64_t>(0,distributableMs-softMs)/3});
    return {true,availableMs<=5,softMs,hardMs};
}
void TimeManager::start(const TimeBudget& budget,std::int64_t receivedAt) {
    originMs=receivedAt;baseSoftMs=budget.softMs;
    softDeadlineMs=budget.timed?receivedAt+budget.softMs:0;
    hardDeadlineMs=budget.timed?receivedAt+budget.hardMs:0;
}
void TimeManager::tighten(std::int64_t receivedAt,std::int64_t remainingMs,int overheadMs) {
    const auto capMs=receivedAt+std::max<std::int64_t>(0,remainingMs-std::max(0,overheadMs));
    auto endMs=hardDeadlineMs.load();
    while(endMs && endMs>capMs && !hardDeadlineMs.compare_exchange_weak(endMs,capMs)) {}
    auto softMs=softDeadlineMs.load();
    while(softMs && softMs>capMs && !softDeadlineMs.compare_exchange_weak(softMs,capMs)) {}
}
bool TimeManager::canStartIteration(std::int64_t lastIterationMs) const {
    const auto hardMs=hardDeadlineMs.load();
    if(!hardMs)return true;
    const auto currentMs=nowMs();
    return currentMs<softDeadlineMs.load() && currentMs+std::max<std::int64_t>(1,lastIterationMs*2)<hardMs;
}
void TimeManager::adjustSoft(double fraction) {
    const auto hardMs=hardDeadlineMs.load();
    if(!hardMs)return;
    const auto deltaMs=static_cast<std::int64_t>(baseSoftMs.load()*fraction);
    auto softMs=softDeadlineMs.load();
    while(!softDeadlineMs.compare_exchange_weak(softMs,std::clamp(softMs+deltaMs,std::min(hardMs,originMs.load()+1),hardMs))) {}
}
bool searchShouldStop(bool force) {
    if(petir_header::stopSearch.load(std::memory_order_relaxed))return true;
    if(!searchTime.expired(force))return false;
    petir_header::stopSearch.store(true,std::memory_order_relaxed);
    return true;
}
}
