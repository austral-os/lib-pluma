#include <catch2/catch_test_macros.hpp>
#include <pluma/Layout/LayoutEngine.hpp>
#include <pluma/Typography/DummyTypography.hpp>

using namespace pluma;

TEST_CASE("Advanced Pagination - Widow and Orphan control", "[layout]") {
    auto shaper = std::make_shared<DummyTextShaper>();
    auto font_manager = std::make_shared<DummyFontManager>();

    FontDescriptor desc;
    desc.family = "Arial";
    desc.size_pt = 12.0f; // character height = 240, width = 120
    auto font = font_manager->getFont(desc);

    LayoutEngine engine(shaper, font);

    // Let's create a scenario where a paragraph has 4 lines.
    // To make a paragraph with 4 lines, we just need words that sum up to more than page_width.
    // Page width = 1000 twips. Each char = 120 twips.
    // 8 chars per line ~ 960 twips. Let's use 8-char words.
    std::string word8 = "1234567 "; // 8 chars (including space) = 960 twips -> exactly 1 line
    std::string text = word8 + word8 + word8 + word8; // 4 lines

    // Line height = 240 twips.

    SECTION("Orphan control - Move block to next page") {
        // Page height = 1200 (fits exactly 5 lines).
        // Two paragraphs, each 4 lines.
        // Para 1 takes 4 lines. Remaining space fits 1 line.
        // Para 2's first line would fit, but it's an orphan (< 2 lines fit).
        // So all of Para 2 should be pushed to Page 2.
        std::string two_paras = word8 + word8 + word8 + word8 + "\n" + word8 + word8 + word8 + word8;
        FormatRegistry registry;
        auto pages = engine.layoutText(two_paras, PageSize(Twips(1000), Twips(1200)), PageMargins(Twips(0), Twips(0), Twips(0), Twips(0)), registry);
        
        REQUIRE(pages.size() == 2);
        
        // Page 1 should only have Para 1 (4 lines)
        REQUIRE(pages[0]->blocks.size() == 1);
        REQUIRE(pages[0]->blocks[0]->lines.size() == 4);
        
        // Page 2 should have Para 2 (4 lines)
        REQUIRE(pages[1]->blocks.size() == 1);
        REQUIRE(pages[1]->blocks[0]->lines.size() == 4);
    }

    SECTION("Widow control - Pull 1 line to next page") {
        // Page height = 720 twips. Fits exactly 3 lines.
        // Para 1 has 4 lines.
        // Line 4 would go to page 2. This leaves 1 line on page 2 (a Widow!).
        // Widow control should pull Line 3 to page 2, so Page 1 has 2 lines, Page 2 has 2 lines.
        FormatRegistry registry;
        auto pages = engine.layoutText(text, PageSize(Twips(1000), Twips(720)), PageMargins(Twips(0), Twips(0), Twips(0), Twips(0)), registry);
        
        REQUIRE(pages.size() == 2);
        
        REQUIRE(pages[0]->blocks.size() == 1);
        REQUIRE(pages[0]->blocks[0]->lines.size() == 2); 
        
        REQUIRE(pages[1]->blocks.size() == 1);
        REQUIRE(pages[1]->blocks[0]->lines.size() == 2); 
    }
}
