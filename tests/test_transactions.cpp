#include <catch2/catch_test_macros.hpp>
#include <pluma/PieceTable.hpp>
#include <pluma/UndoManager.hpp>
#include <pluma/Command.hpp>

using namespace pluma;

TEST_CASE("UndoManager basic transactions", "[undo]") {
    PieceTable pt("Hello");
    pluma::FormatRegistry fr; UndoManager history(pt, fr);

    REQUIRE(pt.getText() == "Hello");
    REQUIRE_FALSE(history.canUndo());
    REQUIRE_FALSE(history.canRedo());

    SECTION("Commit and Undo/Redo") {
        history.beginTransaction();
        history.addCommand(std::make_unique<InsertTextCommand>(5, " World"));
        history.commitTransaction();

        REQUIRE(pt.getText() == "Hello World");
        REQUIRE(history.canUndo());
        REQUIRE_FALSE(history.canRedo());

        history.undo();
        REQUIRE(pt.getText() == "Hello");
        REQUIRE_FALSE(history.canUndo());
        REQUIRE(history.canRedo());

        history.redo();
        REQUIRE(pt.getText() == "Hello World");
        REQUIRE(history.canUndo());
        REQUIRE_FALSE(history.canRedo());
    }

    SECTION("Rollback transaction") {
        history.beginTransaction();
        history.addCommand(std::make_unique<InsertTextCommand>(5, " Universe"));
        REQUIRE(pt.getText() == "Hello Universe");

        history.rollbackTransaction();
        REQUIRE(pt.getText() == "Hello");
        REQUIRE_FALSE(history.canUndo()); // Rolled back, so nothing in history
    }

    SECTION("Multiple commands in one transaction") {
        history.beginTransaction();
        history.addCommand(std::make_unique<InsertTextCommand>(5, " Beautiful"));
        history.addCommand(std::make_unique<InsertTextCommand>(15, " World"));
        history.addCommand(std::make_unique<DeleteTextCommand>(6, 10)); // delete "Beautiful "
        history.commitTransaction();

        REQUIRE(pt.getText() == "Hello World"); // "Hello Beautiful World" -> "Hello World"

        history.undo();
        REQUIRE(pt.getText() == "Hello");

        history.redo();
        REQUIRE(pt.getText() == "Hello World");
    }

    SECTION("History compression / truncation") {
        history.beginTransaction();
        history.addCommand(std::make_unique<InsertTextCommand>(5, " 1"));
        history.commitTransaction();

        history.beginTransaction();
        history.addCommand(std::make_unique<InsertTextCommand>(7, " 2"));
        history.commitTransaction();

        REQUIRE(pt.getText() == "Hello 1 2");

        history.undo();
        REQUIRE(pt.getText() == "Hello 1");

        // Now we make a new transaction, this should truncate the " 2" future
        history.beginTransaction();
        history.addCommand(std::make_unique<InsertTextCommand>(7, " 3"));
        history.commitTransaction();

        REQUIRE(pt.getText() == "Hello 1 3");
        REQUIRE_FALSE(history.canRedo()); // Redo is lost!

        history.undo();
        REQUIRE(pt.getText() == "Hello 1");
    }
}
