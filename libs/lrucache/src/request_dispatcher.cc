#include "request_dispatcher.hxx"

#include <iostream>

namespace lrucache {

request_dispatcher::request_dispatcher(handler_func handler)
    : default_handler_(handler)
{
}

void request_dispatcher::register_handler(schema::RequestContent type,
                                          handler_func handler)
{
    handlers_.insert({type, handler});
}

void request_dispatcher::dispatch(const schema::Request* request,
                                  flatbuffers::FlatBufferBuilder* builder)
{
    auto type = request->content_type();
    auto it = handlers_.find(type);
    if (it == handlers_.end()) {
        default_handler_(request, builder);
        return;
    }
    it->second(request, builder);
    return;
}

} // namespace lrucache
