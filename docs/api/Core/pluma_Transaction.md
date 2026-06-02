# Class `pluma::Transaction`

**Groups multiple**

## Public Methods
- `void addCommand(std::unique_ptr< Command > cmd)` - *Adds a command to the transaction.*
- `void execute(PieceTable &table)` - *Executes all stored commands sequentially on the piece table.*

