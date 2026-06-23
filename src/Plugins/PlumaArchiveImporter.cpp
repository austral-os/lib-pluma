#include <pluma/Plugins/PlumaArchiveImporter.hpp>
#include <pluma/PlumaEditor.hpp>
#include <zip.h>
#include <vector>
#include <nlohmann/json.hpp>
#include <iostream>
#include <filesystem>
#include <regex>
#include <fstream>
#include <chrono>

using json = nlohmann::json;

namespace pluma {
namespace plugins {

// Helper to convert JSON back to PropertyValue variant
PropertyValue JsonToPropertyValue(PropertyId id, const json& val) {
    if (val.is_string()) {
        return val.get<std::string>();
    } else if (val.is_boolean()) {
        return val.get<bool>();
    } else if (val.is_number_float()) {
        return val.get<float>();
    } else if (val.is_number_integer()) {
        // Enums or Color or uint16_t
        if (id == PropertyId::FontWeight) return static_cast<uint16_t>(val.get<int>());
        if (id == PropertyId::TextColor || id == PropertyId::BackgroundColor || 
            id == PropertyId::BorderTopColor || id == PropertyId::BorderRightColor || 
            id == PropertyId::BorderBottomColor || id == PropertyId::BorderLeftColor) {
            return static_cast<Color>(val.get<unsigned int>());
        }
        if (id == PropertyId::TextAlignment) return static_cast<TextAlign>(val.get<int>());
        if (id == PropertyId::VerticalAlignment) return static_cast<VerticalAlign>(val.get<int>());
        if (id == PropertyId::Decoration) return static_cast<TextDecoration>(val.get<int>());
        if (id == PropertyId::ImageWrapMode) return static_cast<TextWrapMode>(val.get<int>());
        if (id == PropertyId::DropCapLines) return val.get<int>();
        
        return static_cast<float>(val.get<int>()); // Fallback
    }
    return std::string("");
}

bool PlumaArchiveImporter::importFile(const std::string& filename, PlumaEditor& editor) {
    int errorp = 0;
    zip_t* archive = zip_open(filename.c_str(), ZIP_RDONLY, &errorp);
    if (!archive) {
        return false;
    }

    zip_stat_t sb;
    if (zip_stat(archive, "document.json", 0, &sb) != 0) {
        zip_close(archive);
        return false;
    }

    zip_file_t* zf = zip_fopen(archive, "document.json", 0);
    if (!zf) {
        zip_close(archive);
        return false;
    }

    std::vector<char> buffer(sb.size);
    zip_fread(zf, buffer.data(), sb.size);
    zip_fclose(zf);

    std::string content(buffer.begin(), buffer.end());

    try {
        json doc = json::parse(content);
        
        std::filesystem::path temp_dir = std::filesystem::temp_directory_path() / 
            ("pluma_assets_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()));
        std::filesystem::create_directories(temp_dir);
        
        zip_int64_t num_entries = zip_get_num_entries(archive, 0);
        for (zip_int64_t i = 0; i < num_entries; i++) {
            const char* name = zip_get_name(archive, i, 0);
            if (name && std::string(name).find("assets/") == 0) {
                zip_file_t* asset_zf = zip_fopen_index(archive, i, 0);
                if (asset_zf) {
                    zip_stat_t asset_sb;
                    zip_stat_index(archive, i, 0, &asset_sb);
                    std::vector<char> asset_buf(asset_sb.size);
                    zip_fread(asset_zf, asset_buf.data(), asset_sb.size);
                    zip_fclose(asset_zf);
                    
                    std::filesystem::path asset_path = temp_dir / std::filesystem::path(name).filename();
                    std::ofstream out(asset_path, std::ios::binary);
                    out.write(asset_buf.data(), asset_buf.size());
                    out.close();
                }
            }
        }
        
        auto load_region = [&](const json& region_obj, DocumentRegion region_enum) {
            editor.setActiveRegion(region_enum);
            std::string doc_text = region_obj["text"].get<std::string>();
            
            // Replace assets
            for (zip_int64_t i = 0; i < num_entries; i++) {
                const char* name = zip_get_name(archive, i, 0);
                if (name && std::string(name).find("assets/") == 0) {
                    std::filesystem::path asset_path = temp_dir / std::filesystem::path(name).filename();
                    std::string to_replace = std::string("|IMAGE:") + "\\w+:" + name + "\\|";
                    std::regex img_regex(R"(\|IMAGE:([^:]+):)" + std::string(name) + R"(\|)");
                    doc_text = std::regex_replace(doc_text, img_regex, "|IMAGE:$1:" + asset_path.string() + "|");
                }
            }
            
            editor.loadText(doc_text);
            
            if (region_obj.contains("styles") && region_obj["styles"].is_array()) {
                for (const auto& style_obj : region_obj["styles"]) {
                    uint32_t start = style_obj["start"];
                    uint32_t length = style_obj["length"];
                    PropertyId prop_id = static_cast<PropertyId>(style_obj["propertyId"].get<int>());
                    PropertyValue prop_val = JsonToPropertyValue(prop_id, style_obj["value"]);
                    
                    editor.applyStyle(start, length, prop_id, prop_val);
                }
            }
        };

        // Load Main Document
        if (doc.contains("text")) {
            load_region(doc, DocumentRegion::Body); // For backwards compatibility doc itself is the region
        }
        
        if (doc.contains("header")) {
            load_region(doc["header"], DocumentRegion::Header);
        }
        
        if (doc.contains("footer")) {
            load_region(doc["footer"], DocumentRegion::Footer);
        }

        // Restore active region to body
        editor.setActiveRegion(DocumentRegion::Body);
        
    } catch (...) {
        zip_close(archive);
        return false;
    }
    
    zip_close(archive);
    return true;
}

bool PlumaArchiveImporter::import(const std::string& /* data */, PlumaEditor& /* editor */) {
    return false; // Not supported for binary archives
}

} // namespace plugins
} // namespace pluma
