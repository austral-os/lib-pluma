/**
 * @file FontDescriptor.hpp
 * @brief Defines the typography descriptors.
 */
#pragma once

#include <string>
#include <cstdint>

namespace pluma {

/**
 * @enum FontWeight
 * @brief Standard typography font weights.
 */
enum class FontWeight : uint16_t {
    Thin = 100,
    Light = 300,
    Regular = 400,
    Medium = 500,
    Bold = 700,
    Black = 900
};

/**
 * @struct FontDescriptor
 * @brief Contains exact specifications to resolve and shape a font.
 */
struct FontDescriptor {
    std::string family = "sans-serif"; ///< The font family name.
    float size_pt = 12.0f;             ///< Font size in points.
    FontWeight weight = FontWeight::Regular; ///< Font weight.
    bool italic = false;               ///< Italic flag.
    
    bool operator==(const FontDescriptor& other) const {
        return family == other.family &&
               size_pt == other.size_pt &&
               weight == other.weight &&
               italic == other.italic;
    }

    bool operator!=(const FontDescriptor& other) const {
        return !(*this == other);
    }
};

} // namespace pluma
