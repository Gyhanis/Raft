#include <errno.h>

#include "socket.h"
#include "raft.h"
#include "def.h"
#include "log.h"

namespace raft {
        int raft_rpc_init(int id) {
                INFO("starting rpc on node %d\n", node_id);
                return mysock::init_socket(id);
        }

        int raft_response_write(const MSG_RAFT& msg, int client) {
                MSG_RAFT msgr;
                msgr.type = MSG_TYPE::ResponseWrite;
                msgr.wresp.leader = leader;
                if (role != Role::Leader) {
                        msgr.wresp.written = false;
                } else {
                        if (list.add_entry(msg.wrqst.key, msg.wrqst.value) == 0) {
                                msgr.wresp.written = true;
                        } else {
                                msgr.wresp.written = false;
                        }
                }
                return mysock::send(client, &msgr, sizeof(msgr));
        }

        int raft_append_entry(int to) {
                MSG_RAFT msgr;
                msgr.type = MSG_TYPE::AppendEntries;
                msgr.append.id = node_id;
                msgr.append.term = currentTerm;
                msgr.append.prevLogIndex = proceeded[to];
                msgr.append.prevLogTerm = list.index(proceeded[to]).term;
                msgr.append.leaderCommit = list.commitIndex;
                msgr.append.entryCnt = list.lastIndex - proceeded[to];
                if (msgr.append.entryCnt > 4) 
                        msgr.append.entryCnt = 4;
                for (int i = 0; i < msgr.append.entryCnt; i++) {
                        msgr.append.entries[i] = list.index(proceeded[to]+1+i);
                }
                return mysock::send(to, &msgr, sizeof(msgr));
        }

        int raft_append_response(const MSG_RAFT& msg) {
                MSG_RAFT msgr;
                msgr.type = MSG_TYPE::AppendEntriesRsp;
                msgr.apresp.id = node_id;
                if (msgr.append.term > currentTerm) {
                        leader = msg.append.id;
                        currentTerm = msg.append.term;
                } else if (msg.append.term < currentTerm) {
                        msgr.apresp.term = currentTerm;
                        msgr.apresp.success = -1;
                        return mysock::send(msg.append.id, &msgr, sizeof(msgr));
                }

                msgr.apresp.term = currentTerm;
                if (msgr.append.id != leader) {
                        msgr.apresp.success = -1;
                } else if (msgr.append.prevLogIndex != list.lastIndex 
                        || msgr.append.prevLogTerm != list.last().term) {
                        msgr.apresp.success = -1;
                } else {
                        int i;
                        for (i = 0; i < msgr.apresp.success; i++) {
                                int r = list.add_entry(msgr.append.entries[i].key, msgr.append.entries[i].value);
                                if (r < 0) break;
                        }
                        msgr.apresp.success = i;
                }
                return mysock::send(msg.append.id, &msgr, sizeof(msgr));
        }

        int raft_rpc_shutdown() {
                return mysock::release_socket();
        }
};