#include <catch2/catch_test_macros.hpp>
#include <pluma/Editor/Selection.hpp>
#include <pluma/Editor/CaretResolver.hpp>
#include <pluma/Layout/LayoutEngine.hpp>
#include <pluma/Typography/DummyTypography.hpp>

using namespace pluma;

TEST_CASE("SelectionRange logic", "[editor]") {
    SelectionRange collapsed{10, 10};
    REQUIRE(collapsed.isCollapsed());
    REQUIRE(collapsed.getStart() == 10);
    REQUIRE(collapsed.getEnd() == 10);
    REQUIRE(collapsed.getLength() == 0);

    SelectionRange forward{5, 15};
    REQUIRE_FALSE(forward.isCollapsed());
    REQUIRE(forward.getStart() == 5);
    REQUIRE(forward.getEnd() == 15);
    REQUIRE(forward.getLength() == 10);

    SelectionRange backward{20, 10};
    REQUIRE_FALSE(backward.isCollapsed());
    REQUIRE(backward.getStart() == 10);
    REQUIRE(backward.getEnd() == 20);
    REQUIRE(backward.getLength() == 10);
}

TEST_CASE("CaretResolver logical to physical", "[editor]") {
    auto shaper = std::make_shared<DummyTextShaper>();
    auto font_manager = std::make_shared<DummyFontManager>();

    FontDescriptor desc;
    desc.family = "Arial";
    desc.size_pt = 12.0f; // Each char = 120 twips wide, height = 240
    
    auto font = font_manager->getFont(desc);
    LayoutEngine engine(shaper, font);

    // "Hello World" -> H=0, e=1, l=2, l=3, o=4, space=5, W=6, o=7, r=8, l=9, d=10
    std::string text = "Hello World";
    FormatRegistry registry;
    auto pages = engine.layoutText(text, PageSize(Twips(2000), Twips(2000)), PageMargins(Twips(0), Twips(0), Twips(0), Twips(0)), registry);

    SECTION("Caret at start (offset 0)") {
        auto rect = CaretResolver::resolveLogicalToPhysical(pages, 0, Twips(0));
        REQUIRE(rect.has_value());
        REQUIRE(rect->x.getValue() == 0); // start of line
        REQUIRE(rect->y.getValue() == 0);
        REQUIRE(rect->height.getValue() == 240);
    }

    SECTION("Caret inside first word (offset 2 - before 'l')") {
        auto rect = CaretResolver::resolveLogicalToPhysical(pages, 2, Twips(0));
        REQUIRE(rect.has_value());
        REQUIRE(rect->x.getValue() == 240); // 2 chars * 120
        REQUIRE(rect->y.getValue() == 0);
    }

    SECTION("Caret at start of second word (offset 6 - before 'W')") {
        auto rect = CaretResolver::resolveLogicalToPhysical(pages, 6, Twips(0));
        REQUIRE(rect.has_value());
        REQUIRE(rect->x.getValue() == 720); // 6 chars (including space) * 120
        REQUIRE(rect->y.getValue() == 0);
    }

    SECTION("Caret at end of text (offset 11)") {
        auto rect = CaretResolver::resolveLogicalToPhysical(pages, 11, Twips(0));
        REQUIRE(rect.has_value());
        REQUIRE(rect->x.getValue() == 1320); // 11 chars * 120
        REQUIRE(rect->y.getValue() == 0);
    }

    SECTION("Out of bounds offset") {
        auto rect = CaretResolver::resolveLogicalToPhysical(pages, 999, Twips(0));
        REQUIRE_FALSE(rect.has_value());
    }
}
