#include <pluma/Plugins/MarkdownExporter.hpp>
#include <pluma/DOM/DOMManager.hpp>

namespace pluma {
namespace plugins {

std::string MarkdownExporter::exportDoc(std::shared_ptr<DocumentSnapshot> snapshot) {
    if (!snapshot) return "";
    
    // For now, since our rich text capabilities are minimal,
    // exporting to markdown is simply retrieving the plain text.
    // If we had bold runs, we would wrap them in **bold**.
    return snapshot->getText();
}

} // namespace plugins
} // namespace pluma
