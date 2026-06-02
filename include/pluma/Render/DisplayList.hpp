/**
 * @file DisplayList.hpp
 * @brief Defines the DisplayList class for decoupled rendering.
 */
#pragma once

#include <memory>
#include <vector>
#include <pluma/Render/DisplayCommand.hpp>

namespace pluma {

/**
 * @class DisplayList
 * @brief An immutable or append-only sequence of rendering commands.
 */
class DisplayList {
public:
    /**
     * @brief Appends a rendering command to the list.
     * @param cmd Unique pointer to the command.
     */
    void addCommand(std::unique_ptr<DisplayCommand> cmd);

    /**
     * @brief Retrieves all commands.
     * @return A vector of unique pointers to DisplayCommands.
     */
    const std::vector<std::unique_ptr<DisplayCommand>>& getCommands() const { return commands_; }

    /**
     * @brief Retrieves only the commands that intersect with the given viewport.
     * This is the core of Viewport Culling logic.
     * @param viewport The visible area.
     * @return A vector of pointers to the visible commands.
     */
    std::vector<const DisplayCommand*> getCommandsInRect(const Rect& viewport) const;

private:
    std::vector<std::unique_ptr<DisplayCommand>> commands_;
};

} // namespace pluma
