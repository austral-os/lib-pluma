#include <pluma/Editor/CaretResolver.hpp>

namespace pluma {

// ---------------------------------------------------------------------------
// Helper: try to resolve a logical offset into a physical rect by scanning
// the lines of a single cell.  Returns nullopt if not found.
// cell_abs_x / cell_abs_y are the absolute document coordinates of the cell's
// top-left corner.
// ---------------------------------------------------------------------------
static std::optional<Rect>
resolveInCellLines(const TableCellBox& cell,
                   Twips cell_abs_x, Twips cell_abs_y,
                   uint32_t logical_offset)
{
    for (const auto& cb : cell.blocks) {
        if (cb->table) {
            Twips table_abs_x = cell_abs_x + cb->getBounds().x;
            Twips table_abs_y = cell_abs_y + cb->getBounds().y;
            for (const auto& row : cb->table->rows) {
                for (const auto& c : row->cells) {
                    Twips c_abs_x = table_abs_x + c->getBounds().x;
                    Twips c_abs_y = table_abs_y + row->getBounds().y;
                    auto result = resolveInCellLines(*c, c_abs_x, c_abs_y, logical_offset);
                    if (result) return result;
                }
            }
        } else {
            for (const auto& line : cb->lines) {
                for (const auto& run_box : line->runs) {
                    uint32_t run_start = run_box->logical_offset;
                    uint32_t run_end   = run_start + run_box->logical_text.length();

                    if (logical_offset >= run_start && logical_offset <= run_end) {
                        uint32_t char_index = logical_offset - run_start;

                        Twips caret_x = cell_abs_x
                                      + cb->getBounds().x
                                      + line->getBounds().x
                                      + run_box->getBounds().x;

                        for (size_t i = 0; i < char_index && i < run_box->run.glyphs.size(); ++i)
                            caret_x = caret_x + run_box->run.glyphs[i].x_advance;

                        Twips char_width(120);
                        if (char_index < run_box->run.glyphs.size())
                            char_width = run_box->run.glyphs[char_index].x_advance;
                        else if (!run_box->run.glyphs.empty())
                            char_width = run_box->run.glyphs.back().x_advance;

                        return Rect{
                            caret_x,
                            cell_abs_y + cb->getBounds().y + line->getBounds().y,
                            char_width,
                            line->getBounds().height
                        };
                    }
                }
            }
        }
    }
    return std::nullopt;
}

// ---------------------------------------------------------------------------
std::optional<Rect>
CaretResolver::resolveLogicalToPhysical(
    const std::vector<std::unique_ptr<PageBox>>& pages,
    uint32_t logical_offset,
    Twips page_gap)
{
    Twips current_page_y(page_gap);

    for (const auto& page : pages) {
        Twips current_block_y = current_page_y + page->getBounds().y;

        for (const auto& block : page->blocks) {
            Twips block_abs_y = current_block_y + block->getBounds().y;

            // ── Table cells ────────────────────────────────────────────────
            if (block->table) {
                Twips table_abs_x = block->getBounds().x;
                Twips table_abs_y = block_abs_y + block->table->getBounds().y;

                for (const auto& row : block->table->rows) {
                    for (const auto& cell : row->cells) {
                        Twips cell_abs_x = table_abs_x + cell->getBounds().x;
                        Twips cell_abs_y = table_abs_y + row->getBounds().y;

                        auto result = resolveInCellLines(
                            *cell, cell_abs_x, cell_abs_y, logical_offset);
                        if (result) return result;
                    }
                }
            }

            // ── Regular text lines ─────────────────────────────────────────
            Twips current_line_y = block_abs_y;
            for (const auto& line : block->lines) {
                for (const auto& run_box : line->runs) {
                    uint32_t run_start = run_box->logical_offset;
                    uint32_t run_end   = run_start + run_box->logical_text.length();

                    if (logical_offset >= run_start && logical_offset <= run_end) {
                        uint32_t char_index = logical_offset - run_start;

                        Twips remaining_space = Twips(
                            block->getBounds().width.getValue() -
                            line->getBounds().width.getValue());
                        if (remaining_space.getValue() < 0) remaining_space = Twips(0);

                        Twips align_offset(0), justify_gap(0);
                        bool is_last_line = (&line == &block->lines.back());
                        if (block->alignment == TextAlign::Center)
                            align_offset = Twips(remaining_space.getValue() / 2);
                        else if (block->alignment == TextAlign::Right)
                            align_offset = remaining_space;
                        else if (block->alignment == TextAlign::Justify &&
                                 !is_last_line && line->runs.size() > 1)
                            justify_gap = Twips(remaining_space.getValue() /
                                                (int)(line->runs.size() - 1));

                        int run_index = 0;
                        for (size_t i = 0; i < line->runs.size(); ++i) {
                            if (line->runs[i].get() == run_box.get()) {
                                run_index = (int)i;
                                break;
                            }
                        }

                        Twips caret_x = block->getBounds().x
                                      + line->getBounds().x
                                      + align_offset
                                      + run_box->getBounds().x
                                      + Twips(justify_gap.getValue() * run_index);

                        for (size_t i = 0;
                             i < char_index && i < run_box->run.glyphs.size();
                             ++i)
                            caret_x = caret_x + run_box->run.glyphs[i].x_advance;

                        Twips char_width(120);
                        if (char_index < run_box->run.glyphs.size())
                            char_width = run_box->run.glyphs[char_index].x_advance;
                        else if (!run_box->run.glyphs.empty())
                            char_width = run_box->run.glyphs.back().x_advance;

                        return Rect{
                            caret_x,
                            current_line_y + line->getBounds().y,
                            char_width,
                            line->getBounds().height
                        };
                    }
                }
            }
        }
        current_page_y = current_page_y + page->getBounds().height + page_gap;
    }

    return std::nullopt;
}

// ---------------------------------------------------------------------------
// Helper: recursively resolve a physical (x, y) click inside a block (can be table or text).
// ---------------------------------------------------------------------------
static std::optional<uint32_t>
resolveClickInBlock(const BlockBox& block, Twips block_abs_x, Twips block_abs_y, Twips x, Twips y)
{
    if (block.table) {
        Twips table_abs_x = block_abs_x + block.table->getBounds().x;
        Twips table_abs_y = block_abs_y + block.table->getBounds().y;

        for (const auto& row : block.table->rows) {
            Twips row_abs_y = table_abs_y + row->getBounds().y;
            for (const auto& cell : row->cells) {
                Twips cell_abs_x = table_abs_x + cell->getBounds().x;
                Twips cell_abs_y = row_abs_y;
                Twips cell_w     = cell->getBounds().width;
                Twips cell_h     = cell->getBounds().height;

                bool in_cell_x = (x.getValue() >= cell_abs_x.getValue() &&
                                  x.getValue() <  (cell_abs_x + cell_w).getValue());
                bool in_cell_y = (y.getValue() >= cell_abs_y.getValue() &&
                                  y.getValue() <  (cell_abs_y + cell_h).getValue());

                if (in_cell_x && in_cell_y) {
                    for (size_t i = 0; i < cell->blocks.size(); ++i) {
                        const auto& cb = cell->blocks[i];
                        Twips cb_abs_x = cell_abs_x + cb->getBounds().x;
                        Twips cb_abs_y = cell_abs_y + cb->getBounds().y;

                        bool is_last_cb = (i == cell->blocks.size() - 1);
                        if (y.getValue() >= (cb_abs_y + cb->getBounds().height).getValue()
                            && !is_last_cb)
                            continue;

                        auto result = resolveClickInBlock(*cb, cb_abs_x, cb_abs_y, x, y);
                        if (result) return result;
                    }
                    // If we missed all blocks (e.g., clicked in padding), return the end of the last block
                    if (!cell->blocks.empty()) {
                        const auto& last_block = cell->blocks.back();
                        if (!last_block->lines.empty()) {
                            const auto& last_line = last_block->lines.back();
                            if (!last_line->runs.empty()) {
                                const auto& last_run = last_line->runs.back();
                                return last_run->logical_offset + last_run->logical_text.length();
                            }
                        }
                    }
                    return std::nullopt;
                }
            }
        }
        return std::nullopt;
    }

    for (size_t l_idx = 0; l_idx < block.lines.size(); ++l_idx) {
        const auto& line = block.lines[l_idx];
        Twips line_abs_y = block_abs_y + line->getBounds().y;
        Twips line_abs_x = block_abs_x;

        Twips remaining_space = Twips(
            block.getBounds().width.getValue() -
            line->getBounds().width.getValue());
        if (remaining_space.getValue() < 0) remaining_space = Twips(0);

        Twips align_offset(0), justify_gap(0);
        bool is_last_line = (l_idx == block.lines.size() - 1);
        if (block.alignment == TextAlign::Center)
            align_offset = Twips(remaining_space.getValue() / 2);
        else if (block.alignment == TextAlign::Right)
            align_offset = remaining_space;
        else if (block.alignment == TextAlign::Justify &&
                 !is_last_line && line->runs.size() > 1)
            justify_gap = Twips(remaining_space.getValue() /
                                (int)(line->runs.size() - 1));

        Twips line_base_x = line_abs_x
                          + line->getBounds().x
                          + align_offset;
        Twips current_justify_offset(0);

        if (y.getValue() >= (line_abs_y + line->getBounds().height).getValue()
            && !is_last_line)
            continue;

        for (size_t r_idx = 0; r_idx < line->runs.size(); ++r_idx) {
            const auto& run_box = line->runs[r_idx];
            Twips run_x     = line_base_x + run_box->getBounds().x + current_justify_offset;
            Twips run_width = run_box->getBounds().width;
            current_justify_offset = current_justify_offset + justify_gap;

            bool is_last_run = (r_idx == line->runs.size() - 1);
            if (x.getValue() >= (run_x + run_width).getValue() && !is_last_run)
                continue;

            uint32_t char_index = 0;
            Twips current_x = run_x;
            for (size_t i = 0; i < run_box->run.glyphs.size(); ++i) {
                Twips adv = run_box->run.glyphs[i].x_advance;
                if (x.getValue() <= (current_x + Twips(adv.getValue() / 2)).getValue())
                    break;
                current_x = current_x + adv;
                char_index++;
            }
            return run_box->logical_offset + char_index;
        }
    }
    
    if (block.lines.empty() && !block.images.empty()) {
        return block.images.front()->logical_offset;
    }
    
    return std::nullopt;
}

// ---------------------------------------------------------------------------
std::optional<uint32_t>
CaretResolver::resolvePhysicalToLogical(
    const std::vector<std::unique_ptr<PageBox>>& pages,
    Twips x, Twips y,
    Twips page_gap)
{
    if (pages.empty()) return 0;

    Twips current_page_y(page_gap);
    for (const auto& page : pages) {
        bool is_last_page = (&page == &pages.back());
        if (y.getValue() >= (current_page_y + page->getBounds().height + page_gap).getValue()
            && !is_last_page) {
            current_page_y = current_page_y + page->getBounds().height + page_gap;
            continue;
        }

        Twips current_block_y = current_page_y + page->getBounds().y;
        for (const auto& block : page->blocks) {
            Twips block_abs_y = current_block_y + block->getBounds().y;

            Twips block_abs_x = block->getBounds().x;

            bool is_last_block = (&block == &page->blocks.back());
            if (y.getValue() >= (block_abs_y + block->getBounds().height).getValue()
                && !is_last_block)
                continue;

            auto result = resolveClickInBlock(*block, block_abs_x, block_abs_y, x, y);
            if (result) return result;
        }

        current_page_y = current_page_y + page->getBounds().height + page_gap;
    }

    // Fallback: end of document
    if (!pages.empty() &&
        !pages.back()->blocks.empty() &&
        !pages.back()->blocks.back()->lines.empty() &&
        !pages.back()->blocks.back()->lines.back()->runs.empty())
    {
        auto& last_run = pages.back()->blocks.back()->lines.back()->runs.back();
        return last_run->logical_offset + last_run->logical_text.length();
    }

    return 0;
}

} // namespace pluma
