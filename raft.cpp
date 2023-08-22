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
        int counter = 0;

        int node_id;
        int currentTerm = 0;
        int leader = -1;
        int votedFor = -1;
        Role role = Role::Follower; 

        EntryList list;

        timespec timer;
        bool timer_timeout() {
                timespec now;
                clock_gettime(CLOCK_MONOTONIC, &now);
                return timer < now;
        }

        int raft_init(int id) {
                node_id = id;
                role = Role::Follower;
                currentTerm = 0;
                timespec ts;
                clock_gettime(CLOCK_MONOTONIC, &ts);
                srand(ts.tv_nsec);
                pending_msg.type = NullMsg;
                list.init();
                return raft_rpc_init(id);
        }

        int raft_loop(int id) {
                raft_init(id);
                while (role != Role::Dead) {
                        switch (role) {
                        case Role::Leader:
                                WARNING("========= Transferred into Leader State ========\n");
                                raft_being_leader();
                                break;
                        case Role::Follower:
                                WARNING("========= Transferred into Follower State ======\n");
                                raft_being_follower();
                                break;
                        case Role::Candidate:
                                WARNING("========= Transferred into Candidate State =====\n");
                                raft_being_candidate();
                                break;
                        case Role::Restart:
                                raft_restart();
                                break;
                        default:
                                ERROR("Am I dead?\n");
                                role = Role::Dead;
                                break;
                        }
                }
                WARNING("Shutting down\n");
                raft_rpc_shutdown();
                return 0;
        }
}