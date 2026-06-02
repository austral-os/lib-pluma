/**
 * @file Command.hpp
 * @brief Defines the Command interface and concrete text editing commands.
 */
#pragma once

#include <cstdint>
#include <string>
#include <pluma/PieceTable.hpp>

namespace pluma {

/**
 * @class Command
 * @brief Base interface for all document mutative commands.
 */
class Command {
public:
    virtual ~Command() = default;

    /**
     * @brief Executes the command against a piece table.
     * @param table The piece table to modify.
     */
    virtual void execute(PieceTable& table) = 0;
};

/**
 * @class InsertTextCommand
 * @brief A command that inserts text into the document.
 */
class InsertTextCommand : public Command {
public:
    /**
     * @brief Constructs an InsertTextCommand.
     * @param offset The logical offset to insert at.
     * @param text The text to insert.
     */
    InsertTextCommand(uint32_t offset, std::string text);
    
    void execute(PieceTable& table) override;

private:
    uint32_t offset_;
    std::string text_;
};

/**
 * @class DeleteTextCommand
 * @brief A command that removes text from the document.
 */
class DeleteTextCommand : public Command {
public:
    /**
     * @brief Constructs a DeleteTextCommand.
     * @param offset The logical offset to begin deletion.
     * @param length The amount of text to delete.
     */
    DeleteTextCommand(uint32_t offset, uint32_t length);
    
    void execute(PieceTable& table) override;

private:
    uint32_t offset_;
    uint32_t length_;
};

} // namespace pluma
