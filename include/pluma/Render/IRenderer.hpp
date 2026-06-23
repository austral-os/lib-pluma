/**
 * @file IRenderer.hpp
 * @brief Defines the abstract interface for all backend renderers.
 */
#pragma once

#include <memory>
#include <pluma/CoreTypes.hpp>
#include <pluma/Typography/Glyph.hpp>
#include <pluma/Style/StyleProperties.hpp>
#include <pluma/Typography/IFontManager.hpp>

namespace pluma {

/**
 * @class IRenderer
 * @brief Hardware/Software agnostic rendering interface.
 */
class IRenderer {
public:
    virtual ~IRenderer() = default;

    /**
     * @brief Draws a solid filled rectangle.
     * @param rect Geometry in twips.
     * @param color ARGB color format.
     */
    virtual void drawRect(const Rect& rect, Color color) = 0;

    /**
     * @brief Draws a line with a given thickness and style.
     * @param start Starting point in twips.
     * @param end Ending point in twips.
     * @param thickness Line thickness in twips.
     * @param color ARGB color format.
     * @param style_index Style (0: solid, 1: dashed, 2: dotted, 3: dash-dot, 4: dash-dot-dot, 5: double)
     */
    virtual void drawLine(const Point& start, const Point& end, Twips thickness, Color color, int style_index) = 0;

    /**
     * @brief Draws a shaped run of text glyphs.
     * @param rect The bounding rect of the text.
     * @param run The shaped text run.
     * @param text The logical text string.
     * @param font The resolved font used for shaping.
     * @param color The text color.
     */
    virtual void drawGlyphRun(const Rect& rect, const ShapedTextRun& run, const std::string& text, std::shared_ptr<IFont> font, Color color) = 0;

    /**
     * @brief Draws an image.
     * @param rect The bounding rect of the image.
     * @param path The path or URI to the image.
     */
    virtual void drawImage(const Rect& rect, const std::string& path) = 0;

    /**
     * @brief Restricts subsequent drawing operations to the given rectangle.
     * @param rect The clipping geometry.
     */
    virtual void pushClip(const Rect& rect) = 0;

    /**
     * @brief Removes the most recently applied clipping rectangle.
     */
    virtual void popClip() = 0;
};

} // namespace pluma
