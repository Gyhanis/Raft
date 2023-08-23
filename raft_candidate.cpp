#include "raft.h"
#include <errno.h>
#include <stdlib.h>

namespace raft {
        int vote_cnt = 0;

        inline void candidate_set_election_timer() {
                clock_gettime(CLOCK_MONOTONIC, &timer);
                timer.tv_sec += LISTEN_TIMEOUT + rand() % NODE_CNT;
        }

        int raft_candidate_write_response(const MSG_RAFT& msg) {
                return raft_rpc_write_response(0);
        }

        int raft_candidate_vote_request() {
                return raft_rpc_vote_request();
        }

        int raft_candidate_vote_response(const MSG_RAFT& msg) {
                int granted = 0;
                if ((msg.vrqst.term == currentTerm && votedFor == node_id) 
                || msg.vrqst.term > currentTerm) {
                        if (list.should_vote(msg.vrqst.lastLogIndex, msg.vrqst.lastLogTerm)) {
                                granted = 1;
                                votedFor = msg.vrqst.id;
                                vote_cnt--;
                        } 
                        if (msg.vrqst.term > currentTerm) {
                                currentTerm = msg.vrqst.term;
                                role = Role::Follower;
                        }
                } 
                if (granted) {
                        INFO("Vote granted\n");
                } else {
                        INFO("Vote rejected\n");
                }
                return raft_rpc_vote_response(granted);
        }

        int raft_candidate_handle_vote_response(const MSG_RAFT& msg) {
                if (msg.vresp.term == currentTerm) {
                        if (msg.vresp.voteGranted) {
                                INFO("Vote Received from %d (term %d)\n", msg.vresp.id, msg.vresp.term);
                                vote_cnt++;
                                if (vote_cnt >= NODE_MAJOR) {
                                        role = Role::Leader;
                                        leader = node_id;
                                }
                        }
                } else if (msg.vresp.term > currentTerm) {
                        currentTerm = msg.vresp.term;
                        role = Role::Follower;
                        //TODO
                }
                return 0;
        }

        int raft_candidate_append_response(const MSG_RAFT& msg) {
                if (msg.append.term >= currentTerm) {
                        role = Role::Follower;
                        leader = msg.append.id;
                        pending_msg = msg;
                        return 0;
                } else {
                        return raft_rpc_append_response(-1);
                }
        }

        int raft_being_candidate() {
                leader = -1;
                currentTerm++;
                WARNING("Entering Candidate State (Term %d)\n", currentTerm);
                votedFor = node_id;
                vote_cnt = 1;
                raft_candidate_vote_request();
                candidate_set_election_timer();
                while (role == Role::Candidate) {
                        // if (counter++ > DEAD_CNT) {
                        //         role = Role::Dead;
                        //         break;
                        // }
                        MSG_RAFT *msgr = raft_rpc_listen();
                        if (msgr) {
                                switch (msgr->type) {
                                case MSG_TYPE::RequestWrite:
                                        INFO("Request write received from %d\n", last_msg_from);
                                        raft_candidate_write_response(*msgr);
                                        break;
                                case MSG_TYPE::RequestVote:
                                        INFO("Request vote received from %d\n", last_msg_from);
                                        raft_candidate_vote_response(*msgr);
                                        break;
                                case MSG_TYPE::RequestVoteRsp:
                                        INFO("Vote response received from %d\n", last_msg_from);
                                        raft_candidate_handle_vote_response(*msgr);
                                        break;
                                case MSG_TYPE::AppendEntries:
                                        INFO("Append entry received from %d\n", last_msg_from);
                                        raft_candidate_append_response(*msgr);
                                        break;
                                case MSG_TYPE::RequestRestart:
                                        WARNING("Restart reqeust received\n");
                                        raft_rpc_restart(msgr->restart.sec);
                                        break;
                                case MSG_TYPE::NullMsg:
                                case MSG_TYPE::AppendEntriesRsp:
                                        break;
                                default:
                                        ERROR("Unknown message received\n");
                                        break;
                                }
                        }
                        if (timer_timeout() && role == Role::Candidate) {
                                currentTerm++;
                                WARNING("Election time out, next term %d\n", currentTerm);
                                votedFor = node_id;
                                vote_cnt = 1;
                                raft_candidate_vote_request();
                                candidate_set_election_timer();
                        }
                } 
                return 0;
        }
}