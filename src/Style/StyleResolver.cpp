#include <pluma/Style/StyleResolver.hpp>

namespace pluma {

PropertyBag StyleResolver::resolve(const PropertyBag& inherited, const PropertyBag& declared) {
    PropertyBag computed;
    
    // 1. First, apply inherited properties (only those that inherit by default)
    for (const auto& [id, val] : inherited.getAll()) {
        if (inheritsByDefault(id)) {
            computed.set(id, val);
        }
    }
    
    // 2. Overwrite with declared properties
    computed.merge(declared);
    
    return computed;
}

FontDescriptor StyleResolver::extractFont(const PropertyBag& computed) {
    FontDescriptor font;
    
    if (auto val = computed.get(PropertyId::FontFamily)) {
        if (auto* str = std::get_if<std::string>(&*val)) {
            font.family = *str;
        }
    }
    
    if (auto val = computed.get(PropertyId::FontSize)) {
        if (auto* size = std::get_if<float>(&*val)) {
            font.size_pt = *size;
        }
    }
    
    if (auto val = computed.get(PropertyId::FontWeight)) {
        if (auto* w = std::get_if<uint16_t>(&*val)) {
            font.weight = static_cast<FontWeight>(*w);
        }
    }
    
    if (auto val = computed.get(PropertyId::FontStyleItalic)) {
        if (auto* italic = std::get_if<bool>(&*val)) {
            font.italic = *italic;
        }
    }
    
    return font;
}

} // namespace pluma
