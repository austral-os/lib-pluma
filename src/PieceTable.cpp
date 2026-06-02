#include <pluma/PieceTable.hpp>
#include <stdexcept>

namespace pluma {

// --- DocumentSnapshot ---

DocumentSnapshot::DocumentSnapshot(RevisionId rev, 
                                   std::vector<Piece> pieces, 
                                   std::shared_ptr<const std::string> original_buffer, 
                                   std::shared_ptr<const std::string> add_buffer)
    : rev_(rev), pieces_(std::move(pieces)), original_buffer_(std::move(original_buffer)), add_buffer_(std::move(add_buffer)) {}

std::string DocumentSnapshot::getText() const {
    std::string result;
    result.reserve(getLength());
    for (const auto& piece : pieces_) {
        if (piece.source == BufferSource::Original) {
            result.append(*original_buffer_, piece.start, piece.length);
        } else {
            result.append(*add_buffer_, piece.start, piece.length);
        }
    }
    return result;
}

uint32_t DocumentSnapshot::getLength() const {
    uint32_t len = 0;
    for (const auto& piece : pieces_) {
        len += piece.length;
    }
    return len;
}

// --- PieceTable ---

PieceTable::PieceTable(std::string original_text)
    : original_buffer_(std::make_shared<std::string>(std::move(original_text))),
      add_buffer_(std::make_shared<std::string>()) {
    if (!original_buffer_->empty()) {
        pieces_.push_back({BufferSource::Original, 0, static_cast<uint32_t>(original_buffer_->length())});
    }
}

PieceTable::Position PieceTable::findPosition(uint32_t logical_offset) const {
    if (logical_offset == 0 && pieces_.empty()) {
        return {0, 0};
    }

    uint32_t current_offset = 0;
    for (size_t i = 0; i < pieces_.size(); ++i) {
        if (logical_offset >= current_offset && logical_offset < current_offset + pieces_[i].length) {
            return {i, logical_offset - current_offset};
        }
        current_offset += pieces_[i].length;
    }

    if (logical_offset == current_offset) {
        return {pieces_.size(), 0};
    }

    throw std::out_of_range("Offset out of bounds");
}

void PieceTable::insert(uint32_t offset, std::string_view text) {
    if (text.empty()) return;

    Position pos = findPosition(offset);
    uint32_t add_start = static_cast<uint32_t>(add_buffer_->length());
    add_buffer_->append(text);
    uint32_t add_len = static_cast<uint32_t>(text.length());

    Piece new_piece{BufferSource::Add, add_start, add_len};

    if (pieces_.empty()) {
        pieces_.push_back(new_piece);
        return;
    }

    if (pos.piece_index == pieces_.size()) {
        auto& last_piece = pieces_.back();
        if (last_piece.source == BufferSource::Add && last_piece.start + last_piece.length == add_start) {
            last_piece.length += add_len;
        } else {
            pieces_.push_back(new_piece);
        }
        return;
    }

    Piece target = pieces_[pos.piece_index];
    if (pos.piece_offset == 0) {
        pieces_.insert(pieces_.begin() + pos.piece_index, new_piece);
    } else {
        Piece p1 = {target.source, target.start, pos.piece_offset};
        Piece p2 = {target.source, target.start + pos.piece_offset, target.length - pos.piece_offset};

        pieces_[pos.piece_index] = p1;
        pieces_.insert(pieces_.begin() + pos.piece_index + 1, new_piece);
        pieces_.insert(pieces_.begin() + pos.piece_index + 2, p2);
    }
}

void PieceTable::remove(uint32_t offset, uint32_t length) {
    if (length == 0) return;

    Position start_pos = findPosition(offset);
    Position end_pos = findPosition(offset + length);

    if (start_pos.piece_index == end_pos.piece_index) {
        Piece target = pieces_[start_pos.piece_index];
        Piece p1 = {target.source, target.start, start_pos.piece_offset};
        Piece p2 = {target.source, target.start + end_pos.piece_offset, target.length - end_pos.piece_offset};

        pieces_.erase(pieces_.begin() + start_pos.piece_index);
        
        int insert_idx = start_pos.piece_index;
        if (p2.length > 0) {
            pieces_.insert(pieces_.begin() + insert_idx, p2);
        }
        if (p1.length > 0) {
            pieces_.insert(pieces_.begin() + insert_idx, p1);
        }
        return;
    }

    Piece start_target = pieces_[start_pos.piece_index];
    bool has_end = end_pos.piece_index < pieces_.size();
    Piece end_target = has_end ? pieces_[end_pos.piece_index] : Piece{BufferSource::Original, 0, 0};

    Piece p1 = {start_target.source, start_target.start, start_pos.piece_offset};
    Piece p2 = {end_target.source, end_target.start + end_pos.piece_offset, end_target.length - end_pos.piece_offset};

    size_t erase_end = has_end ? end_pos.piece_index + 1 : pieces_.size();
    pieces_.erase(pieces_.begin() + start_pos.piece_index, pieces_.begin() + erase_end);

    int insert_idx = start_pos.piece_index;
    if (has_end && p2.length > 0) {
        pieces_.insert(pieces_.begin() + insert_idx, p2);
    }
    if (p1.length > 0) {
        pieces_.insert(pieces_.begin() + insert_idx, p1);
    }
}

std::shared_ptr<DocumentSnapshot> PieceTable::createSnapshot() {
    // Para asegurar thread-safety estricta sin locks (evitando que mutaciones de add_buffer
    // afecten a snapshots leídos por otros threads), sellamos el add_buffer actual y creamos uno nuevo.
    auto sealed_add = add_buffer_;
    add_buffer_ = std::make_shared<std::string>(*sealed_add); // copy contents to new buffer so we can keep appending at correct offsets
    
    // El snapshot se queda con la copia sellada (que ya no mutará).
    // Nota: Aunque el nuevo add_buffer_ comienza con el contenido anterior, las futuras
    // inserciones simplemente se apendizarán. Las piezas existentes apuntan correctamente a los
    // offsets. Esto evita que `std::string::append` en otro thread invalide memoria del snapshot.
    
    return std::make_shared<DocumentSnapshot>(
        RevisionId(next_revision_id_++),
        pieces_, // deep copy of pieces
        original_buffer_,
        sealed_add
    );
}

void PieceTable::restoreSnapshot(const std::shared_ptr<DocumentSnapshot>& snapshot) {
    pieces_ = snapshot->getPieces();
    original_buffer_ = snapshot->getOriginalBuffer();
    // Re-create a mutable add_buffer starting from the snapshot's add buffer contents
    add_buffer_ = std::make_shared<std::string>(*snapshot->getAddBuffer());
}

std::string PieceTable::getText() const {
    std::string result;
    result.reserve(getLength());
    for (const auto& piece : pieces_) {
        if (piece.source == BufferSource::Original) {
            result.append(*original_buffer_, piece.start, piece.length);
        } else {
            result.append(*add_buffer_, piece.start, piece.length);
        }
    }
    return result;
}

uint32_t PieceTable::getLength() const {
    uint32_t len = 0;
    for (const auto& piece : pieces_) {
        len += piece.length;
    }
    return len;
}

} // namespace pluma
