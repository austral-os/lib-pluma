#include <catch2/catch_test_macros.hpp>
#include <pluma/PieceTable.hpp>
#include <pluma/DOM/DOMManager.hpp>
#include <pluma/A11y/A11yTreeBuilder.hpp>

using namespace pluma;
using namespace pluma::a11y;

TEST_CASE("A11yTreeBuilder - Generate Accessible Tree", "[a11y]") {
    PieceTable table;
    std::string text = "Accessible Paragraph";
    table.insert(0, text);

    DOMManager dom_manager;
    auto dom_root = dom_manager.rebuild(table);

    std::vector<std::unique_ptr<PageBox>> dummy_pages;
    auto a11y_root = A11yTreeBuilder::build(*dom_root, text, dummy_pages);

    REQUIRE(a11y_root != nullptr);
    REQUIRE(a11y_root->role == Role::Document);
    REQUIRE((a11y_root->states & StateFlags::Editable) == StateFlags::Editable);
    
    REQUIRE(a11y_root->children.size() == 1);
    auto para = a11y_root->children[0];
    REQUIRE(para->role == Role::Paragraph);
    
    REQUIRE(para->children.size() == 1);
    auto text_run = para->children[0];
    REQUIRE(text_run->role == Role::StaticText);
    REQUIRE(text_run->text_content == "Accessible Paragraph");
    REQUIRE((text_run->states & StateFlags::Selectable) == StateFlags::Selectable);
}
