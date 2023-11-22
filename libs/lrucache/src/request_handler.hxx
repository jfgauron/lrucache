#ifndef LRUCACHE_REQUEST_HANDLER_H_
#define LRUCACHE_REQUEST_HANDLER_H_

#include "raft/raft_manager.hxx"
#include "schema/Request_generated.h"
#include "schema/Response_generated.h"

namespace lrucache {

using response_builder = flatbuffers::FlatBufferBuilder;

class request_handler {
public:
    request_handler(raft_manager* raft);

    void handle_unregistered_request(const schema::Request*,
                                     response_builder* builder);

    void handle_discovery_request(const schema::Request*,
                                  response_builder* builder);

private:
    raft_manager* raft_;
};

} // namespace lrucache

#endif // LRUCACHE_REQUEST_HANDLER_H_