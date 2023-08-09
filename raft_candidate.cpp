#include "socket.h"
#include "raft.h"
#include <errno.h>

namespace raft {
        int vote_cnt = 0;

        inline void candidate_set_election_timer() {
                clock_gettime(CLOCK_MONOTONIC, &timer);
                timer.tv_sec += LISTEN_TIMEOUT + rand() % NODE_CNT;
        }

        int raft_candidate_write_response(const MSG_RAFT& msg) {
                // WARNING("received write request as follower\n");
                MSG_RAFT msgr;
                msgr.type = MSG_TYPE::ResponseWrite;
                msgr.wresp.rid = msg.wrqst.rid;
                msgr.wresp.leader = leader;
                msgr.wresp.written = false;
                return mysock::send(msg.wrqst.cid, &msgr, sizeof(msgr));
        }

        int raft_candidate_request_vote() {
                MSG_RAFT msg;
                msg.type = RequestVote;
                msg.vrqst.id = node_id;
                msg.vrqst.term = currentTerm;
                msg.vrqst.lastLogIndex = list.last().index;
                msg.vrqst.lastLogTerm = list.last().term;
                return mysock::boardcast(&msg, sizeof(msg));
        }

        int raft_candidate_vote_response(const MSG_RAFT& msg) {
                MSG_RAFT msgr;
                msgr.type = MSG_TYPE::RequestVoteRsp;
                msgr.vresp.id = node_id;
                if (msg.vrqst.term < currentTerm) {
                        msgr.vresp.voteGranted = 0;
                } else if (votedFor == node_id || msg.vrqst.term > currentTerm) {
                        if (list.should_vote(msg.vrqst.lastLogIndex, msg.vrqst.lastLogTerm)) {
                                msgr.vresp.voteGranted = 1;
                                votedFor = msg.vrqst.id;
                                vote_cnt--;
                        } else {
                                msgr.vresp.voteGranted = 0;
                        }
                        if (msg.vrqst.term > currentTerm) {
                                currentTerm = msg.vrqst.term;
                                role = Role::Follower;
                        }
                } 
                msgr.vresp.term = currentTerm;
                if (msgr.vresp.voteGranted) {
                        INFO("Vote granted\n");
                } else {
                        INFO("Vote rejected\n");
                }
                return mysock::send(msg.vrqst.id, &msgr, sizeof(msgr));
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
                        role == Role::Follower;
                        leader = msg.append.id;
                        pending_msg = msg;
                } else {
                        MSG_RAFT msgr;
                        msgr.type = AppendEntriesRsp;
                        msgr.apresp.id = node_id;
                        msgr.apresp.term = currentTerm;
                        msgr.apresp.success = -1;
                        mysock::send(msg.append.id, &msgr, sizeof(msgr));
                }
                return 0;
        }

        int raft_being_candidate() {
                leader = -1;
                currentTerm++;
                WARNING("Entering Candidate State (Term %d)\n", currentTerm);
                votedFor = node_id;
                vote_cnt = 1;
                raft_candidate_request_vote();
                candidate_set_election_timer();
                while (role == Role::Candidate) {
                        if (counter++ > DEAD_CNT) {
                                role = Role::Dead;
                                break;
                        }
                        mysock::MSG msg;
                        MSG_RAFT *msgr = (MSG_RAFT*) msg.data;
                        int r = mysock::recv(&msg);
                        if (r < 0) {
                                if (errno != EAGAIN)
                                        INFO("recv failed: %s\n", strerror(errno));
                        } else {
                                switch (msgr->type) {
                                case MSG_TYPE::RequestWrite:
                                        INFO("Request write received from %d\n", msg.from);
                                        raft_candidate_write_response(*msgr);
                                        break;
                                case MSG_TYPE::RequestVote:
                                        INFO("Request vote received from %d\n", msg.from);
                                        raft_candidate_vote_response(*msgr);
                                        break;
                                case MSG_TYPE::RequestVoteRsp:
                                        INFO("Vote response received from %d\n", msg.from);
                                        raft_candidate_handle_vote_response(*msgr);
                                        break;
                                case MSG_TYPE::AppendEntries:
                                        INFO("Append entry received from %d\n", msg.from);
                                        raft_candidate_append_response(*msgr);
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
                                raft_candidate_request_vote();
                                candidate_set_election_timer();
                        }
                } 
                return 0;
        }
}