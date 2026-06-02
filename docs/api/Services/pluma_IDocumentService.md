# Class `pluma::IDocumentService`

**A service that performs asynchronous or decoupled analysis on a**

## Public Methods
- `~IDocumentService()=default`
- `void analyze(std::shared_ptr< DocumentSnapshot > snapshot)=0` - *Performs analysis on a stable snapshot.*
- `std::string getName() const =0` - *Returns the internal name of the service.*

