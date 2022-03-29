#include <misc/dune_timer_resolution.h>

#if defined(_WIN32)
#    include <timeapi.h>
#endif

namespace dune {

#if defined(_WIN32)

DuneTimerResolution::DuneTimerResolution(uint32_t target_ms) : target_ms_(limit_target_ms(target_ms)) {
    if (target_ms_ == std::numeric_limits<uint32_t>::max())
        return;

    resume();
}

DuneTimerResolution::~DuneTimerResolution() {
    suspend();
}

void DuneTimerResolution::suspend() {
    if (!active_)
        return;

    const auto time_end_period = timeEndPeriod(target_ms_);
    if (TIMERR_NOERROR != time_end_period)
        sdl2::log_error("Unable to cleanup multimedia timer (%d)", time_end_period);

    active_ = false;
}

void DuneTimerResolution::resume() {
    if (active_)
        return;

    active_ = TIMERR_NOERROR == timeBeginPeriod(target_ms_);
}

uint32_t DuneTimerResolution::limit_target_ms(uint32_t target_ms) {
    TIMECAPS tc;

    if (TIMERR_NOERROR != timeGetDevCaps(&tc, sizeof tc))
        return std::numeric_limits<uint32_t>::max();

    if (target_ms < tc.wPeriodMin)
        target_ms = tc.wPeriodMin;
    else if (target_ms > tc.wPeriodMax)
        target_ms = tc.wPeriodMax;

    return target_ms;
}

#endif // defined(_WIN32)

} // namespace dune
