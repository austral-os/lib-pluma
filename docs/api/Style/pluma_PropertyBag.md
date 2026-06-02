# Class `pluma::PropertyBag`

**A dictionary storing a specific layer of styles (Declared, Inherited, Computed).**

## Public Methods
- `void set(PropertyId id, PropertyValue value)` - *Sets or overwrites a style property.*
- `std::optional< PropertyValue > get(PropertyId id) const` - *Retrieves a style property if it exists.*
- `void remove(PropertyId id)` - *Removes a property from the bag.*
- `bool has(PropertyId id) const` - *Checks if a property exists in the bag.*
- `const std::unordered_map< PropertyId, PropertyValue > & getAll() const` - *Gets all properties in the bag.*
- `void merge(const PropertyBag &other)` - *Merges another bag into this one. Values in 'other' override existing values.*

