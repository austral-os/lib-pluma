/**
 * @file PlumaArchiveExporter.hpp
 * @brief Exports Pluma documents to the native .pluma zip format.
 */
#pragma once
#include <pluma/Plugins/IExporter.hpp>

namespace pluma {
namespace plugins {

/**
 * @class PlumaArchiveExporter
 * @brief Packages document text and metadata into a .pluma ZIP archive.
 */
class PlumaArchiveExporter : public IExporter {
public:
    std::string exportDoc(std::shared_ptr<DocumentSnapshot> snapshot) override;
    
    bool exportToFile(const std::string& filename, PlumaEditor& editor) override;
};

} // namespace plugins
} // namespace pluma
