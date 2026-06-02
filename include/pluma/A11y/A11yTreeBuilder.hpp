/**
 * @file A11yTreeBuilder.hpp
 * @brief Generates an accessibility tree from the Pluma document state.
 */
#pragma once
#include <memory>
#include <pluma/A11y/A11yNode.hpp>
#include <pluma/DOM/DOMNode.hpp>
#include <pluma/Layout/LayoutBox.hpp>

namespace pluma {

class DOMManager;

namespace a11y {

/**
 * @class A11yTreeBuilder
 * @brief Utility class to map DOM and Layout structures into a unified A11y tree.
 */
class A11yTreeBuilder {
public:
    /**
     * @brief Builds a complete accessible tree.
     * @param dom_root The root of the logical DOM.
     * @param full_text The full text of the document to extract run strings.
     * @param pages The paginated layout containing physical geometry.
     * @return The root A11yNode.
     */
    static std::shared_ptr<A11yNode> build(const DOMNode& dom_root, std::string_view full_text, const std::vector<std::unique_ptr<PageBox>>& pages);

private:
    static std::shared_ptr<A11yNode> traverseDOM(const DOMNode& dom_node, std::string_view full_text);
};

} // namespace a11y
} // namespace pluma
