# Class `pluma::PopClipCommand`

## Public Methods
- `PopClipCommand()=default`
- `CommandType getType() const override` - *Gets the type of the display command.*
- `void execute(IRenderer &renderer) const override` - *Executes this command against a generic renderer.*
- `Rect getBounds() const override` - *Gets the bounding box affected by this command.*

