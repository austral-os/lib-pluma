/**
 * @file RevisionId.hpp
 * @brief Defines the RevisionId class for tracking document snapshots.
 */
#pragma once

#include <cstdint>

namespace pluma {

/**
 * @class RevisionId
 * @brief Represents a unique identifier for a document revision/snapshot.
 */
class RevisionId {
public:
    /**
     * @brief Constructs a RevisionId with a specific value.
     * @param id The underlying 64-bit unsigned integer ID.
     */
    explicit RevisionId(uint64_t id);

    /**
     * @brief Retrieves the raw integer value of the RevisionId.
     * @return The 64-bit unsigned integer representing this revision ID.
     */
    uint64_t getValue() const;

    /**
     * @brief Equality operator.
     * @param other The other RevisionId to compare against.
     * @return true if the revisions are identical, false otherwise.
     */
    bool operator==(const RevisionId& other) const;

    /**
     * @brief Inequality operator.
     * @param other The other RevisionId to compare against.
     * @return true if the revisions differ, false otherwise.
     */
    bool operator!=(const RevisionId& other) const;

private:
    uint64_t id_; ///< The raw identifier value.
};

} // namespace pluma
