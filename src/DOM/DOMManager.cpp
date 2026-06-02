#include <pluma/DOM/DOMManager.hpp>
#include <string_view>

namespace pluma {

std::unique_ptr<DocumentNode> DOMManager::rebuild(const PieceTable& pieceTable) {
    auto doc = std::make_unique<DocumentNode>(generateId());
    
    std::string text = pieceTable.getText();
    if (text.empty()) {
        auto para = std::make_unique<ParagraphNode>(generateId());
        para->addChild(std::make_unique<RunNode>(generateId(), 0, 0));
        doc->addChild(std::move(para));
        return doc;
    }

    uint32_t current_offset = 0;
    size_t start = 0;
    size_t end = text.find('\n');

    while (start < text.length()) {
        size_t len = (end == std::string::npos) ? text.length() - start : end - start;
        
        auto para = std::make_unique<ParagraphNode>(generateId());
        
        // One run per paragraph for now. Styling breaks would split this into multiple runs.
        para->addChild(std::make_unique<RunNode>(generateId(), current_offset, static_cast<uint32_t>(len)));
        
        doc->addChild(std::move(para));
        
        current_offset += static_cast<uint32_t>(len);
        if (end != std::string::npos) {
            current_offset += 1; // Account for the '\n' character
        }

        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
        end = text.find('\n', start);
    }
    
    // If text explicitly ends with a newline, there is an empty paragraph trailing it
    if (!text.empty() && text.back() == '\n') {
        auto para = std::make_unique<ParagraphNode>(generateId());
        para->addChild(std::make_unique<RunNode>(generateId(), current_offset, 0));
        doc->addChild(std::move(para));
    }

    return doc;
}

} // namespace pluma
