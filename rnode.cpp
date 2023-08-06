#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "socket.h"
#include "raft.h"
#include "def.h"
#include "log.h"

char greeting[] = "ping ?";
char response[] = "pong !";

char path[128];

int main(int argc, char* argv[]) {
        if (argc < 2) {
                ERROR("node id required\n");
                exit(0);
        } 
        int id = atoi(argv[1]);
        if (id < 0 || id > 2) {
                ERROR("invalid node id\n");
                exit(0);
        }
        close(2);
        sprintf(path, "log/%s", argv[1]);
        open(path, O_CREAT|O_TRUNC|O_WRONLY, 0666);

        raft::raft_rpc_init(id);

        mysock::MSG msg;
        int r = mysock::recv(&msg); 
        if (r > 0) {
                raft::MSG_RAFT *msgr = (raft::MSG_RAFT*) msg.data;
                raft::raft_response_write(*msgr, msg.from);
        }
        // INFO("boardcasting greeting\n");
        // mysock::boardcast(greeting, sizeof(greeting));
        // mysock::MSG msg_rcv;
        // while (recv(&msg_rcv) >= 0) {
        //         INFO("receiving from %d: %s\n", msg_rcv.from, msg_rcv.data);
        //         if (__builtin_strcmp(msg_rcv.data, greeting) == 0) {
        //                 mysock::send(msg_rcv.from, response, sizeof(response));
        //         }
        // }
        // INFO("timeout, exiting\n");

        INFO("shutting down\n");
        raft::raft_rpc_shutdown();
        return 0;
}

