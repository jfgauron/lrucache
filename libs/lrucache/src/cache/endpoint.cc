#include "endpoint.hxx"

namespace lrucache {

bool endpoint::operator<(const endpoint &node) const
{
    return distance_ < node.distance_;
}

} // namespace lrucache
