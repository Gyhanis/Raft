#include <errno.h>

#include "socket.h"
#include "raft_rpc_client.h"
#include "def.h"
#include "log.h"

namespace raft_client {
        int leader = 0;
        int client_id = -1;
        int request_id = 0;

        int raft_init_client(int id) {
                client_id = id;
                return mysock::init_socket(id);
        }

        int raft_request_write_once(int key, int value) {
                mysock::MSG msg;
                raft_client::MSG_RAFT* msgr = (raft_client::MSG_RAFT*) msg.data;
                msgr->type = raft_client::MSG_TYPE::RequestWrite;
                msgr->wrqst.cid = client_id;
                msgr->wrqst.rid = request_id;
                msgr->wrqst.key = key;
                msgr->wrqst.value = value;
                int r = mysock::send(leader, msgr, sizeof(*msgr));
                if (r < 0) {
                        return -1;
                }
                do {
                        r = recv(&msg);
                        if (r < 0) {
                                WARNING("Error on receiving response: %s\n", strerror(errno));
                                return -1;
                        }
                        if (msgr->type != raft_client::MSG_TYPE::ResponseWrite) {
                                ERROR("Not a response?\n");
                                return -2;
                        }
                } while (msgr->wresp.rid != request_id);
                if (msgr->wresp.written == 0) {
                        if (msgr->wresp.leader == leader) {
                                WARNING("Not written\n");
                                return -3;
                        } else if (msgr->wresp.leader == -1) {
                                WARNING("Not Available\n");
                                return -4;
                        } else {
                                WARNING("Redirect to %d\n", msgr->wresp.leader);
                                leader = msgr->wresp.leader;
                                return -5;
                        }
                }
                request_id++;
                INFO("Data written %d -> %d\n", key, value);
                return 0;
        }

        int raft_request_write(int key, int value, int times) {
                int r;
                for (int i = 0; i < times; i++) {
                        r = raft_request_write_once(key, value);
                        if (r == 0) break;
                        switch (r) {
                        case -1: // try other nodes
                                leader = (leader + 1) % NODE_CNT;
                                break;
                        case -3: // wati for digestion
                        case -4: // wait for election 
                                sleep(2);
                                break;
                        case -2: // retry
                        case -5: // redirect
                                break;
                        }
                }
                return r;
        }

        int raft_client_close() {
                return mysock::release_socket();
        }
}