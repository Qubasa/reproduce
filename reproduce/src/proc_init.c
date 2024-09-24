
#include <fcntl.h>
#include <proc_init.h>
#include <putils.h>
#include <sys/ptrace.h>
#include <unistd.h>

#include <cstd.h>
#include <vdso.h>

void proc_init(pid_t child_pid) {
    DEBUG("Init process %d\n", child_pid);
    remove_vdso(child_pid);

    // Continue tracing the child process and stop it at the next
    // system call
    PTRACE_RETURN(PTRACE_SYSCALL, child_pid, NULL, NULL);
}