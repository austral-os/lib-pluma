/**
 * @file ServiceManager.hpp
 * @brief Manages execution of registered background document services.
 */
#pragma once
#include <vector>
#include <memory>
#include <future>
#include <pluma/Services/IDocumentService.hpp>

namespace pluma {

/**
 * @class ServiceManager
 * @brief Orchestrates background services safely feeding them immutable snapshots.
 */
class ServiceManager {
public:
    /**
     * @brief Registers a new service to be run upon request.
     */
    void registerService(std::shared_ptr<IDocumentService> service);
    
    /**
     * @brief Asynchronously dispatches the snapshot to all registered services.
     * @param snapshot The immutable document state.
     */
    void runAnalysis(std::shared_ptr<DocumentSnapshot> snapshot);

    /**
     * @brief Waits for all currently running analyses to complete.
     */
    void waitForAll();
    
private:
    std::vector<std::shared_ptr<IDocumentService>> services_;
    std::vector<std::future<void>> active_tasks_;
};

} // namespace pluma
