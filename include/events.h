#pragma once

#include <cstd.h>
#include <hashtable.h>
#include <sys/types.h>
#include <sys/user.h>

void handle_syscall(struct child_process *child,
                    struct user_regs_struct *regs_ptr);
void handle_event();
