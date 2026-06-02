#include <pluma/Services/ServiceManager.hpp>
#include <algorithm>

namespace pluma {

void ServiceManager::registerService(std::shared_ptr<IDocumentService> service) {
    if (service) {
        services_.push_back(std::move(service));
    }
}

void ServiceManager::runAnalysis(std::shared_ptr<DocumentSnapshot> snapshot) {
    // Clean up finished tasks
    active_tasks_.erase(
        std::remove_if(active_tasks_.begin(), active_tasks_.end(),
            [](const std::future<void>& f) { return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready; }),
        active_tasks_.end()
    );

    // Launch new tasks for all services using the shared immutable snapshot
    for (auto& service : services_) {
        active_tasks_.push_back(std::async(std::launch::async, [service, snapshot]() {
            service->analyze(snapshot);
        }));
    }
}

void ServiceManager::waitForAll() {
    for (auto& task : active_tasks_) {
        if (task.valid()) {
            task.wait();
        }
    }
    active_tasks_.clear();
}

} // namespace pluma
