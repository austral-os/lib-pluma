#include <pluma/Plugins/PlumaArchiveExporter.hpp>
#include <zip.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <regex>
#include <filesystem>

using json = nlohmann::json;

namespace pluma {
namespace plugins {

// Helper to convert PropertyValue variant to JSON
struct PropertyValueToJson {
    json operator()(const std::string& v) const { return v; }
    json operator()(float v) const { return v; }
    json operator()(uint16_t v) const { return v; }
    json operator()(bool v) const { return v; }
    json operator()(Color v) const { return v; }
    json operator()(TextAlign v) const { return static_cast<int>(v); }
    json operator()(VerticalAlign v) const { return static_cast<int>(v); }
    json operator()(TextDecoration v) const { return static_cast<int>(v); }
    json operator()(TextWrapMode v) const { return static_cast<int>(v); }
    json operator()(int v) const { return v; }
};

std::string PlumaArchiveExporter::exportDoc(std::shared_ptr<DocumentSnapshot> /* snapshot */) {
    return ""; // We only support binary file export
}

bool PlumaArchiveExporter::exportToFile(const std::string& filename, PlumaEditor& editor) {
    int errorp = 0;
    zip_t* archive = zip_open(filename.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &errorp);
    if (!archive) {
        return false;
    }

    json doc;
    doc["version"] = "1.0";
    
    int img_counter = 0;
    
    auto export_region = [&](const std::string& text, const FormatRegistry& format_registry) -> json {
        json region;
        std::regex img_regex(R"(\|IMAGE:([^:]+):([^\|]+)\|)");
        std::smatch match;
        std::string modified_text = text;
        std::string search_text = text;
        
        while (std::regex_search(search_text, match, img_regex)) {
            std::string mode = match[1].str();
            std::string original_path = match[2].str();
            
            if (original_path.find("assets/") != 0) {
                std::filesystem::path path_obj(original_path);
                std::string ext = path_obj.extension().string();
                std::string asset_name = "assets/img_" + std::to_string(img_counter++) + ext;
                
                zip_source_t* img_source = zip_source_file(archive, original_path.c_str(), 0, 0);
                if (img_source) {
                    zip_file_add(archive, asset_name.c_str(), img_source, ZIP_FL_OVERWRITE);
                    
                    std::string to_replace = "|IMAGE:" + mode + ":" + original_path + "|";
                    std::string replace_with = "|IMAGE:" + mode + ":" + asset_name + "|";
                    
                    size_t pos = modified_text.find(to_replace);
                    if (pos != std::string::npos) {
                        modified_text.replace(pos, to_replace.length(), replace_with);
                    }
                }
            }
            search_text = match.suffix().str();
        }
        
        region["text"] = modified_text;
        
        json styles = json::array();
        for (const auto& span : format_registry.getSpans()) {
            const auto& bag = span.style;
            for (const auto& [prop_id, prop_val] : bag.getAll()) {
                json style_obj;
                style_obj["start"] = span.start;
                style_obj["length"] = span.length;
                style_obj["propertyId"] = static_cast<int>(prop_id);
                style_obj["value"] = std::visit(PropertyValueToJson{}, prop_val);
                styles.push_back(style_obj);
            }
        }
        region["styles"] = styles;
        return region;
    };
    
    // Main Document (for backwards compatibility we write it at root)
    json main_region = export_region(editor.getText(), editor.getFormatRegistry());
    doc["text"] = main_region["text"];
    doc["styles"] = main_region["styles"];
    
    // Header and Footer
    doc["header"] = export_region(editor.getHeaderText(), editor.getHeaderFormatRegistry());
    doc["footer"] = export_region(editor.getFooterText(), editor.getFooterFormatRegistry());

    doc["ignored_words"] = editor.getIgnoredWords();

    std::string content = doc.dump(2);

    zip_source_t* source = zip_source_buffer(archive, content.c_str(), content.size(), 0);
    if (source == nullptr || zip_file_add(archive, "document.json", source, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8) < 0) {
        if (source) zip_source_free(source);
        zip_close(archive);
        return false;
    }

    zip_close(archive);
    return true;
}

} // namespace plugins
} // namespace pluma
