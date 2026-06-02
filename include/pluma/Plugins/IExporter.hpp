/**
 * @file IExporter.hpp
 * @brief Interface for exporting documents to external formats.
 */
#pragma once
#include <string>
#include <memory>
#include <pluma/PieceTable.hpp>
#include <pluma/PlumaEditor.hpp>

namespace pluma {
namespace plugins {

/**
 * @class IExporter
 * @brief Base interface for all format exporters.
 */
class IExporter {
public:
    virtual ~IExporter() = default;

    /**
     * @brief Serializes a document snapshot into an external format string.
     * @param snapshot The immutable document state.
     * @return The serialized string representation.
     */
    virtual std::string exportDoc(std::shared_ptr<DocumentSnapshot> snapshot) = 0;

    /**
     * @brief Exports the document directly to a file (useful for binary formats like PDF).
     * @param filename Path to the output file.
     * @param editor The PlumaEditor instance (provides layout geometry).
     * @return True if successful.
     */
    virtual bool exportToFile(const std::string& filename, PlumaEditor& editor) {
        (void)filename;
        (void)editor;
        return false; // Default implementation
    }
};

} // namespace plugins
} // namespace pluma
