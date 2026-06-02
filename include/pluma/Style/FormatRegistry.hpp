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
    void applyStyle(uint32_t start, uint32_t length, PropertyId id, PropertyValue value);
    PropertyBag getStyleAt(uint32_t offset) const;
    void clear();

    void insertText(uint32_t offset, uint32_t length);
    void deleteText(uint32_t offset, uint32_t length);

    const std::vector<StyleSpan>& getSpans() const { return spans_; }

private:
    std::vector<StyleSpan> spans_;
};

} // namespace pluma
