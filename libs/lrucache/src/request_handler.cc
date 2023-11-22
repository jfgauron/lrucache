#include "request_handler.hxx"

namespace lrucache {

request_handler::request_handler(raft_manager* raft)
    : raft_(raft)
{
}

void request_handler::handle_unregistered_request(
        const schema::Request* request, response_builder* builder)
{
}

void request_handler::handle_discovery_request(
        const schema::Request* request, response_builder* builder)
{
    auto request_content = request->content_as_DiscoveryRequest();

    int leader_id = raft_->instance()->get_leader();
    std::vector<nuraft::ptr<nuraft::srv_config>> configs;
    raft_->instance()->get_srv_config_all(configs);

    // build response
    std::vector<flatbuffers::Offset<schema::Endpoint>> endpoints_vecotr;

    for (auto& entry: configs) {
        auto host = builder->CreateString("TODO"); // TODO
        int port = 0; // TODO
        bool is_leader = entry->get_id() == leader_id;
        double latitude = 0.0; // TODO
        double longitude = 0.0; // TODO
        auto geolocation = schema::CreateGeolocation(*builder, latitude, longitude);
        schema::CreateEndpoint(*builder, host, port, is_leader, geolocation);
    }

    auto endpoints = builder->CreateVector(endpoints_vecotr);
    auto content = schema::CreateDiscoveryResponse(*builder, endpoints);
    auto response = schema::CreateResponse(
        *builder,
        schema::ResponseContent_DiscoveryResponse,
        content.Union());
    builder->Finish(response);
}

} // namespace lrucache
