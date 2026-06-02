/**
 * @file LayoutScheduler.hpp
 * @brief Defines the LayoutScheduler for managing asynchronous partial reflows.
 */
#pragma once

#include <cstdint>
#include <functional>
#include <queue>
#include <mutex>
#include <unordered_set>

namespace pluma {

/**
 * @enum TaskPriority
 * @brief Priority levels for layout scheduling.
 */
enum class TaskPriority {
    High,   ///< Immediate viewport reflow, user blocking.
    Medium, ///< Pre-fetching nearby pages.
    Low     ///< Background layout, full document measuring.
};

/**
 * @struct LayoutTask
 * @brief A unit of work for the LayoutScheduler.
 */
struct LayoutTask {
    uint32_t id;                       ///< Unique task identifier.
    TaskPriority priority;             ///< Execution priority.
    std::function<void()> work;        ///< The actual layout computation.

    /**
     * @brief Comparator for priority queues (lower enum value = higher priority).
     */
    bool operator<(const LayoutTask& other) const {
        // We want High (0) to come before Medium (1)
        return priority > other.priority;
    }
};

/**
 * @class LayoutScheduler
 * @brief Schedules and executes layout tasks asynchronously with priority and cancellation.
 */
class LayoutScheduler {
public:
    /**
     * @brief Submits a new layout task for execution.
     * @param priority The priority of the task.
     * @param work The functor to execute.
     * @return The unique ID of the scheduled task.
     */
    uint32_t scheduleTask(TaskPriority priority, std::function<void()> work);

    /**
     * @brief Cancels a pending task.
     * @param task_id The ID of the task to cancel.
     */
    void cancelTask(uint32_t task_id);

    /**
     * @brief Synchronously processes all pending tasks (for testing).
     */
    void processAll();

    /**
     * @brief Checks if there are any tasks waiting to be processed.
     * @return true if there are pending tasks.
     */
    bool hasPendingTasks() const;

private:
    std::priority_queue<LayoutTask> queue_;
    std::unordered_set<uint32_t> cancelled_tasks_;
    uint32_t next_id_ = 1;
};

} // namespace pluma
