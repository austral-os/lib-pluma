/**
 * @file MarkdownExporter.hpp
 * @brief Exports Pluma documents to Markdown format.
 */
#pragma once
#include <pluma/Plugins/IExporter.hpp>

namespace pluma {
namespace plugins {

/**
 * @class MarkdownExporter
 * @brief Generates markdown strings from a Pluma document snapshot.
 */
class MarkdownExporter : public IExporter {
public:
    std::string exportDoc(std::shared_ptr<DocumentSnapshot> snapshot) override;
};

} // namespace plugins
} // namespace pluma
