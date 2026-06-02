# Class `pluma::LayoutBox`

**Base class for all elements in the layout tree.**

## Public Methods
- `~LayoutBox()=default`
- `BoxType getType() const =0` - *Gets the exact type of this box.*
- `Rect getBounds() const` - *Retrieves the bounding geometry of this box relative to its parent.*
- `void setBounds(const Rect &bounds)` - *Sets the bounding geometry of this box.*

