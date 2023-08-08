#include "raft_entry.h"
#include "raft_msg.h"

#define ENTRY_LIST_LEN 2048

namespace raft {
        class EntryList {
        private:
                int lastApplied = 0;
                int commitIndex = 0;
                int lastIndex = 0;
                Entry list[ENTRY_LIST_LEN];
                int proceeded[NODE_CNT];

                bool is_full();
                int get_commitIndex();
                Entry& index(int index);
                Entry& last();
        public:
                int init();
                int append_entry(int key, int value);
                int fill_append_entries(MSG_RAFT& msgr, int to);
                int update_entry(const Entry& e);
                int leader_commit(int id, int cindex);
                int follower_commit(int cindex);
                void print();
        };
}