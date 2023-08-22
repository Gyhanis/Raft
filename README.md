# Simple Implementation of RAFT

This is a simple implementation of raft for study. Each server node and client is simulated with a process in local machine, and communicates with each other using socket (AF_LOCAL).

## Quick Start
1. Create _log_ directory, where the raft nodes output their logs, and _socket_file_ directory, where the socket file will be created for the nodes and client to communicate.
        
        mkdir log socket_file

2. Run make to build the _launcher_ and _rnode_.

        make 

3. Run _launcher_, which will launch the nodes and then turn itself into a client that sends a few write request automatically.

        ./launcher 

4. Logs of nodes can be found in _log_ directory, and the logs of launcher is in stderr.

5. ~~The nodes are expected to stop within a few minutes. If they don't,~~ use the following command to kill them.

        pkill rnode

