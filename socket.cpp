#include <errno.h>

#include "socket.h"
#include "def.h"
#include "log.h"


namespace mysock {
        const struct sockaddr_un _addrs_un[] = {
                {AF_LOCAL, "./socket_file/c1"},
                {AF_LOCAL, "./socket_file/0"},
                {AF_LOCAL, "./socket_file/1"},
                {AF_LOCAL, "./socket_file/2"},
                {AF_LOCAL, "./socket_file/3"},
                {AF_LOCAL, "./socket_file/4"},
        };
        const struct sockaddr_un* addrs_un = &_addrs_un[1];
        #define ADDR_SIZE       sizeof(addrs_un[0])

        const struct sockaddr* _addrs[] = {
                (const struct sockaddr*) &_addrs_un[0],
                (const struct sockaddr*) &_addrs_un[1],
                (const struct sockaddr*) &_addrs_un[2],
                (const struct sockaddr*) &_addrs_un[3],
                (const struct sockaddr*) &_addrs_un[4],
                (const struct sockaddr*) &_addrs_un[5],
        };

        const struct sockaddr** addrs = &_addrs[1];
        #define ADDR_MIN        -1
        #define ADDR_MAX        4

        int socket_id;
        int sock;

        int init_socket(int id) {
                socket_id = id;
                sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
                if (sock < 0) {
                        ERROR("socket: %s\n", strerror(errno));
                        exit(0);
                } 
                set_timeout(SOCKET_DEFAULT_TIMEOUT);
                DEBUG("binding to %s(%d)\n", ((struct sockaddr_un*)(addrs[socket_id]))->sun_path, socket_id);
                int r = bind(sock, addrs[socket_id], ADDR_SIZE);
                if (r < 0) {
                        close(sock);
                        ERROR("error: %s\n", strerror(errno));
                        exit(0);
                }
                return 0;
        }

        int set_timeout(int sec) {
                struct timeval tv = {sec, 0};
                int r = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
                if (r < 0) {
                        WARNING("set_timeout: %s\n", strerror(errno));
                }
                return r;
        }

        int send(int to, void* data, int data_len) {
                if (to == socket_id) {
                        WARNING("sending to myself?\n");
                        return 0;
                } else if (to > ADDR_MAX || to < ADDR_MIN) {
                        ERROR("invalid address\n");
                        return -1;
                }
                MSG msg;
                msg.from = socket_id;
                msg.to = to;
                msg.data_len = data_len;
                __builtin_memcpy(msg.data, data, data_len);
                int r = sendto(sock, &msg, sizeof(msg), 0, addrs[to], ADDR_SIZE);
                if (r < 0) {
                        WARNING("sendto: %s\n", strerror(errno));
                }
                return r;
        }

        int boardcast(void* data, int data_len) {
                MSG msg;
                msg.from = socket_id;
                msg.data_len = data_len;
                __builtin_memcpy(msg.data, data, data_len);
                for (int i = 0; i < NODE_CNT; i++) {
                        if (i == socket_id) continue;
                        msg.to = i;
                        int r = sendto(sock, &msg, sizeof(msg), 0, addrs[i], ADDR_SIZE);
                        if (r < 0) {
                                WARNING("boardcast(to %d): %s\n", i, strerror(errno));
                        }
                }
                return 0;
        }

        int recv(MSG* msg) {
                DEBUG("reading\n");
                int r = read(sock, msg, sizeof(*msg));
                DEBUG("read finished with %d\n", r);
                if (r < 0) {
                        DEBUG("recv: %s\n", strerror(errno));
                } else if (msg->to != socket_id) {
                        WARNING("recv: msg not for me?\n");
                        return -1;
                }
                DEBUG("recv returning\n");
                return r;
        }

        int release_socket() {
                DEBUG("Releasing socket\n");
                close(sock);
                // INFO("unlinking %s(%d)\n", addrs_un[socket_id].sun_path, socket_id);
                unlink(addrs_un[socket_id].sun_path);
                return 0;
        }
}
