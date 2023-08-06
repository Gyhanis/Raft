#include <errno.h>

#include "socket.h"
#include "raft_rpc_client.h"
#include "def.h"
#include "log.h"

namespace raft_client {
        int leader = 0;
        int client_id = -1;

        int raft_init_client(int id) {
                client_id = id;
                return mysock::init_socket(id);
        }

        int raft_request_write(int key, int value) {
                mysock::MSG msg;
                raft_client::MSG_RAFT* msgr = (raft_client::MSG_RAFT*) msg.data;
                msgr->type = raft_client::MSG_TYPE::RequestWrite;
                msgr->wrqst.key = key;
                msgr->wrqst.value = value;
                int r = mysock::send(leader, msgr, sizeof(*msgr));
                if (r < 0) {
                        return -1;
                }
                r = recv(&msg);
                if (r < 0) {
                        ERROR("Error on receiving response: %s\n", strerror(errno));
                        return -1;
                }
                if (msgr->type != raft_client::MSG_TYPE::ResponseWrite) {
                        ERROR("Not a response?\n");
                        return -1;
                }
                if (msgr->wresp.written == 0) {
                        if (msgr->wresp.leader == leader) {
                                ERROR("Not written\n");
                                return -1;
                        } else {
                                WARNING("Need redirent\n");
                                leader = msgr->wresp.leader;
                                return -2;
                        }
                }
                INFO("Data written\n");
                return 0;
        }

        int raft_client_close() {
                return mysock::release_socket();
        }
}