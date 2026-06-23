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
    void clearDecorationGlobally(TextDecoration target_dec);

    void insertText(uint32_t offset, uint32_t length);
    void deleteText(uint32_t offset, uint32_t length);

    const std::vector<StyleSpan>& getSpans() const { return spans_; }
    void setSpans(const std::vector<StyleSpan>& spans) { spans_ = spans; }

private:
    std::vector<StyleSpan> spans_;
};

} // namespace pluma
