#include "petir.hpp"

namespace petir {
TimeManager search_time;
TimeBudget TimeManager::allocate(const ClockLimits& limits) {
    if(!limits.timed)return {};
    const auto overhead=std::int64_t(std::max(0,limits.overhead_ms));
    if(limits.movetime_ms>=0) {
        const auto available=std::max<std::int64_t>(0,limits.movetime_ms-overhead);
        return {true,available<=2,available,available};
    }
    const auto remaining=std::max<std::int64_t>(0,limits.remaining_ms);
    const auto available=std::max<std::int64_t>(0,remaining-overhead);
    const auto horizon=std::int64_t(limits.moves_to_go>0?std::clamp(limits.moves_to_go,1,100):30);
    // Bank future response overhead, especially in long games without increment.
    const auto bank=std::min(remaining/2,overhead*(horizon+1));
    const auto distributable=std::max<std::int64_t>(0,remaining-bank);
    const auto increment=std::max<std::int64_t>(0,limits.increment_ms);
    const auto soft=std::min(available,std::max<std::int64_t>(1,distributable/horizon+increment*4/5));
    const auto hard=std::min(available,soft*3);
    return {true,available<=5,soft,hard};
}
void TimeManager::start(const TimeBudget& budget,std::int64_t received_at) {
    origin_=received_at;base_soft_=budget.soft_ms;
    soft_deadline_=budget.timed?received_at+budget.soft_ms:0;
    hard_deadline_=budget.timed?received_at+budget.hard_ms:0;
}
void TimeManager::tighten(std::int64_t received_at,std::int64_t remaining_ms,int overhead_ms) {
    const auto cap=received_at+std::max<std::int64_t>(0,remaining_ms-std::max(0,overhead_ms));
    auto end=hard_deadline_.load();
    while(end && end>cap && !hard_deadline_.compare_exchange_weak(end,cap)) {}
    auto soft=soft_deadline_.load();
    while(soft && soft>cap && !soft_deadline_.compare_exchange_weak(soft,cap)) {}
}
bool TimeManager::can_start_iteration(std::int64_t last_iteration_ms) const {
    const auto hard=hard_deadline_.load();
    if(!hard)return true;
    const auto now=now_ms();
    return now<soft_deadline_.load() && now+std::max<std::int64_t>(1,last_iteration_ms*2)<hard;
}
void TimeManager::adjust_soft(double fraction) {
    const auto hard=hard_deadline_.load();
    if(!hard)return;
    const auto delta=static_cast<std::int64_t>(base_soft_.load()*fraction);
    auto soft=soft_deadline_.load();
    while(!soft_deadline_.compare_exchange_weak(soft,std::clamp(soft+delta,std::min(hard,origin_.load()+1),hard))) {}
}
bool search_should_stop(bool force) {
    if(petir_header::stop_process.load(std::memory_order_relaxed))return true;
    if(!search_time.expired(force))return false;
    petir_header::stop_process.store(true,std::memory_order_relaxed);
    return true;
}
}
