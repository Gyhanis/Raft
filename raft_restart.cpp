#include "socket.h"
#include "raft.h"
#include <errno.h>

namespace raft {
        int countdown;

        int raft_restart() {
                ERROR("Restarting Node\n");
                raft_rpc_shutdown();

                while(countdown > 0) {
                        WARNING("Restart in %d\n", countdown);
                        sleep(1);
                        countdown--;
                }
                raft_init(node_id);
                return 0;
        }
}