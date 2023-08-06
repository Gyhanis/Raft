
all: launcher rnode 

.PHONY: clean

launcher: launcher.o raft_rpc_client.o socket.o
	g++ $^ -o $@

%.o: %.cpp
	g++ -c $< -o $@

rnode: rnode.o raft_rpc.o raft.o socket.o
	g++ $^ -o $@

clean:
	rm -f *.o 
	rm -f log/*
	rm -f socket_file/*
	rm -f launcher rnode
