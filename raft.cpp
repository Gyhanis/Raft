#include "raft.h"
#include "def.h"

namespace raft {
        int node_id;
        int currentTerm = 0;
        int leader = 0;
        Role role = Role::Follower; 

        EntryList list;
        int proceeding[NODE_CNT];
        int proceeded[NODE_CNT];

        int raft_init(int id) {
                node_id = id;
                if (id == 0) {
                        role = Role::Leader;
                }
                for (int i = 0; i < NODE_CNT; i++) {
                        proceeding[i] = proceeded[i] = 0;
                }
                return raft_rpc_init(id);
        }

        int raft_handle_response(const MSG_RAFT& msg) {
                return 0;
        }
}