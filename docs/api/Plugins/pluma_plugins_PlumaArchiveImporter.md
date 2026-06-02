# Class `pluma::plugins::PlumaArchiveImporter`

**Extracts document text and assets from a .pluma ZIP archive.**

## Public Methods
- `bool importFile(const std::string &filename, PlumaEditor &editor)` - *Reads a .pluma archive and loads it into the editor.*
- `bool import(const std::string &data, PlumaEditor &editor) override` - *Not supported for archives, returns false.*

