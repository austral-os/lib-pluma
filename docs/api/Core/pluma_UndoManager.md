# Class `pluma::UndoManager`

**Handles history tracking, transaction management, and undo/redo functionality.**

## Public Methods
- `UndoManager(PieceTable &table)` - *Constructs an*
- `void beginTransaction()` - *Begins a new transaction block.*
- `void addCommand(std::unique_ptr< Command > cmd)` - *Executes and adds a command to the current transaction.*
- `void commitTransaction()` - *Commits the current transaction to history.*
- `void rollbackTransaction()` - *Cancels the current transaction and restores the snapshot prior to its start.*
- `bool canUndo() const` - *Checks if an undo operation is currently possible.*
- `bool canRedo() const` - *Checks if a redo operation is currently possible.*
- `void undo()` - *Performs an undo, restoring the document to the previous snapshot.*
- `void redo()` - *Performs a redo, reapplying the previously undone transaction's result.*

