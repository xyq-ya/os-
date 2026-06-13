#ifndef SHM_H
#define SHM_H

#include "common.h"

#define SHM_BANK_ADDR 0x300000u

void shm_init(void);
int32_t* shm_bank_balance(void);

#endif
