# Class `pluma::InvalidationTracker`

**Manages a collection of dirty regions, merging them as needed.**

## Public Methods
- `void markDirty(uint32_t offset, uint32_t length)` - *Marks a specific text span as dirty.*
- `void clear()` - *Clears all dirty regions after a successful reflow.*
- `bool hasDirtyRegions() const` - *Checks if there are any pending invalidations.*
- `const std::vector< DirtyRegion > & getRegions() const` - *Gets all currently tracked dirty regions.*

