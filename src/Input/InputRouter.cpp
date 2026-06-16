#include <pluma/Input/InputRouter.hpp>

namespace pluma {

// Commonly used generic keysyms (mocked here, usually provided by xkbcommon)
constexpr uint32_t KEYSYM_Z = 0x007a; // 'z'
constexpr uint32_t KEYSYM_C = 0x0063; // 'c'
constexpr uint32_t KEYSYM_V = 0x0076; // 'v'
constexpr uint32_t KEYSYM_X = 0x0078; // 'x'
constexpr uint32_t KEYSYM_BACKSPACE = 0xff08;
constexpr uint32_t KEYSYM_DELETE = 0xffff;
constexpr uint32_t KEYSYM_LEFT = 0xff51;
constexpr uint32_t KEYSYM_RIGHT = 0xff53;
constexpr uint32_t KEYSYM_UP = 0xff52;
constexpr uint32_t KEYSYM_DOWN = 0xff54;
constexpr uint32_t KEYSYM_INSERT = 0xff63;

InputRouter::InputRouter() {
    // Default standard shortcuts
    registerShortcut(KEYSYM_Z, ModifierFlags::Control, EditorAction::Undo);
    registerShortcut(KEYSYM_Z, ModifierFlags::Control | ModifierFlags::Shift, EditorAction::Redo);

    registerShortcut(KEYSYM_BACKSPACE, ModifierFlags::None, EditorAction::DeleteBackward);
    registerShortcut(KEYSYM_DELETE, ModifierFlags::None, EditorAction::DeleteForward);

    registerShortcut(KEYSYM_LEFT, ModifierFlags::None, EditorAction::MoveCursorLeft);
    registerShortcut(KEYSYM_LEFT, ModifierFlags::Shift, EditorAction::MoveCursorLeft);
    registerShortcut(KEYSYM_RIGHT, ModifierFlags::None, EditorAction::MoveCursorRight);
    registerShortcut(KEYSYM_RIGHT, ModifierFlags::Shift, EditorAction::MoveCursorRight);

    registerShortcut(KEYSYM_UP, ModifierFlags::None, EditorAction::MoveCursorUp);
    registerShortcut(KEYSYM_UP, ModifierFlags::Shift, EditorAction::MoveCursorUp);
    registerShortcut(KEYSYM_DOWN, ModifierFlags::None, EditorAction::MoveCursorDown);
    registerShortcut(KEYSYM_DOWN, ModifierFlags::Shift, EditorAction::MoveCursorDown);

    registerShortcut(KEYSYM_LEFT, ModifierFlags::Control, EditorAction::MoveCursorLeftWord);
    registerShortcut(KEYSYM_LEFT, ModifierFlags::Control | ModifierFlags::Shift, EditorAction::MoveCursorLeftWord);
    registerShortcut(KEYSYM_RIGHT, ModifierFlags::Control, EditorAction::MoveCursorRightWord);
    registerShortcut(KEYSYM_RIGHT, ModifierFlags::Control | ModifierFlags::Shift, EditorAction::MoveCursorRightWord);

    registerShortcut(KEYSYM_INSERT, ModifierFlags::None, EditorAction::ToggleInsertMode);
}

void InputRouter::registerShortcut(uint32_t keysym, ModifierFlags mods, EditorAction action) {
    shortcuts_[{keysym, mods}] = action;
}

bool InputRouter::handleKeyEvent(uint32_t keysym, ModifierFlags mods) {
    auto it = shortcuts_.find({keysym, mods});
    if (it != shortcuts_.end()) {
        if (callback_) {
            callback_(it->second, "", mods);
        }
        return true;
    }
    return false;
}

void InputRouter::handleTextInput(const std::string& utf8_text) {
    if (!utf8_text.empty() && callback_) {
        callback_(EditorAction::InsertText, utf8_text, ModifierFlags::None);
    }
}

} // namespace pluma
