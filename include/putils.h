#pragma once

#include <errno.h>
#include <stdlib.h>
#include <sys/user.h>

unsigned long child_syscall(pid_t child_pid, struct user_regs_struct *regs,
                            unsigned long syscall_number, int num_args, ...);
void replace_buf(pid_t child_pid, void *src_ptr, unsigned long dest,
                 size_t len);
void child_cancel_syscall(pid_t child_pid, struct user_regs_struct *regs);
void child_continue_syscall(pid_t pid, struct user_regs_struct *regs);
void child_returns(pid_t child_pid, struct user_regs_struct *regs,
                   unsigned long ret_val);
unsigned long r_syscall(struct user_regs_struct *regs_ptr);
unsigned long r_arg1(struct user_regs_struct *regs_ptr);
unsigned long r_arg2(struct user_regs_struct *regs_ptr);
unsigned long r_arg3(struct user_regs_struct *regs_ptr);
unsigned long r_arg4(struct user_regs_struct *regs_ptr);
unsigned long r_arg5(struct user_regs_struct *regs_ptr);
unsigned long r_arg6(struct user_regs_struct *regs_ptr);
unsigned long r_return(struct user_regs_struct *regs_ptr);
void w_syscall(struct user_regs_struct *regs_ptr, unsigned long data);
void w_arg1(struct user_regs_struct *regs_ptr, unsigned long data);
void w_arg2(struct user_regs_struct *regs_ptr, unsigned long data);
void w_arg3(struct user_regs_struct *regs_ptr, unsigned long data);
void w_arg4(struct user_regs_struct *regs_ptr, unsigned long data);
void w_arg5(struct user_regs_struct *regs_ptr, unsigned long data);
void w_arg6(struct user_regs_struct *regs_ptr, unsigned long data);
void w_return(struct user_regs_struct *regs_ptr, unsigned long data);
char *read_child_c_string(void *raddr, pid_t child_pid);

#define NO_SUCH_PROCESS ESRCH

#define PTRACE_DELETE(request, pid, addr, data)                                \
    do {                                                                       \
        errno = 0;                                                             \
        if (ptrace((request), (pid), (addr), (data)) == -1) {                  \
            if (errno == NO_SUCH_PROCESS) {                                    \
                WARN("Process %d no longer exists", (pid));                    \
                delete_process((pid));                                         \
                continue;                                                      \
            } else {                                                           \
                perror("ptrace");                                              \
                exit(EXIT_FAILURE);                                            \
            }                                                                  \
        }                                                                      \
    } while (0)

#define PTRACE_RETURN(request, pid, addr, data)                                \
    do {                                                                       \
        errno = 0;                                                             \
        if (ptrace((request), (pid), (addr), (data)) == -1) {                  \
            if (errno == NO_SUCH_PROCESS) {                                    \
                WARN("Process %d no longer exists", (pid));                    \
                return;                                                        \
            } else {                                                           \
                perror("ptrace");                                              \
                exit(EXIT_FAILURE);                                            \
            }                                                                  \
        }                                                                      \
    } while (0)