
namespace raft_client {
        enum MSG_TYPE {
                RequestWrite = 0x001,
                ResponseWrite,
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
                };
        };

        int raft_init_client(int id);
        int raft_request_write_once(int key, int value);
        int raft_request_write(int key, int value, int times);
        int raft_client_close();
}