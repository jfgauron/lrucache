#include "lrucache/client.hxx"

#include "endpoint.hxx"
#include "schema/Endpoint_generated.h"

namespace lrucache {

client::client(client_config config) : config_(config)
{
}

bool client::discover()
{
    // TODO: implement method
    return false;
}

} // namespace lrucache
