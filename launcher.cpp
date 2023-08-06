#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "def.h"
#include "raft_rpc_client.h"

int sock;

int main() {
        char arg[16];
        for (int i = 0; i < NODE_CNT; i++) {
                sprintf(arg, "%d", i);
                int pid = fork();
                if (pid == 0) {
                        execl("./rnode", "./rnode", arg, NULL);
                } 
        }
        raft_client::raft_init_client(-1);
        sleep(1);
        raft_client::raft_request_write(1,1);
        raft_client::raft_client_close();
        return 0;
}
