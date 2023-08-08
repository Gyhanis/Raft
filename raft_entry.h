#pragma once

namespace raft {
        struct Entry {
                int index;
                int term;
                int key;
                int value;
        };
}