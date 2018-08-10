/**
 * S3 Storage Plugin - A plugin for Orthanc DICOM Server for storing
 * DICOM data in Amazon Simple Storage Service (AWS S3).
 *
 * Copyright (C) 2018 (Radpoint Sp. z o.o., Poland)
 * Marek Kwasecki, Bartłomiej Pyciński
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 **/

#ifndef TIMER_H
#define TIMER_H

#include <boost/core/noncopyable.hpp>

#include <chrono>
#include <atomic>
#include <type_traits>
#include <iostream>

namespace OrthancPlugins {

template <typename Clock = std::chrono::high_resolution_clock>
    class stopwatch
    {
        const typename Clock::time_point start_point;
    public:
        stopwatch() :
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

}

#endif // TIMER_H
