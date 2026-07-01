#include <catch2/catch_test_macros.hpp>
#include <pluma/Style/StyleResolver.hpp>
#include <pluma/Style/PropertyBag.hpp>
#include <pluma/Style/FormatRegistry.hpp>
#include <pluma/Style/StyleProperties.hpp>

using namespace pluma;

TEST_CASE("PropertyBag basic operations", "[style]") {
    PropertyBag bag;
    
    REQUIRE_FALSE(bag.has(PropertyId::FontFamily));
    
    bag.set(PropertyId::FontFamily, std::string("Roboto"));
    bag.set(PropertyId::FontSize, 14.5f);
    
    REQUIRE(bag.has(PropertyId::FontFamily));
    REQUIRE(std::get<std::string>(*bag.get(PropertyId::FontFamily)) == "Roboto");
    REQUIRE(std::get<float>(*bag.get(PropertyId::FontSize)) == 14.5f);
    
    bag.remove(PropertyId::FontFamily);
    REQUIRE_FALSE(bag.has(PropertyId::FontFamily));
}

TEST_CASE("StyleResolver Inheritance and Overrides", "[style]") {
    PropertyBag inherited;
    inherited.set(PropertyId::FontFamily, std::string("Arial"));
    inherited.set(PropertyId::FontSize, 12.0f);
    inherited.set(PropertyId::BackgroundColor, Color(0xFFFFFFFF)); // Should NOT inherit
    
    PropertyBag declared;
    declared.set(PropertyId::FontWeight, static_cast<uint16_t>(FontWeight::Bold));
    declared.set(PropertyId::FontSize, 16.0f); // Overrides inherited
    
    PropertyBag computed = StyleResolver::resolve(inherited, declared);
    
    // Inherited naturally
    REQUIRE(std::get<std::string>(*computed.get(PropertyId::FontFamily)) == "Arial");
    
    // Overridden
    REQUIRE(std::get<float>(*computed.get(PropertyId::FontSize)) == 16.0f);
    
    // Declared
    REQUIRE(std::get<uint16_t>(*computed.get(PropertyId::FontWeight)) == static_cast<uint16_t>(FontWeight::Bold));
    
    // Not inherited
    REQUIRE_FALSE(computed.has(PropertyId::BackgroundColor));
}

TEST_CASE("StyleResolver extractFont", "[style]") {
    PropertyBag bag;
    bag.set(PropertyId::FontFamily, std::string("Times New Roman"));
    bag.set(PropertyId::FontStyleItalic, true);
    
    FontDescriptor font = StyleResolver::extractFont(bag);
    
    REQUIRE(font.family == "Times New Roman");
    REQUIRE(font.italic == true);
    REQUIRE(font.size_pt == 12.0f); // Default value because not in bag
    REQUIRE(font.weight == FontWeight::Regular); // Default
}

// ============================================================================
// FormatRegistry getStyleAt correctness
// ============================================================================

TEST_CASE("FormatRegistry getStyleAt returns correct style", "[style]") {
    FormatRegistry reg;
    // span1: [0..10) weight=700
    reg.applyStyle(0, 10, PropertyId::FontWeight, uint16_t(700));
    // span2: [5..15) italic=true  (applied later = higher cascade priority)
    reg.applyStyle(5, 10, PropertyId::FontStyleItalic, true);

    // Offset 3: covered by span1 only
    {
        auto style = reg.getStyleAt(3);
        REQUIRE(std::get<uint16_t>(*style.get(PropertyId::FontWeight)) == 700);
        REQUIRE_FALSE(style.has(PropertyId::FontStyleItalic));
    }

    // Offset 7: covered by both spans (cascade)
    {
        auto style = reg.getStyleAt(7);
        REQUIRE(std::get<uint16_t>(*style.get(PropertyId::FontWeight)) == 700);
        REQUIRE(std::get<bool>(*style.get(PropertyId::FontStyleItalic)) == true);
    }

    // Offset 12: covered by span2 only (span1 ends at 10)
    {
        auto style = reg.getStyleAt(12);
        REQUIRE_FALSE(style.has(PropertyId::FontWeight));
        REQUIRE(std::get<bool>(*style.get(PropertyId::FontStyleItalic)) == true);
    }

    // Offset 100: no style
    {
        auto style = reg.getStyleAt(100);
        REQUIRE(style.getAll().empty());
    }
}

TEST_CASE("FormatRegistry getStyleAt results are stable across calls", "[style]") {
    FormatRegistry reg;
    reg.applyStyle(2, 5, PropertyId::TextColor, static_cast<Color>(0xFF0000FF));
    reg.applyStyle(0, 10, PropertyId::FontSize, 14.0f);

    // Call twice — must get identical results
    auto a = reg.getStyleAt(3);
    auto b = reg.getStyleAt(3);

    REQUIRE(a.getAll().size() == b.getAll().size());
    REQUIRE(std::get<float>(*a.get(PropertyId::FontSize)) == 14.0f);
    REQUIRE(std::get<float>(*b.get(PropertyId::FontSize)) == 14.0f);
    REQUIRE(std::get<Color>(*a.get(PropertyId::TextColor)) == static_cast<Color>(0xFF0000FF));
    REQUIRE(std::get<Color>(*b.get(PropertyId::TextColor)) == static_cast<Color>(0xFF0000FF));
}

TEST_CASE("FormatRegistry applyStyle mutation is visible on subsequent getStyleAt", "[style]") {
    FormatRegistry reg;
    reg.applyStyle(0, 10, PropertyId::FontWeight, uint16_t(400));
    REQUIRE(std::get<uint16_t>(*reg.getStyleAt(5).get(PropertyId::FontWeight)) == 400);

    // Mutate
    reg.applyStyle(0, 10, PropertyId::FontWeight, uint16_t(700));
    // Must reflect the new value (cache was invalidated)
    REQUIRE(std::get<uint16_t>(*reg.getStyleAt(5).get(PropertyId::FontWeight)) == 700);
}

TEST_CASE("FormatRegistry clear resets getStyleAt results", "[style]") {
    FormatRegistry reg;
    reg.applyStyle(0, 5, PropertyId::FontSize, 12.0f);
    REQUIRE(reg.getStyleAt(2).has(PropertyId::FontSize));

    reg.clear();
    REQUIRE_FALSE(reg.getStyleAt(2).has(PropertyId::FontSize));
}

TEST_CASE("FormatRegistry clearDecorationGlobally is reflected in getStyleAt", "[style]") {
    FormatRegistry reg;
    reg.applyStyle(0, 20, PropertyId::Decoration, TextDecoration::Underline);
    reg.applyStyle(0, 20, PropertyId::FontSize, 12.0f);
    REQUIRE(std::get<TextDecoration>(*reg.getStyleAt(5).get(PropertyId::Decoration)) == TextDecoration::Underline);

    reg.clearDecorationGlobally(TextDecoration::Underline);
    // Cache must be invalidated: decoration should be gone
    REQUIRE_FALSE(reg.getStyleAt(5).has(PropertyId::Decoration));
    // FontSize should still be present
    REQUIRE(reg.getStyleAt(5).has(PropertyId::FontSize));
}

TEST_CASE("FormatRegistry insertText shifts style offsets correctly", "[style]") {
    FormatRegistry reg;
    // Style applies at offset 5..10 (span: start=5, length=5)
    reg.applyStyle(5, 5, PropertyId::FontWeight, uint16_t(700));
    REQUIRE(std::get<uint16_t>(*reg.getStyleAt(6).get(PropertyId::FontWeight)) == 700);
    REQUIRE_FALSE(reg.getStyleAt(4).has(PropertyId::FontWeight));

    // Insert before the styled region — shifts offsets
    reg.insertText(0, 10);
    // Now the style should be at offset 15..20 (start=15, length=5)
    REQUIRE_FALSE(reg.getStyleAt(6).has(PropertyId::FontWeight));
    REQUIRE(std::get<uint16_t>(*reg.getStyleAt(16).get(PropertyId::FontWeight)) == 700);
}

TEST_CASE("FormatRegistry deleteText shifts style offsets correctly", "[style]") {
    FormatRegistry reg;
    // Style at offset 5..15 (span: start=5, length=10)
    reg.applyStyle(5, 10, PropertyId::FontWeight, uint16_t(700));
    REQUIRE(std::get<uint16_t>(*reg.getStyleAt(6).get(PropertyId::FontWeight)) == 700);

    // Delete the first 5 chars — style region shifts to start=0, length=10
    reg.deleteText(0, 5);
    REQUIRE(std::get<uint16_t>(*reg.getStyleAt(6).get(PropertyId::FontWeight)) == 700);
}

TEST_CASE("FormatRegistry setSpans replaces style state correctly", "[style]") {
    FormatRegistry reg;
    reg.applyStyle(0, 5, PropertyId::FontSize, 12.0f);
    REQUIRE(reg.getStyleAt(2).has(PropertyId::FontSize));

    // Replace with empty spans
    reg.setSpans({});
    REQUIRE_FALSE(reg.getStyleAt(2).has(PropertyId::FontSize));
}

TEST_CASE("FormatRegistry getStyleRangeEnd returns next style boundary", "[style]") {
    FormatRegistry reg;

    SECTION("No spans returns UINT32_MAX") {
        REQUIRE(reg.getStyleRangeEnd(0) == UINT32_MAX);
        REQUIRE(reg.getStyleRangeEnd(42) == UINT32_MAX);
    }

    SECTION("Single span") {
        reg.applyStyle(5, 10, PropertyId::FontWeight, uint16_t(700));
        // Range [5, 15) is uniform — next change is at 15
        REQUIRE(reg.getStyleRangeEnd(0) == 5);  // span start
        REQUIRE(reg.getStyleRangeEnd(3) == 5);
        REQUIRE(reg.getStyleRangeEnd(5) == 15); // span end
        REQUIRE(reg.getStyleRangeEnd(10) == 15);
        REQUIRE(reg.getStyleRangeEnd(15) == UINT32_MAX);
    }

    SECTION("Two adjacent spans") {
        reg.applyStyle(0, 10, PropertyId::FontWeight, uint16_t(400));
        reg.applyStyle(10, 5, PropertyId::FontWeight, uint16_t(700));
        // Transition at 10 (span0 end) and 15 (span1 end)
        REQUIRE(reg.getStyleRangeEnd(0) == 10);
        REQUIRE(reg.getStyleRangeEnd(5) == 10);
        REQUIRE(reg.getStyleRangeEnd(10) == 15);
        REQUIRE(reg.getStyleRangeEnd(14) == 15);
        REQUIRE(reg.getStyleRangeEnd(15) == UINT32_MAX);
    }

    SECTION("Overlapping spans produce inner transitions") {
        reg.applyStyle(0, 20, PropertyId::FontWeight, uint16_t(400));
        reg.applyStyle(5, 10, PropertyId::FontSize, 14.0f);
        // Boundaries: span0 start=0, span1 start=5, span1 end=15, span0 end=20
        // But getStyleRangeEnd only returns the NEXT boundary after offset
        REQUIRE(reg.getStyleRangeEnd(0) == 5);   // span1 start
        REQUIRE(reg.getStyleRangeEnd(5) == 15);  // span1 end
        REQUIRE(reg.getStyleRangeEnd(15) == 20); // span0 end
        REQUIRE(reg.getStyleRangeEnd(20) == UINT32_MAX);
    }

    SECTION("Beyond all spans returns UINT32_MAX") {
        reg.applyStyle(0, 10, PropertyId::FontSize, 12.0f);
        REQUIRE(reg.getStyleRangeEnd(100) == UINT32_MAX);
    }

    SECTION("Empty spans do not create boundaries") {
        reg.applyStyle(10, 0, PropertyId::FontSize, 12.0f);
        REQUIRE(reg.getStyleRangeEnd(0) == 10);
        REQUIRE(reg.getStyleRangeEnd(10) == UINT32_MAX);
    }

    SECTION("Overflowing spans do not create wrapped end boundaries") {
        StyleSpan span;
        span.start = UINT32_MAX - 5;
        span.length = 10;
        span.style.set(PropertyId::FontWeight, uint16_t(700));
        reg.setSpans({span});

        REQUIRE(reg.getStyleRangeEnd(UINT32_MAX - 10) == UINT32_MAX - 5);
        REQUIRE(reg.getStyleRangeEnd(UINT32_MAX - 5) == UINT32_MAX);
    }
}

TEST_CASE("FormatRegistry cascade ordering is preserved", "[style]") {
    FormatRegistry reg;

    // First span: offset 0..20, weight=400
    reg.applyStyle(0, 20, PropertyId::FontWeight, uint16_t(400));
    // Second span (applied later, so it wins): offset 5..15, weight=700
    reg.applyStyle(5, 10, PropertyId::FontWeight, uint16_t(700));

    // Offset 3: only first span (weight=400)
    REQUIRE(std::get<uint16_t>(*reg.getStyleAt(3).get(PropertyId::FontWeight)) == 400);

    // Offset 10: both spans, second wins (weight=700)
    REQUIRE(std::get<uint16_t>(*reg.getStyleAt(10).get(PropertyId::FontWeight)) == 700);

    // After mutation and re-cache, offset 10 still shows weight=700
    reg.applyStyle(3, 2, PropertyId::FontSize, 10.0f); // unrelated mutation
    REQUIRE(std::get<uint16_t>(*reg.getStyleAt(10).get(PropertyId::FontWeight)) == 700);
}

TEST_CASE("FormatRegistry multiple offsets return independent results", "[style]") {
    FormatRegistry reg;
    reg.applyStyle(0, 5, PropertyId::FontFamily, std::string("Arial"));
    reg.applyStyle(10, 5, PropertyId::FontFamily, std::string("Courier"));

    auto s0 = reg.getStyleAt(0);
    auto s12 = reg.getStyleAt(12);

    REQUIRE(std::get<std::string>(*s0.get(PropertyId::FontFamily)) == "Arial");
    REQUIRE(std::get<std::string>(*s12.get(PropertyId::FontFamily)) == "Courier");

    // Mutate just one region
    reg.applyStyle(10, 5, PropertyId::FontFamily, std::string("Times"));

    // Offset 0 should still be "Arial" (unaffected region)
    REQUIRE(std::get<std::string>(*reg.getStyleAt(0).get(PropertyId::FontFamily)) == "Arial");
    // Offset 12 should reflect the change
    REQUIRE(std::get<std::string>(*reg.getStyleAt(12).get(PropertyId::FontFamily)) == "Times");
}
