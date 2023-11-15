#include "cache_server.hxx"

#include <chrono>
#include <iostream>
#include <thread>

namespace lrucache {

cache_server::cache_server(cache_config config, int server_id) 
    : config_(config)
    , server_id_(server_id)
    , thread_pool_(std::thread::hardware_concurrency())
{
}

void cache_server::run()
{
    sockpp::initialize();

    sockpp::tcp_acceptor acc({config_.host, config_.port });

    if (!acc) {
        std::cerr << "error creating the acceptor: "
            << acc.last_error_str() << std::endl;
        return;
    }

    std::cout << "    -- Replicated LRUCache with Raft --"  << std::endl;
    std::cout << "                         Version 0.1.0"   << std::endl;
    std::cout << "    Server ID:    " << server_id_         << std::endl;
    std::cout << "    Endpoint:     " << config_.endpoint() << std::endl;

    while (true) {
        // accept a new client connection
        sockpp::tcp_socket socket = acc.accept();

        if (!socket) {
            std::cerr << "Error accepting incoming connection: "
                << acc.last_error_str() << std::endl;
            continue;
        }

        thread_pool_.push(handle_connection, std::move(socket));
    }
}

void cache_server::handle_connection(int id, sockpp::tcp_socket& socket)
{
	ssize_t n;
	char buf[512];

    if (!socket.read_timeout(std::chrono::seconds(2))) {
        std::cerr << "Error setting timeout on TCP stream: "
                << socket.last_error_str() << std::endl;
    }

	while ((n = socket.read(buf, sizeof(buf))) > 0)
		socket.write_n(buf, n);

	std::cout << "Connection closed from " << socket.peer_address() << std::endl;
}

}