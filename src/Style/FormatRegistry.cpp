#include <pluma/Style/FormatRegistry.hpp>

namespace pluma {

void FormatRegistry::applyStyle(uint32_t start, uint32_t length, PropertyId id, PropertyValue value) {
    // Try to update an existing exact-match span to avoid unbounded growth (e.g., during mouse drag)
    for (auto it = spans_.rbegin(); it != spans_.rend(); ++it) {
        if (it->start == start && it->length == length) {
            it->style.set(id, value);
            return;
        }
    }

    // Basic implementation: Just append the span.
    // In a real interval tree, this would split and merge spans.
    StyleSpan span;
    span.start = start;
    span.length = length;
    span.style.set(id, value);
    spans_.push_back(std::move(span));
}

PropertyBag FormatRegistry::getStyleAt(uint32_t offset) const {
    PropertyBag computed;
    // Apply in order to simulate cascade (last applied wins)
    for (const auto& span : spans_) {
        if (offset >= span.start && offset < span.start + span.length) {
            computed.merge(span.style);
        }
    }
    return computed;
}

void FormatRegistry::clear() {
    spans_.clear();
}

void FormatRegistry::clearDecorationGlobally(TextDecoration target_dec) {
    for (auto& span : spans_) {
        auto dec = span.style.get(PropertyId::Decoration);
        if (dec && std::get<TextDecoration>(*dec) == target_dec) {
            span.style.remove(PropertyId::Decoration);
        }
    }
}

void FormatRegistry::insertText(uint32_t offset, uint32_t length) {
    for (auto& span : spans_) {
        if (span.start > offset) {
            span.start += length;
        } else if (span.start <= offset && span.start + span.length >= offset) {
            span.length += length;
        }
    }
}

void FormatRegistry::deleteText(uint32_t offset, uint32_t length) {
    uint32_t del_start = offset;
    uint32_t del_end = offset + length;
    
    std::vector<StyleSpan> new_spans;
    for (auto& span : spans_) {
        uint32_t span_end = span.start + span.length;
        
        if (span_end <= del_start) {
            new_spans.push_back(span);
        } else if (span.start >= del_end) {
            span.start -= length;
            new_spans.push_back(span);
        } else {
            uint32_t overlap_start = std::max(span.start, del_start);
            uint32_t overlap_end = std::min(span_end, del_end);
            uint32_t overlap_len = overlap_end - overlap_start;
            
            if (span.length > overlap_len) {
                span.length -= overlap_len;
                if (span.start > del_start) {
                    span.start = del_start;
                }
                new_spans.push_back(span);
            }
        }
    }
    spans_ = std::move(new_spans);
}

} // namespace pluma
