# Class `pluma::ServiceManager`

**Orchestrates background services safely feeding them immutable snapshots.**

## Public Methods
- `void registerService(std::shared_ptr< IDocumentService > service)` - *Registers a new service to be run upon request.*
- `void runAnalysis(std::shared_ptr< DocumentSnapshot > snapshot)` - *Asynchronously dispatches the snapshot to all registered services.*
- `void waitForAll()` - *Waits for all currently running analyses to complete.*

