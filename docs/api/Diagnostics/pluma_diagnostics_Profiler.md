# Class `pluma::diagnostics::Profiler`

**A thread-safe global registry for aggregating profiling measurements.**

## Public Methods
- `void record(const std::string &event_name, double duration_ms)` - *Records a timing measurement for a specific event.*
- `std::string getReport() const` - *Generates a comprehensive report of all recorded measurements.*
- `void clear()` - *Clears all recorded statistics.*

