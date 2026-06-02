# Class `pluma::ITextShaper`

**Interface for converting text into shaped glyph runs.**

## Public Methods
- `~ITextShaper()=default`
- `ShapedTextRun shapeText(std::string_view text, const std::shared_ptr< IFont > &font)=0` - *Shapes a logical text string into a renderable glyph run.*

