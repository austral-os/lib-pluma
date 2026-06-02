/**
 * @file CoreTypes.hpp
 * @brief Defines core coordinate types such as Twips, Point, and Rect.
 */
#pragma once

#include <cstdint>

namespace pluma {

/**
 * @class Twips
 * @brief Represents a measurement in twips (1/20 of a point, 1/1440 of an inch).
 * 
 * Using twips avoids floating point inaccuracies when measuring layout geometries.
 */
class Twips {
public:
    /**
     * @brief Constructs a Twips instance with a value of 0.
     */
    constexpr Twips() : value_(0) {}

    /**
     * @brief Constructs a Twips instance with a specific integer value.
     * @param value The value in twips.
     */
    constexpr explicit Twips(int32_t value) : value_(value) {}

    /**
     * @brief Gets the raw integer value.
     * @return The measurement in twips.
     */
    constexpr int32_t getValue() const { return value_; }

    constexpr bool operator==(const Twips& other) const { return value_ == other.value_; }
    constexpr bool operator!=(const Twips& other) const { return !(*this == other); }
    constexpr Twips operator+(const Twips& other) const { return Twips(value_ + other.value_); }
    constexpr Twips operator-(const Twips& other) const { return Twips(value_ - other.value_); }

private:
    int32_t value_; ///< The stored measurement in twips.
};

/**
 * @struct Point
 * @brief Represents a 2D point using Twips coordinates.
 */
struct Point {
    Twips x; ///< The X coordinate.
    Twips y; ///< The Y coordinate.
};

/**
 * @struct Size
 * @brief Represents 2D dimensions using Twips.
 */
struct Size {
    Twips width;
    Twips height;
};

/**
 * @struct Rect
 * @brief Represents a 2D rectangle using Twips coordinates.
 */
struct Rect {
    Twips x;      ///< The X coordinate of the top-left corner.
    Twips y;      ///< The Y coordinate of the top-left corner.
    Twips width;  ///< The width of the rectangle.
    Twips height; ///< The height of the rectangle.

    /**
     * @brief Checks if this rectangle overlaps with another.
     * @param other The rectangle to check against.
     * @return true if there is an intersection.
     */
    constexpr bool intersects(const Rect& other) const {
        return x.getValue() < (other.x + other.width).getValue() &&
               (x + width).getValue() > other.x.getValue() &&
               y.getValue() < (other.y + other.height).getValue() &&
               (y + height).getValue() > other.y.getValue();
    }
};

} // namespace pluma
