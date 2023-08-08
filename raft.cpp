#include "raft.h"
#include "def.h"
#include "log.h"
#include <stdlib.h>
#include <time.h>
#include <assert.h>

bool operator<(const timespec& ts1, const timespec& ts2) {
        if (ts1.tv_sec == ts2.tv_sec) {
                return ts1.tv_nsec < ts2.tv_nsec;
        } else {
                return ts1.tv_sec < ts2.tv_sec;
        }
}

namespace raft {
        int node_id;
        int currentTerm = 0;
        int leader = 0;
        Role role = Role::Follower; 

        EntryList list;

        int raft_init(int id) {
                node_id = id;
                WARNING("Hard coded id 0 to be leader\n");
                if (id == 0) {
                        role = Role::Leader;
                } 
                list.init();
                return raft_rpc_init(id);
        }
}