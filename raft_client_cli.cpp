#include <errno.h>
#include <iostream>

#include "raft_client.h"
#include "def.h"
#include "log.h"

namespace raft_client {
        int raft_client_cli_loop() {
                while (1) {
                        std::string cmd;
                        std::cin >> cmd;
                        if (cmd == "set") {
                                int key, value;
                                std::cin >> key >> value;
                                std::cout << "setting " << key << "->" << value << std::endl; 
                                if (raft_request_write(key, value, 5) == 0) {
                                        std::cout << "success\n";
                                } else {
                                        std::cout << "failed\n";
                                }
                        } else if (cmd == "get") {
                                int key;
                                std::cin >> key;
                                std::cout << "get " << key << std::endl;
                                std::cout << "Not implemented yet\n";
                        } else if (cmd == "restart") {
                                int node, sec;
                                std::cin >> node >> sec;
                                if (raft_restart(node, sec) == 0) {
                                        std::cout << "success\n";
                                } else {
                                        std::cout << "failed\n";
                                }
                        } else if (cmd == "exit") {
                                for (int i = 0; i < NODE_CNT; i++) {
                                        raft_restart(i, -1);
                                }
                                break;
                        } else {
                                std::cout << "unknown command: " << cmd << std::endl;
                        }
                }
                return 0;
        }
}
