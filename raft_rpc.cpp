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

        int raft_rpc_restart(const MSG_RAFT& msg) {
                MSG_RAFT msgr;
                msgr.type = MSG_TYPE::ResponseRestart;
                msgr.restart_resp.rid = msg.restart.rid;
                msgr.restart_resp.success = 1;
                if (msg.restart.sec == -1) {
                        role = Role::Dead;
                } else {
                        role = Role::Restart;
                        countdown = msg.restart.sec;
                }
                return mysock::send(msg.restart.cid, &msgr, sizeof(msgr));
        }

        int raft_rpc_shutdown() {
                return mysock::release_socket();
        }
};