#include <catch2/catch_test_macros.hpp>
#include <pluma/Style/StyleResolver.hpp>
#include <pluma/Style/PropertyBag.hpp>
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
