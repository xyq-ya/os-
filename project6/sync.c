#include "common.h"
#include "sync.h"

static volatile int bank_mutex = 1;

void sync_init(void) {
    bank_mutex = 1;
}

int sync_try_P(void) {
    if (bank_mutex <= 0) {
        return 0;
    }

    bank_mutex--;
    return 1;
}

void sync_V(void) {
    bank_mutex = 1;
}
