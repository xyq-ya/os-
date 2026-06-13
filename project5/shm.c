#include "common.h"
#include "print.h"
#include "shm.h"

typedef struct bank_shm {
    int32_t balance;
} bank_shm_t;//一个银行余额的结构体

void shm_init(void) {
    bank_shm_t* bank = (bank_shm_t*)SHM_BANK_ADDR;
    bank->balance = 1000;
    kprintf("Shared memory bank init at 0x%x, balance=%d\n",
            SHM_BANK_ADDR, (int)bank->balance);
}//初始化共享内存中的银行余额

int32_t* shm_bank_balance(void) {
    return &((bank_shm_t*)SHM_BANK_ADDR)->balance;
}//获取共享内存中银行余额的指针
