//
// Created by grisha on 12.11.23.
//

#include "LockWithVersion.h"

LockWithVersion::LockWithVersion() {
    vlock.store(0);
}

bool LockWithVersion::try_lock() {
    int cur_vlock = vlock.load();
    if (cur_vlock & 1) {
        return false;
    }
//    int old_v = ((cur_vlock >> 1) << 1) | (0);
//    int new_v = ((cur_vlock >> 1) << 1) | (1);
    return vlock.compare_exchange_strong(cur_vlock, cur_vlock + 1);
}

void LockWithVersion::unlock() {
    int cur_vlock = vlock.load();
    if (cur_vlock & 1) {
        vlock.store(cur_vlock - 1);
    }
}
