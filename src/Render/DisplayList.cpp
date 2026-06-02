#include <pluma/Render/DisplayList.hpp>

namespace pluma {

void DisplayList::addCommand(std::unique_ptr<DisplayCommand> cmd) {
    if (cmd) {
        commands_.push_back(std::move(cmd));
    }
}

std::vector<const DisplayCommand*> DisplayList::getCommandsInRect(const Rect& viewport) const {
    std::vector<const DisplayCommand*> culled;
    culled.reserve(commands_.size()); // Optimistic allocation

    for (const auto& cmd : commands_) {
        // PopClip applies globally to the state machine, we can't cull it arbitrarily.
        // In a true engine, we would track clip depth or ensure matching pushes/pops,
        // but checking intersection is standard for drawing ops.
        if (cmd->getType() == CommandType::PopClip || cmd->getBounds().intersects(viewport)) {
            culled.push_back(cmd.get());
        }
    }

    return culled;
}

} // namespace pluma
