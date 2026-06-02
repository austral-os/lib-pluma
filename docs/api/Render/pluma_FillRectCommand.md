# Class `pluma::FillRectCommand`

## Public Methods
- `FillRectCommand(const Rect &rect, Color color)`
- `CommandType getType() const override` - *Gets the type of the display command.*
- `Rect getBounds() const override` - *Gets the bounding box affected by this command.*
- `void execute(IRenderer &renderer) const override` - *Executes this command against a generic renderer.*
- `Color getColor() const`

