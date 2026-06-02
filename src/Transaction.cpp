#include <pluma/Transaction.hpp>

namespace pluma {

void Transaction::addCommand(std::unique_ptr<Command> cmd) {
    commands_.push_back(std::move(cmd));
}

void Transaction::execute(PieceTable& table) {
    for (auto& cmd : commands_) {
        cmd->execute(table);
    }
}

} // namespace pluma
