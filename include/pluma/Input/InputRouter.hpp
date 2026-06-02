/**
 * @file InputRouter.hpp
 * @brief Translates raw input events (like Wayland Keysyms) into EditorActions.
 */
#pragma once

#include <pluma/Input/InputTypes.hpp>
#include <functional>
#include <unordered_map>
#include <string>

namespace pluma {

/**
 * @class InputRouter
 * @brief Responsible for mapping shortcuts and emitting logical editor commands.
 */
class InputRouter {
public:
    /**
     * @brief Callback type invoked when an action is resolved.
     * The string parameter is only populated for actions like InsertText or Paste.
     */
    using ActionCallback = std::function<void(EditorAction, const std::string&, ModifierFlags)>;

    /**
     * @brief Constructs an InputRouter and sets up default basic shortcuts.
     */
    InputRouter();

    /**
     * @brief Registers the callback that will receive translated actions.
     * @param cb The callback function.
     */
    void setActionCallback(ActionCallback cb) { callback_ = std::move(cb); }

    /**
     * @brief Registers a specific keysym/modifier combination to an action.
     * @param keysym The standard keysym (e.g., from xkbcommon).
     * @param mods The required modifier state.
     * @param action The resulting editor action.
     */
    void registerShortcut(uint32_t keysym, ModifierFlags mods, EditorAction action);

    /**
     * @brief Handles a raw keyboard event.
     * @param keysym The keysym of the pressed key.
     * @param mods The current modifier state.
     * @return true if the event matched a shortcut and was consumed, false otherwise.
     */
    bool handleKeyEvent(uint32_t keysym, ModifierFlags mods);

    /**
     * @brief Handles text input (e.g. from an Input Method or direct typing).
     * @param utf8_text The string of text to insert.
     */
    void handleTextInput(const std::string& utf8_text);

private:
    struct ShortcutKey {
        uint32_t keysym;
        ModifierFlags mods;
        
        bool operator==(const ShortcutKey& other) const {
            return keysym == other.keysym && mods == other.mods;
        }
    };

    struct ShortcutHash {
        std::size_t operator()(const ShortcutKey& k) const {
            return std::hash<uint32_t>()(k.keysym) ^ (std::hash<uint32_t>()(static_cast<uint32_t>(k.mods)) << 1);
        }
    };

    std::unordered_map<ShortcutKey, EditorAction, ShortcutHash> shortcuts_;
    ActionCallback callback_;
};

} // namespace pluma
