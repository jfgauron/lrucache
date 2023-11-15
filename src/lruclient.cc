#include <iostream>

#include "lrucache/client.hxx"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "please provide the path to the INI file" << std::endl;
        return EXIT_FAILURE;
    }

    try {
        auto config = lrucache::client_config::from_file(argv[1]);
        std::cout << "client terminated." << std::endl;
    } catch (std::runtime_error e) {
        std::cout << "invalid INI file: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
