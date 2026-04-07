#pragma once

#include "kernel/mem.h"

void run_elf(const char* name);
_Noreturn void killproc();
pde_t* current_user_pgdir(void);
