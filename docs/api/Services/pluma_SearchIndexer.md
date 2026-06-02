# Class `pluma::SearchIndexer`

**Simple reverse-index service for fast document searching.**

## Public Methods
- `void analyze(std::shared_ptr< DocumentSnapshot > snapshot) override` - *Performs analysis on a stable snapshot.*
- `std::string getName() const override` - *Returns the internal name of the service.*
- `int getWordFrequency(const std::string &word)` - *Retrieves the occurrence frequency of a given word.*

