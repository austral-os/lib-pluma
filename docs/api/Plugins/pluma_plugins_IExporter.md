# Class `pluma::plugins::IExporter`

**Base interface for all format exporters.**

## Public Methods
- `~IExporter()=default`
- `std::string exportDoc(std::shared_ptr< DocumentSnapshot > snapshot)=0` - *Serializes a document snapshot into an external format string.*
- `bool exportToFile(const std::string &filename, PlumaEditor &editor)` - *Exports the document directly to a file (useful for binary formats like PDF).*

