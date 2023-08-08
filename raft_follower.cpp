#include "raft.h"
#include "socket.h"
#include <time.h>
#include <errno.h>

namespace raft {
        timespec election_timer;

        inline void set_elctimer() {
                clock_gettime(CLOCK_MONOTONIC, &election_timer);
                election_timer.tv_sec += ELECTION_TIMEOUT;
                // election_timer = clock() + CLOCKS_PER_SEC * ELECTION_TIMEOUT;
        }

        inline bool should_election() {
                timespec now;
                clock_gettime(CLOCK_MONOTONIC, &now);
                return election_timer < now;
        }

        int raft_follower_write_response(const MSG_RAFT& msg, int client) {
                WARNING("received write request as follower\n");
                MSG_RAFT msgr;
                msgr.type = MSG_TYPE::ResponseWrite;
                msgr.wresp.leader = leader;
                msgr.wresp.written = false;
                return mysock::send(client, &msgr, sizeof(msgr));
        }

        int raft_follower_append_response(const MSG_RAFT& msg) {
                MSG_RAFT msgr;
                msgr.type = MSG_TYPE::AppendEntriesRsp;
                msgr.apresp.id = node_id;
                if (msg.append.term > currentTerm) {
                        INFO("Updating term (%d->%d) from append request\n", currentTerm, msg.append.term);
                        leader = msg.append.id;
                        currentTerm = msg.append.term;
                } else if (msg.append.term < currentTerm) {
                        WARNING("Outdated term in append request\n");
                        msgr.apresp.term = currentTerm;
                        msgr.apresp.success = -1;
                        return mysock::send(msg.append.id, &msgr, sizeof(msgr));
                }

                msgr.apresp.term = currentTerm;
                if (msg.append.id != leader) {
                        WARNING("Illegal leader %d, should be %d\n", msg.append.id, leader);
                        msgr.apresp.success = -1;
                } else {
                        int i;
                        for (i = 0; i < msg.append.entryCnt; i++) {
                                int r = list.update_entry(msg.append.entries[i]);
                                if (r < 0) break;
                        }
                        list.follower_commit(msg.append.leaderCommit);
                        msgr.apresp.success = msg.append.entries[i-1].index;
                        list.print();
                }
                return mysock::send(msg.append.id, &msgr, sizeof(msgr));
        }

        int raft_being_follower() {
                int cnt = 0; 
                mysock::MSG msg;
                MSG_RAFT *msgr = (MSG_RAFT*) msg.data;
                set_elctimer();
                while (role == Role::Follower) {
                        if (cnt++ > 30) break;
                        int r = mysock::recv(&msg);
                        if (r < 0) {
                                INFO("recv failed: %s\n", strerror(errno));
                        } else {
                                switch (msgr->type) {
                                case MSG_TYPE::AppendEntries:
                                        INFO("Append request received\n");
                                        raft_follower_append_response(*msgr);
                                        set_elctimer();
                                        break;
                                case MSG_TYPE::RequestWrite:
                                        raft_follower_write_response(*msgr, msg.from);
                                        break;
                                default:
                                        ERROR("Unexpected message received\n");
                                        break;
                                }
                        }
                        if (should_election()) {
                                WARNING("Leader is dead! Start the election!\n");
                                role = Role::Candidate;
                        }
                }
                return 0;
        }
}