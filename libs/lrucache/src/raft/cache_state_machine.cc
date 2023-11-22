#include "raft/cache_state_machine.hxx"

namespace lrucache {

nuraft::ptr<nuraft::buffer> cache_state_machine::commit(const ulong log_idx,
                                                        nuraft::buffer& data)
{
    op_payload payload;
    decode_log(data, payload);
    
    switch (payload.type) {
        case READ:
            state_.commit_read(payload.key, payload.timestamp);
            break;
        case WRITE:
            state_.commit_write(
                payload.key,
                {payload.data, payload.data_len, payload.expires_at},
                payload.timestamp);
            break;
        case PURGE:
            state_.commit_purge_expired(payload.timestamp);
            break;
    }

    // Return Raft log number as a return result.
    nuraft::ptr<nuraft::buffer> ret = nuraft::buffer::alloc( sizeof(log_idx) );
    nuraft::buffer_serializer bs(ret);
    bs.put_u64(log_idx);
    return ret;
}

void cache_state_machine::commit_config(
        const ulong log_idx,
        nuraft::ptr<nuraft::cluster_config>& new_conf)
{
    // TODO: update config with custom metadata
    last_committed_idx_ = log_idx;
    last_config_idx_ = log_idx;
}

} // namespace lrucache