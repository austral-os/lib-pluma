#include <pluma/Command.hpp>

namespace pluma {

InsertTextCommand::InsertTextCommand(uint32_t offset, std::string text)
    : offset_(offset), text_(std::move(text)) {}

void InsertTextCommand::execute(PieceTable& table) {
    table.insert(offset_, text_);
}

DeleteTextCommand::DeleteTextCommand(uint32_t offset, uint32_t length)
    : offset_(offset), length_(length) {}

void DeleteTextCommand::execute(PieceTable& table) {
    table.remove(offset_, length_);
}

} // namespace pluma
