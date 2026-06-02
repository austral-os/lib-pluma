# Class `pluma::DisplayCommand`

**Abstract base class for drawing operations in the display list.**

## Public Methods
- `~DisplayCommand()=default`
- `CommandType getType() const =0` - *Gets the type of the display command.*
- `Rect getBounds() const =0` - *Gets the bounding box affected by this command.*
- `void execute(IRenderer &renderer) const =0` - *Executes this command against a generic renderer.*

