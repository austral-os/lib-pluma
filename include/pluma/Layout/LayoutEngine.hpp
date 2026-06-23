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
        bool force_footer_space = false
    );

private:
    std::shared_ptr<ITextShaper> shaper_;
    std::shared_ptr<IFont> font_;
};

} // namespace pluma
