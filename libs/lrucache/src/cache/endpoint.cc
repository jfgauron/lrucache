#include "endpoint.hxx"

namespace lrucache {

bool endpoint::operator<(const endpoint &rhs) const
{
    return distance_ < rhs.distance_;
}

} // namespace lrucache
