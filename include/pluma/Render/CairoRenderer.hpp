/**
 * @file CairoRenderer.hpp
 * @brief Defines the concrete renderer implementation using the Cairo graphics library.
 */
#pragma once

#include <cairo.h>
#include <pluma/Render/IRenderer.hpp>

namespace pluma {

/**
 * @class CairoRenderer
 * @brief Implements IRenderer using Cairo.
 */
class CairoRenderer : public IRenderer {
public:
    /**
     * @brief Constructs a CairoRenderer bound to an existing cairo context.
     * @param cr The cairo context.
     */
    explicit CairoRenderer(cairo_t* cr);

    void drawRect(const Rect& rect, Color color) override;
    void drawLine(const Point& start, const Point& end, Twips thickness, Color color, int style_index) override;
    void drawGlyphRun(const Rect& rect, const ShapedTextRun& run, const std::string& text, std::shared_ptr<IFont> font, Color color) override;
    void drawImage(const Rect& rect, const std::string& path) override;
    void pushClip(const Rect& rect) override;
    void popClip() override;

    /**
     * @brief Activa o desactiva el modo de calidad borrador.
     * En modo borrador se usa CAIRO_FILTER_FAST para el escalado de imágenes,
     * lo cual es mucho más rápido durante operaciones de drag.
     */
    void setDraftQuality(bool draft) { draft_quality_ = draft; }

private:
    cairo_t* cr_;
    bool draft_quality_{false}; ///< Si es true, usa filtro rápido para imágenes durante drag

    /**
     * @brief Converts twips to standard user-space pixels.
     * Assuming 96 DPI, 1 pixel = 15 twips.
     */
    double twipsToPixels(Twips twips) const;
    void setCairoColor(Color color);
};

} // namespace pluma
