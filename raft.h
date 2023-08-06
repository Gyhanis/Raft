namespace raft {

enum Role {
        Follower, Candidate, Leader, Dead
};

struct Entry {
        int index;
        int term;
        int key;
        int value;
        bool operator>=(const Entry& e2) const {
                if (term > e2.term) {
                        return true;
                } else if (term == e2.term && index >= e2.term) {
                        return true;
                } else {
                        return false;
                }
        }
};

extern int node_id;
extern int currentTerm;
extern int leader;
extern Role role; 

#define ENTRY_LIST_LEN 2048
struct EntryList {
        int lastApplied = 0;
        int commitIndex = 0;
        int lastIndex = 0;
        Entry list[ENTRY_LIST_LEN];
        int add_entry(int key, int value) {
                if (lastIndex - lastApplied >= ENTRY_LIST_LEN) {
                        return -1;
                } 
                lastIndex++;
                list[lastIndex%ENTRY_LIST_LEN] = Entry{lastIndex, currentTerm, key, value};
                return 0;
        }
        inline Entry& index(int index) {
                return list[index%ENTRY_LIST_LEN];
        }
        inline Entry& last() {
                return list[lastIndex%ENTRY_LIST_LEN];
        }
};

extern EntryList list;
extern int proceeding[];
extern int proceeded[];

}

#include "raft_rpc.h"
