# Class `pluma::plugins::PdfExporter`

**Uses Cairo PDF surfaces to generate a PDF file from the editor's display list.**

## Public Methods
- `std::string exportDoc(std::shared_ptr< DocumentSnapshot > snapshot) override` - *Serializes a document snapshot into an external format string.*
- `bool exportToFile(const std::string &filename, PlumaEditor &editor) override` - *Exports the document directly to a file (useful for binary formats like PDF).*

