#include <catch2/catch_test_macros.hpp>
#include <pluma/PieceTable.hpp>

using namespace pluma;

TEST_CASE("PieceTable basic operations", "[piecetable]") {
    SECTION("Initial text") {
        PieceTable pt("Hello World");
        REQUIRE(pt.getText() == "Hello World");
        REQUIRE(pt.getLength() == 11);
    }

    SECTION("Insert text") {
        PieceTable pt("Hello");
        pt.insert(5, " World");
        REQUIRE(pt.getText() == "Hello World");
        
        pt.insert(5, ",");
        REQUIRE(pt.getText() == "Hello, World");
        
        pt.insert(0, "Say ");
        REQUIRE(pt.getText() == "Say Hello, World");
    }

    SECTION("Remove text") {
        PieceTable pt("Say Hello, World");
        pt.remove(0, 4); // "Hello, World"
        REQUIRE(pt.getText() == "Hello, World");

        pt.remove(5, 1); // "Hello World"
        REQUIRE(pt.getText() == "Hello World");

        pt.remove(5, 6); // "Hello"
        REQUIRE(pt.getText() == "Hello");
    }

    SECTION("Complex operations") {
        PieceTable pt("A text");
        pt.insert(2, "simple ");
        REQUIRE(pt.getText() == "A simple text");

        pt.remove(2, 7);
        REQUIRE(pt.getText() == "A text");

        pt.insert(6, " document");
        REQUIRE(pt.getText() == "A text document");
    }

    SECTION("Snapshot Model") {
        PieceTable pt("Hello");
        auto snap1 = pt.createSnapshot();
        REQUIRE(snap1->getRevision().getValue() == 1);
        REQUIRE(snap1->getText() == "Hello");

        pt.insert(5, " World");
        auto snap2 = pt.createSnapshot();
        REQUIRE(snap2->getRevision().getValue() == 2);
        REQUIRE(snap2->getText() == "Hello World");

        pt.remove(0, 6); // Remove "Hello "
        auto snap3 = pt.createSnapshot();
        REQUIRE(snap3->getRevision().getValue() == 3);
        REQUIRE(snap3->getText() == "World");

        // Verify that old snapshots still see old content
        REQUIRE(snap1->getText() == "Hello");
        REQUIRE(snap2->getText() == "Hello World");
    }
}
