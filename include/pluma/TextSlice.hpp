/**
 * @file TextSlice.hpp
 * @brief Defines the TextSlice class representing a logical view of a text span.
 */
#pragma once

#include <cstdint>
#include <string_view>

namespace pluma {

/**
 * @class TextSlice
 * @brief An immutable slice of text representing a specific logical offset.
 */
class TextSlice {
public:
    /**
     * @brief Constructs a TextSlice.
     * @param text The string view covering this slice.
     * @param logical_offset The logical offset where this slice begins in the document.
     */
    TextSlice(std::string_view text, uint32_t logical_offset)
        : text_(text), logical_offset_(logical_offset) {}

    /**
     * @brief Gets the text content of the slice.
     * @return A std::string_view containing the text.
     */
    std::string_view getText() const { return text_; }

    /**
     * @brief Gets the logical document offset.
     * @return The starting logical offset.
     */
    uint32_t getLogicalOffset() const { return logical_offset_; }

    /**
     * @brief Gets the length of the slice in bytes.
     * @return The length.
     */
    uint32_t getLength() const { return text_.length(); }

private:
    std::string_view text_;      ///< The text view.
    uint32_t logical_offset_;    ///< The document offset.
};

} // namespace pluma
