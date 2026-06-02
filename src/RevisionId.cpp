#include <pluma/RevisionId.hpp>

namespace pluma {

RevisionId::RevisionId(uint64_t id) : id_(id) {}

uint64_t RevisionId::getValue() const {
    return id_;
}

bool RevisionId::operator==(const RevisionId& other) const {
    return id_ == other.id_;
}

bool RevisionId::operator!=(const RevisionId& other) const {
    return !(*this == other);
}

} // namespace pluma
