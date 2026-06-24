#include <iostream>
#include <unordered_map>
#include <mutex>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <pluma/Render/CairoRenderer.hpp>
#include <pluma/Diagnostics/Profiler.hpp>

namespace pluma {

CairoRenderer::CairoRenderer(cairo_t* cr) : cr_(cr) {}

double CairoRenderer::twipsToPixels(Twips twips) const {
    // 1440 twips = 1 inch
    // 96 pixels = 1 inch
    // 1 pixel = 15 twips
    return static_cast<double>(twips.getValue()) / 15.0;
}

void CairoRenderer::setCairoColor(Color color) {
    double a = ((color >> 24) & 0xFF) / 255.0;
    double r = ((color >> 16) & 0xFF) / 255.0;
    double g = ((color >> 8) & 0xFF) / 255.0;
    double b = (color & 0xFF) / 255.0;
    cairo_set_source_rgba(cr_, r, g, b, a);
}

void CairoRenderer::drawRect(const Rect& rect, Color color) {
    setCairoColor(color);
    cairo_rectangle(cr_, 
                    twipsToPixels(rect.x), 
                    twipsToPixels(rect.y), 
                    twipsToPixels(rect.width), 
                    twipsToPixels(rect.height));
    cairo_fill(cr_);
}

void CairoRenderer::drawLine(const Point& start, const Point& end, Twips thickness, Color color, int style_index) {
    setCairoColor(color);
    cairo_set_line_width(cr_, twipsToPixels(thickness));
    cairo_set_line_cap(cr_, CAIRO_LINE_CAP_SQUARE);

    if (style_index == 1) { // Dashed
        double dashes[] = {4.0, 4.0};
        cairo_set_dash(cr_, dashes, 2, 0);
    } else if (style_index == 2) { // Dotted
        double dashes[] = {1.0, 3.0};
        cairo_set_dash(cr_, dashes, 2, 0);
    } else if (style_index == 3) { // Dash-dot
        double dashes[] = {4.0, 2.0, 1.0, 2.0};
        cairo_set_dash(cr_, dashes, 4, 0);
    } else if (style_index == 4) { // Dash-dot-dot
        double dashes[] = {4.0, 2.0, 1.0, 2.0, 1.0, 2.0};
        cairo_set_dash(cr_, dashes, 6, 0);
    } else if (style_index == 5) { // Double (stub: draw thicker for now, ideally 2 lines)
        cairo_set_line_width(cr_, twipsToPixels(thickness) * 2);
        cairo_set_dash(cr_, nullptr, 0, 0);
    } else {
        cairo_set_dash(cr_, nullptr, 0, 0);
    }

    cairo_move_to(cr_, twipsToPixels(start.x), twipsToPixels(start.y));
    cairo_line_to(cr_, twipsToPixels(end.x), twipsToPixels(end.y));
    cairo_stroke(cr_);
    
    // Reset dash
    cairo_set_dash(cr_, nullptr, 0, 0);
}

void CairoRenderer::drawGlyphRun(const Rect& rect, const ShapedTextRun& run, const std::string& text, std::shared_ptr<IFont> font, Color color) {
    (void)run;
    (void)font;
    
    setCairoColor(color);
    
    cairo_save(cr_);
    
    // Fallback simple text rendering
    std::string family = font->getDescriptor().family;
    if (family.empty()) family = "sans-serif";
    
    cairo_font_weight_t weight = CAIRO_FONT_WEIGHT_NORMAL;
    if (static_cast<uint16_t>(font->getDescriptor().weight) >= 700) {
        weight = CAIRO_FONT_WEIGHT_BOLD;
    }
    
    cairo_font_slant_t slant = CAIRO_FONT_SLANT_NORMAL;
    if (font->getDescriptor().italic) {
        slant = CAIRO_FONT_SLANT_ITALIC;
    }
    
    cairo_select_font_face(cr_, family.c_str(), slant, weight);
    cairo_set_font_size(cr_, font->getDescriptor().size_pt); 

    if (font->getDescriptor().italic) {
        cairo_matrix_t font_matrix;
        cairo_get_font_matrix(cr_, &font_matrix);
        // Synthesize italic by skewing the font matrix if the font doesn't provide it natively
        font_matrix.xy = -0.2 * font_matrix.yy;
        cairo_set_font_matrix(cr_, &font_matrix);
    }

    cairo_move_to(cr_, twipsToPixels(rect.x), twipsToPixels(rect.y) + twipsToPixels(run.max_ascent));
    cairo_show_text(cr_, text.c_str());

    cairo_restore(cr_);
}

static cairo_user_data_key_t stbi_data_key;
static std::unordered_map<std::string, cairo_surface_t*> g_image_cache;
static std::mutex g_cache_mutex;

void CairoRenderer::drawImage(const Rect& rect, const std::string& path) {
    PLUMA_PROFILE_SCOPE("CairoRenderer::drawImage");
    cairo_save(cr_);
    double x = twipsToPixels(rect.x);
    double y = twipsToPixels(rect.y);
    double w = twipsToPixels(rect.width);
    double h = twipsToPixels(rect.height);
    
    cairo_surface_t* image = nullptr;
    
    {
        std::lock_guard<std::mutex> lock(g_cache_mutex);
        auto it = g_image_cache.find(path);
        if (it != g_image_cache.end()) {
            image = it->second;
            cairo_surface_reference(image);
        }
    }
    
    if (!image) {
        int img_w = 0, img_h = 0, channels = 0;
        unsigned char* data = stbi_load(path.c_str(), &img_w, &img_h, &channels, 4);
        
        if (data) {
            // Cairo expects pre-multiplied ARGB32 (in native endianness, so BGRA in memory on Little Endian)
            for (int i = 0; i < img_w * img_h * 4; i += 4) {
                unsigned char r = data[i + 0];
                unsigned char g = data[i + 1];
                unsigned char b = data[i + 2];
                unsigned char a = data[i + 3];
                
                // Pre-multiply alpha
                if (a != 255) {
                    r = (r * a) / 255;
                    g = (g * a) / 255;
                    b = (b * a) / 255;
                }
                
                // BGRA format for Cairo ARGB32
                data[i + 0] = b;
                data[i + 1] = g;
                data[i + 2] = r;
                data[i + 3] = a;
            }
            
            int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, img_w);
            image = cairo_image_surface_create_for_data(data, CAIRO_FORMAT_ARGB32, img_w, img_h, stride);
            
            // Attach the data buffer to the surface so it gets freed when the surface is destroyed
            cairo_surface_set_user_data(image, &stbi_data_key, data, (cairo_destroy_func_t)stbi_image_free);
            
            std::lock_guard<std::mutex> lock(g_cache_mutex);
            g_image_cache[path] = image;
            cairo_surface_reference(image); // Keep a reference for the cache
        }
    }
    
    if (image && cairo_surface_status(image) == CAIRO_STATUS_SUCCESS) {
        cairo_translate(cr_, x, y);
        int img_w = cairo_image_surface_get_width(image);
        int img_h = cairo_image_surface_get_height(image);
        cairo_scale(cr_, w / (double)img_w, h / (double)img_h);
        
        cairo_set_source_surface(cr_, image, 0, 0);
        
        // Durante drag: usar filtro rápido para mejorar el rendimiento de render.
        // En calidad normal: usar CAIRO_FILTER_GOOD para suavizado apropiado.
        cairo_pattern_t* pat = cairo_get_source(cr_);
        cairo_pattern_set_filter(pat, draft_quality_ ? CAIRO_FILTER_FAST : CAIRO_FILTER_GOOD);
        
        cairo_paint(cr_);
    } else {
        // Fallback placeholder if image fails to load
        cairo_set_source_rgb(cr_, 0.9, 0.9, 0.9);
        cairo_rectangle(cr_, x, y, w, h);
        cairo_fill_preserve(cr_);
        cairo_set_source_rgb(cr_, 0.3, 0.3, 0.3);
        cairo_set_line_width(cr_, 1.0);
        cairo_stroke(cr_);
        cairo_move_to(cr_, x, y);
        cairo_line_to(cr_, x + w, y + h);
        cairo_move_to(cr_, x + w, y);
        cairo_line_to(cr_, x, y + h);
        cairo_stroke(cr_);
    }
    
    if (image) {
        cairo_surface_destroy(image);
    }
    
    cairo_restore(cr_);
}

void CairoRenderer::pushClip(const Rect& rect) {
    cairo_save(cr_);
    cairo_rectangle(cr_, 
                    twipsToPixels(rect.x), 
                    twipsToPixels(rect.y), 
                    twipsToPixels(rect.width), 
                    twipsToPixels(rect.height));
    cairo_clip(cr_);
}

void CairoRenderer::popClip() {
    cairo_restore(cr_);
}

} // namespace pluma
