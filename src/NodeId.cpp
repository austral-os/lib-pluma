#include <pluma/NodeId.hpp>

namespace pluma {

NodeId::NodeId(uint64_t id) : id_(id) {}

uint64_t NodeId::getValue() const {
    return id_;
}

bool NodeId::operator==(const NodeId& other) const {
    return id_ == other.id_;
}

bool NodeId::operator!=(const NodeId& other) const {
    return !(*this == other);
}

} // namespace pluma
