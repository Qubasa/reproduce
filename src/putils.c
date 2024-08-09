#include <dlfcn.h>
#include <errno.h>
#include <libgen.h>
#include <linux/unistd.h>
#include <stdarg.h>
#include <string.h>
#include <sys/auxv.h>
#include <sys/ptrace.h>
#include <sys/queue.h>
#include <sys/reg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <cstd.h>
#include <putils.h>

void child_exec_syscall(pid_t child_pid, struct user_regs_struct *regs) {

    // Set the registers of the child process
    PTRACE_RETURN(PTRACE_SETREGS, child_pid, NULL, regs);

    // Resume the child process
    PTRACE_RETURN(PTRACE_SYSCALL, child_pid, NULL, NULL);

    int status = 0;
    if (waitpid(child_pid, &status, 0) == -1) {
        perror("waitpid");
        exit(EXIT_FAILURE);
    }

    if (WIFEXITED(status) || WIFSIGNALED(status)) {
        ERROR("Target process exited after syscall\n");
        exit(EXIT_FAILURE);
    }

    // Get the registers of the child process
    PTRACE_RETURN(PTRACE_GETREGS, child_pid, NULL, regs);
}

// Generic function to make a syscall in the child process
unsigned long child_syscall(pid_t child_pid, struct user_regs_struct *regs,
                            unsigned long syscall_number, int num_args, ...) {
    // Set the syscall number and arguments
    w_syscall(regs, syscall_number);

    va_list args;
    va_start(args, num_args);
    if (num_args > 0)
        w_arg1(regs, va_arg(args, unsigned long));
    if (num_args > 1)
        w_arg2(regs, va_arg(args, unsigned long));
    if (num_args > 2)
        w_arg3(regs, va_arg(args, unsigned long));
    if (num_args > 3)
        w_arg4(regs, va_arg(args, unsigned long));
    if (num_args > 4)
        w_arg5(regs, va_arg(args, unsigned long));
    if (num_args > 5)
        w_arg6(regs, va_arg(args, unsigned long));
    va_end(args);

    // Execute the syscall
    child_exec_syscall(child_pid, regs);

    // Return the result of the syscall
    return r_return(regs);
}

// Read registers
unsigned long r_syscall(struct user_regs_struct *regs_ptr) {
    return regs_ptr->orig_rax;
}
unsigned long r_arg1(struct user_regs_struct *regs_ptr) {
    return regs_ptr->rdi;
}

unsigned long r_arg2(struct user_regs_struct *regs_ptr) {
    return regs_ptr->rsi;
}

unsigned long r_arg3(struct user_regs_struct *regs_ptr) {
    return regs_ptr->rdx;
}

unsigned long r_arg4(struct user_regs_struct *regs_ptr) {
    return regs_ptr->rcx;
}

unsigned long r_arg5(struct user_regs_struct *regs_ptr) { return regs_ptr->r8; }

unsigned long r_arg6(struct user_regs_struct *regs_ptr) { return regs_ptr->r9; }

unsigned long r_return(struct user_regs_struct *regs_ptr) {
    return regs_ptr->rax;
}

// Write registers
void w_syscall(struct user_regs_struct *regs_ptr, unsigned long data) {
    regs_ptr->orig_rax = data;
}
void w_arg1(struct user_regs_struct *regs_ptr, unsigned long data) {
    regs_ptr->rdi = data;
}
void w_arg2(struct user_regs_struct *regs_ptr, unsigned long data) {
    regs_ptr->rsi = data;
}
void w_arg3(struct user_regs_struct *regs_ptr, unsigned long data) {
    regs_ptr->rdx = data;
}
void w_arg4(struct user_regs_struct *regs_ptr, unsigned long data) {
    regs_ptr->rcx = data;
}
void w_arg5(struct user_regs_struct *regs_ptr, unsigned long data) {
    regs_ptr->r8 = data;
}
void w_arg6(struct user_regs_struct *regs_ptr, unsigned long data) {
    regs_ptr->r9 = data;
}
void w_return(struct user_regs_struct *regs_ptr, unsigned long data) {
    regs_ptr->rax = data;
}

#define MAX_BUFFER_SIZE 1024
char *read_child_c_string(void *raddr, pid_t child_pid) {
    static char buffer[MAX_BUFFER_SIZE];
    size_t length = 0;
    int done = 0;

    // Read long-sized chunks of memory at a time.
    while (!done && length < MAX_BUFFER_SIZE - 1) {
        errno = 0;
        long result = ptrace(PTRACE_PEEKDATA, child_pid, raddr, NULL);
        if (result == -1) {
            if (errno == NO_SUCH_PROCESS) {
                WARN("Process %d no longer exists", child_pid);
                return NULL;
            }
            perror("ptrace(PTRACE_PEEKDATA)");
            exit(EXIT_FAILURE);
        }
        const char *p = (const char *)&result;
        size_t bytesRead = strnlen(p, sizeof(long));
        if (sizeof(long) != bytesRead) {
            done = 1;
        }

        // Check if the buffer can hold the new characters
        if (length + bytesRead >= MAX_BUFFER_SIZE - 1) {
            ERROR("Buffer overflow while reading child string\n");
            bytesRead = MAX_BUFFER_SIZE - 1 - length;
            done = 1;
        }

        for (unsigned i = 0; i < bytesRead; i++) {
            buffer[length + i] = p[i];
        }

        length += bytesRead;
        raddr = (void *)((char *)raddr + bytesRead);
    }

    buffer[length] = '\0'; // Null-terminate the string
    return buffer;
}

void child_returns(pid_t child_pid, struct user_regs_struct *regs,
                   unsigned long data) {
    w_return(regs, data);
    PTRACE_RETURN(PTRACE_SETREGS, child_pid, 0, regs);
}

void replace_buf(pid_t pid, void *src_ptr, unsigned long dest, size_t len) {
    unsigned char *src = (unsigned char *)src_ptr;

    size_t i;
    for (i = 0; i + sizeof(unsigned long) <= len; i += sizeof(unsigned long)) {
        // Generate a random word (typically the size of a long)
        unsigned long word = *(unsigned long *)(src + i);

        //  Write the random word into the target process's memory at address
        //  addr + i
        PTRACE_RETURN(PTRACE_POKEDATA, pid, dest + i, word);
    }

    if (i < len) {
        // Read the remaining bytes
        errno = 0;
        unsigned long word = ptrace(PTRACE_PEEKDATA, pid, dest + i, NULL);
        if (word == (unsigned long)-1 && errno != 0) {
            if (errno == NO_SUCH_PROCESS) {
                WARN("Process %d no longer exists", pid);
                return;
            }
            perror("ptrace(PTRACE_PEEKDATA)");
            exit(EXIT_FAILURE);
        }
        // Write the remaining bytes
        unsigned char *word_ptr = (unsigned char *)&word;
        for (size_t j = 0; j < sizeof(word); j++) {
            if (i + j < len) {
                word_ptr[j] = src[i + j];
            }
        }

        PTRACE_RETURN(PTRACE_POKEDATA, pid, dest + i, word);
    }
}

void child_cancel_syscall(pid_t pid, struct user_regs_struct *regs) {
    int status = 0;
    regs->orig_rax =
        (unsigned long long)-1; // Replace the system call number with noop
    PTRACE_RETURN(PTRACE_SETREGS, pid, 0, regs);

    // Resume child process and stop child at the next system call
    PTRACE_RETURN(PTRACE_SYSCALL, pid, 0, 0);

    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        exit(EXIT_FAILURE);
    }
    if (WIFEXITED(status) || WIFSIGNALED(status)) {
        fprintf(stderr, "Target process exited after noop syscall\n");
        exit(EXIT_FAILURE);
    }
}

void child_continue_syscall(pid_t pid, struct user_regs_struct *regs) {
    int status = 0;
    // Resume child process and stop child at the next system call
    PTRACE_RETURN(PTRACE_SYSCALL, pid, NULL, NULL);

    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        exit(EXIT_FAILURE);
    }
    if (WIFEXITED(status) || WIFSIGNALED(status)) {
        fprintf(stderr, "Target process exited after noop syscall\n");
        exit(EXIT_FAILURE);
    }
}