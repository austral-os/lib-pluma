/**
 * @file Glyph.hpp
 * @brief Defines the basic structures for glyphs and shaped text runs.
 */
#pragma once

#include <cstdint>
#include <vector>
#include <pluma/CoreTypes.hpp>

namespace pluma {

/**
 * @struct Glyph
 * @brief Represents a single shaped glyph ready for rendering.
 */
struct Glyph {
    uint32_t codepoint; ///< The font-specific glyph index.
    Twips x_advance;    ///< The horizontal advance to the next glyph.
    Twips y_advance;    ///< The vertical advance to the next glyph.
    Twips x_offset;     ///< Horizontal offset for rendering.
    Twips y_offset;     ///< Vertical offset for rendering.
    uint32_t cluster;   ///< The logical character index this glyph corresponds to.
};

/**
 * @struct ShapedTextRun
 * @brief Represents a continuous sequence of shaped glyphs.
 */
struct ShapedTextRun {
    std::vector<Glyph> glyphs; ///< The shaped glyphs.
    Twips total_width;         ///< The total width of the run in Twips.
    Twips max_ascent;          ///< The maximum typographic ascent.
    Twips max_descent;         ///< The maximum typographic descent.
};

} // namespace pluma
