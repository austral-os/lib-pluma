# Class `pluma::IRenderer`

**Hardware/Software agnostic rendering interface.**

## Public Methods
- `~IRenderer()=default`
- `void drawRect(const Rect &rect, Color color)=0` - *Draws a solid filled rectangle.*
- `void drawGlyphRun(const Rect &rect, const ShapedTextRun &run, const std::string &text, std::shared_ptr< IFont > font, Color color)=0` - *Draws a shaped run of text glyphs.*
- `void drawImage(const Rect &rect, const std::string &path)=0` - *Draws an image.*
- `void pushClip(const Rect &rect)=0` - *Restricts subsequent drawing operations to the given rectangle.*
- `void popClip()=0` - *Removes the most recently applied clipping rectangle.*

