#pragma once
#include "raft_entry.h"

namespace raft {
        enum MSG_TYPE {
                NullMsg         = 0x000,
                RequestWrite    = 0x001,
                ResponseWrite,

                RequestVote     = 0x101, 
                RequestVoteRsp,
                AppendEntries,
                AppendEntriesRsp,
        };

        struct MSG_RAFT {
                MSG_TYPE type;
                union {
                        struct {
                                int cid;
                                int rid;
                                int key;
                                int value;
                        } wrqst;
                        struct {
                                int rid;
                                int written;
                                int leader;
                        } wresp;
                        struct {
                                int id;
                                int term;
                                int prevLogIndex;
                                int prevLogTerm;
                                int leaderCommit;
                                int entryCnt;
                                Entry entries[4];
                        } append;
                        struct {
                                int id;
                                int term;
                                int success;
                        } apresp;
                        struct {
                                int id;
                                int term;
                                int lastLogIndex;
                                int lastLogTerm;
                        } vrqst;
                        struct {
                                int id;
                                int term;
                                int voteGranted;
                        } vresp;
                };
        };
}