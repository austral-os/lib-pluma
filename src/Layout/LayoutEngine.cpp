#include <iostream>
#include <cmath>
#include <ctime>

#include <pluma/Layout/LayoutEngine.hpp>
#include <pluma/Diagnostics/Profiler.hpp>
#include <sstream>
#include <string>
#include <pluma/Typography/DummyTypography.hpp>
#include "../Render/stb_image.h"
#include <pluma/Render/ImageMask.hpp>
#include <unordered_map>
#include <mutex>
#include <functional>

namespace pluma {

struct CachedImageInfo {
    int w, h, comp;
    std::shared_ptr<ImageMask> mask;
    std::shared_ptr<ImageMask> tight_mask;
};

static std::mutex g_info_mutex;
static std::unordered_map<std::string, CachedImageInfo> g_info_cache;

LayoutEngine::LayoutEngine(std::shared_ptr<ITextShaper> shaper, std::shared_ptr<IFont> default_font)
    : shaper_(std::move(shaper)), font_(std::move(default_font)) {}

std::vector<std::unique_ptr<PageBox>> LayoutEngine::layoutText(
    std::string_view text, 
    PageSize page_size, 
    PageMargins margins, 
    const FormatRegistry& registry,
    std::string_view header_text,
    const FormatRegistry* header_registry,
    std::string_view footer_text,
    const FormatRegistry* footer_registry,
    const std::function<bool(int)>& has_header_cb,
    const std::function<bool(int)>& has_footer_cb,
    uint32_t logical_offset_base,
    int override_page_number,
    bool force_header_space,
    bool force_footer_space,
    int total_pages
) {
    PLUMA_PROFILE_SCOPE("LayoutEngine::layoutText");
    std::vector<std::unique_ptr<PageBox>> pages;
    



    
    auto setup_page = [&](std::unique_ptr<PageBox>& page, int page_idx, Twips& start_y, Twips& avail_height) {
        page = std::make_unique<PageBox>();
        page->laid_out_total_pages = total_pages;
        page->setBounds({Twips(0), Twips(0), page_size.width, page_size.height});
        Twips header_h(0), footer_h(0);
        
        if (has_header_cb && has_header_cb(page_idx) && !header_text.empty() && header_registry) {
            auto h_pages = layoutText(header_text, page_size, margins, *header_registry, "", nullptr, "", nullptr, nullptr, nullptr, 0, page_idx);
            if (!h_pages.empty()) {
                page->header_blocks = std::move(h_pages[0]->blocks);
                Twips h_y(margins.top);
                for (auto& b : page->header_blocks) {
                    b->setBounds({b->getBounds().x, h_y, b->getBounds().width, b->getBounds().height});
                    h_y = h_y + b->getBounds().height;
                }
                header_h = h_y - margins.top;
            }
        }
        
        if (has_footer_cb && has_footer_cb(page_idx) && !footer_text.empty() && footer_registry) {
            auto f_pages = layoutText(footer_text, page_size, margins, *footer_registry, "", nullptr, "", nullptr, nullptr, nullptr, 0, page_idx);
            if (!f_pages.empty()) {
                page->footer_blocks = std::move(f_pages[0]->blocks);
                Twips f_y(page_size.height.getValue() - margins.bottom.getValue());
                // Adjust f_y to grow upwards or just place it
                Twips total_f_h(0);
                for (auto& b : page->footer_blocks) total_f_h = total_f_h + b->getBounds().height;
                f_y = f_y - total_f_h;
                
                for (auto& b : page->footer_blocks) {
                    b->setBounds({b->getBounds().x, f_y, b->getBounds().width, b->getBounds().height});
                    f_y = f_y + b->getBounds().height;
                }
                footer_h = total_f_h;
            }
        }
        
        start_y = margins.top;
        if (header_h.getValue() > 0) start_y = start_y + header_h + Twips(240);
        else if (force_header_space && has_header_cb && has_header_cb(page_idx)) start_y = start_y + Twips(240) + Twips(240);
        
        Twips bottom = margins.bottom;
        if (footer_h.getValue() > 0) bottom = bottom + footer_h + Twips(240);
        else if (force_footer_space && has_footer_cb && has_footer_cb(page_idx)) bottom = bottom + Twips(240) + Twips(240);
        
        avail_height = Twips(page_size.height.getValue() - start_y.getValue() - bottom.getValue());
    };

    Twips content_width(page_size.width.getValue() - margins.left.getValue() - margins.right.getValue());
    if (content_width.getValue() <= 0) content_width = Twips(1);

    std::unique_ptr<PageBox> current_page;
    Twips current_page_y(0);
    Twips content_height(0);
    setup_page(current_page, pages.size() + 1, current_page_y, content_height);
    Twips column_start_y = margins.top;
    Twips max_page_y = margins.top;

    // 1. Split into paragraphs by \n
    std::string str_text(text);
    std::vector<std::string> paragraphs;
    size_t pos = 0;
    while ((pos = str_text.find('\n')) != std::string::npos) {
        paragraphs.push_back(str_text.substr(0, pos));
        str_text.erase(0, pos + 1);
    }
    paragraphs.push_back(str_text);

    uint32_t logical_offset = 0;

    int total_columns = 1;
    int current_column = 0;
    Twips column_gap(300); // Approx 20px
    
    Twips float_right_w(0);
    Twips float_right_top(0);
    Twips float_right_bottom(0);
    Twips float_left_w(0);
    Twips float_left_top(0);
    Twips float_left_bottom(0);
    
    struct ActiveImageMask {
        Twips abs_x, abs_y;
        Twips width, height;
        std::shared_ptr<ImageMask> mask;
        int pad_px = 0;
    };
    std::vector<ActiveImageMask> active_masks;
    
    std::vector<int> ol_counters(10, 0);
    int last_list_level = 0;

    Twips base_content_width = content_width;

    bool in_table = false;
    std::unique_ptr<TableBox> current_table;
    std::unique_ptr<TableRowBox> current_row;
    std::unique_ptr<TableCellBox> current_cell;
    std::string cell_buffer;
    bool cell_first_para = true;
    uint32_t cell_offset_base = 0;
    int table_cols = 1;
    std::vector<Twips> table_col_widths;
    std::vector<int> active_rowspans;
    int col_idx = 0;
    int table_depth = 0;

    for (const auto& para : paragraphs) {
        if (para.length() >= 9 && para.substr(0, 9) == "|COLUMNS:") {
            size_t end_tag = para.find("|", 9);
            if (end_tag != std::string::npos) {
                std::string num_str = para.substr(9, end_tag - 9);
                if (total_columns != std::stoi(num_str)) {
                    // Changing columns, advance to the maximum Y reached across all columns
                    if (max_page_y.getValue() > current_page_y.getValue()) {
                        current_page_y = max_page_y;
                    }
                }
                try {
                    total_columns = std::max(1, std::stoi(num_str));
                } catch (...) {
                    total_columns = 1;
                }
                current_column = 0; // reset column when layout changes
                column_start_y = current_page_y;
                max_page_y = current_page_y;
                logical_offset += para.length() + 1;
                continue;
                continue;
            }
        }

        if (para.length() >= 11 && para.substr(0, 11) == "|PAGEBREAK|") {
            pages.push_back(std::move(current_page));
            setup_page(current_page, pages.size() + 1, current_page_y, content_height);
            column_start_y = current_page_y;
            max_page_y = current_page_y;
            current_column = 0;
            // Las imágenes flotantes y sus masks son locales a la página.
            // Al cambiar de página se deben limpiar para evitar que afecten el layout de las páginas siguientes.
            active_masks.clear();
            float_right_w = Twips(0); float_right_top = Twips(0); float_right_bottom = Twips(0);
            float_left_w  = Twips(0); float_left_top  = Twips(0); float_left_bottom  = Twips(0);
            logical_offset += para.length() + 1;
            continue;
        }

        if (para.length() >= 11 && para.substr(0, 11) == "|BLANKPAGE|") {
            pages.push_back(std::move(current_page));
            setup_page(current_page, pages.size() + 1, current_page_y, content_height);
            current_page->blank_page_offset = logical_offset;
            pages.push_back(std::move(current_page));
            setup_page(current_page, pages.size() + 1, current_page_y, content_height);
            column_start_y = current_page_y;
            max_page_y = current_page_y;
            current_column = 0;
            // Limpiar estado de imágenes flotantes al cambiar de página
            active_masks.clear();
            float_right_w = Twips(0); float_right_top = Twips(0); float_right_bottom = Twips(0);
            float_left_w  = Twips(0); float_left_top  = Twips(0); float_left_bottom  = Twips(0);
            logical_offset += para.length() + 1;
            continue;
        }

        Twips total_gap = Twips(column_gap.getValue() * (total_columns - 1));
        Twips column_width = Twips((base_content_width.getValue() - total_gap.getValue()) / total_columns);
        content_width = column_width;
        // Line breaking for the entire paragraph (Block)
        auto block = std::make_unique<BlockBox>();
        Twips current_x(0);
        Twips current_y(0);
        auto current_line = std::make_unique<LineBox>();

        // Query paragraph alignment at the start of the paragraph
        PropertyBag para_style = registry.getStyleAt(logical_offset_base + logical_offset);
        if (auto align = para_style.get(PropertyId::TextAlignment)) {
            block->alignment = std::get<TextAlign>(*align);
        }

        Twips spacing_before(0);
        if (auto sb = para_style.get(PropertyId::ParagraphSpacingBefore)) {
            // Convert from Centimeters to Twips (1 cm = 1440 / 2.54 = 566.92913 twips)
            spacing_before = Twips(std::get<float>(*sb) * 566.92913f);
        }
        Twips spacing_after(0);
        if (auto sa = para_style.get(PropertyId::ParagraphSpacingAfter)) {
            // Convert from Centimeters to Twips
            spacing_after = Twips(std::get<float>(*sa) * 566.92913f);
        }

        size_t start = 0;
        
        Twips para_left_indent(0);
        Twips para_right_indent(0);
        Twips para_first_indent(0);

        if (para.length() >= 8 && para.substr(0, 8) == "|INDENT:") {
            size_t end_tag = para.find("|", 8);
            if (end_tag != std::string::npos) {
                std::string content = para.substr(8, end_tag - 8);
                size_t c1 = content.find(":");
                size_t c2 = (c1 != std::string::npos) ? content.find(":", c1 + 1) : std::string::npos;
                try {
                    if (c1 != std::string::npos && c2 != std::string::npos) {
                        float left = std::stof(content.substr(0, c1));
                        float right = std::stof(content.substr(c1 + 1, c2 - c1 - 1));
                        float first = std::stof(content.substr(c2 + 1));
                        para_left_indent = Twips(left * 1440.0f); // convert inches to twips
                        para_right_indent = Twips(right * 1440.0f);
                        para_first_indent = Twips(first * 1440.0f);
                    }
                } catch(...) {}
                start = end_tag + 1;
            }
        }
        
        if (auto indent = para_style.get(PropertyId::ParagraphIndentLeft)) {
            para_left_indent = Twips(std::get<float>(*indent) * 1440.0f);
        }
        if (auto indent = para_style.get(PropertyId::ParagraphIndentRight)) {
            para_right_indent = Twips(std::get<float>(*indent) * 1440.0f);
        }
        if (auto indent = para_style.get(PropertyId::ParagraphIndentFirstLine)) {
            para_first_indent = Twips(std::get<float>(*indent) * 1440.0f);
        }

        int list_type = -1; // -1 none, 0 UL, 1 OL
        int list_level = 0;
        std::string list_style_str;

        if (para.length() >= start + 5 && para.substr(start, 4) == "|UL:") {
            size_t end_tag = para.find("|", start + 4);
            if (end_tag != std::string::npos) {
                list_type = 0;
                std::string content = para.substr(start + 4, end_tag - (start + 4));
                size_t colon = content.find(":");
                try { 
                    if (colon != std::string::npos) {
                        list_level = std::max(1, std::stoi(content.substr(0, colon)));
                        list_style_str = content.substr(colon + 1);
                    } else {
                        list_level = std::max(1, std::stoi(content)); 
                    }
                } catch(...) { list_level = 1; }
                start = end_tag + 1;
            }
        } else if (para.length() >= start + 5 && para.substr(start, 4) == "|OL:") {
            size_t end_tag = para.find("|", start + 4);
            if (end_tag != std::string::npos) {
                list_type = 1;
                std::string content = para.substr(start + 4, end_tag - (start + 4));
                size_t colon = content.find(":");
                try { 
                    if (colon != std::string::npos) {
                        list_level = std::max(1, std::stoi(content.substr(0, colon)));
                        list_style_str = content.substr(colon + 1);
                    } else {
                        list_level = std::max(1, std::stoi(content)); 
                    }
                } catch(...) { list_level = 1; }
                start = end_tag + 1;
            }
        }

        if (list_type != -1) {
            if (list_level < last_list_level) {
                for (int i = list_level + 1; i < 10; ++i) ol_counters[i] = 0;
            }
            if (list_type == 1) { // OL
                ol_counters[list_level]++;
            }
            last_list_level = list_level;
            
            block->list_indent = Twips(list_level * 720); // 0.5 inch per level
            
            if (list_type == 0) { // UL
                if (list_style_str == "circle") block->list_marker = "o";
                else if (list_style_str == "square") block->list_marker = "-";
                else if (list_style_str == "disc") block->list_marker = "\xE2\x80\xA2";
                else {
                    if (list_level % 3 == 1) block->list_marker = "\xE2\x80\xA2";
                    else if (list_level % 3 == 2) block->list_marker = "o";
                    else block->list_marker = "-";
                }
            } else { // OL
                int count = ol_counters[list_level];
                if (list_style_str == "a") {
                    char c = 'a' + ((count - 1) % 26);
                    block->list_marker = std::string(1, c) + ".";
                } else if (list_style_str == "A") {
                    char c = 'A' + ((count - 1) % 26);
                    block->list_marker = std::string(1, c) + ".";
                } else if (list_style_str == "1") {
                    block->list_marker = std::to_string(count) + ".";
                } else {
                    if (list_level % 3 == 1) block->list_marker = std::to_string(count) + ".";
                    else if (list_level % 3 == 2) {
                        char c = 'a' + ((count - 1) % 26);
                        block->list_marker = std::string(1, c) + ".";
                    } else {
                        block->list_marker = std::to_string(count) + ")";
                    }
                }
            }
            
            // Adjust current_x to indent text
            current_x = block->list_indent + para_left_indent + para_first_indent;
        } else {
            for (int i = 0; i < 10; ++i) ol_counters[i] = 0;
            last_list_level = 0;
            current_x = para_left_indent + para_first_indent;
        }

        auto stylesAreEqual = [](const PropertyBag& a, const PropertyBag& b) {
            auto& map_a = a.getAll();
            auto& map_b = b.getAll();
            if (map_a.size() != map_b.size()) return false;
            for (const auto& [k, v] : map_a) {
                auto it = map_b.find(k);
                if (it == map_b.end() || it->second != v) return false;
            }
            return true;
        };

        if (para.length() >= 5 && para.substr(0, 5) == "|TBL:") {
            if (in_table) {
                table_depth++;
                if (!cell_first_para) cell_buffer += "\n";
                cell_buffer += para;
                cell_first_para = false;
                logical_offset += para.length() + 1;
                continue;
            }
            in_table = true;
            table_depth = 1;
            current_table = std::make_unique<TableBox>();
            current_table->logical_offset = logical_offset_base + logical_offset;
            size_t cols_pos = para.find("cols=");
            if (cols_pos != std::string::npos) {
                table_cols = std::stoi(para.substr(cols_pos + 5));
            } else {
                table_cols = 1;
            }
            
            // Try to load custom widths
            table_col_widths.clear();
            if (auto widths_opt = registry.getStyleAt(current_table->logical_offset).get(PropertyId::TableColumnWidths)) {
                std::string widths_str = std::get<std::string>(*widths_opt);
                size_t pos = 0;
                while (pos < widths_str.length()) {
                    size_t comma = widths_str.find(',', pos);
                    std::string token = (comma == std::string::npos) ? widths_str.substr(pos) : widths_str.substr(pos, comma - pos);
                    table_col_widths.push_back(Twips(static_cast<int32_t>(std::stof(token) * 15.0f + 0.5f))); // stored in pts
                    if (comma == std::string::npos) break;
                    pos = comma + 1;
                }
            }
            
            // Fill missing with equal remaining
            if (table_col_widths.size() < static_cast<size_t>(table_cols)) {
                Twips used(0);
                for (auto w : table_col_widths) used = used + w;
                Twips remaining = content_width - used;
                if (remaining.getValue() < 0) remaining = Twips(0);
                int missing = table_cols - table_col_widths.size();
                Twips per_missing(remaining.getValue() / missing);
                for (int i = 0; i < missing; ++i) table_col_widths.push_back(per_missing);
            }
            
            current_table->col_widths = table_col_widths;
            
            active_rowspans.assign(table_cols, 0);
            
            col_idx = 0;
            logical_offset += para.length() + 1;
            continue;
        }

        if (in_table) {
            if (para == "|ROW|") {
                if (table_depth > 1) {
                    if (!cell_first_para) cell_buffer += "\n";
                    cell_buffer += para;
                    cell_first_para = false;
                    logical_offset += para.length() + 1;
                    continue;
                }
                if (current_cell) {
                    if (true) {
                        Twips cell_width(0);
                        int span = current_cell->colspan;
                        for (int i = 0; i < span && (current_cell->col_idx + i) < table_cols; ++i) {
                            cell_width = cell_width + ((static_cast<size_t>(current_cell->col_idx + i) < table_col_widths.size()) ? table_col_widths[current_cell->col_idx + i] : Twips(content_width.getValue() / table_cols));
                        }
                        auto cell_pages = layoutText(cell_buffer, {cell_width, Twips(1000000)}, {Twips(60),Twips(60),Twips(60),Twips(60)}, registry, "", nullptr, "", nullptr, nullptr, nullptr, logical_offset_base + cell_offset_base, override_page_number > 0 ? override_page_number : (int)(pages.size() + 1), false, false, total_pages);
                        if (!cell_pages.empty()) {
                            for (auto& b : cell_pages[0]->blocks) current_cell->blocks.push_back(std::move(b));
                        }
                        current_cell->setBounds({Twips(0), Twips(0), cell_width, Twips(10000)});
                    }
                    if (current_row) current_row->cells.push_back(std::move(current_cell));
                }
                if (current_row) current_table->rows.push_back(std::move(current_row));
                current_row = std::make_unique<TableRowBox>();
                current_cell.reset();
                col_idx = 0;
                
                // Decrement rowspans
                for (int& rs : active_rowspans) {
                    if (rs > 0) rs--;
                }
                
                // Skip columns that are occupied by rowspan from above
                while (col_idx < table_cols && active_rowspans[col_idx] > 0) {
                    col_idx++;
                }

                cell_first_para = true;
                logical_offset += para.length() + 1;
                continue;
            }
            if (para == "|CEL|" || (para.length() > 5 && para.substr(0, 5) == "|CEL:")) {
                if (table_depth > 1) {
                    if (!cell_first_para) cell_buffer += "\n";
                    cell_buffer += para;
                    cell_first_para = false;
                    logical_offset += para.length() + 1;
                    continue;
                }
                if (current_cell) {
                    if (true) {
                        Twips cell_width(0);
                        int span = current_cell->colspan;
                        for (int i = 0; i < span && (current_cell->col_idx + i) < table_cols; ++i) {
                            cell_width = cell_width + ((static_cast<size_t>(current_cell->col_idx + i) < table_col_widths.size()) ? table_col_widths[current_cell->col_idx + i] : Twips(content_width.getValue() / table_cols));
                        }
                        
                        auto cell_pages = layoutText(cell_buffer, {cell_width, Twips(1000000)}, {Twips(60),Twips(60),Twips(60),Twips(60)}, registry, "", nullptr, "", nullptr, nullptr, nullptr, logical_offset_base + cell_offset_base, override_page_number > 0 ? override_page_number : (int)(pages.size() + 1), false, false, total_pages);
                        if (!cell_pages.empty()) {
                            for (auto& b : cell_pages[0]->blocks) current_cell->blocks.push_back(std::move(b));
                        }
                        current_cell->setBounds({Twips(0), Twips(0), cell_width, Twips(10000)});
                    }
                    if (current_row) current_row->cells.push_back(std::move(current_cell));
                    
                    // Advance col_idx for the next cell
                    if (current_row && !current_row->cells.empty()) {
                        col_idx += current_row->cells.back()->colspan;
                        while (col_idx < table_cols && active_rowspans[col_idx] > 0) {
                            col_idx++;
                        }
                    }
                }
                
                current_cell = std::make_unique<TableCellBox>();
                current_cell->col_idx = col_idx;
                current_cell->logical_offset = logical_offset;
                
                // Parse colspan and rowspan
                size_t c_pos = para.find("colspan=");
                if (c_pos != std::string::npos) current_cell->colspan = std::stoi(para.substr(c_pos + 8));
                size_t r_pos = para.find("rowspan=");
                if (r_pos != std::string::npos) current_cell->rowspan = std::stoi(para.substr(r_pos + 8));
                
                // Register our rowspan
                if (current_cell->rowspan > 1) {
                    for (int i = 0; i < current_cell->colspan && (current_cell->col_idx + i) < table_cols; ++i) {
                        active_rowspans[current_cell->col_idx + i] = current_cell->rowspan;
                    }
                }
                
                cell_buffer.clear();
                cell_first_para = true;
                logical_offset += para.length() + 1;
                cell_offset_base = logical_offset;
                continue;
            }
            if (para == "|ENDTBL|") {
                if (table_depth > 1) {
                    table_depth--;
                    if (!cell_first_para) cell_buffer += "\n";
                    cell_buffer += para;
                    cell_first_para = false;
                    logical_offset += para.length() + 1;
                    continue;
                }
                if (current_cell) {
                    if (true) {
                        Twips cell_width(0);
                        int span = current_cell->colspan;
                        for (int i = 0; i < span && (current_cell->col_idx + i) < table_cols; ++i) {
                            cell_width = cell_width + ((static_cast<size_t>(current_cell->col_idx + i) < table_col_widths.size()) ? table_col_widths[current_cell->col_idx + i] : Twips(content_width.getValue() / table_cols));
                        }
                        
                        auto cell_pages = layoutText(cell_buffer, {cell_width, Twips(1000000)}, {Twips(60),Twips(60),Twips(60),Twips(60)}, registry, "", nullptr, "", nullptr, nullptr, nullptr, logical_offset_base + cell_offset_base, override_page_number > 0 ? override_page_number : (int)(pages.size() + 1), false, false, total_pages);
                        if (!cell_pages.empty()) {
                            for (auto& b : cell_pages[0]->blocks) current_cell->blocks.push_back(std::move(b));
                        }
                        current_cell->setBounds({Twips(0), Twips(0), cell_width, Twips(10000)});
                    }
                    if (current_row) current_row->cells.push_back(std::move(current_cell));
                }
                if (current_row) current_table->rows.push_back(std::move(current_row));
                
                in_table = false;
                table_depth = 0;
                
                // Add table to document
                // Wait, table needs to compute cell heights!
                Twips table_height(0);
                std::vector<Twips> row_heights(current_table->rows.size(), Twips(120));
                
                // First pass: basic heights for rowspan=1
                for (size_t r = 0; r < current_table->rows.size(); ++r) {
                    Twips max_h(120);
                    for (auto& cell : current_table->rows[r]->cells) {
                        if (cell->rowspan == 1) {
                            Twips cell_h(120);
                            if (!cell->blocks.empty()) {
                                auto& last_cb = cell->blocks.back();
                                cell_h = last_cb->getBounds().y + last_cb->getBounds().height + Twips(60);
                            }
                            if (cell_h.getValue() > max_h.getValue()) max_h = cell_h;
                        }
                    }
                    row_heights[r] = max_h;
                }
                
                // Second pass: handle rowspan > 1
                for (size_t r = 0; r < current_table->rows.size(); ++r) {
                    for (auto& cell : current_table->rows[r]->cells) {
                        if (cell->rowspan > 1) {
                            Twips cell_h(120);
                            if (!cell->blocks.empty()) {
                                auto& last_cb = cell->blocks.back();
                                cell_h = last_cb->getBounds().y + last_cb->getBounds().height + Twips(60);
                            }
                            Twips span_h(0);
                            size_t end_r = std::min(r + cell->rowspan, current_table->rows.size());
                            for (size_t i = r; i < end_r; ++i) span_h = span_h + row_heights[i];
                            if (cell_h.getValue() > span_h.getValue()) {
                                // Add excess to the last row of the span
                                Twips excess = Twips(cell_h.getValue() - span_h.getValue());
                                row_heights[end_r - 1] = row_heights[end_r - 1] + excess;
                            }
                        }
                    }
                }
                
                // Third pass: set bounds
                for (size_t r = 0; r < current_table->rows.size(); ++r) {
                    auto& row = current_table->rows[r];
                    for (auto& cell : row->cells) {
                        Twips current_x(0);
                        for (int i = 0; i < cell->col_idx && static_cast<size_t>(i) < table_col_widths.size(); ++i) {
                            current_x = current_x + table_col_widths[i];
                        }
                        
                        Twips span_h(0);
                        size_t end_r = std::min(r + cell->rowspan, current_table->rows.size());
                        for (size_t i = r; i < end_r; ++i) span_h = span_h + row_heights[i];
                        
                        cell->setBounds({current_x, Twips(0), cell->getBounds().width, span_h});
                        
                        // Apply vertical alignment to contents
                        CellVerticalAlign v_align = CellVerticalAlign::Top;
                        if (auto val = registry.getStyleAt(cell->logical_offset).get(PropertyId::CellVerticalAlignment)) {
                            v_align = std::get<CellVerticalAlign>(*val);
                        }
                        
                        if (v_align != CellVerticalAlign::Top && !cell->blocks.empty()) {
                            auto& last_cb = cell->blocks.back();
                            Twips content_h = last_cb->getBounds().y + last_cb->getBounds().height;
                            Twips empty_space = Twips(span_h.getValue() - content_h.getValue() - 60);
                            if (empty_space.getValue() > 0) {
                                Twips offset(0);
                                if (v_align == CellVerticalAlign::Middle) {
                                    offset = Twips(empty_space.getValue() / 2);
                                } else if (v_align == CellVerticalAlign::Bottom) {
                                    offset = empty_space;
                                }
                                for (auto& cb : cell->blocks) {
                                    auto b = cb->getBounds();
                                    b.y = b.y + offset;
                                    cb->setBounds(b);
                                }
                            }
                        }
                    }
                    row->setBounds({Twips(0), table_height, content_width, row_heights[r]});
                    table_height = table_height + row_heights[r];
                }
                // --- TABLE PAGINATION LOGIC ---
                std::unique_ptr<TableBox> table_part = std::make_unique<TableBox>();
                table_part->col_widths = current_table->col_widths;
                table_part->logical_offset = current_table->logical_offset;
                table_part->hide_most_borders = current_table->hide_most_borders;

                Twips current_table_y(0);
                Twips part_height(0);

                std::vector<TableCellBox*> active_spans(current_table->col_widths.size(), nullptr);
                std::vector<int> span_remaining(current_table->col_widths.size(), 0);
                std::vector<Twips> span_start_y(current_table->col_widths.size(), Twips(0));

                for (size_t r = 0; r < current_table->rows.size(); ++r) {
                    auto& src_row = current_table->rows[r];
                    Twips rh = src_row->getBounds().height;
                    
                    if (current_page_y.getValue() + part_height.getValue() + rh.getValue() > column_start_y.getValue() + content_height.getValue() && part_height.getValue() > 0) {
                        
                        table_part->setBounds({Twips(0), Twips(0), content_width, part_height});
                        table_part->is_split_bottom = true;
                        auto block = std::make_unique<BlockBox>();
                        block->table = std::move(table_part);
                        block->setBounds({column_start_y, current_page_y, content_width, part_height});
                        current_page->blocks.push_back(std::move(block));
                        
                        current_page_y = current_page_y + part_height;
                        if (current_page_y.getValue() > max_page_y.getValue()) max_page_y = current_page_y;
                        
                        pages.push_back(std::move(current_page));
                        setup_page(current_page, pages.size() + 1, current_page_y, content_height);
                        column_start_y = current_page_y;
                        max_page_y = current_page_y;
                        current_column = 0;
                        active_masks.clear();
                        float_right_w = Twips(0); float_right_top = Twips(0); float_right_bottom = Twips(0);
                        float_left_w  = Twips(0); float_left_top  = Twips(0); float_left_bottom  = Twips(0);
                        
                        table_part = std::make_unique<TableBox>();
                        table_part->col_widths = current_table->col_widths;
                        table_part->logical_offset = current_table->logical_offset;
                        table_part->hide_most_borders = current_table->hide_most_borders;
                        table_part->is_split_top = true;
                        
                        std::vector<std::unique_ptr<TableCellBox>> continuation_cells;
                        
                        for (size_t c = 0; c < active_spans.size(); ++c) {
                            if (span_remaining[c] > 0) {
                                auto orig_cell = active_spans[c];
                                Twips split_h = part_height - span_start_y[c];
                                
                                auto split_cell = std::make_unique<TableCellBox>();
                                split_cell->colspan = orig_cell->colspan;
                                split_cell->rowspan = span_remaining[c];
                                split_cell->col_idx = orig_cell->col_idx;
                                split_cell->logical_offset = orig_cell->logical_offset;
                                
                                orig_cell->rowspan -= span_remaining[c];
                                
                                for (auto it = orig_cell->blocks.begin(); it != orig_cell->blocks.end(); ) {
                                    Twips by = (*it)->getBounds().y;
                                    Twips bh = (*it)->getBounds().height;
                                    
                                    if (by.getValue() >= split_h.getValue()) {
                                        auto b = std::move(*it);
                                        auto bounds = b->getBounds();
                                        bounds.y = bounds.y - split_h;
                                        b->setBounds(bounds);
                                        split_cell->blocks.push_back(std::move(b));
                                        it = orig_cell->blocks.erase(it);
                                    } else if ((by + bh).getValue() > split_h.getValue()) {
                                        auto& orig_block = *it;
                                        auto sub_block = std::make_unique<BlockBox>();
                                        sub_block->alignment = orig_block->alignment;
                                        
                                        Twips sub_y(0);
                                        for (auto line_it = orig_block->lines.begin(); line_it != orig_block->lines.end(); ) {
                                            Twips ly = by + (*line_it)->getBounds().y;
                                            if (ly.getValue() >= split_h.getValue()) {
                                                auto line = std::move(*line_it);
                                                line->setBounds({line->getBounds().x, sub_y, line->getBounds().width, line->getBounds().height});
                                                sub_y = sub_y + line->getBounds().height;
                                                sub_block->lines.push_back(std::move(line));
                                                line_it = orig_block->lines.erase(line_it);
                                            } else {
                                                ++line_it;
                                            }
                                        }
                                        if (!sub_block->lines.empty()) {
                                            sub_block->setBounds({orig_block->getBounds().x, Twips(0), orig_block->getBounds().width, sub_y});
                                            split_cell->blocks.push_back(std::move(sub_block));
                                        }
                                        ++it;
                                    } else {
                                        ++it;
                                    }
                                }
                                
                                auto orig_bounds = orig_cell->getBounds();
                                orig_cell->setBounds({orig_bounds.x, Twips(0), orig_bounds.width, split_h});
                                split_cell->setBounds({orig_bounds.x, Twips(0), orig_bounds.width, orig_bounds.height - split_h});
                                
                                span_start_y[c] = Twips(0);
                                for (int i = 1; i < split_cell->colspan; ++i) {
                                    span_start_y[c + i] = Twips(0);
                                }
                                
                                continuation_cells.push_back(std::move(split_cell));
                                c += (orig_cell->colspan - 1);
                            }
                        }
                        
                        for (auto& cell : continuation_cells) {
                            src_row->cells.push_back(std::move(cell));
                        }
                        std::sort(src_row->cells.begin(), src_row->cells.end(), [](const std::unique_ptr<TableCellBox>& a, const std::unique_ptr<TableCellBox>& b) {
                            return a->col_idx < b->col_idx;
                        });
                        
                        part_height = Twips(0);
                        current_table_y = Twips(0);
                    }
                    
                    for (auto& cell : src_row->cells) {
                        if (cell->rowspan > 1) {
                            active_spans[cell->col_idx] = cell.get();
                            span_remaining[cell->col_idx] = cell->rowspan;
                            span_start_y[cell->col_idx] = current_table_y;
                            for (int c = 1; c < cell->colspan; ++c) {
                                active_spans[cell->col_idx + c] = cell.get();
                                span_remaining[cell->col_idx + c] = cell->rowspan;
                                span_start_y[cell->col_idx + c] = current_table_y;
                            }
                        }
                    }
                    
                    src_row->setBounds({src_row->getBounds().x, current_table_y, src_row->getBounds().width, rh});
                    
                    table_part->rows.push_back(std::move(src_row));
                    part_height = part_height + rh;
                    current_table_y = current_table_y + rh;
                    
                    for (size_t c = 0; c < span_remaining.size(); ++c) {
                        if (span_remaining[c] > 0) {
                            span_remaining[c]--;
                            if (span_remaining[c] == 0) {
                                active_spans[c] = nullptr;
                                span_start_y[c] = Twips(0);
                            }
                        }
                    }
                }
                
                table_part->setBounds({Twips(0), Twips(0), content_width, part_height});
                auto block = std::make_unique<BlockBox>();
                block->table = std::move(table_part);
                block->setBounds({column_start_y, current_page_y, content_width, part_height});
                current_page->blocks.push_back(std::move(block));
                
                if (current_page_y.getValue() + part_height.getValue() > max_page_y.getValue()) {
                    max_page_y = Twips(current_page_y.getValue() + part_height.getValue());
                }
                current_page_y = current_page_y + part_height + Twips(240);
                // --- END TABLE PAGINATION LOGIC ---

                logical_offset += para.length() + 1;
                continue;
            }
            
            if (!cell_first_para) cell_buffer += "\n";
            cell_buffer += para;
            cell_first_para = false;
            logical_offset += para.length() + 1;
            continue;
        }

        if (para.length() >= 7 && para.substr(0, 7) == "|HLINE|") {
            auto block = std::make_unique<BlockBox>();
            block->is_horizontal_line = true;
            Twips line_height = Twips(240); // Standard spacing around the line
            block->setBounds({column_start_y, current_page_y, content_width, line_height});
            current_page_y = current_page_y + line_height + Twips(120); // add some margin
            current_page->blocks.push_back(std::move(block));
            logical_offset += para.length() + 1;
            continue;
        }

        {
            if (para.length() >= 7 && para.substr(0, 7) == "|IMAGE:") {
                size_t end_tag = para.find("|", 7);
                if (end_tag != std::string::npos) {
                    std::string mode_str = para.substr(7, end_tag - 7);
                    
                    std::string image_path = "test-img.png"; // Fallback
                    TextWrapMode mode = TextWrapMode::InLine;
                    
                    // Parse optional mode and path, e.g. |IMAGE:Square:logo.png| or |IMAGE:logo.png|
                    size_t colon_pos = mode_str.find(':');
                    if (colon_pos != std::string::npos) {
                        std::string wrap_str = mode_str.substr(0, colon_pos);
                        image_path = mode_str.substr(colon_pos + 1);
                        if (wrap_str == "Square") mode = TextWrapMode::Square;
                        else if (wrap_str == "Tight") mode = TextWrapMode::Tight;
                        else if (wrap_str == "Through") mode = TextWrapMode::Through;
                        else if (wrap_str == "TopAndBottom") mode = TextWrapMode::TopAndBottom;
                        else if (wrap_str == "BehindText") mode = TextWrapMode::BehindText;
                        else if (wrap_str == "InFrontOfText") mode = TextWrapMode::InFrontOfText;
                        else if (wrap_str == "InLine") mode = TextWrapMode::InLine;
                        else {
                            image_path = mode_str; // Colon was part of the path, or invalid mode. Default InLine.
                        }
                    } else {
                        // If there is no colon, maybe it's just the wrap mode or just the path
                        if (mode_str == "Square") mode = TextWrapMode::Square;
                        else if (mode_str == "Tight") mode = TextWrapMode::Tight;
                        else if (mode_str == "Through") mode = TextWrapMode::Through;
                        else if (mode_str == "TopAndBottom") mode = TextWrapMode::TopAndBottom;
                        else if (mode_str == "BehindText") mode = TextWrapMode::BehindText;
                        else if (mode_str == "InFrontOfText") mode = TextWrapMode::InFrontOfText;
                        else if (mode_str == "InLine") mode = TextWrapMode::InLine;
                        else {
                            image_path = mode_str;
                        }
                    }
                    
                    auto img = std::make_unique<ImageBox>();
                    img->wrap_mode = mode;
                    img->path = image_path;
                    img->logical_offset = logical_offset_base + logical_offset + start;
                    
                    Twips img_w(3000), img_h(3000); // 200x200px approx fallback
                    
                    int real_w = 0, real_h = 0, comp = 0;
                    bool has_info = false;
                    std::shared_ptr<ImageMask> mask;
                    std::shared_ptr<ImageMask> tight_mask;
                    
                    {
                        std::lock_guard<std::mutex> lock(g_info_mutex);
                        auto it = g_info_cache.find(image_path);
                        if (it != g_info_cache.end()) {
                            real_w = it->second.w;
                            real_h = it->second.h;
                            comp = it->second.comp;
                            mask = it->second.mask;
                            tight_mask = it->second.tight_mask;
                            has_info = true;
                        }
                    }
                    if (!has_info) {
                        unsigned char* data = stbi_load(image_path.c_str(), &real_w, &real_h, &comp, 4);
                        if (data) {
                            mask = std::make_shared<ImageMask>(real_w, real_h);
                            tight_mask = std::make_shared<ImageMask>(real_w, real_h);
                            for (int y = 0; y < real_h; ++y) {
                                int start_x = -1;
                                int first_x = -1;
                                int last_x = -1;
                                for (int x = 0; x < real_w; ++x) {
                                    unsigned char alpha = data[(y * real_w + x) * 4 + 3];
                                    if (alpha > 10) {
                                        if (first_x == -1) first_x = x;
                                        last_x = x;
                                        
                                        if (start_x == -1) start_x = x;
                                    } else {
                                        if (start_x != -1) {
                                            mask->addSegment(y, start_x, x - 1);
                                            start_x = -1;
                                        }
                                    }
                                }
                                if (start_x != -1) {
                                    mask->addSegment(y, start_x, real_w - 1);
                                }
                                if (first_x != -1 && last_x != -1) {
                                    tight_mask->addSegment(y, first_x, last_x);
                                }
                            }
                            stbi_image_free(data);
                            
                            std::lock_guard<std::mutex> lock(g_info_mutex);
                            g_info_cache[image_path] = {real_w, real_h, 4, mask, tight_mask};
                            has_info = true;
                        } else {
                            // Fallback to stbi_info if load fails for some reason
                            if (stbi_info(image_path.c_str(), &real_w, &real_h, &comp)) {
                                std::lock_guard<std::mutex> lock(g_info_mutex);
                                g_info_cache[image_path] = {real_w, real_h, comp, nullptr, nullptr};
                                has_info = true;
                            }
                        }
                    }
                    
                    if (has_info && real_w > 0 && real_h > 0) {
                        float aspect_ratio = static_cast<float>(real_h) / static_cast<float>(real_w);
                        // Default to 400px width (6000 twips), or less if the image is smaller
                        float target_width_px = std::min(400.0f, static_cast<float>(real_w));
                        float target_height_px = target_width_px * aspect_ratio;
                        
                        // Limit height to reasonable size (e.g. 600px max)
                        if (target_height_px > 600.0f) {
                            target_height_px = 600.0f;
                            target_width_px = target_height_px / aspect_ratio;
                        }
                        
                        img_w = Twips(static_cast<int32_t>(target_width_px * 15.0f));
                        img_h = Twips(static_cast<int32_t>(target_height_px * 15.0f));
                    }
                    
                    PropertyBag img_style = registry.getStyleAt(img->logical_offset);
                    if (auto w = img_style.get(PropertyId::ImageWidth)) img_w = Twips(std::get<float>(*w) * 15.0f);
                    if (auto h = img_style.get(PropertyId::ImageHeight)) img_h = Twips(std::get<float>(*h) * 15.0f);
                    
                    if (auto id_val = img_style.get(PropertyId::ImageId)) img->image_id = std::get<std::string>(*id_val);
                    if (auto title_val = img_style.get(PropertyId::ImageTitle)) img->title = std::get<std::string>(*title_val);
                    
                    if (auto wrap_val = img_style.get(PropertyId::ImageWrapMode)) {
                        mode = std::get<TextWrapMode>(*wrap_val);
                    }
                    img->wrap_mode = mode; // Keep the img struct consistent
                    
                    if (mode == TextWrapMode::Square || mode == TextWrapMode::Tight || mode == TextWrapMode::Through) {
                        Twips x_pos = column_width - img_w; // Default right
                        if (auto x_val = img_style.get(PropertyId::ImageX)) {
                            x_pos = Twips(std::get<float>(*x_val) * 15.0f);
                        }
                        Twips y_pos = current_y; // Default to current paragraph top
                        if (auto y_val = img_style.get(PropertyId::ImageY)) {
                            y_pos = Twips(std::get<float>(*y_val) * 15.0f);
                        }
                        
                        if (x_pos.getValue() < 0) x_pos = Twips(0);
                        if (x_pos.getValue() > column_width.getValue() - img_w.getValue()) {
                            x_pos = column_width - img_w;
                        }

                        img->setBounds({x_pos, y_pos, img_w, img_h});
                        
                        if (x_pos.getValue() < (column_width.getValue() - img_w.getValue()) / 2) {
                            if ((mode == TextWrapMode::Tight || mode == TextWrapMode::Through) && mask) {
                                auto chosen_mask = (mode == TextWrapMode::Tight) ? tight_mask : mask;
                                active_masks.push_back({
                                    x_pos, current_page_y + y_pos, img_w, img_h, chosen_mask,
                                    (mode == TextWrapMode::Tight) ? 8 : 0
                                });
                            } else {
                                float_left_w = x_pos + img_w + Twips(150); // Add horizontal margin (e.g. 150 twips = 10px)
                                float_left_top = current_page_y + y_pos - Twips(75);
                                float_left_bottom = current_page_y + y_pos + img_h + Twips(75);
                            }
                        } else {
                            if ((mode == TextWrapMode::Tight || mode == TextWrapMode::Through) && mask) {
                                auto chosen_mask = (mode == TextWrapMode::Tight) ? tight_mask : mask;
                                active_masks.push_back({
                                    x_pos, current_page_y + y_pos, img_w, img_h, chosen_mask,
                                    (mode == TextWrapMode::Tight) ? 8 : 0
                                });
                            } else {
                                float_right_w = column_width - x_pos + Twips(150);
                                float_right_top = current_page_y + y_pos - Twips(75);
                                float_right_bottom = current_page_y + y_pos + img_h + Twips(75);
                            }
                        }
                    } else if (mode == TextWrapMode::InLine || mode == TextWrapMode::TopAndBottom) {
                        Twips x_offset(0);
                        if (block->alignment == TextAlign::Center) {
                            x_offset = Twips((content_width.getValue() - img_w.getValue()) / 2);
                        } else if (block->alignment == TextAlign::Right) {
                            x_offset = Twips(content_width.getValue() - img_w.getValue());
                        }
                        
                        img->setBounds({x_offset, current_y, img_w, img_h});
                        current_y = current_y + img_h;
                        
                        auto dummy_line = std::make_unique<LineBox>();
                        dummy_line->setBounds({Twips(0), current_y - img_h, img_w, img_h});
                        block->lines.push_back(std::move(dummy_line));
                    } else {
                        // Absolute positioned behind/front
                        img->setBounds({Twips(1000), current_y, img_w, img_h});
                    }
                    
                    block->images.push_back(std::move(img));
                    start = end_tag + 1;
                    if (start < para.length() && para[start] == ' ') start++;
                }
            }

            int drop_cap_lines = 0;
            Twips dropcap_width(0);
            Twips dropcap_height(0);
            if (para.length() - start >= 9 && para.substr(start, 9) == "|DROPCAP:") {
                size_t end_tag = para.find("|", start + 9);
                if (end_tag != std::string::npos) {
                    std::string num_str = para.substr(start + 9, end_tag - start - 9);
                    try { drop_cap_lines = std::max(1, std::stoi(num_str)); } catch (...) { drop_cap_lines = 1; }
                    start = end_tag + 1;
                    if (start < para.length() && para[start] == ' ') start++;
                }
            }

            PropertyBag dc_style = registry.getStyleAt(logical_offset_base + logical_offset + start - 1);
            if (auto dc_lines_opt = dc_style.get(PropertyId::DropCapLines)) {
                drop_cap_lines = std::get<int>(*dc_lines_opt);
            }
            if (drop_cap_lines > 0 && start < para.length()) {
                std::string first_char = para.substr(start, 1);
                start++;
                auto drop_cap = std::make_unique<RunBox>();
                
                FontDescriptor dc_desc = font_->getDescriptor();
                dc_desc.size_pt = dc_desc.size_pt * drop_cap_lines * 1.15f;
                auto dc_run_font = std::make_shared<DummyFont>(dc_desc);
                ShapedTextRun dc_run = shaper_->shapeText(first_char, dc_run_font);
                
                dropcap_width = dc_run.total_width;
                dropcap_height = Twips(font_->getDescriptor().size_pt * 20.0f * drop_cap_lines * 1.2f);
                
                dc_style.set(PropertyId::FontSize, dc_desc.size_pt);
                
                drop_cap->run = std::move(dc_run);
                drop_cap->style = std::move(dc_style);
                
                // Track dropcap as a left float
                float_left_w = Twips(std::max(float_left_w.getValue(), (dropcap_width + Twips(100)).getValue()));
                float_left_top = current_page_y + current_y;
                float_left_bottom = Twips(std::max(float_left_bottom.getValue(), (current_page_y + current_y + dropcap_height).getValue()));
                
                drop_cap->logical_text = first_char;
                drop_cap->logical_offset = logical_offset_base + logical_offset + start - 1;
                drop_cap->setBounds({para_left_indent + para_first_indent, current_y, dropcap_width, dropcap_height});
                block->drop_cap = std::move(drop_cap);
                
                current_x = dropcap_width + Twips(100) + para_left_indent + para_first_indent;
            }

            Twips current_line_h = Twips(240); // Estimate
            if ((current_page_y + current_y + current_line_h).getValue() > float_left_top.getValue() && 
                (current_page_y + current_y).getValue() < float_left_bottom.getValue()) {
                if (float_left_w.getValue() > current_x.getValue()) {
                    current_x = float_left_w;
                }
            }

        while (start < para.length()) {
            PropertyBag word_style = registry.getStyleAt(logical_offset_base + logical_offset + start);

            if (para[start] == '\v') {
                // Force line break
                Twips line_height = Twips(240); // 12pt approx
                for (const auto& r : current_line->runs) {
                    Twips h = r->run.max_ascent + r->run.max_descent;
                    if (h.getValue() > line_height.getValue()) line_height = h;
                }
                float line_spacing = 1.0f;
                if (auto ls = para_style.get(PropertyId::LineSpacing)) {
                    line_spacing = std::get<float>(*ls);
                }
                line_height = Twips(line_height.getValue() * line_spacing);

                current_line->setBounds({Twips(0), current_y, current_x, line_height});
                block->lines.push_back(std::move(current_line));

                current_line = std::make_unique<LineBox>();
                current_y = current_y + line_height;
                current_x = block->list_indent + para_left_indent;
                
                auto run_box = std::make_unique<RunBox>();
                run_box->logical_text = "\v";
                run_box->logical_offset = logical_offset_base + logical_offset + start;
                run_box->style = std::move(word_style);
                run_box->setBounds({current_x, Twips(0), Twips(0), line_height});
                current_line->runs.push_back(std::move(run_box));
                
                start++;
                continue;
            }

            size_t run_end = start;
            bool is_field = false;
            if (para.length() >= start + 7 && para.substr(start, 7) == "|FIELD:") {
                size_t field_end = para.find("|", start + 7);
                if (field_end != std::string::npos) {
                    run_end = field_end + 1;
                    is_field = true;
                }
            }

            if (!is_field) {
                while (run_end < para.length() && para[run_end] != ' ' && para[run_end] != '\v') {
                    if (para.length() >= run_end + 7 && para.substr(run_end, 7) == "|FIELD:") break;

                    PropertyBag next_style = registry.getStyleAt(logical_offset_base + logical_offset + run_end);
                    if (!stylesAreEqual(word_style, next_style)) {
                        break;
                    }
                    run_end++;
                }
            }

            bool has_space_after = (!is_field && run_end < para.length() && para[run_end] == ' ');

            std::string word = para.substr(start, run_end - start);
            if (has_space_after) {
                word += " ";
            }
            std::string display_text = word;

            if (is_field) {
                if (word == "|FIELD:DATE|") {
                    auto t = std::time(nullptr);
                    auto tm = *std::localtime(&t);
                    char buf[64];
                    std::strftime(buf, sizeof(buf), "%d/%m/%Y", &tm);
                    display_text = buf;
                } else if (word == "|FIELD:TIME|") {
                    auto t = std::time(nullptr);
                    auto tm = *std::localtime(&t);
                    char buf[64];
                    std::strftime(buf, sizeof(buf), "%H:%M", &tm);
                    display_text = buf;
                } else if (word == "|FIELD:TITLE|") {
                    display_text = "Untitled Document";
                } else if (word == "|FIELD:AUTHOR|") {
                    const char* user = std::getenv("USER");
                    display_text = user ? user : "User";
                } else if (word == "|FIELD:PAGE|") {
                    display_text = std::to_string(override_page_number > 0 ? override_page_number : (pages.size() + 1));
                } else if (word == "|FIELD:PAGECOUNT|") {
                    display_text = total_pages > 0 ? std::to_string(total_pages) : "?";
                }
            }

            FontDescriptor desc = font_->getDescriptor();
            if (auto fs = word_style.get(PropertyId::FontSize)) desc.size_pt = std::get<float>(*fs);
            if (auto ff = word_style.get(PropertyId::FontFamily)) desc.family = std::get<std::string>(*ff);
            if (auto fw = word_style.get(PropertyId::FontWeight)) desc.weight = static_cast<FontWeight>(std::get<uint16_t>(*fw));
            if (auto fi = word_style.get(PropertyId::FontStyleItalic)) desc.italic = std::get<bool>(*fi);

            if (auto va = word_style.get(PropertyId::VerticalAlignment)) {
                auto v_align = std::get<VerticalAlign>(*va);
                if (v_align != VerticalAlign::Baseline) {
                    desc.size_pt *= 0.65f;
                }
            }
            
            auto run_font = std::make_shared<DummyFont>(desc);
            ShapedTextRun run = shaper_->shapeText(display_text, run_font);
            Twips word_width = run.total_width;

            auto push_past_masks = [&](Twips& word_x, Twips y_top, Twips word_w, Twips word_h) {
                PLUMA_PROFILE_SCOPE("push_past_masks");
                bool moved = true;
                while (moved) {
                    moved = false;
                    for (const auto& m : active_masks) {
                        if (y_top.getValue() + word_h.getValue() > m.abs_y.getValue() && y_top.getValue() < m.abs_y.getValue() + m.height.getValue()) {
                            int pad = m.pad_px;
                            
                            float layout_w_px = std::max(1.0f, m.width.getValue() / 15.0f);
                            float layout_h_px = std::max(1.0f, m.height.getValue() / 15.0f);
                            
                            float scale_x = static_cast<float>(m.mask->getWidth()) / layout_w_px;
                            float scale_y = static_cast<float>(m.mask->getHeight()) / layout_h_px;
                            
                            int layout_y1 = (y_top - m.abs_y).getValue() / 15 - pad;
                            int layout_y2 = (y_top + word_h - m.abs_y).getValue() / 15 + pad;
                            int layout_x1 = (word_x - m.abs_x).getValue() / 15 - pad;
                            int layout_x2 = (word_x + word_w - m.abs_x).getValue() / 15 + pad;
                            
                            int img_y1 = static_cast<int>(layout_y1 * scale_y);
                            int img_y2 = static_cast<int>(layout_y2 * scale_y);
                            int img_x1 = static_cast<int>(layout_x1 * scale_x);
                            int img_x2 = static_cast<int>(layout_x2 * scale_x);
                            
                            if (img_x1 < m.mask->getWidth() && img_x2 >= 0) {
                                if (m.mask->intersectsRect(img_y1, img_y2, img_x1, img_x2)) {
                                    int img_w_px = img_x2 - img_x1 + 1;
                                    int next_x_px_mask = m.mask->findGap(img_y1, img_y2, img_x1, img_w_px);
                                    if (next_x_px_mask >= m.mask->getWidth() || next_x_px_mask < 0) {
                                        word_x = m.abs_x + m.width + Twips(150);
                                    } else {
                                        int next_x_px_layout = static_cast<int>(std::ceil(next_x_px_mask / scale_x));
                                        if (next_x_px_layout <= layout_x1) {
                                            next_x_px_layout = layout_x1 + 1;
                                        }
                                        word_x = m.abs_x + Twips((next_x_px_layout + pad) * 15);
                                    }
                                    moved = true;
                                }
                            }
                        }
                    }
                }
            };

            push_past_masks(current_x, current_page_y + current_y, word_width, Twips(240));

            Twips dynamic_content_width = column_width;
            if ((current_page_y + current_y + Twips(240)).getValue() > float_right_top.getValue() &&
                (current_page_y + current_y).getValue() < float_right_bottom.getValue()) {
                dynamic_content_width = column_width - float_right_w;
            }
            Twips available_width = Twips((dynamic_content_width - para_right_indent).getValue() - current_x.getValue());
            if (available_width.getValue() < 0) available_width = Twips(0);

            if (word_width.getValue() > available_width.getValue()) {
                Twips base_x = block->list_indent + para_left_indent;
                if ((current_page_y + current_y + Twips(240)).getValue() > float_left_top.getValue() &&
                    (current_page_y + current_y).getValue() < float_left_bottom.getValue()) {
                    if (float_left_w.getValue() > base_x.getValue()) {
                        base_x = float_left_w;
                    }
                }
                if (current_x.getValue() < base_x.getValue()) {
                    current_x = base_x;
                }

                if (current_x.getValue() > (block->list_indent + para_left_indent).getValue()) {
                    Twips line_height = Twips(240); // 12pt approx
                    for (const auto& r : current_line->runs) {
                        Twips h = r->run.max_ascent + r->run.max_descent;
                        if (h.getValue() > line_height.getValue()) line_height = h;
                    }
                    
                    float line_spacing = 1.0f;
                    if (auto ls = para_style.get(PropertyId::LineSpacing)) {
                        line_spacing = std::get<float>(*ls);
                    }
                    line_height = Twips(line_height.getValue() * line_spacing);

                    current_line->setBounds({Twips(0), current_y, current_x, line_height});
                    block->lines.push_back(std::move(current_line));

                    current_line = std::make_unique<LineBox>();
                    current_y = current_y + line_height;
                    
                    Twips base_x = block->list_indent + para_left_indent;
                    if (drop_cap_lines > 0 && current_y.getValue() < dropcap_height.getValue()) {
                        base_x = dropcap_width + Twips(100) + para_left_indent;
                    }
                    if ((current_page_y + current_y + line_height).getValue() > float_left_top.getValue() &&
                        (current_page_y + current_y).getValue() < float_left_bottom.getValue()) {
                        if (float_left_w.getValue() > base_x.getValue()) {
                            base_x = float_left_w;
                        }
                    }
                    current_x = base_x;
                    
                    Twips next_dynamic_content_width = column_width;
                    if ((current_page_y + current_y + line_height).getValue() > float_right_top.getValue() &&
                        (current_page_y + current_y).getValue() < float_right_bottom.getValue()) {
                        next_dynamic_content_width = column_width - float_right_w;
                    }
                    
                    push_past_masks(current_x, current_page_y + current_y, word_width, line_height);
                    
                    available_width = Twips((next_dynamic_content_width - para_right_indent).getValue() - current_x.getValue());
                    if (available_width.getValue() < 0) available_width = Twips(0);
                }

                if (word_width.getValue() > available_width.getValue() && available_width.getValue() > 0) {
                    size_t fit_len = 0;
                    Twips fit_width(0);
                    for (size_t i = 0; i < run.glyphs.size(); ++i) {
                        if (fit_width.getValue() + run.glyphs[i].x_advance.getValue() > available_width.getValue() && fit_len > 0) {
                            break;
                        }
                        fit_width = fit_width + run.glyphs[i].x_advance;
                        fit_len++;
                    }
                    
                    if (fit_len == 0) fit_len = 1;
                    
                    if (fit_len < run.glyphs.size()) {
                        run_end = start + fit_len;
                        has_space_after = false;
                        word = para.substr(start, fit_len);
                        run = shaper_->shapeText(word, run_font);
                        word_width = run.total_width;
                    }
                }
            }

            auto run_box = std::make_unique<RunBox>();
            run_box->run = std::move(run);
            run_box->logical_text = word;
            run_box->display_text = display_text;
            run_box->logical_offset = logical_offset_base + logical_offset + start;
            run_box->style = std::move(word_style);
            run_box->setBounds({current_x, Twips(0), word_width, run_box->run.max_ascent + run_box->run.max_descent});

            current_line->runs.push_back(std::move(run_box));
            current_x = current_x + word_width;

            start = run_end;
            if (has_space_after) start++;
        }
        } // End of |TABLE| else branch

        if (!current_line->runs.empty()) {
            Twips line_height = Twips(180);
            for (const auto& r : current_line->runs) {
                Twips h = r->run.max_ascent + r->run.max_descent;
                if (h.getValue() > line_height.getValue()) line_height = h;
            }
            
            float line_spacing = 1.0f;
            if (auto ls = para_style.get(PropertyId::LineSpacing)) {
                line_spacing = std::get<float>(*ls);
            }
            line_height = Twips(line_height.getValue() * line_spacing);

            current_line->setBounds({Twips(0), current_y, current_x, line_height});
            current_y = current_y + line_height;
            block->lines.push_back(std::move(current_line));
        } else {
            // Empty paragraph (e.g. consecutive newlines)
            Twips line_height = Twips(180); // default 12pt approx
            float line_spacing = 1.0f;
            if (auto ls = para_style.get(PropertyId::LineSpacing)) {
                line_spacing = std::get<float>(*ls);
            }
            line_height = Twips(line_height.getValue() * line_spacing);

            auto dummy_run = std::make_unique<RunBox>();
            dummy_run->logical_offset = logical_offset_base + logical_offset + start;
            dummy_run->setBounds({current_x, Twips(0), Twips(0), line_height});
            dummy_run->run.max_ascent = Twips(144);
            dummy_run->run.max_descent = Twips(36);
            current_line->runs.push_back(std::move(dummy_run));

            current_line->setBounds({Twips(0), current_y, current_x, line_height});
            current_y = current_y + line_height;
            block->lines.push_back(std::move(current_line));
        }

        block->setBounds({margins.left, Twips(0), content_width, current_y}); // Apply left margin offset
        logical_offset += para.length() + 1; // account for \n

        if (spacing_before.getValue() > 0) {
            current_page_y = current_page_y + spacing_before;
        }

        // 2. Paginate the block with Widow & Orphan control
        size_t line_index = 0;
        while (line_index < block->lines.size()) {
            size_t fit_count = 0;
            Twips test_y = current_page_y;
            
            for (size_t i = line_index; i < block->lines.size(); ++i) {
                Twips lh = block->lines[i]->getBounds().height;
                // Bound check against the content height limit (page height - bottom margin - footer)
                if ((test_y + lh).getValue() > (column_start_y.getValue() + content_height.getValue())) break;
                test_y = test_y + lh;
                fit_count++;
            }

            size_t remaining_in_block = block->lines.size() - line_index;

            // Apply Widow/Orphan rules if the block gets split
            if (fit_count < remaining_in_block) {
                // Orphan control: If < 2 lines fit on the current page, push the whole block to next page
                if (fit_count > 0 && fit_count < 2) {
                    fit_count = 0;
                }
                // Widow control: If pushing the rest leaves < 2 lines on the next page (Widow),
                // we pull one more line from the current page to the next page.
                else if (fit_count > 0 && (remaining_in_block - fit_count) < 2) {
                    fit_count -= 1;
                    if (fit_count < 2) {
                        fit_count = 0; // if taking 1 leaves the first page with < 2, just push everything
                    }
                }
            }

            // If no lines can fit, trigger a page break
            if (fit_count == 0) {
                if (!current_page->blocks.empty() || current_page_y.getValue() > column_start_y.getValue()) {
                    if (current_column + 1 < total_columns) {
                        current_column++;
                        current_page_y = column_start_y;
                    } else {
                        pages.push_back(std::move(current_page));
                        setup_page(current_page, pages.size() + 1, current_page_y, content_height);
                        column_start_y = current_page_y;
                        max_page_y = current_page_y;
                        current_column = 0;
                        // Bug fix: limpiar máscaras de imagen y floats al avanzar a la página siguiente.
                        // Sin esto, una imagen en modo Tight en pág N afecta el wrap de texto
                        // en pág N+1 porque las coordenadas absolutas se reinician al mismo valor.
                        active_masks.clear();
                        float_right_w = Twips(0); float_right_top = Twips(0); float_right_bottom = Twips(0);
                        float_left_w  = Twips(0); float_left_top  = Twips(0); float_left_bottom  = Twips(0);
                    }
                    continue; // Retry fitting on the clean page/column
                } else {
                    // Even on a fresh page, the line doesn't fit (maybe it's a huge line).
                    // Force it to fit to avoid infinite loop.
                    fit_count = 1;
                }
            }

            // Create a paginated block holding the fitted lines
            auto sub_block = std::make_unique<BlockBox>();
            sub_block->alignment = block->alignment;
            Twips sub_y(0);
            for (size_t i = 0; i < fit_count; ++i) {
                auto& line = block->lines[line_index + i];
                line->setBounds({line->getBounds().x, sub_y, line->getBounds().width, line->getBounds().height});
                sub_y = sub_y + line->getBounds().height;
                sub_block->lines.push_back(std::move(line));
            }
            
            Twips current_page_x = margins.left + Twips((column_width.getValue() + column_gap.getValue()) * current_column);
            sub_block->setBounds({current_page_x, current_page_y, column_width, sub_y});
            if (line_index == 0 && block->table) {
                sub_block->table = std::move(block->table);
            }
            if (line_index == 0 && !block->images.empty()) {
                sub_block->images = std::move(block->images);
            }
            if (line_index == 0 && block->drop_cap) {
                sub_block->drop_cap = std::move(block->drop_cap);
            }
            sub_block->list_indent = block->list_indent;
            if (line_index == 0) {
                sub_block->list_marker = block->list_marker;
            }
            current_page->blocks.push_back(std::move(sub_block));
            current_page_y = current_page_y + sub_y;
            if (current_page_y.getValue() > max_page_y.getValue()) {
                max_page_y = current_page_y;
            }

            line_index += fit_count;
        }

        if (spacing_after.getValue() > 0) {
            current_page_y = current_page_y + spacing_after;
        }
    }

    if (current_page) {
        pages.push_back(std::move(current_page));
    }

    return pages;
}

} // namespace pluma
