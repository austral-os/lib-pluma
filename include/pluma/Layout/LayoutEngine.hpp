/**
 * @file LayoutEngine.hpp
 * @brief Performs line breaking and pagination for the layout tree.
 */
#pragma once

#include <vector>
#include <memory>
#include <string_view>
#include <pluma/Layout/LayoutBox.hpp>
#include <pluma/Layout/PageSize.hpp>
#include <pluma/Typography/ITextShaper.hpp>

#include <pluma/Style/FormatRegistry.hpp>
#include <functional>

namespace pluma {

/**
 * @struct TableCellCache
 * @brief Opaque cache for table-cell layout results (defined in LayoutEngine.cpp).
 *
 * Cache key includes cell text, width, offset, page number, total pages,
 * and a style-properties hash that also accounts for span position, so
 * moving a style range invalidates the cache even if the property set
 * is unchanged.
 */
struct TableCellCache;

/**
 * @class LayoutEngine
 * @brief The engine responsible for computing geometry and constructing the Layout Tree.
 */
class LayoutEngine {
public:
    /**
     * @brief Constructs a LayoutEngine.
     * @param shaper The text shaper used to measure and resolve glyphs.
     * @param default_font The font used for shaping.
     */
    LayoutEngine(std::shared_ptr<ITextShaper> shaper, std::shared_ptr<IFont> default_font);

    ~LayoutEngine();

    /**
     * @brief Lays out a raw string into a series of paginated boxes.
     * 
     * Performs word-boundary line breaking and overflow pagination.
     * 
     * @param text The text to layout.
     * @param page_size The available dimensions per page.
     * @param margins The margins of the page.
     * @param registry The style registry for format extraction.
     * @return A vector of unique pointers to the generated PageBoxes.
     */
    std::vector<std::unique_ptr<PageBox>> layoutText(
        std::string_view text, 
        PageSize page_size, 
        PageMargins margins, 
        const FormatRegistry& registry,
        std::string_view header_text = "",
        const FormatRegistry* header_registry = nullptr,
        std::string_view footer_text = "",
        const FormatRegistry* footer_registry = nullptr,
        const std::function<bool(int)>& has_header_cb = nullptr,
        const std::function<bool(int)>& has_footer_cb = nullptr,
        uint32_t logical_offset_base = 0,
        int override_page_number = -1,
        bool force_header_space = false,
        bool force_footer_space = false,
        int total_pages = -1
    );

    /// @brief Returns the number of table-cell cache hits in the most recent
    ///        top-level layoutText() call.  Exposed for test verification.
    int cell_cache_hits() const { return cell_cache_hits_; }

    /// @brief Returns the number of table-cell cache misses (incl. bypass) in
    ///        the most recent top-level layoutText() call.  Exposed for tests.
    int cell_cache_misses() const { return cell_cache_misses_; }

private:
    /**
     * @brief Computes a hash of style properties ONLY within the given
     *        [range_start, range_end) offset range in the registry.
     *
     * Unlike the previous global hash (which included ALL spans with
     * absolute positions), this hash is invariant under text insertion
     * or deletion OUTSIDE the range — the cell's own style runs are
     * identical.  Style changes WITHIN the range correctly change the
     * hash (cache miss).
     *
     * Iterates by calling getStyleAt / getStyleRangeEnd to skip ahead
     * efficiently inside the range; typically 1-3 iterations per cell.
     */
    static size_t computeCellStyleHash(const FormatRegistry& registry,
                                       uint32_t range_start,
                                       uint32_t range_end);

    std::shared_ptr<ITextShaper> shaper_;
    std::shared_ptr<IFont> font_;

    /// @brief Opaque table-cell layout cache (defined in LayoutEngine.cpp).
    std::unique_ptr<TableCellCache> cell_cache_;

    /// @brief Layout recursion depth (0 = idle, 1 = top-level layout).
    int layout_depth_ = 0;

    /// @brief Table-cell cache hit counter (reset per top-level layoutText call).
    int cell_cache_hits_ = 0;

    /// @brief Table-cell cache miss counter (reset per top-level layoutText call).
    int cell_cache_misses_ = 0;
};

} // namespace pluma
