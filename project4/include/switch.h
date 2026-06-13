#ifndef SWITCH_H
#define SWITCH_H

#include "common.h"

void cpu_switch_to_user(uint32_t entry, uint32_t user_stack_top);

#endif
