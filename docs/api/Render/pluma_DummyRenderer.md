# Class `pluma::DummyRenderer`

**Records drawing calls into an observable log for test assertions.**

## Public Methods
- `void drawRect(const Rect &rect, Color color) override` - *Draws a solid filled rectangle.*
- `void drawGlyphRun(const Rect &rect, const ShapedTextRun &run, const std::string &text, std::shared_ptr< IFont > font, Color color) override` - *Draws a shaped run of text glyphs.*
- `void drawImage(const Rect &rect, const std::string &path) override` - *Draws an image.*
- `void pushClip(const Rect &rect) override` - *Restricts subsequent drawing operations to the given rectangle.*
- `void popClip() override` - *Removes the most recently applied clipping rectangle.*

