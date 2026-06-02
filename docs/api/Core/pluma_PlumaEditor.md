# Class `pluma::PlumaEditor`

**The primary entry point for client applications to embed the document engine.**

Orchestrates input routing, text mutations, layout scheduling, and rendering.



### 💡 Example Usage: Interacting with Tables

```cpp
// 1. Mouse down near a column border triggers resize mode
editor.onMouseDown(x, y, pluma::MouseButton::Left, pluma::ModifierFlags::None);

// 2. Moving the mouse updates the column width in real time
editor.onMouseMove(new_x, y, pluma::ModifierFlags::None);

// 3. Mouse up commits the layout
editor.onMouseUp(new_x, y, pluma::MouseButton::Left, pluma::ModifierFlags::None);
```
## Public Methods
- `PlumaEditor(std::shared_ptr< ITextShaper > shaper, std::shared_ptr< IFont > default_font)` - *Constructs a new*
- `void setCursorStateCallback(std::function< void(const CursorState &)> callback)` - *Sets a callback that is fired whenever the cursor position changes.*
- `void loadText(const std::string &text)` - *Completely replaces the document content with new text.*
- `std::shared_ptr< DocumentSnapshot > getSnapshot()` - *Gets an immutable snapshot of the current document state.*
- `void setViewport(Twips width, Twips height)` - *Sets the size of the viewport. Triggers a relayout if necessary.*
- `void setPageSize(const PageSize &size)` - *Sets the physical page size of the document. Triggers a relayout.*
- `void setMargins(const PageMargins &margins)` - *Sets the physical margins of the document page. Triggers a relayout.*
- `void showMargins()` - *Enables the visual rendering of the page margins.*
- `void hideMargins()` - *Disables the visual rendering of the page margins.*
- `void setMarginColor(Color color)` - *Sets the color used to draw the page margins.*
- `void setWorkspaceBackgroundColor(Color color)` - *Sets the background color of the workspace (the area behind the pages).*
- `void setPageBackgroundColor(Color color)` - *Sets the background color of the page canvas.*
- `void setDefaultTextColor(Color color)` - *Sets the default text color used when no specific text color is applied. This decouples the logical text color (e.g., black for printing) from the UI theme.*
- `void setSelectionColor(Color color)` - *Sets the color used to draw the background of selected text.*
- `void setCaretVisible(bool visible)` - *Toggles the visibility of the blinking text cursor (caret).*
- `void setCaretStyle(CaretStyle style)` - *Sets the visual style of the caret.*
- `void setCaretColor(std::optional< Color > color)` - *Sets an explicit color for the caret. If nullopt, it inherits the text color.*
- `void setCaretBlink(bool blinks, uint32_t interval_ms=500)` - *Configures caret blinking behavior.*
- `bool onBlinkTimer()` - *Called by a timer to toggle the caret blink state.*
- `Size getDocumentBounds() const` - *Gets the total physical size of the document (all pages + gaps).*
- `void setPageGap(Twips gap)` - *Sets the gap between pages.*
- `void applyStyle(uint32_t start, uint32_t length, PropertyId id, PropertyValue value)` - *Applies a text style to a specific range of characters.*
- `std::string getSelectedText() const` - *Gets the text currently selected by the user.*
- `void selectImage(uint32_t offset)` - *Programmatically selects an image at the given logical offset.*
- `void selectTableColumn(uint32_t table_offset, int col)`
- `void setSelection(uint32_t anchor, uint32_t head)` - *Programmatically selects a range of text.*
- `void deleteSelectedImage()`
- `void deleteForward()`
- `void setHeader(const std::string &header)`
- `void setFooter(const std::string &footer)`
- `void deleteSelection()` - *Deletes the currently selected text.*
- `void pasteText(const std::string &text)` - *Pastes text at the current cursor position, replacing any active selection.*
- `void setScroll(Twips x, Twips y)` - *Sets the current scroll position for render virtualization.*
- `bool onKeyPress(uint32_t keysym, ModifierFlags mods)` - *Event: Key pressed.*
- `bool onKeyRelease(uint32_t keysym, ModifierFlags mods)` - *Event: Key released.*
- `void onTextInput(const std::string &text)` - *Event: Text input from IM or typing.*
- `bool onMouseDown(double x, double y, MouseButton button, ModifierFlags mods)` - *Event: Mouse button pressed.*
- `bool onMouseUp(double x, double y, MouseButton button, ModifierFlags mods)` - *Event: Mouse button released.*
- `bool onMouseMove(double x, double y, ModifierFlags mods)` - *Event: Mouse pointer moved.*
- `bool onMouseDoubleClick(double x, double y, MouseButton button, ModifierFlags mods)` - *Event: Mouse double click.*
- `bool onScroll(double delta_x, double delta_y, ModifierFlags mods)` - *Event: Scroll wheel or trackpad.*
- `void render(IRenderer &renderer)` - *Renders the current state of the document.*
- `std::string getText() const` - *Exposes the text for test assertions.*
- `uint32_t getCursorOffset() const` - *Returns the current cursor position.*
- `const FormatRegistry & getFormatRegistry() const` - *Exposes the format registry for serialization.*
- `void toggleInsertMode()` - *Toggles between Insert and Replace text input modes.*
- `InsertMode getInsertMode() const` - *Gets the current text input mode.*

