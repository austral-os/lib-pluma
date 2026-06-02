# Class `pluma::TextSlice`

**An immutable slice of text representing a specific logical offset.**

## Public Methods
- `TextSlice(std::string_view text, uint32_t logical_offset)` - *Constructs a*
- `std::string_view getText() const` - *Gets the text content of the slice.*
- `uint32_t getLogicalOffset() const` - *Gets the logical document offset.*
- `uint32_t getLength() const` - *Gets the length of the slice in bytes.*

