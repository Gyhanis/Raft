#include "raft.h"

namespace raft {
        inline bool EntryList::is_full() {
                return lastIndex - lastApplied >= ENTRY_LIST_LEN;
        }

        inline Entry& EntryList::last() {
                return list[lastIndex%ENTRY_LIST_LEN];
        }

        inline Entry& EntryList::index(int index) {
                return list[index%ENTRY_LIST_LEN];
        }

        int EntryList::init() {
                for (int i = 0; i < NODE_CNT; i++) {
                        proceeded[i] = 0;
                }
                return 0;
        }

        int EntryList::append_entry(int key, int value) {
                if (is_full()) {
                        return -1;
                } 
                lastIndex++;
                list[lastIndex%ENTRY_LIST_LEN] = 
                        Entry{lastIndex, currentTerm, key, value};
                return 0;
        }

        int EntryList::fill_append_entries(MSG_RAFT& msgr, int to) {
                msgr.append.prevLogIndex = proceeded[to];
                msgr.append.prevLogTerm = index(proceeded[to]).term;
                msgr.append.leaderCommit = commitIndex;
                msgr.append.entryCnt = lastIndex - proceeded[to];
                if (msgr.append.entryCnt > 4) 
                        msgr.append.entryCnt = 4;
                for (int i = 0; i < msgr.append.entryCnt; i++) {
                        msgr.append.entries[i] = index(proceeded[to]+1+i);
                }
                return 0;
        }

        int EntryList::update_entry(const Entry& e) {
                if (e.index == lastIndex + 1) {
                        if (!is_full()) {
                                lastIndex++;
                                index(lastIndex) = e;
                        } else {
                                WARNING("Log list is full, append rejected\n");
                                return -1;
                        }
                } else if (commitIndex < e.index && e.index <= lastIndex) {
                        Entry& ec = index(e.index);
                        if (ec.term < e.term) {
                                lastIndex = e.index;
                                ec = e;
                        } else if (ec.term == e.term) {
                                WARNING("Repeated update\n");
                        } else {
                                ERROR("Attempting to update with outdated data\n");
                                return -1;
                        }
                } else if (e.index <= commitIndex) {
                        ERROR("Attempting to modify commited Index\n");
                        return -1;
                } else if (e.index > lastIndex + 1) {
                        ERROR("Invalid index\n");
                        return -1;
                }
                return 0;
        }

        int EntryList::leader_commit(int id, int cindex) {
                if (cindex > proceeded[id]) {
                        if (cindex > lastIndex) {
                                ERROR("What the fuck?\n");
                                return -1;
                        }
                        proceeded[id] = cindex;
                } else if (cindex < proceeded[id]) {
                        WARNING("recommit?\n");
                        return -1;
                }
                for (int i = 0; i < NODE_CNT; i++) {
                        if (proceeded[i] <= commitIndex) {
                                continue;
                        }
                        int cnt = 0;
                        for (int j = 0; j < NODE_CNT; j++) {
                                if (proceeded[j] >= proceeded[i]) {
                                        cnt++;
                                }
                        }
                        if (cnt >= NODE_MAJOR-1) {
                                commitIndex = proceeded[i];
                        }
                }
                lastApplied = commitIndex;
                return 0;
        }

        int EntryList::follower_commit(int cindex) {
                if (cindex > lastIndex) {
                        commitIndex = lastIndex;
                } else {
                        commitIndex = cindex;
                }
                lastApplied = commitIndex;
                return 0;
        }

        void EntryList::print() {
                INFO("======== Log State ===========\n");
                INFO("lastApplied:%d\n", lastApplied);
                INFO("commitIndex:%d\n", commitIndex);
                INFO("lastIndex:%d\n", lastIndex);
                INFO("uncommited entries:\n");
                for (int i = commitIndex+1; i <= lastIndex; i++) {
                        INFO("(%d) %d:\t%d\n", i, index(i).key, index(i).value);
                }
                if (role == Role::Leader) {
                        INFO("Proceeded: ");
                        for (int i = 0; i < NODE_CNT; i++) {
                                if (i == node_id) {
                                        INFO("%d ", lastIndex);
                                } else {
                                        INFO("%d ", proceeded[i]);
                                }
                        }
                }
                INFO("\n");
        }
}