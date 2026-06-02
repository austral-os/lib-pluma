/**
 * @file PieceTable.hpp
 * @brief Defines the PieceTable and DocumentSnapshot for the text storage engine.
 */
#pragma once

#include <string>
#include <vector>
#include <string_view>
#include <cstdint>
#include <memory>
#include <pluma/TextSlice.hpp>
#include <pluma/RevisionId.hpp>

namespace pluma {

/**
 * @enum BufferSource
 * @brief Identifies whether a piece originates from the original or the add buffer.
 */
enum class BufferSource {
    Original, ///< Belongs to the original unedited text.
    Add       ///< Belongs to the append-only add buffer.
};

/**
 * @struct Piece
 * @brief Represents a continuous span of text within a buffer.
 */
struct Piece {
    BufferSource source; ///< The source buffer.
    uint32_t start;      ///< The starting offset in the source buffer.
    uint32_t length;     ///< The length of the span.
};

/**
 * @class DocumentSnapshot
 * @brief Represents an immutable snapshot of the document's state at a specific revision.
 */
class DocumentSnapshot {
public:
    /**
     * @brief Constructs a new DocumentSnapshot.
     * @param rev The RevisionId of this snapshot.
     * @param pieces A deep copy of the pieces array.
     * @param original_buffer Shared pointer to the immutable original buffer.
     * @param add_buffer Shared pointer to the sealed add buffer.
     */
    DocumentSnapshot(RevisionId rev, 
                     std::vector<Piece> pieces, 
                     std::shared_ptr<const std::string> original_buffer, 
                     std::shared_ptr<const std::string> add_buffer);

    /**
     * @brief Gets the revision ID of the snapshot.
     * @return The RevisionId.
     */
    RevisionId getRevision() const { return rev_; }

    /**
     * @brief Generates the full text of the snapshot.
     * @return The complete text string.
     */
    std::string getText() const;

    /**
     * @brief Gets the total length of the text in the snapshot.
     * @return The length in characters/bytes.
     */
    uint32_t getLength() const;

    /**
     * @brief Gets the internal piece descriptors.
     * @return A reference to the vector of Pieces.
     */
    const std::vector<Piece>& getPieces() const { return pieces_; }

    std::shared_ptr<const std::string> getOriginalBuffer() const { return original_buffer_; }
    std::shared_ptr<const std::string> getAddBuffer() const { return add_buffer_; }

private:
    RevisionId rev_;
    std::vector<Piece> pieces_;
    std::shared_ptr<const std::string> original_buffer_;
    std::shared_ptr<const std::string> add_buffer_;
};

/**
 * @class PieceTable
 * @brief The core text storage engine utilizing the piece table data structure.
 */
class PieceTable {
public:
    /**
     * @brief Constructs a PieceTable with optional initial text.
     * @param original_text The starting text of the document.
     */
    explicit PieceTable(std::string original_text = "");

    /**
     * @brief Inserts text at the specified logical offset.
     * @param offset The logical offset to insert at.
     * @param text The text to insert.
     */
    void insert(uint32_t offset, std::string_view text);

    /**
     * @brief Removes a length of text starting at a logical offset.
     * @param offset The logical offset to start removing.
     * @param length The number of characters/bytes to remove.
     */
    void remove(uint32_t offset, uint32_t length);

    /**
     * @brief Retrieves the full current text.
     * @return The concatenated text string.
     */
    std::string getText() const;

    /**
     * @brief Retrieves the total length of the document.
     * @return The document length.
     */
    uint32_t getLength() const;

    /**
     * @brief Creates a structural sharing snapshot of the current state.
     * @return A shared pointer to the new DocumentSnapshot.
     */
    std::shared_ptr<DocumentSnapshot> createSnapshot();

    /**
     * @brief Restores the piece table to a previously taken snapshot.
     * @param snapshot The snapshot to restore.
     */
    void restoreSnapshot(const std::shared_ptr<DocumentSnapshot>& snapshot);

    /**
     * @brief Gets the raw internal piece array.
     * @return Reference to the pieces vector.
     */
    const std::vector<Piece>& getPieces() const { return pieces_; }

private:
    std::shared_ptr<const std::string> original_buffer_;
    std::shared_ptr<std::string> add_buffer_;
    std::vector<Piece> pieces_;
    uint64_t next_revision_id_ = 1;

    struct Position {
        size_t piece_index;
        uint32_t piece_offset;
    };
    Position findPosition(uint32_t logical_offset) const;
};

} // namespace pluma
