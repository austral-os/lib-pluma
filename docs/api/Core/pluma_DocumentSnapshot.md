# Class `pluma::DocumentSnapshot`

**Represents an immutable snapshot of the document's state at a specific revision.**

## Public Methods
- `DocumentSnapshot(RevisionId rev, std::vector< Piece > pieces, std::shared_ptr< const std::string > original_buffer, std::shared_ptr< const std::string > add_buffer)` - *Constructs a new*
- `RevisionId getRevision() const` - *Gets the revision ID of the snapshot.*
- `std::string getText() const` - *Generates the full text of the snapshot.*
- `uint32_t getLength() const` - *Gets the total length of the text in the snapshot.*
- `const std::vector< Piece > & getPieces() const` - *Gets the internal piece descriptors.*
- `std::shared_ptr< const std::string > getOriginalBuffer() const`
- `std::shared_ptr< const std::string > getAddBuffer() const`

