#include "raft.h"
#include <time.h>
#include <errno.h>

namespace raft {

        inline void set_heartbeat_timer() {
                clock_gettime(CLOCK_MONOTONIC, &timer);
                timer.tv_sec += HEARTBEAT_TIMEOUT;
        }

        int raft_leader_write_response(const MSG_RAFT& msg) {
                int written = 0;
                if (list.append_entry(msg.wrqst.key, msg.wrqst.value) == 0) {
                        written = 1;
                } else {
                        ERROR("add entry failed\n");
                        written = 0;
                }
                list.print();
                return raft_rpc_write_response(written);
        }

        int raft_leader_append_entries(int to) {
                return raft_rpc_append_request(to);
        }

        int raft_leader_heartbeat() {
                int res = 0;
                for (int i = 0; i < NODE_CNT; i++) {
                        if (i != node_id) {
                                if (raft_leader_append_entries(i) > 0) {
                                        res++;
                                }
                        }
                }
                return res;
        }

        int raft_leader_handle_append_response(const MSG_RAFT& msg) {
                if (msg.apresp.success >= 0) {
                        list.leader_commit(msg.apresp.id, msg.apresp.success);
                } else if (msg.apresp.term > currentTerm) {
                        currentTerm = msg.apresp.term;
                        leader = -1;
                        role = Role::Follower;
                } else {
                        // list.rollback(msg.apresp.id);
                        ERROR("append failed\n");
                }

                return 0;
        }

        int raft_leader_append_response(const MSG_RAFT& msg) {
                if (msg.append.term > currentTerm) {
                        currentTerm = msg.append.term;
                        leader = msg.append.id;
                        pending_msg = msg;
                        role = Role::Follower;
                } else if (msg.append.term == currentTerm) {
                        ERROR("Another leader ?\n");
                }else {
                        WARNING("Received outdated append\n");
                }
                return 0;
        }

        int raft_leader_vote_response(const MSG_RAFT& msg) {
                int granted = 0;
                if (currentTerm > msg.vrqst.term) {
                        granted = 0;
                        votedFor = -1;
                } else if (votedFor == -1 || currentTerm < msg.vrqst.term) {
                        if (currentTerm < msg.vrqst.term) {
                                currentTerm = msg.vrqst.term;
                                role = Role::Follower;
                                votedFor = -1;
                                leader = -1;
                        }
                        if (list.should_vote(msg.vrqst.lastLogIndex, msg.vrqst.lastLogTerm)) {
                                granted = 1;
                                votedFor = msg.vrqst.id;
                        } else {
                                granted = 0;
                        }
                } else {
                        granted = 0;
                }
                return raft_rpc_vote_response(granted);
        }



        int raft_being_leader() {
                WARNING("Entering Leader State\n");
                INFO("Sending starting heartbeat\n");
                list.reset_preceeded();
                while (raft_leader_heartbeat() < NODE_MAJOR - 1)
                        sleep(1);
                set_heartbeat_timer();
                while (role == Role::Leader) {
                        // if (counter > DEAD_CNT) {
                        //         role = Role::Dead;
                        //         break;
                        // } 
                        // counter++;
                        MSG_RAFT *msgr = raft_rpc_listen();
                        if (msgr) {
                                switch (msgr->type) {
                                case MSG_TYPE::RequestWrite:
                                        INFO("Write request received from %d\n", last_msg_from);
                                        raft_leader_write_response(*msgr);
                                        break;
                                case MSG_TYPE::AppendEntries:
                                        INFO("Append entries received from %d\n", last_msg_from);
                                        raft_leader_append_response(*msgr);
                                        break;
                                case MSG_TYPE::AppendEntriesRsp:
                                        INFO("Append entries response received from %d\n", last_msg_from);
                                        raft_leader_handle_append_response(*msgr);
                                        break;
                                case MSG_TYPE::RequestVote:
                                        INFO("Request vote received from %d\n", last_msg_from);
                                        raft_leader_vote_response(*msgr);
                                        break;
                                case MSG_TYPE::RequestRestart:
                                        WARNING("Restart reqeust received\n");
                                        raft_rpc_restart(msgr->restart.sec);
                                        break;
                                case MSG_TYPE::NullMsg:
                                case MSG_TYPE::RequestVoteRsp:
                                        break;
                                default:
                                        ERROR("Unknown message received:%d\n", msgr->type);
                                        break;
                                } 
                        }
                        if (timer_timeout() && role == Role::Leader) {
                                INFO("Sending Heartbeat\n");
                                raft_leader_heartbeat();
                                set_heartbeat_timer();
                        }
                }
                return 0;
        }
}