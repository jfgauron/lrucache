#include "network_manager.hxx"

#include <iostream>

namespace lrucache {

constexpr int CHUNK_SIZE = 1024;

network_manager::network_manager(int server_id,
                                 std::string host,
                                 unsigned short port,
                                 request_dispatcher* dispatcher)
    : server_id_(server_id)
    , host_(host)
    , port_(port)
    , dispatcher_(dispatcher)
    , thread_pool_(std::thread::hardware_concurrency())
{
    sockpp::initialize();
}

void lrucache::network_manager::listen()
{
    sockpp::tcp_acceptor acc({host_, port_});

    if (!acc) {
        std::cerr << "error creating the acceptor: "
            << acc.last_error_str() << std::endl;
        return;
    }

    std::cout << "    -- Replicated LRUCache with Raft --"  << std::endl;
    std::cout << "                         Version 0.1.0"   << std::endl;
    std::cout << "    Server ID:    " << server_id_         << std::endl;
    std::cout << "    Endpoint:     " << endpoint()         << std::endl;

    while (true) {
        // accept a new client connection
        sockpp::tcp_socket socket = acc.accept();

        if (!socket) {
            std::cerr << "Error accepting incoming connection: "
                << acc.last_error_str() << std::endl;
            continue;
        }

        auto handler = [this, s = std::move(socket)](int id) mutable  {
            this->handle_connection(id, std::move(s));
        };
        thread_pool_.push(std::move(handler));
    }
}

void network_manager::handle_connection(int id, sockpp::tcp_socket&& socket)
{
	ssize_t n = 0;
    ssize_t expected = -1;
	unsigned char buf[CHUNK_SIZE];
    std::vector<uint8_t> data;

    if (!socket.read_timeout(std::chrono::seconds(2))) {
        std::cerr << "Error setting timeout on TCP stream: "
                << socket.last_error_str() << std::endl;
        return;
    }

	while (n = socket.read(buf, sizeof(buf)) > 0) {
        data.insert(data.end(), buf, buf + n);

        while (data.size() >= sizeof(expected) || expected >= 0) {
            if (expected < 0) {
                std::memcpy(&expected, data.data(), sizeof(expected));
                data.erase(data.begin(), data.begin() + sizeof(expected));
                data.resize(expected);
            }

            if (data.size() < expected)
                break;

            // handle request
            flatbuffers::FlatBufferBuilder builder(1024);
            auto request = schema::GetRequest(data.data());
            dispatcher_->dispatch(request, &builder);

            // write response from builder
            // TODO

            data.erase(data.begin(), data.begin() + expected);
            expected = -1;
        }
    }

	std::cout << "Connection closed from " << socket.peer_address() << std::endl;
}

std::string network_manager::endpoint()
{
    return host_ + ":" + std::to_string(port_);
}

} // namespace lrucache
