#pragma once
#include <vector>
#include <cstdint>
#include <pluma/Style/StyleProperties.hpp>
#include <pluma/Style/PropertyBag.hpp>

namespace pluma {

struct StyleSpan {
    uint32_t start;
    uint32_t length;
    PropertyBag style;
};

class FormatRegistry {
public:
    FormatRegistry() = default;
    FormatRegistry(const FormatRegistry&) = default;
    FormatRegistry& operator=(const FormatRegistry&) = default;
    FormatRegistry(FormatRegistry&&) noexcept = default;
    FormatRegistry& operator=(FormatRegistry&&) noexcept = default;
    ~FormatRegistry() = default;

    void applyStyle(uint32_t start, uint32_t length, PropertyId id, PropertyValue value);
    PropertyBag getStyleAt(uint32_t offset) const;
    void clear();
    void clearDecorationGlobally(TextDecoration target_dec);

    void insertText(uint32_t offset, uint32_t length);
    void deleteText(uint32_t offset, uint32_t length);

    /// Returns the next offset (> offset) where the computed style may change.
    /// Returns UINT32_MAX if no style change occurs after offset.
    /// Useful for layout passes that walk offsets monotonically:
    /// within [offset, result) the computed style is guaranteed constant.
    uint32_t getStyleRangeEnd(uint32_t offset) const;

    const std::vector<StyleSpan>& getSpans() const { return spans_; }
    void setSpans(const std::vector<StyleSpan>& spans);

private:
    std::vector<StyleSpan> spans_;
};

} // namespace pluma
