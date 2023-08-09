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
        public:
                inline Entry& last() {
                        return list[lastIndex%ENTRY_LIST_LEN];
                }

                inline Entry& index(int index) {
                        return list[index%ENTRY_LIST_LEN];
                }

                int init();
                int reset_preceeded();
                bool check_last_entry(int _index, int term);
                bool should_vote(int _index, int term);
                int append_entry(int key, int value);
                int fill_append_entries(MSG_RAFT& msgr, int to);
                int update_entry(const Entry& e);
                int leader_commit(int id, int cindex);
                int follower_commit(int cindex);
                void print();
        };
}