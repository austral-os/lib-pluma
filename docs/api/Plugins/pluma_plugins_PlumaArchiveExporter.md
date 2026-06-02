# Class `pluma::plugins::PlumaArchiveExporter`

**Packages document text and metadata into a .pluma ZIP archive.**

## Public Methods
- `std::string exportDoc(std::shared_ptr< DocumentSnapshot > snapshot) override` - *Serializes a document snapshot into an external format string.*
- `bool exportToFile(const std::string &filename, PlumaEditor &editor) override` - *Exports the document directly to a file (useful for binary formats like PDF).*

