/**
 * @file UndoManager.hpp
 * @brief Defines the UndoManager for managing document history via snapshots.
 */
#pragma once

#include <vector>
#include <memory>
#include <pluma/Transaction.hpp>
#include <pluma/PieceTable.hpp>

namespace pluma {

/**
 * @class UndoManager
 * @brief Handles history tracking, transaction management, and undo/redo functionality.
 */
class UndoManager {
public:
    /**
     * @brief Constructs an UndoManager linked to a specific PieceTable.
     * @param table The piece table to manage.
     */
    explicit UndoManager(PieceTable& table);

    /**
     * @brief Begins a new transaction block.
     * @throws std::logic_error if a transaction is already in progress.
     */
    void beginTransaction();

    /**
     * @brief Executes and adds a command to the current transaction.
     * @param cmd The command to execute and store.
     * @throws std::logic_error if no transaction is active.
     */
    void addCommand(std::unique_ptr<Command> cmd);

    /**
     * @brief Commits the current transaction to history.
     */
    void commitTransaction();

    /**
     * @brief Cancels the current transaction and restores the snapshot prior to its start.
     */
    void rollbackTransaction();

    /**
     * @brief Checks if an undo operation is currently possible.
     * @return true if there is history to undo.
     */
    bool canUndo() const;

    /**
     * @brief Checks if a redo operation is currently possible.
     * @return true if there are truncated operations to redo.
     */
    bool canRedo() const;

    /**
     * @brief Performs an undo, restoring the document to the previous snapshot.
     */
    void undo();

    /**
     * @brief Performs a redo, reapplying the previously undone transaction's result.
     */
    void redo();

private:
    PieceTable& table_;

    struct HistoryEntry {
        std::shared_ptr<DocumentSnapshot> before_snapshot;
        std::shared_ptr<Transaction> transaction;
        std::shared_ptr<DocumentSnapshot> after_snapshot;
    };

    std::vector<HistoryEntry> history_;
    size_t current_index_ = 0; 

    std::unique_ptr<Transaction> current_transaction_;
    std::shared_ptr<DocumentSnapshot> snapshot_before_transaction_;
};

} // namespace pluma
