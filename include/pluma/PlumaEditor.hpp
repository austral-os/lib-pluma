/**
 * @file PlumaEditor.hpp
 * @brief The main facade unifying all lib-pluma subsystems.
 */
#pragma once

#include <memory>
#include <string>
#include <vector>

#include <pluma/CoreTypes.hpp>
#include <pluma/PieceTable.hpp>
#include <pluma/UndoManager.hpp>
#include <pluma/Command.hpp>
#include <pluma/DOM/DOMManager.hpp>
#include <pluma/Layout/LayoutEngine.hpp>
#include <pluma/Layout/LayoutScheduler.hpp>
#include <pluma/Input/InputRouter.hpp>
#include <pluma/Editor/Selection.hpp>
#include <pluma/Editor/CaretResolver.hpp>
#include <pluma/Render/IRenderer.hpp>
#include <pluma/Render/DisplayList.hpp>
#include <pluma/Layout/PageSize.hpp>
#include <pluma/Style/FormatRegistry.hpp>
#include <optional>
#include <functional>

namespace pluma {

enum class CursorObjectType {
    Text,
    Image,
    Table,
    TableRow,
    TableColumn,
    TableCell
};

struct CursorState {
    uint32_t logical_offset;
    PropertyBag style;
    CursorObjectType object_type;
};

/**
 * @enum CaretStyle
 * @brief Visual style of the text caret.
 */
enum class CaretStyle {
    Line,
    Underline,
    Block
};

enum class DragMode { 
    None, 
    Text, 
    ImageResize,
    ImageMove,
    TableColResize,
    TableRowResize,
    TableCellSelection
};

enum class TableSelectionMode {
    None,
    Cell,
    Row,
    Column,
    Table
};

struct TableSelection {
    TableSelectionMode mode{TableSelectionMode::None};
    uint32_t table_offset{0};
    int row{-1};
    int col{-1};
    int end_row{-1};
    int end_col{-1};
};

enum class ResizeHandle { 
    None, 
    TopLeft, 
    TopRight, 
    BottomLeft, 
    BottomRight, 
    Top, 
    Bottom, 
    Left, 
    Right 
};

/**
 * @class PlumaEditor
 * @brief The primary entry point for client applications to embed the document engine.
 * 
 * Orchestrates input routing, text mutations, layout scheduling, and rendering.
 */
class PlumaEditor {
public:
    /**
     * @brief Constructs a new PlumaEditor.
     * @param shaper The text shaper backend to use.
     * @param default_font The base font used for layout and rendering.
     */
    PlumaEditor(std::shared_ptr<ITextShaper> shaper, std::shared_ptr<IFont> default_font);

    /**
     * @brief Sets a callback that is fired whenever the cursor position changes.
     */
    void setCursorStateCallback(std::function<void(const CursorState&)> callback) {
        cursor_state_callback_ = callback;
    }

    /**
     * @brief Completely replaces the document content with new text.
     * @param text The new document content.
     */
    void loadText(const std::string& text);

    /**
     * @brief Gets an immutable snapshot of the current document state.
     * @return A snapshot for background analysis or exporting.
     */
    std::shared_ptr<DocumentSnapshot> getSnapshot();

    /**
     * @brief Sets the size of the viewport. Triggers a relayout if necessary.
     * @param width Width in Twips.
     * @param height Height in Twips.
     */
    void setViewport(Twips width, Twips height);

    /**
     * @brief Sets the physical page size of the document. Triggers a relayout.
     * @param size The new page size.
     */
    void setPageSize(const PageSize& size);
    PageSize getPageSize() const { return page_size_; }

    /**
     * @brief Sets the physical margins of the document page. Triggers a relayout.
     * @param margins The new page margins.
     */
    void setMargins(const PageMargins& margins);

    /**
     * @brief Enables the visual rendering of the page margins.
     */
    void showMargins();

    /**
     * @brief Disables the visual rendering of the page margins.
     */
    void hideMargins();

    /**
     * @brief Sets the color used to draw the page margins.
     */
    void setMarginColor(Color color);

    /**
     * @brief Sets the background color of the workspace (the area behind the pages).
     */
    void setWorkspaceBackgroundColor(Color color);

    /**
     * @brief Sets the background color of the page canvas.
     */
    void setPageBackgroundColor(Color color);
    Color getPageBackgroundColor() const { return page_bg_color_; }

    /**
     * @brief Sets the default text color used when no specific text color is applied.
     * This decouples the logical text color (e.g., black for printing) from the UI theme.
     */
    void setDefaultTextColor(Color color);
    Color getDefaultTextColor() const { return default_text_color_; }

    /**
     * @brief Sets the color used to draw the background of selected text.
     */
    void setSelectionColor(Color color);

    /**
     * @brief Toggles the visibility of the blinking text cursor (caret).
     */
    void setCaretVisible(bool visible);

    /**
     * @brief Sets the visual style of the caret.
     */
    void setCaretStyle(CaretStyle style);

    /**
     * @brief Sets an explicit color for the caret. If nullopt, it inherits the text color.
     */
    void setCaretColor(std::optional<Color> color);

    /**
     * @brief Configures caret blinking behavior.
     * @param blinks Whether the caret should blink.
     * @param interval_ms The blink interval in milliseconds.
     */
    void setCaretBlink(bool blinks, uint32_t interval_ms = 500);

    /**
     * @brief Called by a timer to toggle the caret blink state.
     * @return true if a re-render is required.
     */
    bool onBlinkTimer();

    /**
     * @brief Gets the number of pages in the current document.
     */
    size_t getPageCount() const { return current_pages_.size(); }

    /**
     * @brief Gets the total physical size of the document (all pages + gaps).
     * @return The Size structure in Twips.
     */
    Size getDocumentBounds() const;

    /**
     * @brief Sets the gap between pages.
     */
    void setPageGap(Twips gap);

    /**
     * @brief Applies a text style to a specific range of characters.
     */
    void applyStyle(uint32_t start, uint32_t length, PropertyId id, PropertyValue value);

    /**
     * @brief Gets the text currently selected by the user.
     * @return A UTF-8 string containing the selection, or empty if no selection.
     */
    std::string getSelectedText() const;
    
    /**
     * @brief Programmatically selects an image at the given logical offset.
     */
    void selectImage(uint32_t offset) {
        selected_image_offset_ = offset;
    }
    
    void insertTableRowAbove();
    void insertTableRowBelow();
    void insertTableColumnLeft();
    void insertTableColumnRight();
    
    void mergeTableCells();
    void splitTableCells(bool horizontally = true);
    void splitTable();

    void selectTableColumn(uint32_t table_offset, int col) {
        table_selection_.mode = TableSelectionMode::Column;
        table_selection_.table_offset = table_offset;
        table_selection_.col = col;
        // updateLayout not needed just for render, but good for completeness
    }
    
    /**
     * @brief Programmatically selects a range of text.
     */
    void setSelection(uint32_t anchor, uint32_t head) {
        selection_.anchor = anchor;
        selection_.head = head;
        // Deselect table/image just in case
        table_selection_.mode = TableSelectionMode::None;
        selected_image_offset_ = std::nullopt;
        updateCursorState();
    }
    
    void deleteSelectedImage();
    void deleteForward();

    void setHeader(const std::string& header);
    void setFooter(const std::string& footer);

    /**
     * @brief Deletes the currently selected text.
     */
    void deleteSelection();

    /**
     * @brief Pastes text at the current cursor position, replacing any active selection.
     * @param text The UTF-8 string to insert.
     */
    void pasteText(const std::string& text);

    /**
     * @brief Sets the current scroll position for render virtualization.
     * @param x Horizontal scroll in Twips.
     * @param y Vertical scroll in Twips.
     */
    void setScroll(Twips x, Twips y);

    /**
     * @brief Event: Key pressed
     * @param keysym Standard keysym (e.g., from xkbcommon).
     * @param mods Active modifier keys.
     * @return true if the editor consumed the event.
     */
    bool onKeyPress(uint32_t keysym, ModifierFlags mods);

    /**
     * @brief Event: Key released
     * @param keysym Standard keysym.
     * @param mods Active modifier keys.
     * @return true if consumed.
     */
    bool onKeyRelease(uint32_t keysym, ModifierFlags mods);

    /**
     * @brief Event: Text input from IM or typing
     * @param text UTF-8 encoded string.
     */
    void onTextInput(const std::string& text);

    /**
     * @brief Event: Mouse button pressed
     * @param x X coordinate relative to editor bounds (Twips or Pixels? Let's assume viewport scale)
     * @param y Y coordinate relative to editor bounds
     */
    bool onMouseDown(double x, double y, MouseButton button, ModifierFlags mods);

    /**
     * @brief Event: Mouse button released
     */
    bool onMouseUp(double x, double y, MouseButton button, ModifierFlags mods);

    /**
     * @brief Event: Mouse pointer moved
     */
    bool onMouseMove(double x, double y, ModifierFlags mods);

    /**
     * @brief Event: Mouse double click
     */
    bool onMouseDoubleClick(double x, double y, MouseButton button, ModifierFlags mods);

    /**
     * @brief Event: Scroll wheel or trackpad
     */
    bool onScroll(double delta_x, double delta_y, ModifierFlags mods);

    /**
     * @brief Renders the current state of the document.
     * @param renderer The target renderer interface.
     */
    void render(IRenderer& renderer);

    /**
     * @brief Finds the table cell at the given absolute coordinates.
     * @return A tuple of (TableBox*, row_idx, col_idx). Returns (nullptr, -1, -1) if no cell is found.
     */
    std::tuple<TableBox*, int, int> findTableCellAt(Twips absolute_x, Twips absolute_y) const;

    /**
     * @brief Exposes the text for test assertions.
     */
    std::string getText() const { return document_.getText(); }
    
    /**
     * @brief Returns the current cursor position.
     */
    uint32_t getCursorOffset() const { return selection_.head; }

    /**
     * @brief Returns the current selection range.
     */
    SelectionRange getSelectionRange() const { return selection_; }

    /**
     * @brief Exposes the format registry for serialization.
     */
    const FormatRegistry& getFormatRegistry() const { return format_registry_; }

    /**
     * @brief Toggles between Insert and Replace text input modes.
     */
    void toggleInsertMode() {
        insert_mode_ = (insert_mode_ == InsertMode::Insert) ? InsertMode::Replace : InsertMode::Insert;
    }

    /**
     * @brief Gets the current text input mode.
     */
    InsertMode getInsertMode() const { return insert_mode_; }

    void insertTextAtCursor(const std::string& text);
    
    /**
     * @brief Performs an undo operation on the document history.
     */
    void undo();

    /**
     * @brief Performs a redo operation on the document history.
     */
    void redo();
    
private:
    void onEditorAction(EditorAction action, const std::string& text, ModifierFlags mods);
    void deleteBackward();
    void updateLayout();

    PieceTable document_;
    UndoManager undo_manager_;
    DOMManager dom_manager_;
    std::shared_ptr<ITextShaper> shaper_;
    std::shared_ptr<IFont> default_font_;
    LayoutEngine layout_engine_;
    LayoutScheduler scheduler_;
    InputRouter input_router_;

    Twips width_{10000};
    Twips height_{10000};
    Twips viewport_x_{0};
    Twips viewport_y_{0};
    Twips page_gap_{400}; // approx 26 pixels gap between pages
    PageSize page_size_{PageSizes::A4};
    PageMargins page_margins_{};
    FormatRegistry format_registry_;
    bool show_margins_{false};
    Color margin_color_{0xAAFF0000};
    Color workspace_bg_color_{0xFFE0E0E0};
    Color page_bg_color_{0xFFFFFFFF};
    Color default_text_color_{0xFF000000};
    Color selection_color_{0x660078D7}; // Standard blue translucent
    bool caret_visible_{true};
    CaretStyle caret_style_{CaretStyle::Line};
    std::optional<Color> caret_color_{std::nullopt};
    bool caret_blinks_{true};
    uint32_t caret_blink_interval_ms_{500};
    bool caret_blink_state_{true};

    SelectionRange selection_{0, 0};
    std::vector<std::unique_ptr<PageBox>> current_pages_;
    
    DragMode drag_mode_{DragMode::None};
    ResizeHandle active_handle_{ResizeHandle::None};
    std::optional<uint32_t> selected_image_offset_;
    Twips drag_start_x_{0}, drag_start_y_{0};
    Twips drag_initial_w_{0}, drag_initial_h_{0};
    Twips drag_start_abs_img_y_{0};
    Twips drag_start_img_height_{0};

    
    std::optional<uint32_t> active_table_offset_;
    int active_table_col_{-1};
    int active_table_row_{-1};
    
    TableSelection table_selection_;
    
    InsertMode insert_mode_{InsertMode::Insert};
    
    std::string header_text_;
    std::string footer_text_;
    std::function<void(const CursorState&)> cursor_state_callback_;
    
    void updateCursorState();
};

} // namespace pluma
