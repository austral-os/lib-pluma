/**
 * @file NodeId.hpp
 * @brief Defines the NodeId class for uniquely identifying DOM nodes.
 */
#pragma once

#include <cstdint>

namespace pluma {

/**
 * @class NodeId
 * @brief Represents a unique identifier for a node in the document object model.
 */
class NodeId {
public:
    /**
     * @brief Constructs a NodeId with a specific value.
     * @param id The underlying 64-bit unsigned integer ID.
     */
    explicit NodeId(uint64_t id);

    /**
     * @brief Retrieves the raw integer value of the NodeId.
     * @return The 64-bit unsigned integer representing this ID.
     */
    uint64_t getValue() const;

    /**
     * @brief Equality operator.
     * @param other The other NodeId to compare against.
     * @return true if the IDs are identical, false otherwise.
     */
    bool operator==(const NodeId& other) const;

    /**
     * @brief Inequality operator.
     * @param other The other NodeId to compare against.
     * @return true if the IDs differ, false otherwise.
     */
    bool operator!=(const NodeId& other) const;

private:
    uint64_t id_; ///< The raw identifier value.
};

} // namespace pluma
