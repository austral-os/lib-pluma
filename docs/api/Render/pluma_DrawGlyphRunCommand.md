# Class `pluma::DrawGlyphRunCommand`

## Public Methods
- `DrawGlyphRunCommand(const Rect &rect, ShapedTextRun run, std::string text, std::shared_ptr< IFont > font, Color color)`
- `CommandType getType() const override` - *Gets the type of the display command.*
- `Rect getBounds() const override` - *Gets the bounding box affected by this command.*
- `void execute(IRenderer &renderer) const override` - *Executes this command against a generic renderer.*
- `const ShapedTextRun & getRun() const`
- `const std::string & getText() const`
- `std::shared_ptr< IFont > getFont() const`
- `Color getColor() const`

