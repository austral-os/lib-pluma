#include <pluma/UndoManager.hpp>
#include <stdexcept>

namespace pluma {

UndoManager::UndoManager(PieceTable& table)
    : table_(table) {
}

void UndoManager::beginTransaction() {
    if (current_transaction_) {
        throw std::logic_error("Transaction already in progress");
    }
    current_transaction_ = std::make_unique<Transaction>();
    snapshot_before_transaction_ = table_.createSnapshot();
}

void UndoManager::addCommand(std::unique_ptr<Command> cmd) {
    if (!current_transaction_) {
        throw std::logic_error("No transaction in progress");
    }
    cmd->execute(table_); // Apply immediately to the table
    current_transaction_->addCommand(std::move(cmd));
}

void UndoManager::commitTransaction() {
    if (!current_transaction_) {
        throw std::logic_error("No transaction in progress");
    }

    // If we are not at the end of the history (i.e. we have undone some actions),
    // a new commit truncates the redo history.
    if (current_index_ < history_.size()) {
        history_.erase(history_.begin() + current_index_, history_.end());
    }

    auto after_snapshot = table_.createSnapshot();

    HistoryEntry entry{
        std::move(snapshot_before_transaction_),
        std::move(current_transaction_), // transaction becomes shared via shared_ptr conversion if we used it, but let's just move
        std::move(after_snapshot)
    };

    history_.push_back(std::move(entry));
    current_index_++;

    current_transaction_.reset();
    snapshot_before_transaction_.reset();
}

void UndoManager::rollbackTransaction() {
    if (!current_transaction_) {
        throw std::logic_error("No transaction in progress");
    }
    table_.restoreSnapshot(snapshot_before_transaction_);
    current_transaction_.reset();
    snapshot_before_transaction_.reset();
}

bool UndoManager::canUndo() const {
    return current_index_ > 0;
}

bool UndoManager::canRedo() const {
    return current_index_ < history_.size();
}

void UndoManager::undo() {
    if (!canUndo()) return;

    current_index_--;
    table_.restoreSnapshot(history_[current_index_].before_snapshot);
}

void UndoManager::redo() {
    if (!canRedo()) return;

    table_.restoreSnapshot(history_[current_index_].after_snapshot);
    current_index_++;
}

} // namespace pluma
