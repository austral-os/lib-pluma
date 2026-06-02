#include <pluma/Diagnostics/Profiler.hpp>
#include <sstream>
#include <iomanip>

namespace pluma {
namespace diagnostics {

Profiler& Profiler::getInstance() {
    static Profiler instance;
    return instance;
}

void Profiler::record(const std::string& event_name, double duration_ms) {
    std::lock_guard<std::mutex> lock(mutex_);
    cumulative_times_[event_name] += duration_ms;
    call_counts_[event_name] += 1;
}

std::string Profiler::getReport() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::stringstream ss;
    ss << "--- Pluma Profiler Report ---\n";
    ss << std::left << std::setw(30) << "Event Name" 
       << std::setw(15) << "Calls" 
       << std::setw(20) << "Total Time (ms)" 
       << "Avg Time (ms)\n";
    ss << std::string(80, '-') << "\n";
    
    for (const auto& [name, total_time] : cumulative_times_) {
        uint64_t counts = call_counts_.at(name);
        double avg = (counts > 0) ? (total_time / counts) : 0.0;
        
        ss << std::left << std::setw(30) << name 
           << std::setw(15) << counts 
           << std::setw(20) << total_time 
           << avg << "\n";
    }
    return ss.str();
}

void Profiler::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    cumulative_times_.clear();
    call_counts_.clear();
}

ScopedTimer::ScopedTimer(const std::string& event_name) 
    : event_name_(event_name), start_time_(std::chrono::high_resolution_clock::now()) {
}

ScopedTimer::~ScopedTimer() {
    auto end_time = std::chrono::high_resolution_clock::now();
    double duration_ms = std::chrono::duration<double, std::milli>(end_time - start_time_).count();
    Profiler::getInstance().record(event_name_, duration_ms);
}

} // namespace diagnostics
} // namespace pluma
