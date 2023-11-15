#include <iostream>

#include "lrucache/cache_config.hxx"
#include "lrucache/server.hxx"

int main(int argc, char** argv) {

    if (argc < 2) {
        std::cout << "please provide the path to the INI file" << std::endl;
        return EXIT_FAILURE;
    }

    try {
        auto config = lrucache::cache_config::from_file(argv[1]);
        lrucache::server server(config);
    
        server.run();

        std::cout << "server terminated." << std::endl;
    } catch (std::runtime_error e) {
        std::cout << "error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
