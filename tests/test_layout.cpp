#include <catch2/catch_test_macros.hpp>
#include <pluma/Layout/LayoutEngine.hpp>
#include <pluma/Typography/DummyTypography.hpp>
#include <sstream>

using namespace pluma;

/// Helper: render a page tree to a simple string for comparison.
static std::string pageTreeToString(const std::vector<std::unique_ptr<PageBox>>& pages) {
    std::ostringstream os;
    for (size_t p = 0; p < pages.size(); ++p) {
        os << "Page " << p << ":\n";
        for (const auto& block : pages[p]->blocks) {
            if (block->table) {
                os << "  Table (rows=" << block->table->rows.size() << "):\n";
                for (const auto& row : block->table->rows) {
                    os << "    Row:\n";
                    for (const auto& cell : row->cells) {
                        os << "      Cell (col=" << cell->col_idx
                           << " cols=" << cell->colspan
                           << " rows=" << cell->rowspan << "):\n";
                        for (const auto& cb : cell->blocks) {
                            os << "        Block:\n";
                            for (const auto& line : cb->lines) {
                                os << "          Line";
                                for (const auto& run : line->runs) {
                                    os << " [" << run->logical_text << "]";
                                }
                                os << "\n";
                            }
                        }
                    }
                }
            } else {
                os << "  Block text:\n";
                for (const auto& line : block->lines) {
                    os << "    Line:";
                    for (const auto& run : line->runs) {
                        os << " [" << run->display_text << "]";
                    }
                    os << "\n";
                }
            }
        }
    }
    return os.str();
}

TEST_CASE("Basic Layout Engine wrapping and pagination", "[layout]") {
    auto shaper = std::make_shared<DummyTextShaper>();
    auto font_manager = std::make_shared<DummyFontManager>();

    FontDescriptor desc;
    desc.family = "Arial";
    desc.size_pt = 12.0f; // Each char is 120 twips wide, ascent=192, descent=48 (height=240)
    
    auto font = font_manager->getFont(desc);
    LayoutEngine engine(shaper, font);

    SECTION("Single short line") {
        std::string text = "Hello World";
        // 11 chars * 120 = 1320 twips
        FormatRegistry registry;
        auto pages = engine.layoutText(text, PageSize(Twips(2000), Twips(2000)), PageMargins(Twips(0), Twips(0), Twips(0), Twips(0)), registry);
        
        REQUIRE(pages.size() == 1);
        REQUIRE(pages[0]->blocks.size() == 1);
        REQUIRE(pages[0]->blocks[0]->lines.size() == 1);
        
        auto& line = pages[0]->blocks[0]->lines[0];
        REQUIRE(line->runs.size() == 2); // "Hello " and "World"
        REQUIRE(line->getBounds().width.getValue() == 1320);
        REQUIRE(line->getBounds().height.getValue() == 240);
    }

    SECTION("Line wrapping") {
        std::string text = "Word1 Word2 Word3";
        // Each word is 5 chars + 1 space (except last)
        // "Word1 " = 6 * 120 = 720
        // "Word2 " = 6 * 120 = 720
        // "Word3" = 5 * 120 = 600
        
        // Let's set page width to 1000 twips, so Word1 and Word2 can't fit on one line (720+720=1440)
        FormatRegistry registry;
        auto pages = engine.layoutText(text, PageSize(Twips(1000), Twips(2000)), PageMargins(Twips(0), Twips(0), Twips(0), Twips(0)), registry);
        
        REQUIRE(pages.size() == 1);
        REQUIRE(pages[0]->blocks[0]->lines.size() == 3);
        
        REQUIRE(pages[0]->blocks[0]->lines[0]->runs[0]->logical_text == "Word1 ");
        REQUIRE(pages[0]->blocks[0]->lines[1]->runs[0]->logical_text == "Word2 ");
        REQUIRE(pages[0]->blocks[0]->lines[2]->runs[0]->logical_text == "Word3");
    }

    SECTION("Pagination") {
        std::string text = "Line1 Line2 Line3 Line4";
        // Each is 720 width. If page width is 1000, we get 4 lines.
        // Each line is 240 twips high. 4 lines = 960 twips height.
        // If page height is 600, it can only fit 2 lines (480 twips) per page.
        
        FormatRegistry registry;
        auto pages = engine.layoutText(text, PageSize(Twips(1000), Twips(600)), PageMargins(Twips(0), Twips(0), Twips(0), Twips(0)), registry);
        
        REQUIRE(pages.size() == 2);
        REQUIRE(pages[0]->blocks[0]->lines.size() == 2); // Line1, Line2
        REQUIRE(pages[1]->blocks[0]->lines.size() == 2); // Line3, Line4
    }
}

TEST_CASE("Table-cell layout cache correctness", "[layout][cache]") {
    auto shaper = std::make_shared<DummyTextShaper>();
    auto font_manager = std::make_shared<DummyFontManager>();
    FontDescriptor desc;
    desc.family = "Arial";
    desc.size_pt = 12.0f;
    auto font = font_manager->getFont(desc);
    LayoutEngine engine(shaper, font);

    PageSize page_size(Twips(5000), Twips(10000));
    PageMargins margins(Twips(60), Twips(60), Twips(60), Twips(60));

    // NOTE: Pluma raw text format requires |CEL| on its own line (exact match).
    // Cell content follows on subsequent lines until next |CEL|, |ROW|, or |ENDTBL|.

    SECTION("Same cell content produces identical layout across calls") {
        FormatRegistry reg;
        std::string doc =
            "Before the table\n"
            "|TBL:cols=2|\n"
            "|ROW|\n"
            "|CEL|\n"
            "Cell A content\n"
            "|CEL|\n"
            "Cell B content\n"
            "|ENDTBL|\n"
            "After the table";

        auto pages1 = engine.layoutText(doc, page_size, margins, reg);
        REQUIRE_FALSE(pages1.empty());

        // Second call with identical input — should exercise the cache
        auto pages2 = engine.layoutText(doc, page_size, margins, reg);
        REQUIRE_FALSE(pages2.empty());

        // The page trees must be structurally identical
        auto s1 = pageTreeToString(pages1);
        auto s2 = pageTreeToString(pages2);
        REQUIRE(s1 == s2);
    }

    SECTION("Typing after the table reuses cell cache") {
        FormatRegistry reg;
        std::string base =
            "|TBL:cols=2|\n"
            "|ROW|\n"
            "|CEL|\n"
            "Alpha\n"
            "|CEL|\n"
            "Beta\n"
            "|ENDTBL|\n";

        auto ref = engine.layoutText(base + "First para", page_size, margins, reg);
        REQUIRE_FALSE(ref.empty());
        // First layout: empty cache → all misses, zero hits.
        REQUIRE(engine.cell_cache_hits() == 0);
        REQUIRE(engine.cell_cache_misses() > 0);

        // Typing after the table (adding more text at the end) should NOT
        // affect cached cell layouts — cells are identical.
        auto after = engine.layoutText(base + "First para\nSecond para", page_size, margins, reg);
        REQUIRE_FALSE(after.empty());

        // Second layout with identical table cells → cache hits
        REQUIRE(engine.cell_cache_hits() > 0);

        // The table portion (first part of the tree) must be identical.
        auto find_table = [](const std::vector<std::unique_ptr<PageBox>>& pages) -> const TableBox* {
            for (const auto& pg : pages)
                for (const auto& blk : pg->blocks)
                    if (blk->table) return blk->table.get();
            return nullptr;
        };
        auto* t1 = find_table(ref);
        auto* t2 = find_table(after);
        REQUIRE(t1 != nullptr);
        REQUIRE(t2 != nullptr);
        REQUIRE(t1->rows.size() == t2->rows.size());
        for (size_t r = 0; r < t1->rows.size(); ++r) {
            REQUIRE(t1->rows[r]->cells.size() == t2->rows[r]->cells.size());
            for (size_t c = 0; c < t1->rows[r]->cells.size(); ++c) {
                auto& c1 = t1->rows[r]->cells[c];
                auto& c2 = t2->rows[r]->cells[c];
                REQUIRE(c1->blocks.size() == c2->blocks.size());
                for (size_t b = 0; b < c1->blocks.size(); ++b) {
                    REQUIRE(c1->blocks[b]->lines.size() == c2->blocks[b]->lines.size());
                    for (size_t l = 0; l < c1->blocks[b]->lines.size(); ++l) {
                        REQUIRE(c1->blocks[b]->lines[l]->runs.size() ==
                                c2->blocks[b]->lines[l]->runs.size());
                        for (size_t rn = 0; rn < c1->blocks[b]->lines[l]->runs.size(); ++rn) {
                            REQUIRE(c1->blocks[b]->lines[l]->runs[rn]->logical_text ==
                                    c2->blocks[b]->lines[l]->runs[rn]->logical_text);
                        }
                    }
                }
            }
        }
    }

    SECTION("Typing before the table reuses cell cache with offset adjustment") {
        FormatRegistry reg;
        std::string table =
            "|TBL:cols=1|\n"
            "|ROW|\n"
            "|CEL|\n"
            "Cell data\n"
            "|ENDTBL|";

        // Layout with no prefix — populates cache
        auto no_prefix = engine.layoutText(table, page_size, margins, reg);
        REQUIRE_FALSE(no_prefix.empty());
        int hits_after_first = engine.cell_cache_hits();

        // Layout with prefix text — cell offset shifts but text/width/styles
        // are identical. Cache should HIT with offset delta applied.
        auto with_prefix = engine.layoutText("Preamble\n" + table, page_size, margins, reg);
        REQUIRE_FALSE(with_prefix.empty());

        // Cache must HIT: same cell text, width, eff_page, cell-local styles
        REQUIRE(engine.cell_cache_hits() > hits_after_first);

        // Verify the cell content is preserved and offsets are correctly adjusted
        auto find_table = [](const std::vector<std::unique_ptr<PageBox>>& pages) -> const TableBox* {
            for (const auto& pg : pages)
                for (const auto& blk : pg->blocks)
                    if (blk->table) return blk->table.get();
            return nullptr;
        };
        auto* tp = find_table(with_prefix);
        REQUIRE(tp != nullptr);
        REQUIRE(tp->rows.size() == 1);
        REQUIRE(tp->rows[0]->cells.size() == 1);
        REQUIRE(tp->rows[0]->cells[0]->blocks.size() >= 1);
        // Accumulate text across runs (DummyTextShaper splits on word boundaries)
        std::string cell_text;
        for (const auto& blk : tp->rows[0]->cells[0]->blocks) {
            for (const auto& line : blk->lines) {
                for (const auto& run : line->runs) {
                    cell_text += run->logical_text;
                }
            }
        }
        REQUIRE(cell_text.find("Cell data") != std::string::npos);

        // Verify logical offsets are adjusted: the run offset should include
        // the "Preamble\n" prefix (9 chars) compared to the no_prefix version.
        auto* tp_no_prefix = find_table(no_prefix);
        REQUIRE(tp_no_prefix != nullptr);
        REQUIRE(tp_no_prefix->rows[0]->cells[0]->blocks.size() >= 1);
        REQUIRE(tp->rows[0]->cells[0]->blocks.size() ==
                tp_no_prefix->rows[0]->cells[0]->blocks.size());
        // Compare logical offsets of first run to verify delta was applied
        uint32_t offset_no_prefix = tp_no_prefix->rows[0]->cells[0]->blocks[0]
                                        ->lines[0]->runs[0]->logical_offset;
        uint32_t offset_with_prefix = tp->rows[0]->cells[0]->blocks[0]
                                        ->lines[0]->runs[0]->logical_offset;
        // "Preamble\n" adds 9 chars → offset delta should be 9
        REQUIRE(offset_with_prefix == offset_no_prefix + 9);
    }

    SECTION("Cache hit counter increments on repeated identical layout") {
        FormatRegistry reg;
        std::string doc =
            "|TBL:cols=1|\n"
            "|ROW|\n"
            "|CEL|\n"
            "Cache me\n"
            "|ENDTBL|";

        // First layout — all cells miss (no prior cache entries)
        auto first = engine.layoutText(doc, page_size, margins, reg);
        REQUIRE_FALSE(first.empty());
        int hits_first = engine.cell_cache_hits();

        // Second layout with identical input — every cell must hit
        auto second = engine.layoutText(doc, page_size, margins, reg);
        REQUIRE_FALSE(second.empty());
        int hits_second = engine.cell_cache_hits();
        int misses_second = engine.cell_cache_misses();

        // At least one cache hit from the second call
        REQUIRE(hits_second > hits_first);
        // No additional misses (identical input, cache populated)
        REQUIRE(misses_second == 0);
    }

    SECTION("Cache miss on changed cell content proves counter tracks misses") {
        FormatRegistry reg;
        std::string doc1 =
            "|TBL:cols=1|\n"
            "|ROW|\n"
            "|CEL|\n"
            "First version\n"
            "|ENDTBL|";
        std::string doc2 =
            "|TBL:cols=1|\n"
            "|ROW|\n"
            "|CEL|\n"
            "Second version\n"
            "|ENDTBL|";

        // First layout populates cache (discard pages, just verify no crash)
        REQUIRE_FALSE(engine.layoutText(doc1, page_size, margins, reg).empty());

        // Second layout has different cell text — must miss
        auto second = engine.layoutText(doc2, page_size, margins, reg);
        REQUIRE_FALSE(second.empty());
        int misses_after = engine.cell_cache_misses();

        // The second call had at least one miss (different cell text)
        REQUIRE(misses_after > 0);
    }

    SECTION("Style property change within cell invalidates cache (miss proves re-layout)") {
        FormatRegistry reg;
        std::string doc =
            "|TBL:cols=1|\n"
            "|ROW|\n"
            "|CEL|\n"
            "Text\n"
            "|ENDTBL|";

        // First layout with plain style
        auto first = engine.layoutText(doc, page_size, margins, reg);
        REQUIRE_FALSE(first.empty());

        // Apply a style at the cell's text range [25,30) (after |TBL:|ROW:|CEL| markers:
        // |TBL:cols=1|\n = 13, |ROW|\n = +6 → 19, |CEL|\n = +6 → 25).
        // This intersects the cell "Text\n" → MUST invalidate the cell-local hash.
        reg.applyStyle(25, 5, PropertyId::FontWeight, uint16_t(700));

        // Second layout — cell range has bold now, hash differs → cache miss
        auto second = engine.layoutText(doc, page_size, margins, reg);
        REQUIRE_FALSE(second.empty());

        // Cache should NOT have hit — cell-local style changed
        REQUIRE(engine.cell_cache_hits() == 0);
        REQUIRE(engine.cell_cache_misses() > 0);

        // The layout should still be valid (just re-laid-out with bold).
        // No crash, no corruption.
        auto find_table = [](const std::vector<std::unique_ptr<PageBox>>& pages) -> const TableBox* {
            for (const auto& pg : pages)
                for (const auto& blk : pg->blocks)
                    if (blk->table) return blk->table.get();
            return nullptr;
        };
        auto* tbl = find_table(second);
        REQUIRE(tbl != nullptr);
        REQUIRE(tbl->rows.size() == 1);
        REQUIRE(tbl->rows[0]->cells.size() == 1);
    }

    SECTION("Style range shift within cell invalidates cache when run boundaries change") {
        FormatRegistry reg;
        std::string doc =
            "|TBL:cols=1|\n"
            "|ROW|\n"
            "|CEL|\n"
            "Styled\n"
            "|ENDTBL|";

        // Cell text "Styled\n" starts at logical offset 25.
        // Apply bold to the SECOND half of the cell (offsets 28-30 within cell range 25-31):
        // This gives: [25,28) plain + [28,31) bold  (run split inside cell).
        reg.applyStyle(28, 3, PropertyId::FontWeight, uint16_t(700));

        auto first = engine.layoutText(doc, page_size, margins, reg);
        REQUIRE_FALSE(first.empty());

        // Now shift the same bold property to cover the FIRST half instead:
        // [25,28) bold + [28,31) plain  — same property set, DIFFERENT run boundaries.
        reg.clear();
        reg.applyStyle(25, 3, PropertyId::FontWeight, uint16_t(700));

        auto second = engine.layoutText(doc, page_size, margins, reg);
        REQUIRE_FALSE(second.empty());

        // Second call must NOT hit — the bold moved to a different part of the cell,
        // producing different run lengths in the cell-local hash.
        REQUIRE(engine.cell_cache_hits() == 0);
        REQUIRE(engine.cell_cache_misses() > 0);
    }

    SECTION("Header/footer registry style change invalidates header cell cache") {
        // Two registries: one for body, one for header.
        // The header contains a table cell; changing header-registry styles
        // must invalidate the header cell cache keys even though the body
        // registry is unchanged.  (Regression: without the fix the cache key
        // used a stale body hash from depth==1, so header style changes
        // would incorrectly produce cache hits.)
        FormatRegistry body_reg;
        FormatRegistry header_reg;
        header_reg.applyStyle(0, 30, PropertyId::FontWeight, uint16_t(700));

        std::string body_text =
            "|TBL:cols=1|\n"
            "|ROW|\n"
            "|CEL|\n"
            "Body cell\n"
            "|ENDTBL|\n";

        std::string header_text =
            "|TBL:cols=1|\n"
            "|ROW|\n"
            "|CEL|\n"
            "Header cell\n"
            "|ENDTBL|\n";

        auto always_header = [](int) { return true; };

        // First layout: populates cache with body-registry hash and
        // header-registry hash (separate keys).
        auto first = engine.layoutText(body_text, page_size, margins, body_reg,
                                        header_text, &header_reg,
                                        "", nullptr, always_header, nullptr,
                                        0, -1, false, false);
        REQUIRE_FALSE(first.empty());

        // Change the header registry style (move property to different span)
        header_reg.clear();
        header_reg.applyStyle(5, 25, PropertyId::FontWeight, uint16_t(400));

        // Second layout — header cells should miss (changed header hash)
        // while body cells may hit (unchanged body hash).
        auto second = engine.layoutText(body_text, page_size, margins, body_reg,
                                         header_text, &header_reg,
                                         "", nullptr, always_header, nullptr,
                                         0, -1, false, false);
        REQUIRE_FALSE(second.empty());

        // At least one cache miss must come from the header table cell
        // with the changed registry hash.
        REQUIRE(engine.cell_cache_misses() > 0);

        // Verify structural correctness: body cell content preserved
        auto find_table = [](const std::vector<std::unique_ptr<PageBox>>& pages) -> const TableBox* {
            for (const auto& pg : pages)
                for (const auto& blk : pg->blocks)
                    if (blk->table) return blk->table.get();
            return nullptr;
        };
        auto* tbl = find_table(second);
        REQUIRE(tbl != nullptr);
        REQUIRE(tbl->rows.size() == 1);
        REQUIRE(tbl->rows[0]->cells.size() == 1);
        // Accumulate cell text across runs
        std::string cell_text;
        for (const auto& blk : tbl->rows[0]->cells[0]->blocks) {
            for (const auto& line : blk->lines) {
                for (const auto& run : line->runs) {
                    cell_text += run->logical_text;
                }
            }
        }
        REQUIRE(cell_text.find("Body cell") != std::string::npos);
    }
}
