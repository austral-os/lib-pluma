# Class `pluma::InputRouter`

**Responsible for mapping shortcuts and emitting logical editor commands.**

## Public Methods
- `InputRouter()` - *Constructs an*
- `void setActionCallback(ActionCallback cb)` - *Registers the callback that will receive translated actions.*
- `void registerShortcut(uint32_t keysym, ModifierFlags mods, EditorAction action)` - *Registers a specific keysym/modifier combination to an action.*
- `bool handleKeyEvent(uint32_t keysym, ModifierFlags mods)` - *Handles a raw keyboard event.*
- `void handleTextInput(const std::string &utf8_text)` - *Handles text input (e.g. from an Input Method or direct typing).*

