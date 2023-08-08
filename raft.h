#pragma once 

#include "def.h"
#include "log.h"
#include "raft_msg.h"
#include "raft_entlist.h"
#include <time.h>

#define LISTEN_TIMEOUT          2
#define HEARTBEAT_TIMEOUT       5
#define ELECTION_TIMEOUT        8

#define NODE_MAJOR              ((NODE_CNT+1)/2)

bool operator<(const timespec& ts1, const timespec& ts2);

namespace raft {

        enum Role {
                Follower, Candidate, Leader, Dead
        };

        extern int node_id;
        extern int currentTerm;
        extern int leader;
        extern Role role; 

        extern EntryList list;

        int raft_rpc_init(int id);
        int raft_rpc_shutdown();

        int raft_init(int id);
        int raft_being_leader();
        int raft_being_follower();
        void raft_print_log_state();

}

