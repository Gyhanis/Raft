#include "socket.h"
#include "raft.h"
#include <errno.h>

namespace raft {
        int countdown;

        int raft_restart(int sec) {
                ERROR("Restarting Node\n");
                raft_rpc_shutdown();

                for (int cd = sec; cd > 0; cd--) {
                        WARNING("Restart in %d\n", cd);
                        sleep(1);
                }
                raft_init(node_id);
                return 0;
        }
}