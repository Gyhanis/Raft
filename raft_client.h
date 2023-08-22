
namespace raft_client {
        enum MSG_TYPE {
                RequestWrite = 0x001,
                ResponseWrite,
                RequestRestart,
                ResponseRestart,
        };

        struct MSG_RAFT {
                MSG_TYPE type;
                union {
                        struct {
                                int cid;
                                int rid;
                                int key;
                                int value;
                        } wrqst;
                        struct {
                                int rid;
                                int written;
                                int leader;
                        } wresp;
                        struct {
                                int cid;
                                int rid;
                                int sec;
                        } restart;
                        struct {
                                int rid;
                                int success;
                        } restart_resp;
                };
        };

        int raft_init_client(int id);
        int raft_request_write_once(int key, int value);
        int raft_request_write(int key, int value, int times);
        int raft_restart(int id, int sec);
        int raft_client_cli_loop();
        int raft_client_close();
}