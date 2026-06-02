# Class `pluma::DisplayList`

**An immutable or append-only sequence of rendering commands.**

## Public Methods
- `void addCommand(std::unique_ptr< DisplayCommand > cmd)` - *Appends a rendering command to the list.*
- `const std::vector< std::unique_ptr< DisplayCommand > > & getCommands() const` - *Retrieves all commands.*
- `std::vector< const DisplayCommand * > getCommandsInRect(const Rect &viewport) const` - *Retrieves only the commands that intersect with the given viewport. This is the core of Viewport Culling logic.*

