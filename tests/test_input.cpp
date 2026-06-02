#include <catch2/catch_test_macros.hpp>
#include <pluma/Input/InputRouter.hpp>
#include <vector>
#include <string>

using namespace pluma;

TEST_CASE("InputRouter Modifier Flags", "[input]") {
    ModifierFlags mods = ModifierFlags::Control | ModifierFlags::Shift;
    REQUIRE(hasModifier(mods, ModifierFlags::Control));
    REQUIRE(hasModifier(mods, ModifierFlags::Shift));
    REQUIRE_FALSE(hasModifier(mods, ModifierFlags::Alt));
}

TEST_CASE("InputRouter handles shortcuts", "[input]") {
    InputRouter router;

    std::vector<EditorAction> emitted_actions;
    std::vector<std::string> emitted_text;

    router.setActionCallback([&](EditorAction action, const std::string& text, ModifierFlags mods) {
        (void)mods;
        emitted_actions.push_back(action);
        emitted_text.push_back(text);
    });

    // Ctrl+Z
    bool handled = router.handleKeyEvent(0x007a, ModifierFlags::Control);
    REQUIRE(handled);
    REQUIRE(emitted_actions.size() == 1);
    REQUIRE(emitted_actions.back() == EditorAction::Undo);

    // Ctrl+Shift+Z
    handled = router.handleKeyEvent(0x007a, ModifierFlags::Control | ModifierFlags::Shift);
    REQUIRE(handled);
    REQUIRE(emitted_actions.size() == 2);
    REQUIRE(emitted_actions.back() == EditorAction::Redo);

    // Unmapped shortcut (Alt+Z)
    handled = router.handleKeyEvent(0x007a, ModifierFlags::Alt);
    REQUIRE_FALSE(handled);
    REQUIRE(emitted_actions.size() == 2); // No new action
}

TEST_CASE("InputRouter handles text input", "[input]") {
    InputRouter router;

    std::vector<EditorAction> emitted_actions;
    std::vector<std::string> emitted_text;

    router.setActionCallback([&](EditorAction action, const std::string& text, ModifierFlags mods) {
        (void)mods;
        emitted_actions.push_back(action);
        emitted_text.push_back(text);
    });

    router.handleTextInput("Hello UTF-8 ñ");
    REQUIRE(emitted_actions.size() == 1);
    REQUIRE(emitted_actions.back() == EditorAction::InsertText);
    REQUIRE(emitted_text.back() == "Hello UTF-8 ñ");
}
