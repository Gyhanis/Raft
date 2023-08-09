#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

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
        raft::raft_loop(id);
        return 0;
}

