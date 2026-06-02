/**
 * @file A11yNode.hpp
 * @brief Represents a semantic node for Accessibility (a11y) frameworks like AT-SPI2.
 */
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <pluma/CoreTypes.hpp>

namespace pluma {
namespace a11y {

/**
 * @brief Semantic roles defined similarly to AT-SPI2 / ARIA roles.
 */
enum class Role {
    Document,
    Paragraph,
    StaticText,
    Image,
    Unknown
};

/**
 * @brief State flags for the accessible node.
 */
enum StateFlags : uint32_t {
    None = 0,
    Focusable = 1 << 0,
    Focused = 1 << 1,
    Editable = 1 << 2,
    Selectable = 1 << 3,
    Selected = 1 << 4
};

inline StateFlags operator|(StateFlags a, StateFlags b) {
    return static_cast<StateFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline StateFlags operator&(StateFlags a, StateFlags b) {
    return static_cast<StateFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

/**
 * @class A11yNode
 * @brief An accessible node containing text, semantic role, and screen coordinates.
 */
class A11yNode {
public:
    Role role{Role::Unknown};
    StateFlags states{StateFlags::None};
    
    std::string name;
    std::string text_content;
    
    // Bounds in document physical coordinates (Twips)
    Rect physical_bounds{Twips(0), Twips(0), Twips(0), Twips(0)};
    
    std::vector<std::shared_ptr<A11yNode>> children;

    void addChild(std::shared_ptr<A11yNode> child) {
        children.push_back(std::move(child));
    }
};

} // namespace a11y
} // namespace pluma
