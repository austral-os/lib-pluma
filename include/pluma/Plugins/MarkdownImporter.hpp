/**
 * @file MarkdownImporter.hpp
 * @brief Imports Markdown text into Pluma.
 */
#pragma once
#include <pluma/Plugins/IImporter.hpp>

namespace pluma {
namespace plugins {

/**
 * @class MarkdownImporter
 * @brief Basic parser that converts markdown into Pluma text structure.
 */
class MarkdownImporter : public IImporter {
public:
    bool import(const std::string& data, PlumaEditor& editor) override;
};

} // namespace plugins
} // namespace pluma
