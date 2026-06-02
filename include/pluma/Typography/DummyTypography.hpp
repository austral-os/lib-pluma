/**
 * @file DummyTypography.hpp
 * @brief Dummy implementation of the typography pipeline for testing purposes.
 */
#pragma once

#include <pluma/Typography/IFontManager.hpp>
#include <pluma/Typography/ITextShaper.hpp>

namespace pluma {

/**
 * @class DummyFont
 * @brief A mock font that just holds a descriptor.
 */
class DummyFont : public IFont {
public:
    explicit DummyFont(FontDescriptor desc);
    FontDescriptor getDescriptor() const override;

private:
    FontDescriptor desc_;
};

/**
 * @class DummyFontManager
 * @brief A mock font manager that simply returns DummyFonts.
 */
class DummyFontManager : public IFontManager {
public:
    std::shared_ptr<IFont> getFont(const FontDescriptor& desc) override;
};

/**
 * @class DummyTextShaper
 * @brief A mock text shaper that assigns a fixed width to every byte for testing.
 */
class DummyTextShaper : public ITextShaper {
public:
    ShapedTextRun shapeText(std::string_view text, const std::shared_ptr<IFont>& font) override;
};

} // namespace pluma
