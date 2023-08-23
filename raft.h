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

#define DEAD_CNT                100

bool operator<(const timespec& ts1, const timespec& ts2);

namespace raft {

        extern int counter;
        extern const int &last_msg_from;

        enum Role {
                Follower, Candidate, Leader, Dead, Restart
        };

        extern int node_id;
        extern int currentTerm;
        extern int leader;
        extern int votedFor;
        extern Role role; 

        extern EntryList list;

        extern MSG_RAFT pending_msg;

        extern timespec timer;
        bool timer_timeout();
        extern int countdown;

        int raft_rpc_init(int id);
        int raft_rpc_shutdown();
        int raft_rpc_restart(int sec);
        MSG_RAFT* raft_rpc_listen();
        int raft_rpc_vote_request();
        int raft_rpc_vote_response(int granted);
        int raft_rpc_write_response(int written);
        int raft_rpc_append_request(int to);
        int raft_rpc_append_response(int success);

        int raft_init(int id);
        int raft_restart();
        int raft_being_leader();
        int raft_being_follower();
        int raft_being_candidate();
        void raft_print_log_state();

        int raft_loop(int id);
}

