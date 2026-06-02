#include <catch2/catch_test_macros.hpp>
#include <pluma/PlumaEditor.hpp>
#include <pluma/Typography/DummyTypography.hpp>
#include <pluma/Render/DummyRenderer.hpp>

using namespace pluma;

TEST_CASE("Render Virtualization - Viewport Culling", "[virtualization]") {
    auto shaper = std::make_shared<DummyTextShaper>();
    auto font_manager = std::make_shared<DummyFontManager>();

    FontDescriptor desc;
    desc.family = "Arial";
    desc.size_pt = 12.0f; // character height = 240, width = 120
    auto font = font_manager->getFont(desc);

    PlumaEditor editor(shaper, font);
    
    // Create a 10-line document
    std::string text = "Line1\nLine2\nLine3\nLine4\nLine5\nLine6\nLine7\nLine8\nLine9\nLine10";
    editor.onTextInput(text);

    // Disable margins so blocks start at y=0, avoiding culling offset mismatch
    editor.setMargins(PageMargins(Twips(0), Twips(0), Twips(0), Twips(0)));

    // Set viewport height to 240 twips (exactly 1 line).
    // Plus the prefetch margin is 1 page height (240 twips).
    // So the rendered area is viewport_y - 240 to viewport_y + 480.
    // That means it will render 3 lines maximum.
    editor.setViewport(Twips(5000), Twips(240));

    SECTION("Scroll to top") {
        editor.setScroll(Twips(0), Twips(0));
        
        DummyRenderer renderer;
        editor.render(renderer);
        
        // Count how many drawGlyphRun commands were executed
        int draw_count = 0;
        for (const auto& log : renderer.log) {
            if (log.find("drawGlyphRun") != std::string::npos) {
                draw_count++;
            }
        }
        
        // Lines 1, 2 should be in the [0, 480] range. Line 3 starts at 480, height 240, intersects partially? 
        // 480 doesn't strictly intersect if we do <, but our logic might include it.
        // It's definitely much less than 10 lines!
        REQUIRE(draw_count <= 5);
        REQUIRE(draw_count > 0);
    }

    SECTION("Scroll to bottom") {
        // Line 10 is at y = 9 * 240 = 2160
        editor.setScroll(Twips(0), Twips(2160));
        
        DummyRenderer renderer;
        editor.render(renderer);
        
        int draw_count = 0;
        for (const auto& log : renderer.log) {
            if (log.find("drawGlyphRun") != std::string::npos) {
                draw_count++;
            }
        }
        
        // Should only render the last few lines (culling works)
        REQUIRE(draw_count <= 5);
        REQUIRE(draw_count > 0);
    }
}
