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
        
        std::string modified_text = "";
        size_t last_pos = 0;
        
        struct Shift { size_t orig_pos; int diff; };
        std::vector<Shift> shifts;
        int current_diff = 0;

        auto next = std::sregex_iterator(text.begin(), text.end(), img_regex);
        auto end = std::sregex_iterator();

        while (next != end) {
            std::smatch match = *next;
            std::string mode = match[1].str();
            std::string original_path = match[2].str();
            
            modified_text += text.substr(last_pos, match.position() - last_pos);
            
            std::string replacement = match.str();
            if (original_path.find("assets/") != 0) {
                std::filesystem::path path_obj(original_path);
                std::string ext = path_obj.extension().string();
                std::string asset_name = "assets/img_" + std::to_string(img_counter++) + ext;
                
                zip_source_t* img_source = zip_source_file(archive, original_path.c_str(), 0, 0);
                if (img_source) {
                    zip_file_add(archive, asset_name.c_str(), img_source, ZIP_FL_OVERWRITE);
                    replacement = "|IMAGE:" + mode + ":" + asset_name + "|";
                }
            }
            
            modified_text += replacement;
            
            int diff = static_cast<int>(replacement.length()) - static_cast<int>(match.length());
            if (diff != 0) {
                current_diff += diff;
                shifts.push_back({static_cast<size_t>(match.position() + match.length()), current_diff});
            }
            
            last_pos = match.position() + match.length();
            next++;
        }
        modified_text += text.substr(last_pos);
        
        region["text"] = modified_text;
        
        auto get_new_pos = [&](size_t orig_pos) -> size_t {
            int total_diff = 0;
            for (const auto& shift : shifts) {
                if (orig_pos >= shift.orig_pos) total_diff = shift.diff;
                else break;
            }
            return orig_pos + total_diff;
        };
        
        json styles = json::array();
        for (const auto& span : format_registry.getSpans()) {
            const auto& bag = span.style;
            for (const auto& [prop_id, prop_val] : bag.getAll()) {
                json style_obj;
                size_t new_start = get_new_pos(span.start);
                size_t new_end = get_new_pos(span.start + span.length);
                style_obj["start"] = new_start;
                style_obj["length"] = new_end > new_start ? new_end - new_start : 0;
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
