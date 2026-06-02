# Class `pluma::plugins::IImporter`

**Base interface for all format importers.**

## Public Methods
- `~IImporter()=default`
- `bool import(const std::string &data, PlumaEditor &editor)=0` - *Parses an external format and loads it into the editor.*

