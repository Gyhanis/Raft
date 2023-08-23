#include "raft.h"
#include <stdlib.h>
#include <time.h>
#include <errno.h>

namespace raft {
        MSG_RAFT pending_msg;

        inline void follower_set_election_timer() {
                clock_gettime(CLOCK_MONOTONIC, &timer);
                timer.tv_sec += ELECTION_TIMEOUT + rand() % NODE_CNT;
        }

        int raft_follower_write_response(const MSG_RAFT& msg) {
                // WARNING("received write request as follower\n");
                return raft_rpc_write_response(0);
        }

        int raft_follower_append_response(const MSG_RAFT& msg) {
                if (msg.append.term > currentTerm) {
                        INFO("Updating term (%d->%d) from append request\n", currentTerm, msg.append.term);
                        leader = msg.append.id;
                        currentTerm = msg.append.term;
                } else if (msg.append.term < currentTerm) {
                        WARNING("Outdated term in append request\n");
                        return raft_rpc_append_response(-1);
                } else if (leader == -1) {
                        leader = msg.append.id;
                } 

                int success = 0;
                if (msg.append.id != leader) {
                        WARNING("Illegal leader %d, should be %d\n", msg.append.id, leader);
                        // return without sending response
                        return 0;
                } else if (!list.check_last_entry(msg.append.prevLogIndex, 
                                msg.append.prevLogTerm)) {
                        WARNING("Last log mismatch!\n");
                        success = -1;
                } else {
                        int i;
                        for (i = 0; i < msg.append.entryCnt; i++) {
                                int r = list.update_entry(msg.append.entries[i]);
                                if (r < 0) break;
                        }
                        list.follower_commit(msg.append.leaderCommit);
                        success = msg.append.entries[i-1].index;
                        list.print();
                }
                return raft_rpc_append_response(success);
        }

        int raft_follower_vote_response(const MSG_RAFT& msg) {
                int granted = 0;
                if (currentTerm > msg.vrqst.term) {
                        votedFor = -1;
                } else if (votedFor == -1 || currentTerm < msg.vrqst.term) {
                        if (currentTerm < msg.vrqst.term) {
                                currentTerm = msg.vrqst.term;
                                follower_set_election_timer();
                                votedFor = -1;
                                leader = -1;
                        }
                        if (list.should_vote(msg.vrqst.lastLogIndex, msg.vrqst.lastLogTerm)) {
                                granted = 1;
                                votedFor = msg.vrqst.id;
                        }
                }
                if (granted) {
                        INFO("Vote granted (term %d)\n", msg.vrqst.term);
                } else {
                        INFO("Vote rejected (term %d)\n", msg.vrqst.term);
                }
                return raft_rpc_vote_response(granted);
        }

        int raft_being_follower() {
                WARNING("Entering Follower State\n");
                int cnt = 0; 
                if (pending_msg.type == MSG_TYPE::AppendEntries) {
                        raft_follower_append_response(pending_msg);
                        pending_msg.type = NullMsg;
                }
                follower_set_election_timer();
                
                while (role == Role::Follower) {
                        // if (counter++ > DEAD_CNT) {
                        //         role = Role::Dead;
                        //         break;
                        // }
                        MSG_RAFT *msgr = raft_rpc_listen();
                        if (msgr) {
                                switch (msgr->type) {
                                case MSG_TYPE::AppendEntries:
                                        INFO("Append request received from %d\n", last_msg_from);
                                        raft_follower_append_response(*msgr);
                                        follower_set_election_timer();
                                        break;
                                case MSG_TYPE::RequestWrite:
                                        INFO("Write request received from %d\n", last_msg_from);
                                        raft_follower_write_response(*msgr);
                                        break;
                                case MSG_TYPE::RequestVote:
                                        INFO("Request vote received from %d\n", last_msg_from);
                                        raft_follower_vote_response(*msgr);
                                        break;
                                case MSG_TYPE::RequestRestart:
                                        WARNING("Restart reqeust received\n");
                                        raft_rpc_restart(msgr->restart.sec);
                                        break;
                                case MSG_TYPE::NullMsg:
                                case MSG_TYPE::RequestVoteRsp:
                                case MSG_TYPE::AppendEntriesRsp:
                                        break;
                                default:
                                        ERROR("Unknown message received\n");
                                        break;
                                }
                        }
                        if (timer_timeout()) {
                                WARNING("Leader is dead! Start the election!\n");
                                role = Role::Candidate;
                        }
                }
                return 0;
        }
}