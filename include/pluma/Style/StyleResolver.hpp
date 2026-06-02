/**
 * @file StyleResolver.hpp
 * @brief Defines the StyleResolver for calculating final computed styles.
 */
#pragma once

#include <pluma/Style/PropertyBag.hpp>
#include <pluma/Style/FontDescriptor.hpp>

namespace pluma {

/**
 * @class StyleResolver
 * @brief Utility class responsible for merging inherited and declared styles to generate computed styles.
 */
class StyleResolver {
public:
    /**
     * @brief Resolves computed style by merging inherited properties with declared properties.
     * 
     * Declared properties always override inherited ones. Properties that do not inherit by default
     * are discarded from the inherited bag.
     * 
     * @param inherited The bag of properties inherited from parent nodes.
     * @param declared The bag of properties explicitly declared on this node.
     * @return The final computed PropertyBag.
     */
    static PropertyBag resolve(const PropertyBag& inherited, const PropertyBag& declared);
    
    /**
     * @brief Extracts a formal FontDescriptor from a computed bag.
     * 
     * Falls back to default values for properties that are missing from the bag.
     * 
     * @param computed The computed property bag.
     * @return The extracted FontDescriptor.
     */
    static FontDescriptor extractFont(const PropertyBag& computed);
};

} // namespace pluma
