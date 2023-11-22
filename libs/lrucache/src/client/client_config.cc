#include "lrucache/client_config.hxx"
#include "ini/ini.h"

namespace lrucache {

client_config client_config::from_file(std::string path)
{
    inih::INIReader r{path};
    client_config config;

    config.discovery_address =
            r.Get<std::string>("cluster", "discovery_address");
    config.discovery_port =
            r.Get<unsigned short int>("cluster", "discovery_port");
    config.fallback_address =
            r.Get<std::string>("cluster", "fallback_address");
    config.fallback_port =
            r.Get<unsigned short int>("cluster", "fallback_port");

    return config;
}

} // namespace lrucache
