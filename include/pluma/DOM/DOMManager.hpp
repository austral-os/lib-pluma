/**
 * @file DOMManager.hpp
 * @brief Defines the DOMManager responsible for syncing text storage with the logical tree.
 */
#pragma once

#include <memory>
#include <pluma/DOM/DOMNode.hpp>
#include <pluma/PieceTable.hpp>

namespace pluma {

/**
 * @class DOMManager
 * @brief Handles creation, synchronization, and updates of the DOM tree based on the PieceTable.
 */
class DOMManager {
public:
    /**
     * @brief Rebuilds the entire logical DOM tree by reading the PieceTable.
     * 
     * Splits text into paragraphs upon encountering '\\n' characters.
     * 
     * @param pieceTable The text storage engine to read from.
     * @return A unique pointer to the generated DocumentNode root.
     */
    std::unique_ptr<DocumentNode> rebuild(const PieceTable& pieceTable);

private:
    uint64_t next_id_ = 1;

    /**
     * @brief Generates a monotonic unique node identifier.
     */
    NodeId generateId() { return NodeId(next_id_++); }
};

} // namespace pluma
