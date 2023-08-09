#include "raft.h"
#include "socket.h"
#include <time.h>
#include <errno.h>

namespace raft {

        inline void set_heartbeat_timer() {
                clock_gettime(CLOCK_MONOTONIC, &timer);
                timer.tv_sec += HEARTBEAT_TIMEOUT;
        }

        int raft_leader_write_response(const MSG_RAFT& msg) {
                MSG_RAFT msgr;
                msgr.type = MSG_TYPE::ResponseWrite;
                msgr.wresp.rid = msg.wrqst.rid;
                msgr.wresp.leader = leader;
                if (list.append_entry(msg.wrqst.key, msg.wrqst.value) == 0) {
                        msgr.wresp.written = true;
                } else {
                        ERROR("add entry failed\n");
                        msgr.wresp.written = false;
                }
                list.print();
                return mysock::send(msg.wrqst.cid, &msgr, sizeof(msgr));
        }

        int raft_leader_append_entries(int to) {
                MSG_RAFT msgr;
                msgr.type = MSG_TYPE::AppendEntries;
                msgr.append.id = node_id;
                msgr.append.term = currentTerm;
                list.fill_append_entries(msgr, to);
                return mysock::send(to, &msgr, sizeof(msgr));
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
                } else {
                        //TODO
                        ERROR("append failed\n");
                }

                return 0;
        }

        int raft_leader_append_response(const MSG_RAFT& msg) {
                if (msg.append.term > currentTerm) {
                        currentTerm = msg.append.term;
                        leader = msg.append.id;
                        role = Role::Follower;
                } else if (msg.append.term == currentTerm) {
                        ERROR("Another leader ?\n");
                }else {
                        WARNING("Received outdated append\n");
                }
                return 0;
        }

        int raft_leader_vote_response(const MSG_RAFT& msg) {
                MSG_RAFT msgr;
                msgr.type = RequestVoteRsp;
                msgr.vresp.id = node_id;
                if (currentTerm > msg.vrqst.term) {
                        msgr.vresp.voteGranted = 0;
                        votedFor = -1;
                } else if (votedFor == -1 || currentTerm < msg.vrqst.term) {
                        if (currentTerm < msg.vrqst.term) {
                                currentTerm = msg.vrqst.term;
                                role = Role::Follower;
                                votedFor = -1;
                                leader = -1;
                        }
                        if (list.should_vote(msg.vrqst.lastLogIndex, msg.vrqst.lastLogTerm)) {
                                msgr.vresp.voteGranted = 1;
                                votedFor = msg.vrqst.id;
                        } else {
                                msgr.vresp.voteGranted = 0;
                        }
                } else {
                        msgr.vresp.voteGranted = 0;
                }
                msgr.vresp.term = currentTerm;
                return mysock::send(msg.vrqst.id, &msgr, sizeof(msgr));
        }



        int raft_being_leader() {
                WARNING("Entering Leader State\n");
                mysock::MSG msg;
                MSG_RAFT *msgr = (MSG_RAFT*) msg.data;
                INFO("Sending starting heartbeat\n");
                list.reset_preceeded();
                while (raft_leader_heartbeat() < NODE_MAJOR - 1)
                        sleep(1);
                set_heartbeat_timer();
                while (role == Role::Leader) {
                        if (counter++ > DEAD_CNT) {
                                role = Role::Dead;
                                break;
                        }
                        int r = mysock::recv(&msg);
                        if (r < 0) {
                                if (errno != EAGAIN)
                                        INFO("recv failed: %s\n", strerror(errno));
                        } else {
                                switch (msgr->type) {
                                case MSG_TYPE::RequestWrite:
                                        INFO("Write request received from %d\n", msg.from);
                                        raft_leader_write_response(*msgr);
                                        break;
                                case MSG_TYPE::AppendEntries:
                                        INFO("Append entries received from %d\n", msg.from);
                                        raft_leader_append_response(*msgr);
                                        break;
                                case MSG_TYPE::AppendEntriesRsp:
                                        INFO("Append entries response received from %d\n", msg.from);
                                        raft_leader_handle_append_response(*msgr);
                                        break;
                                case MSG_TYPE::RequestVote:
                                        INFO("Request vote received from %d\n", msg.from);
                                        raft_leader_vote_response(*msgr);
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