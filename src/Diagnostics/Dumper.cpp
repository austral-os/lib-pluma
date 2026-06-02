#include <pluma/Diagnostics/Dumper.hpp>
#include <sstream>

namespace pluma {
namespace diagnostics {

std::string Dumper::dumpDOM(const DOMNode& root, int depth) {
    std::stringstream ss;
    std::string indent(depth * 2, ' ');
    
    ss << indent << "DOMNode[";
    switch (root.getType()) {
        case NodeType::Document: ss << "Document"; break;
        case NodeType::Paragraph: ss << "Paragraph"; break;
        case NodeType::Run: {
            const auto& run = static_cast<const RunNode&>(root);
            ss << "Run, offset: " << run.getOffset() << ", len: " << run.getLength();
            break;
        }
        case NodeType::Table: ss << "Table"; break;
        case NodeType::TableRow: ss << "TableRow"; break;
        case NodeType::TableCell: ss << "TableCell"; break;
        case NodeType::Image: ss << "Image"; break;
        default: ss << "Unknown"; break;
    }
    ss << "] (id: " << root.getId().getValue() << ")\n";

    for (const auto& child : root.getChildren()) {
        ss << dumpDOM(*child, depth + 1);
    }
    
    return ss.str();
}

std::string Dumper::dumpLayout(const std::vector<std::unique_ptr<PageBox>>& pages) {
    std::stringstream ss;
    for (size_t i = 0; i < pages.size(); ++i) {
        ss << "Page " << i << ":\n";
        ss << dumpLayoutBox(*pages[i], 1);
    }
    return ss.str();
}

std::string Dumper::dumpLayoutBox(const LayoutBox& box, int depth) {
    std::stringstream ss;
    std::string indent(depth * 2, ' ');
    
    ss << indent << "LayoutBox[";
    switch (box.getType()) {
        case BoxType::Page: ss << "Page"; break;
        case BoxType::Block: ss << "Block"; break;
        case BoxType::Line: ss << "Line"; break;
        case BoxType::Run: ss << "Run"; break;
        case BoxType::Table: ss << "Table"; break;
        case BoxType::TableRow: ss << "TableRow"; break;
        case BoxType::TableCell: ss << "TableCell"; break;
        case BoxType::Image: ss << "Image"; break;
    }
    
    const Rect& bounds = box.getBounds();
    ss << ", x=" << bounds.x.getValue() << ", y=" << bounds.y.getValue() 
       << ", w=" << bounds.width.getValue() << ", h=" << bounds.height.getValue();
       
    ss << "]\n";
    
    if (box.getType() == BoxType::Page) {
        for (const auto& child : static_cast<const PageBox&>(box).blocks) {
            ss << dumpLayoutBox(*child, depth + 1);
        }
    } else if (box.getType() == BoxType::Block) {
        for (const auto& child : static_cast<const BlockBox&>(box).lines) {
            ss << dumpLayoutBox(*child, depth + 1);
        }
    } else if (box.getType() == BoxType::Line) {
        for (const auto& child : static_cast<const LineBox&>(box).runs) {
            ss << dumpLayoutBox(*child, depth + 1);
        }
    }
    
    return ss.str();
}

} // namespace diagnostics
} // namespace pluma
