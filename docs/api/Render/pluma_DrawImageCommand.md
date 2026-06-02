# Class `pluma::DrawImageCommand`

## Public Methods
- `DrawImageCommand(const Rect &rect, std::string path)`
- `CommandType getType() const override` - *Gets the type of the display command.*
- `Rect getBounds() const override` - *Gets the bounding box affected by this command.*
- `void execute(IRenderer &renderer) const override` - *Executes this command against a generic renderer.*
- `const std::string & getPath() const`

