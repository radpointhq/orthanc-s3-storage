#ifndef TIMER_H
#define TIMER_H

#include <boost/core/noncopyable.hpp>

#include <chrono>
#include <atomic>
#include <type_traits>
#include <iostream>

namespace OrthancPlugins {

    template <typename Clock = std::chrono::high_resolution_clock>
    class StopwatchT
    {
        const typename Clock::time_point start_point;
    public:
        StopwatchT() :
            start_point(Clock::now())
        {}

        template <
            typename Units = typename std::chrono::microseconds,
            typename Rep = typename Clock::duration::rep>
        Rep elapsed() const
        {
            std::atomic_thread_fence(std::memory_order_relaxed);
            auto counted_time = std::chrono::duration_cast<Units>(Clock::now() - start_point);
            std::atomic_thread_fence(std::memory_order_relaxed);
            return counted_time.count();
        }
    };

    typedef StopwatchT<> Stopwatch;

}

#endif // TIMER_H
