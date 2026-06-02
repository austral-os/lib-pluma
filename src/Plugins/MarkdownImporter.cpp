#include <pluma/Plugins/MarkdownImporter.hpp>
#include <pluma/PlumaEditor.hpp>

namespace pluma {
namespace plugins {

bool MarkdownImporter::import(const std::string& data, PlumaEditor& editor) {
    // In a full implementation we would parse Markdown ast and emit styled runs.
    // For MVP phase, we just load the plain text.
    editor.loadText(data);
    return true;
}

} // namespace plugins
} // namespace pluma
