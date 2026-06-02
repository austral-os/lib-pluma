# Class `pluma::PieceTable`

**The core text storage engine utilizing the piece table data structure.**

## Public Methods
- `PieceTable(std::string original_text="")` - *Constructs a*
- `void insert(uint32_t offset, std::string_view text)` - *Inserts text at the specified logical offset.*
- `void remove(uint32_t offset, uint32_t length)` - *Removes a length of text starting at a logical offset.*
- `std::string getText() const` - *Retrieves the full current text.*
- `uint32_t getLength() const` - *Retrieves the total length of the document.*
- `std::shared_ptr< DocumentSnapshot > createSnapshot()` - *Creates a structural sharing snapshot of the current state.*
- `void restoreSnapshot(const std::shared_ptr< DocumentSnapshot > &snapshot)` - *Restores the piece table to a previously taken snapshot.*
- `const std::vector< Piece > & getPieces() const` - *Gets the raw internal piece array.*

