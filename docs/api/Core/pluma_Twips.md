# Class `pluma::Twips`

**Represents a measurement in twips (1/20 of a point, 1/1440 of an inch).**

Using twips avoids floating point inaccuracies when measuring layout geometries.

## Public Methods
- `constexpr Twips()` - *Constructs a*
- `constexpr Twips(int32_t value)` - *Constructs a*
- `constexpr int32_t getValue() const` - *Gets the raw integer value.*
- `constexpr bool operator==(const Twips &other) const`
- `constexpr bool operator!=(const Twips &other) const`
- `constexpr Twips operator+(const Twips &other) const`
- `constexpr Twips operator-(const Twips &other) const`

