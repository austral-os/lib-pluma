/**
 * @file LayoutBox.hpp
 * @brief Defines the basic geometric boxes for the layout tree.
 */
#pragma once

#include <vector>
#include <memory>
#include <string>
#include <pluma/CoreTypes.hpp>
#include <pluma/Typography/Glyph.hpp>

#include <pluma/Style/PropertyBag.hpp>
#include <pluma/Style/StyleProperties.hpp>

namespace pluma {

/**
 * @enum BoxType
 * @brief Categorizes the type of a layout box.
 */
enum class BoxType {
    Page,
    Block,
    Line,
    Run,
    Table,
    TableRow,
    TableCell,
    Image
};

class TableBox;
class ImageBox;

/**
 * @class LayoutBox
 * @brief Base class for all elements in the layout tree.
 */
class LayoutBox {
public:
    virtual ~LayoutBox() = default;

    /**
     * @brief Gets the exact type of this box.
     * @return The BoxType enumeration.
     */
    virtual BoxType getType() const = 0;

    /**
     * @brief Retrieves the bounding geometry of this box relative to its parent.
     * @return The bounding Rect.
     */
    Rect getBounds() const { return bounds_; }

    /**
     * @brief Sets the bounding geometry of this box.
     * @param bounds The new bounding Rect.
     */
    void setBounds(const Rect& bounds) { bounds_ = bounds; }

private:
    Rect bounds_{Twips(0), Twips(0), Twips(0), Twips(0)};
};

/**
 * @class RunBox
 * @brief Represents a continuous sequence of shaped glyphs with the same style.
 */
class RunBox : public LayoutBox {
public:
    BoxType getType() const override { return BoxType::Run; }

    ShapedTextRun run;      ///< The shaped glyphs and metrics.
    std::string logical_text; ///< The raw text underlying this run.
    uint32_t logical_offset;  ///< The document offset where this run begins.
    PropertyBag style;        ///< The rich text styles applied to this run.
};

/**
 * @class LineBox
 * @brief Represents a single horizontal line of text.
 */
class LineBox : public LayoutBox {
public:
    BoxType getType() const override { return BoxType::Line; }

    std::vector<std::unique_ptr<RunBox>> runs; ///< The text runs within this line.
    Twips baseline{0};                         ///< The typographic baseline offset.
};

/**
 * @class BlockBox
 * @brief Represents a block-level element, typically a paragraph.
 */
class BlockBox : public LayoutBox {
public:
    BoxType getType() const override { return BoxType::Block; }

    std::vector<std::unique_ptr<LineBox>> lines; ///< The formatted lines within this block.
    std::vector<std::unique_ptr<ImageBox>> images; ///< Floating or absolute images in this block
    std::unique_ptr<TableBox> table;             ///< Optional table within this block.
    std::unique_ptr<RunBox> drop_cap;            ///< Drop cap for this block
    
    std::string list_marker;                     ///< Bullet or number string
    Twips list_indent{0};                        ///< Indentation for list item
    
    TextAlign alignment{TextAlign::Left};        ///< Paragraph alignment.
};

/**
 * @class PageBox
 * @brief Represents a physical or virtual printed page.
 */
class PageBox : public LayoutBox {
public:
    BoxType getType() const override { return BoxType::Page; }

    std::vector<std::unique_ptr<BlockBox>> blocks; ///< The blocks rendered on this page.
    std::vector<std::unique_ptr<BlockBox>> header_blocks;
    std::vector<std::unique_ptr<BlockBox>> footer_blocks;
};

/**
 * @class TableCellBox
 * @brief Represents a cell in a table.
 */
class TableCellBox : public LayoutBox {
public:
    BoxType getType() const override { return BoxType::TableCell; }

    std::vector<std::unique_ptr<BlockBox>> blocks; ///< Blocks inside the cell
};

/**
 * @class TableRowBox
 * @brief Represents a row in a table.
 */
class TableRowBox : public LayoutBox {
public:
    BoxType getType() const override { return BoxType::TableRow; }

    std::vector<std::unique_ptr<TableCellBox>> cells; ///< Cells within this row
};

/**
 * @class TableBox
 * @brief Represents a table.
 */
class TableBox : public LayoutBox {
public:
    BoxType getType() const override { return BoxType::Table; }

    std::vector<std::unique_ptr<TableRowBox>> rows; ///< Rows within this table
    bool hide_most_borders = false;                 ///< Demo flag to hide borders
    uint32_t logical_offset{0};
};

/**
 * @class ImageBox
 * @brief Represents an image.
 */
class ImageBox : public LayoutBox {
public:
    BoxType getType() const override { return BoxType::Image; }

    std::string path;
    TextWrapMode wrap_mode = TextWrapMode::InLine;
    uint32_t logical_offset = 0;
    std::string image_id;
    std::string title;
};

} // namespace pluma
