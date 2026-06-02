/**
 * @file Selection.hpp
 * @brief Defines the data structures for managing document selections.
 */
#pragma once

#include <cstdint>
#include <algorithm>

namespace pluma {

/**
 * @struct SelectionRange
 * @brief Represents a logical selection within the document text.
 */
struct SelectionRange {
    uint32_t anchor; ///< The fixed logical offset where the selection began.
    uint32_t head;   ///< The moving logical offset where the caret currently is.

    /**
     * @brief Determines if the selection is empty (a simple caret).
     * @return true if anchor and head are identical.
     */
    bool isCollapsed() const { return anchor == head; }

    /**
     * @brief Gets the starting logical offset of the selection.
     */
    uint32_t getStart() const { return std::min(anchor, head); }

    /**
     * @brief Gets the ending logical offset of the selection.
     */
    uint32_t getEnd() const { return std::max(anchor, head); }
    
    /**
     * @brief Gets the length of the selected text.
     */
    uint32_t getLength() const { return getEnd() - getStart(); }
};

} // namespace pluma
