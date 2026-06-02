#include <pluma/Typography/DummyTypography.hpp>

namespace pluma {

// DummyFont
DummyFont::DummyFont(FontDescriptor desc) : desc_(std::move(desc)) {}

FontDescriptor DummyFont::getDescriptor() const {
    return desc_;
}

// DummyFontManager
std::shared_ptr<IFont> DummyFontManager::getFont(const FontDescriptor& desc) {
    return std::make_shared<DummyFont>(desc);
}

// DummyTextShaper
ShapedTextRun DummyTextShaper::shapeText(std::string_view text, const std::shared_ptr<IFont>& font) {
    ShapedTextRun run;
    run.max_ascent = Twips(static_cast<int32_t>(font->getDescriptor().size_pt * 20.0f * 0.8f));
    run.max_descent = Twips(static_cast<int32_t>(font->getDescriptor().size_pt * 20.0f * 0.2f));
    
    Twips current_x(0);
    Twips char_width(static_cast<int32_t>(font->getDescriptor().size_pt * 10.0f)); // dummy 0.5em width

    for (size_t i = 0; i < text.length(); ++i) {
        Glyph g;
        g.codepoint = static_cast<uint32_t>(text[i]);
        g.x_advance = char_width;
        g.y_advance = Twips(0);
        g.x_offset = Twips(0);
        g.y_offset = Twips(0);
        g.cluster = static_cast<uint32_t>(i);
        
        run.glyphs.push_back(g);
        current_x = current_x + char_width;
    }

    run.total_width = current_x;
    return run;
}

} // namespace pluma
