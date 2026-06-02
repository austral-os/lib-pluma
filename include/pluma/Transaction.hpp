/**
 * @file Transaction.hpp
 * @brief Defines the Transaction class for grouping document commands.
 */
#pragma once

#include <vector>
#include <memory>
#include <pluma/Command.hpp>
#include <pluma/PieceTable.hpp>

namespace pluma {

/**
 * @class Transaction
 * @brief Groups multiple Command objects into a single atomic execution unit.
 */
class Transaction {
public:
    /**
     * @brief Adds a command to the transaction.
     * @param cmd A unique pointer to the command to add.
     */
    void addCommand(std::unique_ptr<Command> cmd);

    /**
     * @brief Executes all stored commands sequentially on the piece table.
     * @param table The piece table to apply commands to.
     */
    void execute(PieceTable& table);

private:
    std::vector<std::unique_ptr<Command>> commands_; ///< The list of commands in the transaction.
};

} // namespace pluma
