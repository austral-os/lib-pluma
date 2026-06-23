/**
 * @file StyleProperties.hpp
 * @brief Defines the available style properties and their valid values.
 */
#pragma once

#include <variant>
#include <string>
#include <cstdint>

namespace pluma {

enum class TextAlign {
    Left,
    Center,
    Right,
    Justify
};

enum class VerticalAlign {
    Baseline,
    Subscript,
    Superscript
};

enum class TextDecoration {
    None,
    Underline,
    StrikeThrough,
    SpellingError
};

enum class TextWrapMode {
    InLine,
    Square,
    Tight,
    Through,
    TopAndBottom,
    BehindText,
    InFrontOfText
};

/**
 * @enum PropertyId
 * @brief Unique identifiers for all supported styling properties.
 */
enum class PropertyId {
    FontFamily,
    FontSize,
    FontWeight,
    FontStyleItalic,
    TextColor,
    BackgroundColor,
    TextAlignment,
    VerticalAlignment,
    Decoration,
    LineSpacing,
    ParagraphSpacingBefore,
    ParagraphSpacingAfter,
    ParagraphIndentLeft,
    ParagraphIndentRight,
    ParagraphIndentFirstLine,
    TableWidth,
    TableColumnWidths,
    TableRowHeights,
    RowHeight,
    BorderTopVisible,
    BorderTopWidth,
    BorderTopColor,
    BorderTopStyle,
    BorderRightVisible,
    BorderRightWidth,
    BorderRightColor,
    BorderRightStyle,
    BorderBottomVisible,
    BorderBottomWidth,
    BorderBottomColor,
    BorderBottomStyle,
    BorderLeftVisible,
    BorderLeftWidth,
    BorderLeftColor,
    BorderLeftStyle,
    ImagePath,
    ImageWrapMode,
    ImageWidth,
    ImageHeight,
    ImageId,
    ImageTitle,
    DropCapLines,
    ImageX,
    ImageY,
    Language,
    SpellCheckEnabled
};

using Color = uint32_t; ///< Format: ARGB

/**
 * @typedef PropertyValue
 * @brief Variant holding the value of any style property.
 */
using PropertyValue = std::variant<
    std::string,    // FontFamily
    float,          // FontSize
    uint16_t,       // FontWeight
    bool,           // FontStyleItalic
    Color,          // TextColor, BackgroundColor
    TextAlign,      // TextAlignment
    VerticalAlign,  // VerticalAlignment
    TextDecoration, // Decoration
    TextWrapMode,   // ImageWrapMode
    int             // DropCapLines
>;

/**
 * @brief Determines if a given property cascades (inherits) by default.
 * @param id The property identifier.
 * @return true if it inherits down the DOM tree automatically.
 */
inline bool inheritsByDefault(PropertyId id) {
    switch (id) {
        case PropertyId::FontFamily:
        case PropertyId::FontSize:
        case PropertyId::FontWeight:
        case PropertyId::FontStyleItalic:
        case PropertyId::TextColor:
        case PropertyId::TextAlignment:
        case PropertyId::LineSpacing:
        case PropertyId::ParagraphSpacingBefore:
        case PropertyId::ParagraphSpacingAfter:
        case PropertyId::ParagraphIndentLeft:
        case PropertyId::ParagraphIndentRight:
        case PropertyId::ParagraphIndentFirstLine:
        case PropertyId::Language:
        case PropertyId::SpellCheckEnabled:
            return true;
        case PropertyId::BackgroundColor:
        case PropertyId::VerticalAlignment:
        case PropertyId::Decoration:
        case PropertyId::TableWidth:
        case PropertyId::TableColumnWidths:
        case PropertyId::TableRowHeights:
        case PropertyId::RowHeight:
        case PropertyId::BorderTopVisible:
        case PropertyId::BorderTopWidth:
        case PropertyId::BorderTopColor:
        case PropertyId::BorderTopStyle:
        case PropertyId::BorderRightVisible:
        case PropertyId::BorderRightWidth:
        case PropertyId::BorderRightColor:
        case PropertyId::BorderRightStyle:
        case PropertyId::BorderBottomVisible:
        case PropertyId::BorderBottomWidth:
        case PropertyId::BorderBottomColor:
        case PropertyId::BorderBottomStyle:
        case PropertyId::BorderLeftVisible:
        case PropertyId::BorderLeftWidth:
        case PropertyId::BorderLeftColor:
        case PropertyId::BorderLeftStyle:
        case PropertyId::ImagePath:
        case PropertyId::ImageWrapMode:
        case PropertyId::ImageWidth:
        case PropertyId::ImageHeight:
        case PropertyId::ImageId:
        case PropertyId::ImageTitle:
        case PropertyId::DropCapLines:
        case PropertyId::ImageX:
        case PropertyId::ImageY:
            return false;
        default:
            return false;
    }
}

} // namespace pluma
