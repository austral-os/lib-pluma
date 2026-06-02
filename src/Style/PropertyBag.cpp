#include <pluma/Style/PropertyBag.hpp>

namespace pluma {

void PropertyBag::set(PropertyId id, PropertyValue value) {
    properties_[id] = std::move(value);
}

std::optional<PropertyValue> PropertyBag::get(PropertyId id) const {
    auto it = properties_.find(id);
    if (it != properties_.end()) {
        return it->second;
    }
    return std::nullopt;
}

void PropertyBag::remove(PropertyId id) {
    properties_.erase(id);
}

bool PropertyBag::has(PropertyId id) const {
    return properties_.find(id) != properties_.end();
}

void PropertyBag::merge(const PropertyBag& other) {
    for (const auto& [id, val] : other.properties_) {
        properties_[id] = val; // Copy value
    }
}

} // namespace pluma
