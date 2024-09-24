#include <dlfcn.h>
#include <fcntl.h>
#include <libgen.h>
#include <linux/unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/queue.h>
#include <sys/reg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstd.h>
#include <events.h>
#include <putils.h>

void redirect_dev_random(pid_t child_pid, struct user_regs_struct *regs_ptr) {
    unsigned long openat_path = r_arg2(regs_ptr);
    char *openat_path_str = read_child_c_string((void *)openat_path, child_pid);
    if (strcmp(openat_path_str, "/dev/random") == 0 ||
        strcmp(openat_path_str, "/dev/urandom") == 0) {
        WARN("Redirecting openat from %s to stdin. Waiting for input...",
             openat_path_str);
        child_continue_syscall(child_pid, regs_ptr);
        child_returns(child_pid, regs_ptr, stdin->_fileno);
    }
}

// Remeber that the child process stop before and after a syscall.
void handle_syscall(struct child_process *child,
                    struct user_regs_struct *regs_ptr) {
    pid_t child_pid = child->pid;
    /* Filter system calls */
    // getrandom
    switch (r_syscall(regs_ptr)) {
    case __NR_getrandom:;
        DEBUG("hooked syscall getrandom");

        unsigned long buf_addr = r_arg1(regs_ptr);
        unsigned long buf_len = r_arg2(regs_ptr);
        child_cancel_syscall(child_pid, regs_ptr);

        void *testdata = malloc(buf_len);
        memset(testdata, 0x22, buf_len);
        replace_buf(child_pid, testdata, buf_addr, buf_len);
        free(testdata);

        child_returns(child_pid, regs_ptr, buf_len);
        break;

    case __NR_gettimeofday:;
        DEBUG("hooked syscall gettimeofday");

        unsigned long timeval_addr = r_arg1(regs_ptr);
        child_cancel_syscall(child_pid, regs_ptr);

        replace_buf(child_pid, &child->gettime_data, timeval_addr,
                    sizeof(child->gettime_data));

        child->gettime_data.tv_sec += 1;
        child->gettime_data.tv_nsec += 1;

        child_returns(child_pid, regs_ptr, 0);
        break;

    case __NR_time:;
        DEBUG("hooked syscall time");

        unsigned long time_arg = r_arg1(regs_ptr);
        child_cancel_syscall(child_pid, regs_ptr);

        child->gettime_data.tv_sec += 1;
        child->gettime_data.tv_nsec += 1;

        if (time_arg != 0) {
            replace_buf(child_pid, &child->gettime_data, time_arg,
                        sizeof(child->gettime_data));
        }

        child_returns(child_pid, regs_ptr, child->gettime_data.tv_sec);
        break;

    case __NR_clock_gettime:;
        // DEBUG("hooked syscall clock_gettime");

        unsigned long clock_gettime_arg = r_arg2(regs_ptr);
        child_cancel_syscall(child_pid, regs_ptr);

        replace_buf(child_pid, &child->gettime_data, clock_gettime_arg,
                    sizeof(child->gettime_data));

        child->gettime_data.tv_sec += 1;
        child->gettime_data.tv_nsec += 1;

        child_returns(child_pid, regs_ptr, 0);
        break;

    // open
    case __NR_openat:;
        redirect_dev_random(child_pid, regs_ptr);
        break;
    case __NR_open:
        DEBUG("hooked syscall open");
        redirect_dev_random(child_pid, regs_ptr);
        break;
    case __NR_openat2:
        DEBUG("hooked syscall openat2");
        redirect_dev_random(child_pid, regs_ptr);
        break;

    default:
        break;
    }
}