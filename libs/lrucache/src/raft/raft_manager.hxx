#ifndef LRUCACHE_RAFT_MANAGER_H_
#define LRUCACHE_RAFT_MANAGER_H_

#include "lrucache/cache_config.hxx"
#include "libnuraft/nuraft.hxx"

namespace lrucache {

class raft_manager {
public:
    raft_manager(cache_config config, int server_id);

    nuraft::ptr<nuraft::raft_server> instance() { return m_instance_; }

private:
    cache_config config_;

    int server_id_;

    nuraft::ptr<nuraft::state_mgr> state_mgr_;
    nuraft::ptr<nuraft::state_machine> state_machine_;
    nuraft::raft_launcher launcher_;
    nuraft::ptr<nuraft::raft_server> m_instance_;
};

}

#endif // LRUCACHE_RAFT_MANAGER_H_