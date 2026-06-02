/**
 * @file PropertyBag.hpp
 * @brief Defines the PropertyBag class for storing style layers.
 */
#pragma once

#include <unordered_map>
#include <optional>
#include <pluma/Style/StyleProperties.hpp>

namespace pluma {

/**
 * @class PropertyBag
 * @brief A dictionary storing a specific layer of styles (Declared, Inherited, Computed).
 */
class PropertyBag {
public:
    /**
     * @brief Sets or overwrites a style property.
     * @param id The property identifier.
     * @param value The value variant.
     */
    void set(PropertyId id, PropertyValue value);

    /**
     * @brief Retrieves a style property if it exists.
     * @param id The property identifier.
     * @return std::optional containing the value if present.
     */
    std::optional<PropertyValue> get(PropertyId id) const;

    /**
     * @brief Removes a property from the bag.
     * @param id The property identifier.
     */
    void remove(PropertyId id);

    /**
     * @brief Checks if a property exists in the bag.
     * @param id The property identifier.
     * @return true if present.
     */
    bool has(PropertyId id) const;

    /**
     * @brief Gets all properties in the bag.
     * @return Read-only map of the properties.
     */
    const std::unordered_map<PropertyId, PropertyValue>& getAll() const { return properties_; }

    /**
     * @brief Merges another bag into this one. Values in 'other' override existing values.
     * @param other The overriding property bag.
     */
    void merge(const PropertyBag& other); 

private:
    std::unordered_map<PropertyId, PropertyValue> properties_;
};

} // namespace pluma
