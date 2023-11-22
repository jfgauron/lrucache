#ifndef LRUCACHE_REQUEST_DISPATCHER_H_
#define LRUCACHE_REQUEST_DISPATCHER_H_

#include <unordered_map>

#include "flatbuffers/flatbuffers.h"

#include "schema/Request_generated.h"
#include "schema/Response_generated.h"

namespace lrucache {

using handler_func = std::function<void(const schema::Request*,
                                        flatbuffers::FlatBufferBuilder*)>;

class request_dispatcher {
public:
    request_dispatcher(handler_func handler);
    void register_handler(schema::RequestContent type, handler_func);
    void dispatch(const schema::Request* request,
                  flatbuffers::FlatBufferBuilder* response_builder);
private:
    std::unordered_map<schema::RequestContent, handler_func> handlers_;

    handler_func default_handler_;
};

} // namespace lrucache

#endif // LRUCACHE_REQUEST_DISPATCHER_H_