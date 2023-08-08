#include "raft.h"
#include "socket.h"
#include <time.h>
#include <errno.h>

namespace raft {
        timespec heartbeat_timer;

        inline void set_hbtimer() {
                clock_gettime(CLOCK_MONOTONIC, &heartbeat_timer);
                heartbeat_timer.tv_sec += HEARTBEAT_TIMEOUT;
        }

        inline bool should_heartbeat() {
                timespec now;
                clock_gettime(CLOCK_MONOTONIC, &now);
                // DEBUG("should heartbeat: now %ld timer %ld\n", now, heartbeat_timer);
                return heartbeat_timer < now;
        }

        int raft_leader_write_response(const MSG_RAFT& msg, int client) {
                MSG_RAFT msgr;
                msgr.type = MSG_TYPE::ResponseWrite;
                msgr.wresp.leader = leader;
                if (list.append_entry(msg.wrqst.key, msg.wrqst.value) == 0) {
                        msgr.wresp.written = true;
                } else {
                        ERROR("add entry failed\n");
                        msgr.wresp.written = false;
                }
                list.print();
                return mysock::send(client, &msgr, sizeof(msgr));
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
                                if (raft_leader_append_entries(i) == 0) {
                                        res++;
                                }
                        }
                }
                return res;
        }

        int raft_leader_handle_append_response(const MSG_RAFT& msg) {
                INFO("received append response from %d\n", msg.apresp.id);
                if (msg.apresp.success > 0) {
                        list.leader_commit(msg.apresp.id, msg.apresp.success);
                } else {
                        //TODO
                        ERROR("append failed, forcing stop\n");
                        // raft_rpc_shutdown();
                        // exit(0);
                }

                return 0;
        }

        int raft_being_leader() {
                int cnt = 0;
                mysock::MSG msg;
                MSG_RAFT *msgr = (MSG_RAFT*) msg.data;
                set_hbtimer();
                while (role == Role::Leader) {
                        if (cnt++ > 20) break;
                        int r = mysock::recv(&msg);
                        if (r < 0) {
                                INFO("recv failed: %s\n", strerror(errno));
                        } else {
                                switch (msgr->type) {
                                case MSG_TYPE::RequestWrite:
                                        INFO("write request received\n");
                                        raft_leader_write_response(*msgr, msg.from);
                                        break;
                                case MSG_TYPE::AppendEntriesRsp:
                                        raft_leader_handle_append_response(*msgr);
                                        break;
                                default:
                                        ERROR("Unknown message received:%d\n", msgr->type);
                                        break;
                                } 
                        }
                        if (should_heartbeat()) {
                                INFO("Sending Heartbeat\n");
                                raft_leader_heartbeat();
                                set_hbtimer();
                        }
                }
                return 0;
        }
}