#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "def.h"
#include "raft_rpc_client.h"

int sock;

int main() {
        char arg[16];
        // timespec ts;
        // clock_gettime(CLOCK_MONOTONIC, &ts);
        // printf("%ld.%09ld\n", ts.tv_sec, ts.tv_nsec);
        // long i = clock();
        // printf("%ld\n", i);
        for (int i = 0; i < NODE_CNT; i++) {
                sprintf(arg, "%d", i);
                int pid = fork();
                if (pid == 0) {
                        execl("./rnode", "./rnode", arg, NULL);
                } 
        }
        raft_client::raft_init_client(-1);
        for (int i = 1; i <= 5; i++) {
                sleep(2);
                raft_client::raft_request_write(i,2*i);
        }
        raft_client::raft_client_close();
        return 0;
}
