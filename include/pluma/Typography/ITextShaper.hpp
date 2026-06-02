/**
 * @file ITextShaper.hpp
 * @brief Interface for text shaping, abstracting HarfBuzz.
 */
#pragma once

#include <string_view>
#include <memory>
#include <pluma/Typography/Glyph.hpp>
#include <pluma/Typography/IFontManager.hpp>

namespace pluma {

/**
 * @class ITextShaper
 * @brief Interface for converting text into shaped glyph runs.
 */
class ITextShaper {
public:
    virtual ~ITextShaper() = default;

    /**
     * @brief Shapes a logical text string into a renderable glyph run.
     * @param text The UTF-8 text string to shape.
     * @param font The resolved font to use for shaping.
     * @return The shaped text run containing layout metrics and glyphs.
     */
    virtual ShapedTextRun shapeText(std::string_view text, const std::shared_ptr<IFont>& font) = 0;
};

} // namespace pluma
