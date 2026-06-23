/**
 * @file CaretResolver.hpp
 * @brief Utility to resolve logical offsets to physical coordinates.
 */
#pragma once

#include <vector>
#include <memory>
#include <optional>
#include <pluma/CoreTypes.hpp>
#include <pluma/Layout/LayoutBox.hpp>

namespace pluma {

/**
 * @class CaretResolver
 * @brief Maps logical document offsets to physical rendering rectangles.
 */
class CaretResolver {
public:
    /**
     * @brief Resolves a logical offset into a physical bounding rect for the caret.
     * 
     * Iterates through the layout tree to find the precise visual location
     * of the requested logical text offset.
     * 
     * @param pages The fully laid out page boxes.
     * @param logical_offset The document offset to search for.
     * @param page_gap The vertical gap between pages.
     * @return std::optional containing the physical Rect if found.
     */
    static std::optional<Rect> resolveLogicalToPhysical(const std::vector<std::unique_ptr<PageBox>>& pages, uint32_t logical_offset, Twips page_gap, DocumentRegion region = DocumentRegion::Body);

    /**
     * @brief Resolves a physical coordinate to a logical text offset.
     * 
     * Iterates the layout tree to find which character the physical coordinate falls on.
     * Used for hit-testing (e.g. mouse clicks).
     * 
     * @param pages The fully laid out page boxes.
     * @param x The physical X coordinate (absolute document space).
     * @param y The physical Y coordinate (absolute document space).
     * @param page_gap The vertical gap between pages.
     * @return std::optional containing the logical offset if found.
     */
    static std::optional<uint32_t> resolvePhysicalToLogical(const std::vector<std::unique_ptr<PageBox>>& pages, Twips x, Twips y, Twips page_gap, DocumentRegion region = DocumentRegion::Body);
};

} // namespace pluma
