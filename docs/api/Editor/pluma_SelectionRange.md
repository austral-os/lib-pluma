# Struct `pluma::SelectionRange`

**Represents a logical selection within the document text.**

## Public Methods
- `bool isCollapsed() const` - *Determines if the selection is empty (a simple caret).*
- `uint32_t getStart() const` - *Gets the starting logical offset of the selection.*
- `uint32_t getEnd() const` - *Gets the ending logical offset of the selection.*
- `uint32_t getLength() const` - *Gets the length of the selected text.*

