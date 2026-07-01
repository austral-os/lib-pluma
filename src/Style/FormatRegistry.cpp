#include <pluma/Style/FormatRegistry.hpp>
#include <pluma/Diagnostics/Profiler.hpp>
#include <algorithm>
#include <utility>

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
    PLUMA_PROFILE_SCOPE("FormatRegistry::getStyleAt");

    // Linear scan of spans, merging covering styles in cascade order
    PropertyBag computed;
    for (const auto& span : spans_) {
        const uint32_t span_end = span.start + span.length;
        if (span_end >= span.start && offset >= span.start && offset < span_end) {
            computed.merge(span.style);
        }
    }
    return computed;
}

uint32_t FormatRegistry::getStyleRangeEnd(uint32_t offset) const {
    PLUMA_PROFILE_SCOPE("FormatRegistry::getStyleRangeEnd");

    uint32_t next_change = UINT32_MAX;
    for (const auto& span : spans_) {
        const uint32_t span_end = span.start + span.length;
        if (span.start > offset && span.start < next_change) {
            next_change = span.start;
        }
        if (span_end >= span.start && span_end > offset && span_end < next_change) {
            next_change = span_end;
        }
    }
    return next_change;
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
    
    // Clean up empty spans
    spans_.erase(std::remove_if(spans_.begin(), spans_.end(), [](const StyleSpan& span) {
        return span.style.getAll().empty();
    }), spans_.end());
}

void FormatRegistry::insertText(uint32_t offset, uint32_t length) {
    PLUMA_PROFILE_SCOPE("FormatRegistry::insertText");
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
    spans_.clear();
    for (auto& span : new_spans) {
        if (span.length > 0 && !span.style.getAll().empty()) {
            spans_.push_back(std::move(span));
        }
    }
}

void FormatRegistry::setSpans(const std::vector<StyleSpan>& spans) {
    spans_ = spans;
}

} // namespace pluma
