/**
 * @file PlumaArchiveImporter.hpp
 * @brief Imports native .pluma zip archives.
 */
#pragma once
#include <pluma/Plugins/IImporter.hpp>

namespace pluma {
namespace plugins {

/**
 * @class PlumaArchiveImporter
 * @brief Extracts document text and assets from a .pluma ZIP archive.
 */
class PlumaArchiveImporter : public IImporter {
public:
    /**
     * @brief Reads a .pluma archive and loads it into the editor.
     * @param filename Path to the .pluma file.
     * @param editor The target editor.
     * @return True if successful.
     */
    bool importFile(const std::string& filename, PlumaEditor& editor);

    /**
     * @brief Not supported for archives, returns false.
     */
    bool import(const std::string& data, PlumaEditor& editor) override;
};

} // namespace plugins
} // namespace pluma
