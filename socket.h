#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SOCKET_DEFAULT_TIMEOUT 10

extern int socket_id;


namespace mysock {
        union MSG {
                struct {
                        int from;
                        int to;
                        int data_len;
                        char data[0];
                };
                char raw[128];
        };

        int init_socket(int id);
        int set_timeout(int sec);
        int send(int to, void* data, int data_len);
        int boardcast(void* data, int data_len);
        int recv(MSG* msg);
        int release_socket();
}
