#include <iostream>

#include <pluma/PlumaEditor.hpp>
#include <pluma/Typography/DummyTypography.hpp>

namespace pluma {

PlumaEditor::PlumaEditor(std::shared_ptr<ITextShaper> shaper, std::shared_ptr<IFont> default_font)
    : undo_manager_(document_),
      shaper_(std::move(shaper)),
      default_font_(std::move(default_font)),
      layout_engine_(shaper_, default_font_) {
    
    // Bind the router's action dispatcher to this editor's execution logic
    input_router_.setActionCallback([this](EditorAction action, const std::string& text, ModifierFlags mods) {
        this->onEditorAction(action, text, mods);
    });

    updateLayout();
}

void PlumaEditor::setPageSize(const PageSize& size) {
    page_size_ = size;
    updateLayout();
}

void PlumaEditor::setMargins(const PageMargins& margins) {
    page_margins_ = margins;
    updateLayout();
}

void PlumaEditor::showMargins() {
    show_margins_ = true;
}

void PlumaEditor::hideMargins() {
    show_margins_ = false;
}

void PlumaEditor::setMarginColor(Color color) {
    margin_color_ = color;
}

void PlumaEditor::setWorkspaceBackgroundColor(Color color) {
    workspace_bg_color_ = color;
}

void PlumaEditor::setPageBackgroundColor(Color color) {
    page_bg_color_ = color;
}

void PlumaEditor::setDefaultTextColor(Color color) {
    default_text_color_ = color;
}

void PlumaEditor::setSelectionColor(Color color) {
    selection_color_ = color;
}

void PlumaEditor::setCaretVisible(bool visible) {
    caret_visible_ = visible;
}

void PlumaEditor::setHeader(const std::string& header) {
    header_text_ = header;
    updateLayout();
}

void PlumaEditor::setFooter(const std::string& footer) {
    footer_text_ = footer;
    updateLayout();
}

void PlumaEditor::setCaretStyle(CaretStyle style) {
    caret_style_ = style;
}

void PlumaEditor::setCaretColor(std::optional<Color> color) {
    caret_color_ = color;
}

void PlumaEditor::setCaretBlink(bool blinks, uint32_t interval_ms) {
    caret_blinks_ = blinks;
    caret_blink_interval_ms_ = interval_ms;
    caret_blink_state_ = true;
}

bool PlumaEditor::onBlinkTimer() {
    if (!caret_blinks_ || !caret_visible_ || !selection_.isCollapsed()) {
        return false;
    }
    caret_blink_state_ = !caret_blink_state_;
    return true; // Request re-render
}

Size PlumaEditor::getDocumentBounds() const {
    Twips total_height(0);
    Twips max_width(0);
    
    for (const auto& page : current_pages_) {
        Twips ph = page->getBounds().height;
        Twips pw = page->getBounds().width;
        total_height = total_height + ph + page_gap_;
        if (pw.getValue() > max_width.getValue()) {
            max_width = pw;
        }
    }
    
    if (!current_pages_.empty()) {
        total_height = total_height - page_gap_;
    }
    
    total_height = total_height + page_gap_ + page_gap_;
    max_width = max_width + page_gap_ + page_gap_;
    
    return {max_width, total_height};
}

void PlumaEditor::setPageGap(Twips gap) {
    page_gap_ = gap;
    updateLayout();
}

void PlumaEditor::applyStyle(uint32_t start, uint32_t length, PropertyId id, PropertyValue value) {
    format_registry_.applyStyle(start, length, id, value);
    updateLayout();
}

void PlumaEditor::setViewport(Twips width, Twips height) {
    width_ = width;
    height_ = height;
    // We no longer trigger a full updateLayout() just because the window size changes,
    // because layout depends on the physical page_size_, not the viewport window!
}

bool PlumaEditor::onKeyPress(uint32_t keysym, ModifierFlags mods) {
    return input_router_.handleKeyEvent(keysym, mods);
}

bool PlumaEditor::onKeyRelease(uint32_t keysym, ModifierFlags mods) {
    (void)keysym; (void)mods;
    // Phase 1 stub
    return false;
}

void PlumaEditor::onTextInput(const std::string& text) {
    input_router_.handleTextInput(text);
}

bool PlumaEditor::onMouseDown(double x, double y, MouseButton button, ModifierFlags mods) {
    if (button != MouseButton::Left) return false;

    // Convert pixels to Twips (assuming standard 15 twips per pixel) and add scroll offset
    Twips absolute_x = Twips(static_cast<int32_t>(x * 15.0)) + viewport_x_;
    Twips absolute_y = Twips(static_cast<int32_t>(y * 15.0)) + viewport_y_;

    Twips page_x = current_pages_.empty() ? Twips(0) : Twips(std::max(0, (width_.getValue() - current_pages_[0]->getBounds().width.getValue()) / 2));
    absolute_x = absolute_x - page_x;

    // 1. Check if we hit an image or its resize handle
    Twips current_page_y(page_gap_);
    auto process_blocks = [&](auto& self, const std::vector<std::unique_ptr<BlockBox>>& blocks, Twips base_x, Twips base_y) -> bool {
        for (const auto& block : blocks) {
            Twips block_absolute_y = base_y + block->getBounds().y;
            Twips block_absolute_x = base_x + block->getBounds().x;
            for (const auto& img : block->images) {
                Twips img_abs_x = block_absolute_x + img->getBounds().x;
                Twips img_abs_y = block_absolute_y + img->getBounds().y;
                Rect img_rect{img_abs_x, img_abs_y, img->getBounds().width, img->getBounds().height};
                
                if (selected_image_offset_.has_value() && *selected_image_offset_ == img->logical_offset) {
                    Twips hw(120); // 8px handle width
                    Rect br{img_rect.x + img_rect.width - hw, img_rect.y + img_rect.height - hw, hw, hw};
                    if (br.intersects({absolute_x, absolute_y, Twips(1), Twips(1)})) {
                        drag_mode_ = DragMode::ImageResize;
                        active_handle_ = ResizeHandle::BottomRight;
                        drag_start_x_ = absolute_x;
                        drag_start_y_ = absolute_y;
                        drag_initial_w_ = img_rect.width;
                        drag_initial_h_ = img_rect.height;
                        return true;
                    }
                }
                
                if (img_rect.intersects({absolute_x, absolute_y, Twips(1), Twips(1)})) {
                    selected_image_offset_ = img->logical_offset;
                    selection_.head = selection_.anchor = img->logical_offset;
                    drag_mode_ = DragMode::ImageMove;
                    drag_start_x_ = absolute_x;
                    drag_start_y_ = absolute_y;
                    // Store image's content-relative position (NOT abs which includes margins/page_gap)
                    drag_initial_w_ = img->getBounds().x;  // ImageX: block-relative
                    drag_initial_h_ = img->getBounds().y;  // ImageY: block-relative
                    drag_start_abs_img_y_ = img_abs_y;
                    drag_start_img_height_ = img->getBounds().height;
                    updateLayout();
                    return true;
                }
            }
            if (block->table) {
                Twips table_abs_x = block_absolute_x + block->table->getBounds().x;
                Twips table_abs_y = block_absolute_y + block->table->getBounds().y;
                int row_idx = 0;
                for (const auto& row : block->table->rows) {
                    int col_idx = 0;
                    for (const auto& cell : row->cells) {
                        Twips cell_x = table_abs_x + cell->getBounds().x;
                        Twips cell_y = table_abs_y + row->getBounds().y;
                        Twips cell_w = cell->getBounds().width;
                        Twips cell_h = cell->getBounds().height;

                        Rect left_handler{cell_x - Twips(150), cell_y, Twips(150), cell_h};
                        if (col_idx == 0 && left_handler.intersects({absolute_x, absolute_y, Twips(1), Twips(1)})) {
                            table_selection_.mode = TableSelectionMode::Row;
                            table_selection_.table_offset = block->table->logical_offset;
                            table_selection_.row = row_idx;
                            table_selection_.col = -1;
                            updateLayout();
                            return true;
                        }

                        Rect top_handler{cell_x, cell_y - Twips(150), cell_w, Twips(150)};
                        if (row_idx == 0 && top_handler.intersects({absolute_x, absolute_y, Twips(1), Twips(1)})) {
                            table_selection_.mode = TableSelectionMode::Column;
                            table_selection_.table_offset = block->table->logical_offset;
                            table_selection_.row = -1;
                            table_selection_.col = col_idx;
                            updateLayout();
                            return true;
                        }

                        Rect right_edge{cell_x + cell_w - Twips(45), cell_y, Twips(90), cell_h};
                        if (right_edge.intersects({absolute_x, absolute_y, Twips(1), Twips(1)})) {
                            drag_mode_ = DragMode::TableColResize;
                            active_table_offset_ = block->table->logical_offset;
                            active_table_col_ = col_idx;
                            drag_start_x_ = absolute_x;
                            drag_initial_w_ = cell_w;
                            return true;
                        }

                        Rect bottom_edge{cell_x, cell_y + cell_h - Twips(45), cell_w, Twips(90)};
                        if (bottom_edge.intersects({absolute_x, absolute_y, Twips(1), Twips(1)})) {
                            drag_mode_ = DragMode::TableRowResize;
                            active_table_offset_ = block->table->logical_offset;
                            active_table_row_ = row_idx;
                            drag_start_y_ = absolute_y;
                            drag_initial_h_ = cell_h;
                            return true;
                        }
                        
                        if (self(self, cell->blocks, cell_x, cell_y)) {
                            return true;
                        }
                        
                        col_idx++;
                    }
                    row_idx++;
                }
            }
        }
        return false;
    };

    current_page_y = page_gap_;
    for (const auto& page : current_pages_) {
        if (process_blocks(process_blocks, page->blocks, Twips(0), current_page_y)) {
            return true;
        }
        current_page_y = current_page_y + page->getBounds().height + page_gap_;
    }

    selected_image_offset_ = std::nullopt;
    drag_mode_ = DragMode::Text;
    table_selection_.mode = TableSelectionMode::None; // Clear structural selection when clicking outside handlers

    auto offset_opt = CaretResolver::resolvePhysicalToLogical(current_pages_, absolute_x, absolute_y, page_gap_);
    if (offset_opt.has_value()) {
        selection_.head = *offset_opt;
        
        if (!hasModifier(mods, ModifierFlags::Shift)) {
            selection_.anchor = selection_.head;
        }
        
        updateLayout();
        return true;
    }

    return false;
}

bool PlumaEditor::onMouseUp(double x, double y, MouseButton button, ModifierFlags mods) {
    (void)x; (void)y; (void)mods;
    if (button == MouseButton::Left) {
        if (drag_mode_ == DragMode::ImageMove && selected_image_offset_.has_value()) {
            uint32_t src_offset = *selected_image_offset_;
            uint32_t dest_offset = selection_.head;

            if (src_offset != dest_offset) {
                std::string doc_text = document_.getText();
                std::string tag_content = doc_text.substr(src_offset);
                size_t end_idx = tag_content.find("|", 1);
                if (end_idx != std::string::npos && end_idx > 6 && tag_content.substr(0, 7) == "|IMAGE:") {
                    uint32_t tag_len = end_idx + 1;
                    std::string img_tag = tag_content.substr(0, tag_len);
                    
                    uint32_t actual_src = src_offset;
                    uint32_t actual_len = tag_len;
                    
                    if (actual_src + actual_len < doc_text.length() && doc_text[actual_src + actual_len] == ' ') {
                        actual_len++;
                    }
                    if (actual_src > 0 && doc_text[actual_src - 1] == '\n' && 
                        actual_src + actual_len < doc_text.length() && doc_text[actual_src + actual_len] == '\n') {
                        actual_len++;
                    }
                    
                    PropertyBag bag = format_registry_.getStyleAt(src_offset);

                    undo_manager_.beginTransaction();
                    undo_manager_.addCommand(std::make_unique<DeleteTextCommand>(actual_src, actual_len));
                    format_registry_.deleteText(actual_src, actual_len);

                    if (dest_offset > actual_src) {
                        if (dest_offset >= actual_src + actual_len) {
                            dest_offset -= actual_len;
                        } else {
                            dest_offset = actual_src;
                        }
                    }

                    std::string new_doc_text = document_.getText();
                    uint32_t para_start = dest_offset;
                    while (para_start > 0 && new_doc_text[para_start - 1] != '\n') {
                        para_start--;
                    }
                    dest_offset = para_start;
                    
                    Twips absolute_x = Twips(static_cast<int32_t>(x * 15.0)) + viewport_x_;
                    Twips page_x = current_pages_.empty() ? Twips(0) : Twips(std::max(0, (width_.getValue() - current_pages_[0]->getBounds().width.getValue()) / 2));
                    Twips relative_x = absolute_x - page_x;
                    float new_image_x_pt = relative_x.getValue() / 15.0f;
                    
                    std::string to_insert = img_tag;
                    uint32_t tag_offset = dest_offset;

                    undo_manager_.addCommand(std::make_unique<InsertTextCommand>(dest_offset, to_insert));
                    format_registry_.insertText(dest_offset, to_insert.length());

                    for (auto& [prop_id, prop_val] : bag.getAll()) {
                        format_registry_.applyStyle(tag_offset, tag_len, prop_id, prop_val);
                    }
                    format_registry_.applyStyle(tag_offset, tag_len, PropertyId::ImageX, new_image_x_pt);

                    undo_manager_.commitTransaction();

                    selected_image_offset_ = tag_offset;
                    selection_.anchor = selection_.head = tag_offset;
                    updateLayout();
                    updateCursorState();
                }
            } else {
                std::string doc_text = document_.getText();
                std::string tag_content = doc_text.substr(src_offset);
                size_t end_idx = tag_content.find("|", 1);
                if (end_idx != std::string::npos && end_idx > 6 && tag_content.substr(0, 7) == "|IMAGE:") {
                    uint32_t tag_len = end_idx + 1;
                    Twips absolute_x_up = Twips(static_cast<int32_t>(x * 15.0)) + viewport_x_;
                    Twips absolute_y_up = Twips(static_cast<int32_t>(y * 15.0)) + viewport_y_;
                    Twips page_x = current_pages_.empty() ? Twips(0) : Twips(std::max(0, (width_.getValue() - current_pages_[0]->getBounds().width.getValue()) / 2));
                    absolute_x_up = absolute_x_up - page_x;
                    Twips dx = absolute_x_up - drag_start_x_;
                    Twips dy = absolute_y_up - drag_start_y_;
                    
                    if (std::abs(dx.getValue()) < 45 && std::abs(dy.getValue()) < 45) {
                        drag_mode_ = DragMode::None;
                        active_handle_ = ResizeHandle::None;
                        return true;
                    }
                    
                    Twips new_img_abs_y = drag_start_abs_img_y_ + dy;
                    Twips doc_top = page_gap_ + page_margins_.top;
                    Twips doc_bottom = current_pages_.empty() ? Twips(0) : Twips(current_pages_.size() * (page_size_.height.getValue() + page_gap_.getValue()) + page_gap_.getValue() - page_margins_.bottom.getValue());
                    if (new_img_abs_y.getValue() < doc_top.getValue()) {
                        dy = doc_top - drag_start_abs_img_y_;
                        new_img_abs_y = doc_top;
                    } else if ((new_img_abs_y + drag_start_img_height_).getValue() > doc_bottom.getValue()) {
                        dy = doc_bottom - drag_start_img_height_ - drag_start_abs_img_y_;
                        new_img_abs_y = doc_bottom - drag_start_img_height_;
                    }
                    
                    Twips anchor_search_y = absolute_y_up;
                    auto drop_offset_opt = CaretResolver::resolvePhysicalToLogical(current_pages_, absolute_x_up, anchor_search_y, page_gap_);
                    if (drop_offset_opt.has_value()) {
                        uint32_t new_offset = *drop_offset_opt;
                        
                        std::string full_text = document_.getText();
                        while (new_offset > 0 && full_text[new_offset - 1] != '\n') {
                            new_offset--;
                        }
                        
                        // Prevent dropping inside its own tag
                        if (new_offset >= src_offset && new_offset < src_offset + tag_len) {
                            new_offset = src_offset; // stay in place
                        }
                        
                        undo_manager_.beginTransaction();
                        
                        // Extract properties before deleting
                        PropertyBag img_props = format_registry_.getStyleAt(src_offset);
                        std::string img_tag = tag_content.substr(0, tag_len);
                        
                        if (new_offset != src_offset) {
                            document_.remove(src_offset, tag_len);
                            format_registry_.deleteText(src_offset, tag_len);
                            
                            if (new_offset > src_offset) {
                                new_offset -= tag_len;
                            }
                            
                            document_.insert(new_offset, img_tag);
                            format_registry_.insertText(new_offset, tag_len);
                            for (const auto& kv : img_props.getAll()) {
                                format_registry_.applyStyle(new_offset, tag_len, kv.first, kv.second);
                            }
                        }
                        
                        // To compute ImageY, we need the new anchor paragraph's absolute Y.
                        Twips new_block_abs_y(page_gap_);
                        Twips current_page_y_search(page_gap_);
                        for (const auto& page : current_pages_) {
                            bool found = false;
                            Twips current_block_y = current_page_y_search + page->getBounds().y;
                            for (const auto& blk : page->blocks) {
                                Twips block_abs_y = current_block_y + blk->getBounds().y;
                                bool is_last_block = (&blk == &page->blocks.back());
                                if (anchor_search_y.getValue() >= (block_abs_y + blk->getBounds().height).getValue() && !is_last_block) {
                                    continue;
                                }
                                new_block_abs_y = block_abs_y;
                                found = true;
                                break;
                            }
                            if (found) break;
                            current_page_y_search = current_page_y_search + page->getBounds().height + page_gap_;
                        }
                        
                        Twips new_img_y_relative = new_img_abs_y - new_block_abs_y;
                        
                        float new_image_x_pt = (drag_initial_w_ + dx).getValue() / 15.0f;
                        float new_image_y_pt = new_img_y_relative.getValue() / 15.0f;
                        
                        format_registry_.applyStyle(new_offset, tag_len, PropertyId::ImageX, new_image_x_pt);
                        format_registry_.applyStyle(new_offset, tag_len, PropertyId::ImageY, new_image_y_pt);
                        
                        undo_manager_.commitTransaction();
                        updateLayout();
                        selection_.head = selection_.anchor = new_offset;
                    }
                }
            }
        }

        drag_mode_ = DragMode::None;
        active_handle_ = ResizeHandle::None;
        return true;
    }
    return false;
}

bool PlumaEditor::onMouseMove(double x, double y, ModifierFlags mods) {
    if (drag_mode_ == DragMode::None) return false;

    Twips absolute_x = Twips(static_cast<int32_t>(x * 15.0)) + viewport_x_;
    Twips absolute_y = Twips(static_cast<int32_t>(y * 15.0)) + viewport_y_;

    Twips page_x = current_pages_.empty() ? Twips(0) : Twips(std::max(0, (width_.getValue() - current_pages_[0]->getBounds().width.getValue()) / 2));
    absolute_x = absolute_x - page_x;

    if (drag_mode_ == DragMode::ImageResize && selected_image_offset_.has_value()) {
        Twips dx = absolute_x - drag_start_x_;
        Twips dy = absolute_y - drag_start_y_;
        
        Twips new_w = drag_initial_w_ + dx;
        Twips new_h = drag_initial_h_ + dy;
        
        if (hasModifier(mods, ModifierFlags::Shift)) {
            // Preserve aspect ratio based on initial size
            float ratio = (float)drag_initial_w_.getValue() / (float)drag_initial_h_.getValue();
            new_h = Twips(static_cast<int32_t>((float)new_w.getValue() / ratio));
        }
        
        if (new_w.getValue() < 300) new_w = Twips(300);
        if (new_h.getValue() < 300) new_h = Twips(300);
        
        float w_pt = new_w.getValue() / 15.0f;
        float h_pt = new_h.getValue() / 15.0f;
        
        // Apply directly to format registry for live feedback
        format_registry_.applyStyle(*selected_image_offset_, 1, PropertyId::ImageWidth, w_pt);
        format_registry_.applyStyle(*selected_image_offset_, 1, PropertyId::ImageHeight, h_pt);
        
        updateLayout();
        return true;
    }
    
    if (drag_mode_ == DragMode::TableColResize && active_table_offset_.has_value()) {
        Twips dx = absolute_x - drag_start_x_;
        Twips new_w = drag_initial_w_ + dx;
        if (new_w.getValue() < 300) new_w = Twips(300);

        // Find the table block and redistribute column widths so the total
        // table width stays constant (== content width between margins).

        auto find_table_with_width = [&](auto& self, const std::vector<std::unique_ptr<BlockBox>>& blocks, uint32_t offset) -> std::pair<TableBox*, Twips> {
            for (const auto& block : blocks) {
                if (block->table) {
                    if (block->table->logical_offset == offset) return {block->table.get(), block->getBounds().width};
                    for (const auto& row : block->table->rows) {
                        for (const auto& cell : row->cells) {
                            auto res = self(self, cell->blocks, offset);
                            if (res.first) return res;
                        }
                    }
                }
            }
            return {nullptr, Twips(0)};
        };

        std::pair<TableBox*, Twips> target = {nullptr, Twips(0)};
        for (const auto& page : current_pages_) {
            target = find_table_with_width(find_table_with_width, page->blocks, *active_table_offset_);
            if (target.first) break;
        }

        if (target.first) {
            TableBox* t = target.first;
            Twips block_width = target.second;
            if (!t->rows.empty()) {
                const auto& cells = t->rows[0]->cells;
                int num_cols = (int)cells.size();
                if (num_cols > 0) {
                    std::vector<float> widths(num_cols);
                    float total_w = 0.0f;
                    for (int i = 0; i < num_cols; ++i) {
                        widths[i] = (float)cells[i]->getBounds().width.getValue();
                        total_w += widths[i];
                    }

                    int right_cols = num_cols - active_table_col_ - 1;
                    float left_w = 0.0f;
                    for (int i = 0; i < active_table_col_; ++i) {
                        left_w += widths[i];
                    }
                    
                    float max_w;
                    if (right_cols == 0) {
                        max_w = (float)block_width.getValue() - left_w;
                    } else {
                        max_w = total_w - left_w - (float)(right_cols * 300);
                    }
                    
                    float new_w_f = (float)new_w.getValue();
                    if (new_w_f > max_w) new_w_f = max_w;
                    if (new_w_f < 300.0f) new_w_f = 300.0f;

                    float delta = new_w_f - widths[active_table_col_];

                    float right_total = 0.0f;
                    for (int i = active_table_col_ + 1; i < num_cols; ++i)
                        right_total += widths[i];

                    widths[active_table_col_] = new_w_f;

                    if (right_total > 0.0f && right_cols > 0) {
                        for (int i = active_table_col_ + 1; i < num_cols; ++i) {
                            float share = widths[i] / right_total;
                            widths[i] -= delta * share;
                            if (widths[i] < 300.0f) widths[i] = 300.0f;
                        }
                        float new_total = 0.0f;
                        for (float w : widths) new_total += w;
                        float drift = total_w - new_total;
                        widths[num_cols - 1] += drift;
                        if (widths[num_cols - 1] < 300.0f) widths[num_cols - 1] = 300.0f;
                    }

                    std::string new_widths;
                    for (int i = 0; i < num_cols; ++i) {
                        if (i > 0) new_widths += ",";
                        new_widths += std::to_string(widths[i] / 15.0f);
                    }
                    format_registry_.applyStyle(*active_table_offset_, 5, PropertyId::TableColumnWidths, new_widths);
                }
            }
        }

        updateLayout();
        return true;
    }


    if (drag_mode_ == DragMode::TableRowResize && active_table_offset_.has_value()) {
        Twips dy = absolute_y - drag_start_y_;
        Twips new_h = drag_initial_h_ + dy;
        if (new_h.getValue() < 300) new_h = Twips(300);
        
        auto find_table = [&](auto& self, const std::vector<std::unique_ptr<BlockBox>>& blocks, uint32_t offset) -> TableBox* {
            for (const auto& block : blocks) {
                if (block->table) {
                    if (block->table->logical_offset == offset) return block->table.get();
                    for (const auto& row : block->table->rows) {
                        for (const auto& cell : row->cells) {
                            if (auto t = self(self, cell->blocks, offset)) return t;
                        }
                    }
                }
            }
            return nullptr;
        };

        TableBox* target_table = nullptr;
        for (const auto& page : current_pages_) {
            target_table = find_table(find_table, page->blocks, *active_table_offset_);
            if (target_table) break;
        }

        if (target_table) {
            std::string new_heights = "";
            int i = 0;
            for (const auto& row : target_table->rows) {
                float h_pt = (i == active_table_row_) ? (new_h.getValue() / 15.0f) : (row->getBounds().height.getValue() / 15.0f);
                if (i > 0) new_heights += ",";
                new_heights += std::to_string(h_pt);
                i++;
            }
            format_registry_.applyStyle(*active_table_offset_, 5, PropertyId::TableRowHeights, new_heights);
        }

        updateLayout();
        return true;
    }

    if (drag_mode_ == DragMode::ImageMove && selected_image_offset_.has_value()) {
        uint32_t src_offset = *selected_image_offset_;
        std::string doc_text = document_.getText();
        std::string tag_content = doc_text.substr(src_offset);
        size_t end_idx = tag_content.find("|", 1);
        if (end_idx != std::string::npos && end_idx > 6 && tag_content.substr(0, 7) == "|IMAGE:") {
            uint32_t tag_len = end_idx + 1;
            PropertyBag bag = format_registry_.getStyleAt(src_offset);
            if (auto wrap_val = bag.get(PropertyId::ImageWrapMode)) {
                auto mode = std::get<TextWrapMode>(*wrap_val);
                if (mode == TextWrapMode::Square || mode == TextWrapMode::Tight || mode == TextWrapMode::Through) {
                    // Delta-based: new_pos = initial_img_content_pos + mouse_delta
                    Twips dx = absolute_x - drag_start_x_;
                    Twips dy = absolute_y - drag_start_y_;
                    
                    Twips new_img_abs_y = drag_start_abs_img_y_ + dy;
                    Twips doc_top = page_gap_ + page_margins_.top;
                    Twips doc_bottom = current_pages_.empty() ? Twips(0) : Twips(current_pages_.size() * (page_size_.height.getValue() + page_gap_.getValue()) + page_gap_.getValue() - page_margins_.bottom.getValue());
                    if (new_img_abs_y.getValue() < doc_top.getValue()) {
                        dy = doc_top - drag_start_abs_img_y_;
                    } else if ((new_img_abs_y + drag_start_img_height_).getValue() > doc_bottom.getValue()) {
                        dy = doc_bottom - drag_start_img_height_ - drag_start_abs_img_y_;
                    }

                    Twips new_img_x = drag_initial_w_ + dx;
                    Twips new_img_y = drag_initial_h_ + dy;
                    float new_image_x_pt = new_img_x.getValue() / 15.0f;
                    float new_image_y_pt = new_img_y.getValue() / 15.0f;
                    format_registry_.applyStyle(src_offset, tag_len, PropertyId::ImageX, new_image_x_pt);
                    format_registry_.applyStyle(src_offset, tag_len, PropertyId::ImageY, new_image_y_pt);
                    updateLayout();
                    return true;
                }
            }
            
            // InLine or TopAndBottom mode: Option A (Classic text drag).
            // Resolve the logical offset at the current mouse position.
            // For Option A, the user expects the caret to follow their mouse pointer
            Twips anchor_search_y = absolute_y;
            auto drop_offset_opt = CaretResolver::resolvePhysicalToLogical(current_pages_, absolute_x, anchor_search_y, page_gap_);
            if (drop_offset_opt.has_value()) {
                uint32_t new_offset = *drop_offset_opt;
                std::string full_text = document_.getText();
                while (new_offset > 0 && full_text[new_offset - 1] != '\n') {
                    new_offset--;
                }
                
                if (selection_.head != new_offset) {
                    selection_.head = new_offset;
                    selection_.anchor = new_offset;
                    caret_blink_state_ = true; // Force caret visible while dragging
                    updateLayout();
                }
            }
        }
        return true;
    }

    if (drag_mode_ == DragMode::Text) {
        auto offset_opt = CaretResolver::resolvePhysicalToLogical(current_pages_, absolute_x, absolute_y, page_gap_);
        if (offset_opt.has_value()) {
            if (selection_.head != *offset_opt) {
                selection_.head = *offset_opt;
                updateLayout();
            }
            return true;
        }
    }

    return false;
}

bool PlumaEditor::onMouseDoubleClick(double x, double y, MouseButton button, ModifierFlags mods) {
    (void)mods;
    if (button != MouseButton::Left) return false;

    Twips absolute_x = Twips(static_cast<int32_t>(x * 15.0)) + viewport_x_;
    Twips absolute_y = Twips(static_cast<int32_t>(y * 15.0)) + viewport_y_;

    auto offset_opt = CaretResolver::resolvePhysicalToLogical(current_pages_, absolute_x, absolute_y, page_gap_);
    if (offset_opt.has_value()) {
        uint32_t offset = *offset_opt;
        std::string text = document_.getText();
        
        if (text.empty()) return true;

        // Find word start
        uint32_t start = offset;
        while (start > 0 && std::isalnum(text[start - 1])) {
            start--;
        }

        // Find word end
        uint32_t end = offset;
        while (end < text.length() && std::isalnum(text[end])) {
            end++;
        }

        selection_.anchor = start;
        selection_.head = end;
        
        updateLayout();
        updateCursorState();
        return true;
    }

    return false;
}

bool PlumaEditor::onScroll(double delta_x, double delta_y, ModifierFlags mods) {
    (void)delta_x; (void)delta_y; (void)mods;
    // Phase 1 stub
    return false;
}

void PlumaEditor::onEditorAction(EditorAction action, const std::string& text, ModifierFlags mods) {
    caret_blink_state_ = true; // Reset blink state on any action to keep it visible while typing
    
    switch (action) {
        case EditorAction::InsertText:
            if (selected_image_offset_.has_value()) {
                deleteSelectedImage();
            }
            insertTextAtCursor(text);
            break;
        case EditorAction::DeleteBackward:
        case EditorAction::DeleteForward:
            if (selected_image_offset_.has_value()) {
                deleteSelectedImage();
            } else if (action == EditorAction::DeleteBackward) {
                deleteBackward();
            } else {
                deleteForward();
            }
            break;
        case EditorAction::Undo:
            undo_manager_.undo();
            // Very naive cursor restoration for now
            selection_.head = selection_.anchor = document_.getLength();
            updateLayout();
            updateCursorState();
            break;
        case EditorAction::Redo:
            undo_manager_.redo();
            selection_.head = selection_.anchor = document_.getLength();
            updateLayout();
            updateCursorState();
            break;
        case EditorAction::MoveCursorLeft:
            if (selection_.head > 0) {
                selection_.head--;
                // Skip non-renderable positions (table tag characters like |ROW|, |CEL|…)
                // A position is non-renderable when it has no run box in the layout.
                while (selection_.head > 0 &&
                       !CaretResolver::resolveLogicalToPhysical(current_pages_, selection_.head, page_gap_)) {
                    selection_.head--;
                }
                if (!hasModifier(mods, ModifierFlags::Shift)) selection_.anchor = selection_.head;
                updateCursorState();
            }
            break;
        case EditorAction::MoveCursorRight:
            if (selection_.head < document_.getLength()) {
                selection_.head++;
                // Skip non-renderable positions (table tag characters)
                while (selection_.head < document_.getLength() &&
                       !CaretResolver::resolveLogicalToPhysical(current_pages_, selection_.head, page_gap_)) {
                    selection_.head++;
                }
                if (!hasModifier(mods, ModifierFlags::Shift)) selection_.anchor = selection_.head;
                updateCursorState();
            }
            break;
        case EditorAction::MoveCursorLeftWord:
            if (selection_.head > 0) {
                std::string content = document_.getText();
                uint32_t pos = selection_.head;
                while (pos > 0 && std::isspace(content[pos - 1])) pos--;
                while (pos > 0 && !std::isspace(content[pos - 1])) pos--;
                selection_.head = pos;
                if (!hasModifier(mods, ModifierFlags::Shift)) selection_.anchor = selection_.head;
                updateCursorState();
            }
            break;
        case EditorAction::MoveCursorRightWord:
            if (selection_.head < document_.getLength()) {
                std::string content = document_.getText();
                uint32_t pos = selection_.head;
                uint32_t len = document_.getLength();
                while (pos < len && !std::isspace(content[pos])) pos++;
                while (pos < len && std::isspace(content[pos])) pos++;
                selection_.head = pos;
                if (!hasModifier(mods, ModifierFlags::Shift)) selection_.anchor = selection_.head;
                updateCursorState();
            }
            break;
        case EditorAction::MoveCursorUp: {
            if (auto rect = CaretResolver::resolveLogicalToPhysical(current_pages_, selection_.head, page_gap_)) {
                // Use half a line-height (90 twips) so we reliably land in the line above
                Twips half_line(90);
                Twips target_y = Twips(std::max(0, rect->y.getValue() - half_line.getValue()));
                if (auto new_offset = CaretResolver::resolvePhysicalToLogical(current_pages_, rect->x, target_y, page_gap_)) {
                    if (*new_offset != selection_.head) {
                        selection_.head = *new_offset;
                        if (!hasModifier(mods, ModifierFlags::Shift)) selection_.anchor = selection_.head;
                        updateCursorState();
                    }
                }
            }
            break;
        }
        case EditorAction::MoveCursorDown: {
            if (auto rect = CaretResolver::resolveLogicalToPhysical(current_pages_, selection_.head, page_gap_)) {
                // Land in the middle of the line below
                Twips target_y = rect->y + rect->height + Twips(rect->height.getValue() / 2);
                if (auto new_offset = CaretResolver::resolvePhysicalToLogical(current_pages_, rect->x, target_y, page_gap_)) {
                    if (*new_offset != selection_.head) {
                        selection_.head = *new_offset;
                        if (!hasModifier(mods, ModifierFlags::Shift)) selection_.anchor = selection_.head;
                        updateCursorState();
                    }
                }
            }
            break;
        }
        case EditorAction::Cut:
            deleteSelection();
            break;
        case EditorAction::ToggleInsertMode:
            toggleInsertMode();
            break;
        default:
            break;
    }
}

void PlumaEditor::undo() {
    onEditorAction(EditorAction::Undo, "", ModifierFlags::None);
}

void PlumaEditor::redo() {
    onEditorAction(EditorAction::Redo, "", ModifierFlags::None);
}

void PlumaEditor::insertTextAtCursor(const std::string& text) {
    if (!selection_.isCollapsed()) {
        deleteSelection();
    } else if (insert_mode_ == InsertMode::Replace) {
        // Replace mode: Overwrite characters if not at EOF
        uint32_t current_len = document_.getLength();
        if (selection_.head < current_len) {
            std::string existing = document_.getText().substr(selection_.head, 1);
            if (existing != "\n") {
                undo_manager_.beginTransaction();
                undo_manager_.addCommand(std::make_unique<DeleteTextCommand>(selection_.head, text.length()));
                format_registry_.deleteText(selection_.head, text.length());
                undo_manager_.addCommand(std::make_unique<InsertTextCommand>(selection_.head, text));
                format_registry_.insertText(selection_.head, text.length());
                undo_manager_.commitTransaction();
                
                selection_.head = selection_.anchor = selection_.head + text.length();
                updateLayout();
                updateCursorState();
                return;
            }
        }
    }

    std::string text_to_insert = text;
    if (text == "\n") {
        uint32_t para_start = selection_.getStart();
        std::string doc_text = document_.getText();
        while (para_start > 0 && doc_text[para_start - 1] != '\n') {
            para_start--;
        }
        std::string para_text = doc_text.substr(para_start, selection_.getStart() - para_start);
        
        std::string copied_tags = "";
        size_t search_start = 0;
        if (para_text.length() >= 8 && para_text.substr(0, 8) == "|INDENT:") {
            size_t end_tag = para_text.find("|", 8);
            if (end_tag != std::string::npos) {
                search_start = end_tag + 1;
                copied_tags += para_text.substr(0, search_start);
            }
        }
        
        if (para_text.length() >= search_start + 4 && 
           (para_text.substr(search_start, 4) == "|UL:" || para_text.substr(search_start, 4) == "|OL:")) {
            size_t end_list_tag = para_text.find("|", search_start + 4);
            if (end_list_tag != std::string::npos) {
                copied_tags += para_text.substr(search_start, end_list_tag + 1 - search_start);
            }
        }
        
        if (!copied_tags.empty()) {
            if (para_text == copied_tags) {
                undo_manager_.beginTransaction();
                undo_manager_.addCommand(std::make_unique<DeleteTextCommand>(para_start, copied_tags.length()));
                format_registry_.deleteText(para_start, copied_tags.length());
                undo_manager_.commitTransaction();
                
                selection_.head = selection_.anchor = para_start;
                updateLayout();
                updateCursorState();
                return;
            } else {
                text_to_insert += copied_tags;
            }
        }
    }

    uint32_t start = selection_.getStart();
    undo_manager_.beginTransaction();
    undo_manager_.addCommand(std::make_unique<InsertTextCommand>(start, text_to_insert));
    format_registry_.insertText(start, text_to_insert.length());
    undo_manager_.commitTransaction();
    
    selection_.head = selection_.anchor = start + text_to_insert.length();
    updateLayout();
    updateCursorState();
}

void PlumaEditor::deleteBackward() {
    if (!selection_.isCollapsed()) {
        deleteSelection();
    } else if (selection_.head > 0) {
        uint32_t para_start = selection_.head;
        std::string doc_text = document_.getText();
        while (para_start > 0 && doc_text[para_start - 1] != '\n') {
            para_start--;
        }
        std::string para_text = doc_text.substr(para_start, selection_.head - para_start);
        
        size_t search_start = 0;
        if (para_text.length() >= 8 && para_text.substr(0, 8) == "|INDENT:") {
            size_t end_tag = para_text.find("|", 8);
            if (end_tag != std::string::npos) search_start = end_tag + 1;
        }
        
        size_t end_list_tag = std::string::npos;
        if (para_text.length() >= search_start + 4 && 
           (para_text.substr(search_start, 4) == "|UL:" || para_text.substr(search_start, 4) == "|OL:")) {
            end_list_tag = para_text.find("|", search_start + 4);
        }
        
        size_t total_tags_length = 0;
        if (end_list_tag != std::string::npos) {
            total_tags_length = end_list_tag + 1;
        } else if (search_start > 0) {
            total_tags_length = search_start;
        }
        
        if (para_text.length() >= 7 && para_text.substr(0, 7) == "|IMAGE:") {
            size_t end_img = para_text.find("|", 7);
            if (end_img != std::string::npos) {
                total_tags_length = end_img + 1;
            }
        }

        if (total_tags_length > 0 && selection_.head > para_start && selection_.head <= para_start + total_tags_length) {
            undo_manager_.beginTransaction();
            undo_manager_.addCommand(std::make_unique<DeleteTextCommand>(para_start, total_tags_length));
            format_registry_.deleteText(para_start, total_tags_length);
            undo_manager_.commitTransaction();
            selection_.head = selection_.anchor = para_start;
            updateLayout();
            updateCursorState();
            return;
        }
        
        // If cursor is at the start of an image paragraph and we press Backspace, delete the image instead of merging
        if (selection_.head == para_start && para_text.length() >= 7 && para_text.substr(0, 7) == "|IMAGE:") {
            size_t end_img = para_text.find("|", 7);
            if (end_img != std::string::npos) {
                size_t len = end_img + 1;
                undo_manager_.beginTransaction();
                undo_manager_.addCommand(std::make_unique<DeleteTextCommand>(para_start, len));
                format_registry_.deleteText(para_start, len);
                undo_manager_.commitTransaction();
                updateLayout();
                updateCursorState();
                return;
            }
        }

        uint32_t start = selection_.head - 1;
        
        // Also check if we are deleting the newline right before an image, to prevent merging
        if (start < doc_text.length() && doc_text[start] == '\n') {
            uint32_t next_para_start = start + 1;
            if (next_para_start + 7 <= doc_text.length() && doc_text.substr(next_para_start, 7) == "|IMAGE:") {
                size_t end_img = doc_text.find("|", next_para_start + 7);
                if (end_img != std::string::npos) {
                    size_t len = end_img - next_para_start + 1;
                    undo_manager_.beginTransaction();
                    undo_manager_.addCommand(std::make_unique<DeleteTextCommand>(next_para_start, len));
                    format_registry_.deleteText(next_para_start, len);
                    undo_manager_.commitTransaction();
                    updateLayout();
                    updateCursorState();
                    return; // Deleted the image instead of the newline
                }
            }
        }

        undo_manager_.beginTransaction();
        undo_manager_.addCommand(std::make_unique<DeleteTextCommand>(start, 1));
        format_registry_.deleteText(start, 1);
        undo_manager_.commitTransaction();
        selection_.head = selection_.anchor = start;
        updateLayout();
        updateCursorState();
    }
}

void PlumaEditor::deleteForward() {
    if (!selection_.isCollapsed()) {
        deleteSelection();
    } else if (selection_.head < document_.getLength()) {
        uint32_t start = selection_.head;
        std::string doc_text = document_.getText();
        
        if (start + 7 <= doc_text.length() && doc_text.substr(start, 7) == "|IMAGE:") {
            size_t end_img = doc_text.find("|", start + 7);
            if (end_img != std::string::npos) {
                size_t len = end_img - start + 1;
                undo_manager_.beginTransaction();
                undo_manager_.addCommand(std::make_unique<DeleteTextCommand>(start, len));
                format_registry_.deleteText(start, len);
                undo_manager_.commitTransaction();
                updateLayout();
                updateCursorState();
                return;
            }
        }
        
        undo_manager_.beginTransaction();
        undo_manager_.addCommand(std::make_unique<DeleteTextCommand>(start, 1));
        format_registry_.deleteText(start, 1);
        undo_manager_.commitTransaction();
        updateLayout();
        updateCursorState();
    }
}

void PlumaEditor::deleteSelectedImage() {
    if (!selected_image_offset_.has_value()) return;
    
    uint32_t offset = *selected_image_offset_;
    std::string text_str = document_.getText();
    
    if (offset < text_str.length() && text_str.substr(offset, 7) == "|IMAGE:") {
        size_t end_pos = text_str.find('|', offset + 7);
        if (end_pos != std::string::npos) {
            uint32_t len = end_pos - offset + 1;
            
            // Check for trailing space as generated by LayoutEngine
            if (offset + len < text_str.length() && text_str[offset + len] == ' ') {
                len++;
            }
            
            undo_manager_.beginTransaction();
            undo_manager_.addCommand(std::make_unique<DeleteTextCommand>(offset, len));
            format_registry_.deleteText(offset, len);
            undo_manager_.commitTransaction();
            
            selection_.head = selection_.anchor = offset;
            selected_image_offset_ = std::nullopt;
            drag_mode_ = DragMode::None;
            updateLayout();
            updateCursorState();
        }
    }
}

void PlumaEditor::updateLayout() {
    // In a full implementation, we'd sync the DOM and use InvalidationTracker.
    // For the facade glue, a full rebuild ensures consistency.
    current_pages_ = layout_engine_.layoutText(document_.getText(), page_size_, page_margins_, format_registry_, header_text_, footer_text_);
    updateCursorState();
}

std::string PlumaEditor::getSelectedText() const {
    if (selection_.isCollapsed()) return "";
    return document_.getText().substr(selection_.getStart(), selection_.getLength());
}

void PlumaEditor::deleteSelection() {
    if (!selection_.isCollapsed()) {
        uint32_t start = selection_.getStart();
        undo_manager_.beginTransaction();
        undo_manager_.addCommand(std::make_unique<DeleteTextCommand>(start, selection_.getLength()));
        format_registry_.deleteText(start, selection_.getLength());
        undo_manager_.commitTransaction();
        selection_.head = selection_.anchor = start;
        updateLayout();
    }
}

void PlumaEditor::pasteText(const std::string& text) {
    if (text.empty()) return;
    
    undo_manager_.beginTransaction();
    uint32_t insert_pos = selection_.head;
    
    if (!selection_.isCollapsed()) {
        insert_pos = selection_.getStart();
        undo_manager_.addCommand(std::make_unique<DeleteTextCommand>(insert_pos, selection_.getLength()));
        format_registry_.deleteText(insert_pos, selection_.getLength());
    }
    
    undo_manager_.addCommand(std::make_unique<InsertTextCommand>(insert_pos, text));
    format_registry_.insertText(insert_pos, text.length());
    undo_manager_.commitTransaction();
    
    selection_.head = selection_.anchor = insert_pos + text.length();
    updateLayout();
}

void PlumaEditor::loadText(const std::string& text) {
    undo_manager_.beginTransaction();
    if (document_.getLength() > 0) {
        undo_manager_.addCommand(std::make_unique<DeleteTextCommand>(0, document_.getLength()));
        format_registry_.clear();
    }
    if (!text.empty()) {
        undo_manager_.addCommand(std::make_unique<InsertTextCommand>(0, text));
        // text is clean, no styles to insert since we just cleared
    }
    undo_manager_.commitTransaction();

    selection_ = {static_cast<uint32_t>(text.length()), static_cast<uint32_t>(text.length())};
    updateLayout();
}

std::shared_ptr<DocumentSnapshot> PlumaEditor::getSnapshot() {
    return document_.createSnapshot();
}

void PlumaEditor::setScroll(Twips x, Twips y) {
    viewport_x_ = x;
    viewport_y_ = y;
}

void PlumaEditor::render(IRenderer& renderer) {
    // Draw workspace background for the viewport
    renderer.drawRect({Twips(0), Twips(0), width_, height_}, workspace_bg_color_);

    DisplayList display_list;

    // Viewport rect in document coordinates (Prefetch margin: 1 page height)
    Twips prefetch = height_;
    Twips view_y_min = Twips(std::max(0, viewport_y_.getValue() - prefetch.getValue()));
    Twips view_y_max = viewport_y_ + height_ + prefetch;
    
    Rect viewport_rect{viewport_x_, view_y_min, width_, Twips(view_y_max.getValue() - view_y_min.getValue())};

    std::function<void(const std::unique_ptr<TableBox>&, Twips, Twips)> renderTable;
    renderTable = [&](const std::unique_ptr<TableBox>& table, Twips table_abs_x, Twips table_abs_y) {
        int row_idx = 0;
        for (const auto& row : table->rows) {
            int col_idx = 0;
            for (const auto& cell : row->cells) {
                Twips cx = table_abs_x + cell->getBounds().x;
                Twips cy = table_abs_y + row->getBounds().y;
                Twips cw = cell->getBounds().width;
                Twips ch = cell->getBounds().height;
                
                Twips thick(15); // 1px
                uint8_t r8 = (default_text_color_ >> 16) & 0xFF;
                uint8_t g8 = (default_text_color_ >>  8) & 0xFF;
                uint8_t b8 = (default_text_color_      ) & 0xFF;
                Color bcol = (0x99u << 24) | (r8 << 16) | (g8 << 8) | b8; // ~60% alpha
                
                bool draw_top = true, draw_bottom = true, draw_left = true, draw_right = true;
                if (table->hide_most_borders) {
                    draw_top = draw_left = draw_right = false;
                    draw_bottom = (row_idx == 0); // Only bottom border of the first row
                }

                if (draw_top) display_list.addCommand(std::make_unique<FillRectCommand>(Rect{cx, cy, cw, thick}, bcol));
                if (draw_bottom) display_list.addCommand(std::make_unique<FillRectCommand>(Rect{cx, cy + ch - thick, cw, thick}, bcol));
                if (draw_left) display_list.addCommand(std::make_unique<FillRectCommand>(Rect{cx, cy, thick, ch}, bcol));
                if (draw_right) display_list.addCommand(std::make_unique<FillRectCommand>(Rect{cx + cw - thick, cy, thick, ch}, bcol));

                bool is_selected = false;
                if (table_selection_.mode != TableSelectionMode::None && table_selection_.table_offset == table->logical_offset) {
                    if (table_selection_.mode == TableSelectionMode::Table) is_selected = true;
                    else if (table_selection_.mode == TableSelectionMode::Cell && table_selection_.col == col_idx && table_selection_.row == row_idx) is_selected = true;
                    else if (table_selection_.mode == TableSelectionMode::Row && table_selection_.row == row_idx) is_selected = true;
                    else if (table_selection_.mode == TableSelectionMode::Column && table_selection_.col == col_idx) is_selected = true;
                }
                
                if (is_selected) {
                    display_list.addCommand(std::make_unique<FillRectCommand>(Rect{cx, cy, cw, ch}, selection_color_));
                }

                Color cell_default_text_color = default_text_color_;

                for (const auto& cell_block : cell->blocks) {
                    for (const auto& cell_line : cell_block->lines) {
                        for (const auto& run : cell_line->runs) {
                            Twips draw_x = cx + cell_block->getBounds().x + cell_line->getBounds().x + run->getBounds().x;
                            Twips draw_y = cy + cell_block->getBounds().y + cell_line->getBounds().y + run->getBounds().y;
                            Rect run_rect{draw_x, draw_y, run->getBounds().width, run->getBounds().height};

                            Color text_color = cell_default_text_color;
                            if (auto c = run->style.get(PropertyId::TextColor))
                                text_color = std::get<Color>(*c);

                            FontDescriptor desc = default_font_->getDescriptor();
                            if (auto fs = run->style.get(PropertyId::FontSize))   desc.size_pt = std::get<float>(*fs);
                            if (auto ff = run->style.get(PropertyId::FontFamily))  desc.family  = std::get<std::string>(*ff);
                            if (auto fw = run->style.get(PropertyId::FontWeight))  desc.weight  = static_cast<FontWeight>(std::get<uint16_t>(*fw));
                            if (auto fi = run->style.get(PropertyId::FontStyleItalic)) desc.italic = std::get<bool>(*fi);
                            auto run_font = std::make_shared<DummyFont>(desc);

                            if (!selection_.isCollapsed() && run->logical_offset != UINT32_MAX) {
                                uint32_t run_start  = run->logical_offset;
                                uint32_t run_end    = run_start + (uint32_t)run->logical_text.length();
                                uint32_t sel_start  = selection_.getStart();
                                uint32_t sel_end    = selection_.getEnd();
                                uint32_t ovl_start  = std::max(run_start, sel_start);
                                uint32_t ovl_end    = std::min(run_end,   sel_end);
                                if (ovl_start < ovl_end) {
                                    Twips sel_x(0), sel_w(0), cur_x(0);
                                    for (size_t gi = 0; gi < run->run.glyphs.size(); ++gi) {
                                        uint32_t ci  = run_start + (uint32_t)gi;
                                        Twips adv    = run->run.glyphs[gi].x_advance;
                                        if (ci >= ovl_start && ci < ovl_end) {
                                            if (ci == ovl_start) sel_x = cur_x;
                                            sel_w = sel_w + adv;
                                        }
                                        cur_x = cur_x + adv;
                                    }
                                    Rect sel_rect{run_rect.x + sel_x, run_rect.y, sel_w, run_rect.height};
                                    display_list.addCommand(std::make_unique<FillRectCommand>(sel_rect, selection_color_));
                                }
                            }

                            display_list.addCommand(std::make_unique<DrawGlyphRunCommand>(
                                run_rect,
                                run->run,
                                run->logical_text,
                                run_font,
                                text_color
                            ));
                        }
                    }
                    
                    for (const auto& img : cell_block->images) {
                        Twips img_abs_x = cx + cell_block->getBounds().x + img->getBounds().x;
                        Twips img_abs_y = cy + cell_block->getBounds().y + img->getBounds().y;
                        Rect img_rect{img_abs_x, img_abs_y, img->getBounds().width, img->getBounds().height};
                        display_list.addCommand(std::make_unique<DrawImageCommand>(img_rect, img->path));
                        
                        if (selected_image_offset_.has_value() && *selected_image_offset_ == img->logical_offset) {
                            Twips hw(120); // 8px
                            Color hc(0xFF0078D7); // Blue handles
                            
                            display_list.addCommand(std::make_unique<FillRectCommand>(Rect{img_rect.x, img_rect.y, hw, hw}, hc));
                            display_list.addCommand(std::make_unique<FillRectCommand>(Rect{img_rect.x + img_rect.width - hw, img_rect.y, hw, hw}, hc));
                            display_list.addCommand(std::make_unique<FillRectCommand>(Rect{img_rect.x, img_rect.y + img_rect.height - hw, hw, hw}, hc));
                            display_list.addCommand(std::make_unique<FillRectCommand>(Rect{img_rect.x + img_rect.width - hw, img_rect.y + img_rect.height - hw, hw, hw}, hc));
                        }
                    }
                    
                    if (cell_block->table) {
                        renderTable(cell_block->table, cx + cell_block->getBounds().x, cy + cell_block->getBounds().y);
                    }
                }
                col_idx++;
            }
            row_idx++;
        }
    };

    Twips current_page_y(page_gap_);
    for (const auto& page : current_pages_) {
        Twips page_x = Twips(std::max(0, (width_.getValue() - page->getBounds().width.getValue()) / 2));
        Rect page_rect{page_x, current_page_y, page->getBounds().width, page->getBounds().height};
        
        // Render Virtualization: Cull pages entirely outside the viewport
        if (!viewport_rect.intersects(page_rect)) {
            current_page_y = current_page_y + page->getBounds().height + page_gap_;
            continue;
        }

        // Draw page background
        display_list.addCommand(std::make_unique<FillRectCommand>(
            Rect{page_rect.x - viewport_x_, page_rect.y - viewport_y_, page_rect.width, page_rect.height},
            page_bg_color_
        ));

        if (show_margins_) {
            Twips m_left = page_margins_.left;
            Twips m_top = page_margins_.top;
            Twips m_right = page_margins_.right;
            Twips m_bottom = page_margins_.bottom;

            Twips w = page->getBounds().width;
            Twips h = page->getBounds().height;
            Twips t = Twips(15); // 1 pixel stroke approx

            // Top margin line
            display_list.addCommand(std::make_unique<FillRectCommand>(
                Rect{page_rect.x + m_left - viewport_x_, current_page_y + m_top - viewport_y_, w - m_left - m_right, t},
                margin_color_
            ));
            // Bottom margin line
            display_list.addCommand(std::make_unique<FillRectCommand>(
                Rect{page_rect.x + m_left - viewport_x_, current_page_y + h - m_bottom - viewport_y_, w - m_left - m_right, t},
                margin_color_
            ));
            // Left margin line
            display_list.addCommand(std::make_unique<FillRectCommand>(
                Rect{page_rect.x + m_left - viewport_x_, current_page_y + m_top - viewport_y_, t, h - m_top - m_bottom},
                margin_color_
            ));
            // Right margin line
            display_list.addCommand(std::make_unique<FillRectCommand>(
                Rect{page_rect.x + w - m_right - viewport_x_, current_page_y + m_top - viewport_y_, t, h - m_top - m_bottom + t},
                margin_color_
            ));
        }

        auto render_block = [&](const BlockBox* block, Twips absolute_x, Twips absolute_y) {
            Rect block_rect{absolute_x + block->getBounds().x, absolute_y, block->getBounds().width, block->getBounds().height};
            
            // Cull blocks
            if (!viewport_rect.intersects(block_rect)) {
                return;
            }

            for (const auto& line : block->lines) {
                // Apply Paragraph Alignment
                Twips remaining_space = Twips(block->getBounds().width.getValue() - line->getBounds().width.getValue());
                if (remaining_space.getValue() < 0) remaining_space = Twips(0);
                
                Twips align_offset(0);
                Twips justify_gap(0);
                bool is_last_line = (&line == &block->lines.back());
                if (block->alignment == TextAlign::Center) {
                    align_offset = Twips(remaining_space.getValue() / 2);
                } else if (block->alignment == TextAlign::Right) {
                    align_offset = remaining_space;
                } else if (block->alignment == TextAlign::Justify && !is_last_line && line->runs.size() > 1) {
                    justify_gap = Twips(remaining_space.getValue() / (line->runs.size() - 1));
                }

                Twips line_base_x = absolute_x + block->getBounds().x + line->getBounds().x + align_offset;
                Twips current_justify_offset(0);

                for (const auto& run : line->runs) {
                    // Physical drawing rect relative to the viewport (scroll offset applied)
                    Twips draw_x = line_base_x + run->getBounds().x + current_justify_offset - viewport_x_;
                    Twips draw_y = absolute_y + line->getBounds().y - viewport_y_;

                    Rect run_rect{
                        draw_x,
                        draw_y,
                        run->getBounds().width,
                        run->getBounds().height
                    };
                    
                    current_justify_offset = current_justify_offset + justify_gap;
                    
                    // Extract styles
                    uint32_t text_color = default_text_color_;
                    if (auto c = run->style.get(PropertyId::TextColor)) {
                        text_color = std::get<Color>(*c);
                    }

                    FontDescriptor desc = default_font_->getDescriptor();
                    float original_size = desc.size_pt;
                    if (auto fs = run->style.get(PropertyId::FontSize)) {
                        desc.size_pt = std::get<float>(*fs);
                        original_size = desc.size_pt;
                    }
                    if (auto ff = run->style.get(PropertyId::FontFamily)) desc.family = std::get<std::string>(*ff);
                    if (auto fw = run->style.get(PropertyId::FontWeight)) desc.weight = static_cast<FontWeight>(std::get<uint16_t>(*fw));
                    if (auto fi = run->style.get(PropertyId::FontStyleItalic)) desc.italic = std::get<bool>(*fi);

                    VerticalAlign v_align = VerticalAlign::Baseline;
                    if (auto va = run->style.get(PropertyId::VerticalAlignment)) {
                        v_align = std::get<VerticalAlign>(*va);
                        if (v_align != VerticalAlign::Baseline) {
                            desc.size_pt *= 0.65f;
                        }
                    }

                    auto run_font = std::make_shared<DummyFont>(desc);

                    if (v_align == VerticalAlign::Subscript) {
                        draw_y = draw_y + Twips(original_size * 15.0f * 0.4f);
                    } else if (v_align == VerticalAlign::Superscript) {
                        draw_y = draw_y - Twips(original_size * 15.0f * 0.2f);
                    }

                    Rect run_rect_adj = run_rect;
                    run_rect_adj.y = draw_y;

                    // Draw text background if specified
                    if (auto bg = run->style.get(PropertyId::BackgroundColor)) {
                        Color bg_color = std::get<Color>(*bg);
                        if ((bg_color & 0xFF000000) != 0) {
                            display_list.addCommand(std::make_unique<FillRectCommand>(run_rect_adj, bg_color));
                        }
                    }

                    // Draw selection background if text is selected
                    if (!selection_.isCollapsed() && run->logical_offset != UINT32_MAX) {
                        uint32_t run_start = run->logical_offset;
                        uint32_t run_end = run_start + run->logical_text.length();
                        uint32_t sel_start = selection_.getStart();
                        uint32_t sel_end = selection_.getEnd();

                        uint32_t overlap_start = std::max(run_start, sel_start);
                        uint32_t overlap_end = std::min(run_end, sel_end);

                        if (overlap_start < overlap_end) {
                            Twips sel_offset_x(0);
                            Twips sel_width(0);
                            Twips current_x(0);
                            for (size_t i = 0; i < run->run.glyphs.size(); ++i) {
                                uint32_t char_index = run_start + i;
                                Twips adv = run->run.glyphs[i].x_advance;
                                if (char_index >= overlap_start && char_index < overlap_end) {
                                    if (char_index == overlap_start) sel_offset_x = current_x;
                                    sel_width = sel_width + adv;
                                }
                                current_x = current_x + adv;
                            }
                            Rect sel_rect{run_rect_adj.x + sel_offset_x, run_rect_adj.y, sel_width, run_rect_adj.height};
                            display_list.addCommand(std::make_unique<FillRectCommand>(sel_rect, selection_color_));
                        }
                    }

                    display_list.addCommand(std::make_unique<DrawGlyphRunCommand>(
                        run_rect_adj,
                        run->run,
                        run->logical_text,
                        run_font,
                        text_color
                    ));

                    // Text Decoration
                    if (auto dec = run->style.get(PropertyId::Decoration)) {
                        if (std::get<TextDecoration>(*dec) == TextDecoration::Underline) {
                            Rect underline_rect = run_rect;
                            underline_rect.y = underline_rect.y + run_rect.height - Twips(40); // Baseline offset approx
                            underline_rect.height = Twips(20);
                            display_list.addCommand(std::make_unique<FillRectCommand>(underline_rect, text_color));
                        }
                    }
                }
            }

            if (block->table) {
                Twips table_abs_x = block_rect.x - viewport_x_;
                Twips table_abs_y = block_rect.y + block->table->getBounds().y - viewport_y_;
                renderTable(block->table, table_abs_x, table_abs_y);
            }
            
            if (!block->list_marker.empty()) {
                Twips marker_x = block_rect.x + block->list_indent - Twips(400) - viewport_x_;
                Twips marker_y = block_rect.y - viewport_y_;
                // Get height of first line to align marker
                Twips marker_h(180);
                Twips max_ascent(144);
                if (!block->lines.empty() && !block->lines[0]->runs.empty()) {
                    marker_h = block->lines[0]->getBounds().height;
                    max_ascent = block->lines[0]->runs[0]->run.max_ascent;
                }
                Rect marker_rect{marker_x, marker_y, Twips(300), marker_h};
                
                auto run_font = std::make_shared<DummyFont>(default_font_->getDescriptor());
                
                // We create a dummy ShapedTextRun for it
                ShapedTextRun marker_run;
                marker_run.total_width = Twips(300);
                marker_run.max_ascent = max_ascent;
                marker_run.max_descent = Twips(48);
                
                display_list.addCommand(std::make_unique<DrawGlyphRunCommand>(
                    marker_rect,
                    marker_run,
                    block->list_marker,
                    run_font,
                    default_text_color_
                ));
            }
            
            if (block->drop_cap) {
                Twips dc_abs_x = block_rect.x + block->drop_cap->getBounds().x - viewport_x_;
                Twips dc_abs_y = block_rect.y + block->drop_cap->getBounds().y - viewport_y_;
                Rect dc_rect{dc_abs_x, dc_abs_y, block->drop_cap->getBounds().width, block->drop_cap->getBounds().height};
                
                uint32_t text_color = default_text_color_;
                if (auto tc = block->drop_cap->style.get(PropertyId::TextColor)) {
                    text_color = std::get<uint32_t>(*tc);
                }
                
                FontDescriptor desc = default_font_->getDescriptor();
                if (auto fs = block->drop_cap->style.get(PropertyId::FontSize)) desc.size_pt = std::get<float>(*fs);
                auto run_font = std::make_shared<DummyFont>(desc);
                
                if (!selection_.isCollapsed() && block->drop_cap->logical_offset != UINT32_MAX) {
                    uint32_t run_start = block->drop_cap->logical_offset;
                    uint32_t run_end = run_start + block->drop_cap->logical_text.length();
                    uint32_t sel_start = selection_.getStart();
                    uint32_t sel_end = selection_.getEnd();

                    uint32_t overlap_start = std::max(run_start, sel_start);
                    uint32_t overlap_end = std::min(run_end, sel_end);

                    if (overlap_start < overlap_end) {
                        Twips sel_offset_x(0);
                        Twips sel_width(0);
                        Twips current_x(0);
                        for (size_t i = 0; i < block->drop_cap->run.glyphs.size(); ++i) {
                            uint32_t char_index = run_start + i;
                            Twips adv = block->drop_cap->run.glyphs[i].x_advance;
                            if (char_index >= overlap_start && char_index < overlap_end) {
                                if (char_index == overlap_start) sel_offset_x = current_x;
                                sel_width = sel_width + adv;
                            }
                            current_x = current_x + adv;
                        }
                        Rect sel_rect{dc_rect.x + sel_offset_x, dc_rect.y, sel_width, dc_rect.height};
                        display_list.addCommand(std::make_unique<FillRectCommand>(sel_rect, selection_color_));
                    }
                }
                
                display_list.addCommand(std::make_unique<DrawGlyphRunCommand>(
                    dc_rect,
                    block->drop_cap->run,
                    block->drop_cap->logical_text,
                    run_font,
                    text_color
                ));
            }

            for (const auto& img : block->images) {
                Twips img_abs_x = block_rect.x + img->getBounds().x - viewport_x_;
                Twips img_abs_y = block_rect.y + img->getBounds().y - viewport_y_;
                Rect img_rect{img_abs_x, img_abs_y, img->getBounds().width, img->getBounds().height};
                display_list.addCommand(std::make_unique<DrawImageCommand>(img_rect, img->path));
                
                if (!img->title.empty()) {
                    auto run_font = std::make_shared<DummyFont>(default_font_->getDescriptor());
                    ShapedTextRun title_run;
                    title_run.total_width = Twips(img->title.length() * 100); // dummy width
                    title_run.max_ascent = Twips(192);
                    title_run.max_descent = Twips(48);
                    
                    Rect title_rect{
                        img_rect.x, 
                        img_rect.y + img_rect.height + Twips(40), 
                        img_rect.width, 
                        Twips(240)
                    };
                    
                    display_list.addCommand(std::make_unique<DrawGlyphRunCommand>(
                        title_rect,
                        title_run,
                        img->title,
                        run_font,
                        0xFF555555 // Gray color for title
                    ));
                }
                
                if (selected_image_offset_.has_value() && *selected_image_offset_ == img->logical_offset) {
                    Twips hw(120); // 8px
                    Color hc(0xFF0078D7); // Blue handles
                    
                    // TL, TR, BL, BR
                    display_list.addCommand(std::make_unique<FillRectCommand>(Rect{img_rect.x, img_rect.y, hw, hw}, hc));
                    display_list.addCommand(std::make_unique<FillRectCommand>(Rect{img_rect.x + img_rect.width - hw, img_rect.y, hw, hw}, hc));
                    display_list.addCommand(std::make_unique<FillRectCommand>(Rect{img_rect.x, img_rect.y + img_rect.height - hw, hw, hw}, hc));
                    display_list.addCommand(std::make_unique<FillRectCommand>(Rect{img_rect.x + img_rect.width - hw, img_rect.y + img_rect.height - hw, hw, hw}, hc));
                }
            }
        };

        for (const auto& block : page->header_blocks) {
            render_block(block.get(), page_rect.x, current_page_y + block->getBounds().y);
        }
        for (const auto& block : page->footer_blocks) {
            render_block(block.get(), page_rect.x, current_page_y + block->getBounds().y);
        }

        for (const auto& block : page->blocks) {
            render_block(block.get(), page_rect.x, current_page_y + block->getBounds().y);
        }
        current_page_y = current_page_y + page->getBounds().height + page_gap_;
    }

    // Execute the display list
    for (const auto& cmd : display_list.getCommands()) {
        cmd->execute(renderer);
    }

    // Draw the caret if it is visible and there is no active selection
    if (caret_visible_ && caret_blink_state_ && selection_.isCollapsed()) {
        auto caret_rect_opt = CaretResolver::resolveLogicalToPhysical(current_pages_, selection_.head, page_gap_);
        if (caret_rect_opt.has_value()) {
            Rect caret = *caret_rect_opt;
            Twips page_x = current_pages_.empty() ? Twips(0) : Twips(std::max(0, (width_.getValue() - current_pages_[0]->getBounds().width.getValue()) / 2));
            caret.x = caret.x + page_x;
            // Apply scroll offset
            caret.x = caret.x - viewport_x_;
            caret.y = caret.y - viewport_y_;
            
            // Determine Color
            Color active_caret_color = default_text_color_;
            if (caret_color_.has_value()) {
                active_caret_color = *caret_color_;
            } else {
                auto bag = format_registry_.getStyleAt(selection_.head);
                if (auto c = bag.get(PropertyId::TextColor)) {
                    active_caret_color = std::get<Color>(*c);
                }
            }
            
            // Apply Caret Style
            if (caret_style_ == CaretStyle::Line) {
                caret.width = Twips(15); // 1 px line
            } else if (caret_style_ == CaretStyle::Underline) {
                caret.y = caret.y + caret.height - Twips(30); // 2 px line at bottom
                caret.height = Twips(30);
            } // Block uses full char width and line height
            
            // Simple culling for caret
            Rect view_bounds{Twips(0), Twips(0), width_, height_};
            if (view_bounds.intersects(caret)) {
                renderer.drawRect(caret, active_caret_color);
            }
        }
    }
}

void PlumaEditor::updateCursorState() {
    if (!cursor_state_callback_) return;

    CursorState state;
    state.logical_offset = selection_.head;
    state.style = format_registry_.getStyleAt(state.logical_offset);

    // Determine object type based on current selection or text
    if (selected_image_offset_.has_value()) {
        state.object_type = CursorObjectType::Image;
        state.logical_offset = *selected_image_offset_; // overrides to image offset
    } else if (table_selection_.mode != TableSelectionMode::None) {
        if (table_selection_.mode == TableSelectionMode::Cell) state.object_type = CursorObjectType::TableCell;
        else if (table_selection_.mode == TableSelectionMode::Row) state.object_type = CursorObjectType::TableRow;
        else if (table_selection_.mode == TableSelectionMode::Column) state.object_type = CursorObjectType::TableColumn;
        else state.object_type = CursorObjectType::Table;
    } else {
        // Simple heuristic: check the text at cursor
        std::string text_str = document_.getText();
        if (state.logical_offset < text_str.length()) {
            if (text_str.substr(state.logical_offset, 7) == "|IMAGE:") {
                state.object_type = CursorObjectType::Image;
            } else if (text_str.substr(state.logical_offset, 5) == "|CEL|") {
                state.object_type = CursorObjectType::TableCell;
            } else {
                state.object_type = CursorObjectType::Text;
            }
        } else {
            state.object_type = CursorObjectType::Text;
        }
    }

    cursor_state_callback_(state);
}

} // namespace pluma
