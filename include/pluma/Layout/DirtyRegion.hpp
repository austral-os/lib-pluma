/**
 * @file DirtyRegion.hpp
 * @brief Defines invalidation tracking for the incremental layout system.
 */
#pragma once

#include <cstdint>
#include <vector>

namespace pluma {

/**
 * @struct DirtyRegion
 * @brief Represents a continuous span of text offsets that have been invalidated.
 */
struct DirtyRegion {
    uint32_t start_offset; ///< The logical offset where invalidation begins.
    uint32_t length;       ///< The length of the invalidated span.

    /**
     * @brief Checks if this region overlaps with another.
     * @param other The other dirty region.
     * @return true if there is an overlap.
     */
    bool overlaps(const DirtyRegion& other) const {
        return (start_offset < other.start_offset + other.length) &&
               (other.start_offset < start_offset + length);
    }
};

/**
 * @class InvalidationTracker
 * @brief Manages a collection of dirty regions, merging them as needed.
 */
class InvalidationTracker {
public:
    /**
     * @brief Marks a specific text span as dirty.
     * @param offset The starting logical offset.
     * @param length The number of affected characters/bytes.
     */
    void markDirty(uint32_t offset, uint32_t length) {
        if (length == 0) return;
        regions_.push_back({offset, length});
        // A robust implementation would merge overlapping regions here.
    }

    /**
     * @brief Clears all dirty regions after a successful reflow.
     */
    void clear() {
        regions_.clear();
    }

    /**
     * @brief Checks if there are any pending invalidations.
     * @return true if there are dirty regions.
     */
    bool hasDirtyRegions() const {
        return !regions_.empty();
    }

    /**
     * @brief Gets all currently tracked dirty regions.
     * @return A read-only vector of DirtyRegion.
     */
    const std::vector<DirtyRegion>& getRegions() const {
        return regions_;
    }

private:
    std::vector<DirtyRegion> regions_;
};

} // namespace pluma
