/**
 * @file IFontManager.hpp
 * @brief Interface for managing fonts, abstracting Fontconfig/FreeType.
 */
#pragma once

#include <string>
#include <memory>
#include <pluma/Style/FontDescriptor.hpp>

namespace pluma {

/**
 * @class IFont
 * @brief Represents a resolved and loaded font instance.
 */
class IFont {
public:
    virtual ~IFont() = default;
    
    /**
     * @brief Gets the descriptor associated with this font.
     * @return The FontDescriptor.
     */
    virtual FontDescriptor getDescriptor() const = 0;
};

/**
 * @class IFontManager
 * @brief Interface for querying and caching system fonts.
 */
class IFontManager {
public:
    virtual ~IFontManager() = default;

    /**
     * @brief Resolves a font descriptor to an actual loaded font.
     * @param desc The requested font specifications.
     * @return A shared pointer to the resolved font.
     */
    virtual std::shared_ptr<IFont> getFont(const FontDescriptor& desc) = 0;
};

} // namespace pluma
