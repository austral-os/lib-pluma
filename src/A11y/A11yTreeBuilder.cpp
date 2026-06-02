#include <pluma/A11y/A11yTreeBuilder.hpp>
#include <pluma/DOM/DOMManager.hpp>

namespace pluma {
namespace a11y {

std::shared_ptr<A11yNode> A11yTreeBuilder::build(const DOMNode& dom_root, std::string_view full_text, const std::vector<std::unique_ptr<PageBox>>& /* pages */) {
    // Traverse the logical DOM to build the semantic a11y tree
    auto a11y_root = traverseDOM(dom_root, full_text);
    
    if (a11y_root) {
        // Enforce document-level flags
        a11y_root->states = a11y_root->states | StateFlags::Focusable | StateFlags::Editable;
    }
    
    // In a full implementation, we would now map the 'pages' (LayoutBox) 
    // to each A11yNode to calculate precise geometric bounding boxes for AT-SPI2.
    // For this foundational phase, we extract the structural semantics and text contents.

    return a11y_root;
}

std::shared_ptr<A11yNode> A11yTreeBuilder::traverseDOM(const DOMNode& dom_node, std::string_view full_text) {
    auto node = std::make_shared<A11yNode>();
    
    switch (dom_node.getType()) {
        case NodeType::Document:
            node->role = Role::Document;
            node->name = "Document Root";
            break;
        case NodeType::Paragraph:
            node->role = Role::Paragraph;
            break;
        case NodeType::Run: {
            node->role = Role::StaticText;
            const auto& run_node = static_cast<const RunNode&>(dom_node);
            node->text_content = std::string(full_text.substr(run_node.getOffset(), run_node.getLength()));
            node->states = StateFlags::Selectable;
            break;
        }
        default:
            node->role = Role::Unknown;
            break;
    }

    for (const auto& child : dom_node.getChildren()) {
        auto a11y_child = traverseDOM(*child, full_text);
        if (a11y_child) {
            node->addChild(a11y_child);
        }
    }

    return node;
}

} // namespace a11y
} // namespace pluma
