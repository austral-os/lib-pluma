/**
 * @file IImporter.hpp
 * @brief Interface for importing documents from external formats.
 */
#pragma once
#include <string>

namespace pluma {

class PlumaEditor;

namespace plugins {

/**
 * @class IImporter
 * @brief Base interface for all format importers.
 */
class IImporter {
public:
    virtual ~IImporter() = default;

    /**
     * @brief Parses an external format and loads it into the editor.
     * @param data The raw data to parse.
     * @param editor The PlumaEditor instance to populate.
     * @return True if successful.
     */
    virtual bool import(const std::string& data, PlumaEditor& editor) = 0;
};

} // namespace plugins
} // namespace pluma
