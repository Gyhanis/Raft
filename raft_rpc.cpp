#include <errno.h>

#include "socket.h"
#include "raft.h"
#include "def.h"
#include "log.h"

namespace raft {
        static mysock::MSG msg_r;
        static MSG_RAFT *_msg_r = (MSG_RAFT*) msg_r.data;
        static MSG_RAFT msg_s;
        const int &last_msg_from = msg_r.from;

        int raft_rpc_init(int id) {
                INFO("starting rpc on node %d\n", node_id);
                int r = mysock::init_socket(id);
                if (r < -1) return -1;
                r = mysock::set_timeout(LISTEN_TIMEOUT);
                return r;
        }

        MSG_RAFT* raft_rpc_listen() {
                int r = mysock::recv(&msg_r);
                if (r < 0) {
                        if (errno != EAGAIN) {
                                WARNING("recv failed: %s\n", strerror(errno));
                        }
                        return nullptr;
                } else {
                        return _msg_r;
                }
        }

        int raft_rpc_restart(int sec) {
                msg_s.type = MSG_TYPE::ResponseRestart;
                msg_s.restart_resp.rid = _msg_r->restart.rid;
                msg_s.restart_resp.success = 1;
                if (sec == -1) {
                        role = Role::Dead;
                } else {
                        role = Role::Restart;
                        countdown = sec;
                }
                INFO("last_msg_from: %d\n", last_msg_from);
                return mysock::send(last_msg_from, &msg_s, sizeof(msg_s));
        }

        int raft_rpc_vote_request() {
                msg_s.type = RequestVote;
                msg_s.vrqst.id = node_id;
                msg_s.vrqst.term = currentTerm;
                msg_s.vrqst.lastLogIndex = list.last().index;
                msg_s.vrqst.lastLogTerm = list.last().term;
                return mysock::boardcast(&msg_s, sizeof(msg_s));
        }

        int raft_rpc_vote_response(int granted) {
                msg_s.type = RequestVoteRsp;
                msg_s.vresp.id = node_id;
                msg_s.vresp.term = _msg_r->vrqst.term;
                msg_s.vresp.voteGranted = granted;
                return mysock::send(last_msg_from, &msg_s, sizeof(msg_s));
        }

        int raft_rpc_write_response(int written) {
                msg_s.type = ResponseWrite;
                msg_s.wresp.rid = _msg_r->wrqst.rid;
                msg_s.wresp.leader = leader;
                msg_s.wresp.written = written;
                return mysock::send(last_msg_from, &msg_s, sizeof(msg_s));
        }

        int raft_rpc_append_request(int to) {
                msg_s.type = MSG_TYPE::AppendEntries;
                msg_s.append.id = node_id;
                msg_s.append.term = currentTerm;
                int len;
                Entry* ev = list.get_entries_for(to, &len);
                msg_s.append.prevLogIndex = ev[0].index;
                msg_s.append.prevLogTerm = ev[0].term;
                msg_s.append.leaderCommit = list.get_commite_index();
                msg_s.append.entryCnt = len-1;
                for (int i = 0; i < msg_s.append.entryCnt; i++) {
                        msg_s.append.entries[i] = ev[i+1];
                }
                return mysock::send(to, &msg_s, sizeof(msg_s));
        }

        int raft_rpc_append_response(int success) {
                msg_s.type = AppendEntriesRsp;
                msg_s.apresp.id = node_id;
                msg_s.apresp.term = currentTerm;
                msg_s.apresp.success = success;
                return mysock::send(last_msg_from, &msg_s, sizeof(msg_s));
        }

        int raft_rpc_shutdown() {
                return mysock::release_socket();
        }
};