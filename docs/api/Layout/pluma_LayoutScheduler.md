# Class `pluma::LayoutScheduler`

**Schedules and executes layout tasks asynchronously with priority and cancellation.**

## Public Methods
- `uint32_t scheduleTask(TaskPriority priority, std::function< void()> work)` - *Submits a new layout task for execution.*
- `void cancelTask(uint32_t task_id)` - *Cancels a pending task.*
- `void processAll()` - *Synchronously processes all pending tasks (for testing).*
- `bool hasPendingTasks() const` - *Checks if there are any tasks waiting to be processed.*

