/**
 * @file PdfExporter.hpp
 * @brief Exports Pluma documents to PDF format using Cairo.
 */
#pragma once
#include <pluma/Plugins/IExporter.hpp>

namespace pluma {
namespace plugins {

/**
 * @class PdfExporter
 * @brief Uses Cairo PDF surfaces to generate a PDF file from the editor's display list.
 */
class PdfExporter : public IExporter {
public:
    std::string exportDoc(std::shared_ptr<DocumentSnapshot> snapshot) override;
    
    bool exportToFile(const std::string& filename, PlumaEditor& editor) override;
};

} // namespace plugins
} // namespace pluma
