#include <pluma/Layout/LayoutScheduler.hpp>

namespace pluma {

uint32_t LayoutScheduler::scheduleTask(TaskPriority priority, std::function<void()> work) {
    uint32_t id = next_id_++;
    queue_.push({id, priority, std::move(work)});
    return id;
}

void LayoutScheduler::cancelTask(uint32_t task_id) {
    cancelled_tasks_.insert(task_id);
}

void LayoutScheduler::processAll() {
    while (!queue_.empty()) {
        LayoutTask task = std::move(const_cast<LayoutTask&>(queue_.top()));
        queue_.pop();

        if (cancelled_tasks_.find(task.id) == cancelled_tasks_.end()) {
            if (task.work) {
                task.work();
            }
        } else {
            cancelled_tasks_.erase(task.id);
        }
    }
}

bool LayoutScheduler::hasPendingTasks() const {
    // If the queue is not empty, there might be tasks (unless all are cancelled, 
    // but typically we just return !empty for a quick check)
    return !queue_.empty();
}

} // namespace pluma
