#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <pluma/Render/CairoRenderer.hpp>

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

void CairoRenderer::drawImage(const Rect& rect, const std::string& path) {
    cairo_save(cr_);
    double x = twipsToPixels(rect.x);
    double y = twipsToPixels(rect.y);
    double w = twipsToPixels(rect.width);
    double h = twipsToPixels(rect.height);
    
    int img_w = 0, img_h = 0, channels = 0;
    unsigned char* data = stbi_load(path.c_str(), &img_w, &img_h, &channels, 4);
    cairo_surface_t* image = nullptr;
    
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
    }
    
    if (image && cairo_surface_status(image) == CAIRO_STATUS_SUCCESS) {
        cairo_translate(cr_, x, y);
        cairo_scale(cr_, w / (double)img_w, h / (double)img_h);
        
        cairo_set_source_surface(cr_, image, 0, 0);
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
    if (data) {
        stbi_image_free(data);
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
