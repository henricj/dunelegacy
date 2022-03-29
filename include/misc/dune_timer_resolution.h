
#ifndef DUNE_TIMER_RESOLUTION_H
#define DUNE_TIMER_RESOLUTION_H

namespace dune {
#if defined(_WIN32)

class DuneTimerResolution final {
public:
    explicit DuneTimerResolution(uint32_t target_ms = 1);
    ~DuneTimerResolution();

    void suspend();
    void resume();

private:
    void restore();

    static uint32_t limit_target_ms(uint32_t target_ms);

    const uint32_t target_ms_;
    bool active_{};
};
#else  // defined(_WIN32)
class DuneTimerResolution {
public:
    explicit DuneTimerResolution(uint32_t target_ms = 1) { }

    void suspend() { }
    void resume() { }
};
#endif // defined(_WIN32)

} // namespace dune
#endif // DUNE_TIMER_RESOLUTION_H
