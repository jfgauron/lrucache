#ifndef LRUCACHE_CACHE_STATE_MACHINE_H_
#define LRUCACHE_CACHE_STATE_MACHINE_H_

#include "libnuraft/nuraft.hxx"
#include "lrucache/cache_config.hxx"

#include "cache/cache_state.hxx"

namespace lrucache {

class cache_state_machine : public nuraft::state_machine {
public:
    cache_state_machine(cache_config config, bool async_snapshot = false)
        : state_(config)
    {
    }

    ~cache_state_machine() {}

    // TODO: this entire code is garbage
    enum op_type : int {
        READ = 0x1,
        WRITE = 0x2,
        PURGE = 0x3
    };

    struct op_payload {
        op_type type;
        time_t timestamp;
        std::string key;
        size_t data_len;
        unsigned char* data;
        time_t expires_at;
    };

    static nuraft::ptr<nuraft::buffer> encode_log(const op_payload& payload)
    {
        size_t size = sizeof(payload.type) + sizeof(payload.timestamp);
        if (payload.type != op_type::PURGE) {
            size += sizeof(size_t) + payload.key.size();
        }
        if (payload.type == op_type::WRITE) {
            size += sizeof(size_t) + payload.data_len + sizeof(time_t);
        }
        nuraft::ptr<nuraft::buffer> log = nuraft::buffer::alloc(size);
        nuraft::buffer_serializer bs(log);

        bs.put_raw(&payload.type, sizeof(payload.type));
        bs.put_raw(&payload.timestamp, sizeof(payload.timestamp));
        if (payload.type != op_type::PURGE) {
            bs.put_str(payload.key);
        }
        if (payload.type == op_type::WRITE) {
            bs.put_bytes(payload.data, payload.data_len);
            bs.put_raw(&payload.expires_at, sizeof(payload.expires_at));
        }

        return log;
    }

    static void decode_log(nuraft::buffer& log, op_payload& payload)
    {
        nuraft::buffer_serializer bs(log);
        memcpy(&payload.type,
               bs.get_raw(sizeof(payload.type)),
               sizeof(payload.type));
        memcpy(&payload.timestamp,
               bs.get_raw(sizeof(payload.timestamp)),
               sizeof(payload.timestamp));
        if (payload.type != op_type::PURGE) {
            payload.key = bs.get_str();
        }
        if (payload.type == op_type::WRITE) {
            auto data = bs.get_bytes(payload.data_len);
            memcpy(payload.data, data, payload.data_len);
        }
    }
    // TODO: end of garbage code

    /**
     * Commit the given Raft log.
     *
     * NOTE:
     *   Given memory buffer is owned by caller, so that
     *   commit implementation should clone it if user wants to
     *   use the memory even after the commit call returns.
     *
     *   Here provide a default implementation for facilitating the
     *   situation when application does not care its implementation.
     *
     * @param log_idx Raft log number to commit.
     * @param data Payload of the Raft log.
     * @return Raft log number wrapped in a nuraft::buffer
     */
    nuraft::ptr<nuraft::buffer> commit(const ulong log_idx,
                                       nuraft::buffer& data);

    /**
     * Handler on the commit of a configuration change.
     *
     * @param log_idx Raft log number of the configuration change.
     * @param new_conf New cluster configuration.
     */
    void commit_config(const ulong log_idx,
                       nuraft::ptr<nuraft::cluster_config>& new_conf);

private:
    cache_state state_;
    std::atomic<uint64_t> last_committed_idx_;
    std::atomic<uint64_t> last_config_idx_;
};

} // namespace lrucache

#endif // LRUCACHE_CACHE_STATE_MACHINE_H_