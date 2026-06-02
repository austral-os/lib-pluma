#include <catch2/catch_test_macros.hpp>
#include <pluma/Layout/LayoutEngine.hpp>
#include <pluma/Typography/DummyTypography.hpp>

using namespace pluma;

TEST_CASE("Basic Layout Engine wrapping and pagination", "[layout]") {
    auto shaper = std::make_shared<DummyTextShaper>();
    auto font_manager = std::make_shared<DummyFontManager>();

    FontDescriptor desc;
    desc.family = "Arial";
    desc.size_pt = 12.0f; // Each char is 120 twips wide, ascent=192, descent=48 (height=240)
    
    auto font = font_manager->getFont(desc);
    LayoutEngine engine(shaper, font);

    SECTION("Single short line") {
        std::string text = "Hello World";
        // 11 chars * 120 = 1320 twips
        FormatRegistry registry;
        auto pages = engine.layoutText(text, PageSize(Twips(2000), Twips(2000)), PageMargins(Twips(0), Twips(0), Twips(0), Twips(0)), registry);
        
        REQUIRE(pages.size() == 1);
        REQUIRE(pages[0]->blocks.size() == 1);
        REQUIRE(pages[0]->blocks[0]->lines.size() == 1);
        
        auto& line = pages[0]->blocks[0]->lines[0];
        REQUIRE(line->runs.size() == 2); // "Hello " and "World"
        REQUIRE(line->getBounds().width.getValue() == 1320);
        REQUIRE(line->getBounds().height.getValue() == 240);
    }

    SECTION("Line wrapping") {
        std::string text = "Word1 Word2 Word3";
        // Each word is 5 chars + 1 space (except last)
        // "Word1 " = 6 * 120 = 720
        // "Word2 " = 6 * 120 = 720
        // "Word3" = 5 * 120 = 600
        
        // Let's set page width to 1000 twips, so Word1 and Word2 can't fit on one line (720+720=1440)
        FormatRegistry registry;
        auto pages = engine.layoutText(text, PageSize(Twips(1000), Twips(2000)), PageMargins(Twips(0), Twips(0), Twips(0), Twips(0)), registry);
        
        REQUIRE(pages.size() == 1);
        REQUIRE(pages[0]->blocks[0]->lines.size() == 3);
        
        REQUIRE(pages[0]->blocks[0]->lines[0]->runs[0]->logical_text == "Word1 ");
        REQUIRE(pages[0]->blocks[0]->lines[1]->runs[0]->logical_text == "Word2 ");
        REQUIRE(pages[0]->blocks[0]->lines[2]->runs[0]->logical_text == "Word3");
    }

    SECTION("Pagination") {
        std::string text = "Line1 Line2 Line3 Line4";
        // Each is 720 width. If page width is 1000, we get 4 lines.
        // Each line is 240 twips high. 4 lines = 960 twips height.
        // If page height is 600, it can only fit 2 lines (480 twips) per page.
        
        FormatRegistry registry;
        auto pages = engine.layoutText(text, PageSize(Twips(1000), Twips(600)), PageMargins(Twips(0), Twips(0), Twips(0), Twips(0)), registry);
        
        REQUIRE(pages.size() == 2);
        REQUIRE(pages[0]->blocks[0]->lines.size() == 2); // Line1, Line2
        REQUIRE(pages[1]->blocks[0]->lines.size() == 2); // Line3, Line4
    }
}
