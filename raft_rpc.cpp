#include <errno.h>

#include "socket.h"
#include "raft.h"
#include "def.h"
#include "log.h"

namespace raft {
        int raft_rpc_init(int id) {
                INFO("starting rpc on node %d\n", node_id);
                int r = mysock::init_socket(id);
                if (r < -1) return -1;
                r = mysock::set_timeout(LISTEN_TIMEOUT);
                return r;
        }

        int raft_rpc_shutdown() {
                return mysock::release_socket();
        }
};