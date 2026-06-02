#include <catch2/catch_test_macros.hpp>
#include <pluma/Typography/DummyTypography.hpp>

using namespace pluma;

TEST_CASE("Typography Pipeline dummy implementation", "[typography]") {
    DummyFontManager font_manager;
    DummyTextShaper shaper;

    FontDescriptor desc;
    desc.family = "Arial";
    desc.size_pt = 12.0f;

    auto font = font_manager.getFont(desc);
    REQUIRE(font != nullptr);
    REQUIRE(font->getDescriptor().family == "Arial");

    std::string_view text = "Hello";
    ShapedTextRun run = shaper.shapeText(text, font);

    // 5 characters = 5 glyphs
    REQUIRE(run.glyphs.size() == 5);
    
    // Each glyph width is size_pt * 10 = 120 twips
    // Total width = 5 * 120 = 600 twips
    REQUIRE(run.total_width.getValue() == 600);

    // Ascent is 12.0 * 20 * 0.8 = 192
    REQUIRE(run.max_ascent.getValue() == 192);

    // Verify first glyph
    REQUIRE(run.glyphs[0].codepoint == 'H');
    REQUIRE(run.glyphs[0].x_advance.getValue() == 120);
    REQUIRE(run.glyphs[0].cluster == 0);
}
