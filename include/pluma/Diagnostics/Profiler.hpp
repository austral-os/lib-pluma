/**
 * @file Profiler.hpp
 * @brief Global timing registry for tracking function performance.
 */
#pragma once
#include <string>
#include <chrono>
#include <unordered_map>
#include <mutex>

namespace pluma {
namespace diagnostics {

/**
 * @class Profiler
 * @brief A thread-safe global registry for aggregating profiling measurements.
 */
class Profiler {
public:
    /**
     * @brief Gets the singleton instance of the Profiler.
     * @return The Profiler instance.
     */
    static Profiler& getInstance();

    /**
     * @brief Records a timing measurement for a specific event.
     * @param event_name The name of the profiled event.
     * @param duration_ms The duration in milliseconds.
     */
    void record(const std::string& event_name, double duration_ms);

    /**
     * @brief Generates a comprehensive report of all recorded measurements.
     * @return A formatted string with aggregated profiling data.
     */
    std::string getReport() const;

    /**
     * @brief Clears all recorded statistics.
     */
    void clear();

private:
    Profiler() = default;
    
    mutable std::mutex mutex_;
    std::unordered_map<std::string, double> cumulative_times_;
    std::unordered_map<std::string, uint64_t> call_counts_;
};

/**
 * @class ScopedTimer
 * @brief RAII timer for automatic performance tracking of code blocks.
 */
class ScopedTimer {
public:
    /**
     * @brief Starts the timer for the given event name.
     * @param event_name Identifier for this timing block.
     */
    explicit ScopedTimer(const std::string& event_name);
    
    /**
     * @brief Stops the timer and records the result to the Profiler.
     */
    ~ScopedTimer();

private:
    std::string event_name_;
    std::chrono::high_resolution_clock::time_point start_time_;
};

} // namespace diagnostics
} // namespace pluma

/**
 * @def PLUMA_PROFILE_SCOPE
 * @brief Macro to quickly profile the current scope.
 */
#define PLUMA_PROFILE_SCOPE(name) ::pluma::diagnostics::ScopedTimer __timer_##__LINE__(name)
