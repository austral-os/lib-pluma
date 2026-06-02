#include <catch2/catch_test_macros.hpp>
#include <pluma/DOM/DOMManager.hpp>
#include <pluma/PieceTable.hpp>

using namespace pluma;

TEST_CASE("DOMManager rebuild from PieceTable", "[dom]") {
    PieceTable table;
    DOMManager manager;

    SECTION("Empty PieceTable") {
        auto doc = manager.rebuild(table);
        
        REQUIRE(doc->getType() == NodeType::Document);
        REQUIRE(doc->getChildren().size() == 1);
        
        const auto& para = doc->getChildren()[0];
        REQUIRE(para->getType() == NodeType::Paragraph);
        REQUIRE(para->getChildren().size() == 1);
        
        auto run = static_cast<RunNode*>(para->getChildren()[0].get());
        REQUIRE(run->getType() == NodeType::Run);
        REQUIRE(run->getOffset() == 0);
        REQUIRE(run->getLength() == 0);
    }

    SECTION("Single Paragraph") {
        table.insert(0, "Hello World");
        auto doc = manager.rebuild(table);
        
        REQUIRE(doc->getChildren().size() == 1);
        
        const auto& para = doc->getChildren()[0];
        auto run = static_cast<RunNode*>(para->getChildren()[0].get());
        
        REQUIRE(run->getOffset() == 0);
        REQUIRE(run->getLength() == 11);
    }

    SECTION("Multiple Paragraphs") {
        table.insert(0, "Line 1\nLine 2\nLine 3");
        auto doc = manager.rebuild(table);
        
        REQUIRE(doc->getChildren().size() == 3);
        
        auto run1 = static_cast<RunNode*>(doc->getChildren()[0]->getChildren()[0].get());
        auto run2 = static_cast<RunNode*>(doc->getChildren()[1]->getChildren()[0].get());
        auto run3 = static_cast<RunNode*>(doc->getChildren()[2]->getChildren()[0].get());
        
        REQUIRE(run1->getLength() == 6); // "Line 1"
        REQUIRE(run1->getOffset() == 0);
        
        REQUIRE(run2->getLength() == 6); // "Line 2"
        REQUIRE(run2->getOffset() == 7); // "Line 1\n"
        
        REQUIRE(run3->getLength() == 6); // "Line 3"
        REQUIRE(run3->getOffset() == 14); // "Line 1\nLine 2\n"
    }

    SECTION("Trailing Newline") {
        table.insert(0, "Text\n");
        auto doc = manager.rebuild(table);
        
        // "Text" + empty trailing paragraph
        REQUIRE(doc->getChildren().size() == 2);
        
        auto run1 = static_cast<RunNode*>(doc->getChildren()[0]->getChildren()[0].get());
        auto run2 = static_cast<RunNode*>(doc->getChildren()[1]->getChildren()[0].get());
        
        REQUIRE(run1->getLength() == 4);
        REQUIRE(run2->getLength() == 0);
        REQUIRE(run2->getOffset() == 5);
    }
}
