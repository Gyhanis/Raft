CFLAG=-O2

all: launcher rnode 

.PHONY: clean

launcher: launcher.o raft_rpc_client.o socket.o
	g++ $^ -o $@

RAFT_HEADER=raft.h raft_entry.h raft_entlist.h raft_msg.h
launcher.o: 		raft_rpc_client.h
raft_entlist.o: 	$(RAFT_HEADER)
raft_follower.o: 	$(RAFT_HEADER) socket.h
raft_leader.o: 		$(RAFT_HEADER) socket.h
raft_candidate.o:	$(RAFT_HEADER) socket.h
raft_rpc_client.o: 	socket.h raft_rpc_client.h 
raft_rpc.o:		socket.h raft.h raft_msg.h
raft.o:			raft.h raft_msg.h
rnode.o:		raft.h
socket.o:		socket.h 

%.o: %.cpp def.h log.h
	g++ $(CFLAG) -c $< -o $@

rnode: rnode.o raft_leader.o raft_follower.o raft_candidate.o raft_rpc.o raft_entlist.o raft.o socket.o
	g++ $^ -o $@

clean:
	rm -f *.o 
	rm -f log/*
	rm -f socket_file/*
	rm -f launcher rnode
