#include "raft_endpoint.hxx"

namespace lrucache {

bool raft_endpoint::operator<(const raft_endpoint &rhs) const
{
    return distance_ < rhs.distance_;
}

} // namespace lrucache
