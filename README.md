# Simple Implementation of RAFT

This is a simple implementation of raft for study. Each server node and client is simulated with a process in local machine, and communicates with each other using socket (AF_LOCAL).

## Quick Start
1. Create _log_ directory, where the raft nodes output their logs, and _socket_file_ directory, where the socket file will be created for the nodes and client to communicate.
        
        mkdir log socket_file

2. Run make to build the _launcher_ and _rnode_.

        make 

3. Run _launcher_, which will launch the nodes and then turn itself into a client that sends 5 write requests automatically.

        ./launcher 

4. Logs of nodes can be found in _log_ directory, and the logs of launcher is in stderr.

5. After 5 requests are sent, the client reads the stdin for further commands. The commands will be introduced later.

6. In case the raft nodes are not shutted down as expected, use the following bash command to kill them.

        pkill rnode

## Client commands

After the launcher started the node and sent 5 write requests, it starts to read commands from stdin. Currently available commands are listed below:

* `set <key> <value>` Set a kv pair. Both key and value should be integers.
* `restart <node> <sec>` Shutdown a node and restart it in `sec` seconds. Valid `node`s are 0,1,2.
* `restart <node> -1` Shutdown a node without restarting it.
* `exit` Shutdown all nodes and terminate the client.

The command line interface is **NOT** rubost, and may run into unexpected states if a command is illegal.