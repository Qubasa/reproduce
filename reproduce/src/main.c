
#include <libgen.h>
#include <linux/unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/queue.h>
#include <sys/reg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <cstd.h>
#include <events.h>
#include <hashtable.h>
#include <sys/queue.h>

void initialize_seed() {
    const char *seed_str = getenv("R_SEED");
    unsigned int custom_seed = 123456890;
    if (seed_str) {
        custom_seed = (unsigned int)strtoul(seed_str, NULL, 10);
        DEBUG("Using custom seed: %u", custom_seed);
    }
    srandom(custom_seed);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <program> [args...]\n", argv[0]);
        return 1;
    }

    // Returns 0 if the current process is the child process
    // Returns the child's PID if the current process is the parent process
    pid_t child_pid = fork();

    initialize_seed();

    if (child_pid == 0) { // Child process

        // Notify the parent that the child is ready to be traced
        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
            perror("ptrace(PTRACE_TRACEME)");
            return 1;
        }
        kill(getpid(), SIGSTOP); // Stop the child to let the parent continue

        // execvp only returns on error
        execvp(argv[1], &argv[1]);
        perror("=========execvp===============");
        return 1;

    } else if (child_pid > 0) { // Parent process
        // print in yellow parent pid
        DEBUG("Tracer pid: %d", getpid());
        DEBUG("Child pid: %d", child_pid);
        int status;
        // Wait for the child process to stop
        if (waitpid(child_pid, &status, 0) == -1) {
            perror("waitpid");
            return 1;
        }

        // Check if the child process has exited
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            fprintf(stderr, "Target process exited.\n");
            return 0;
        }

        // Set options to trace fork, vfork, and clone
        if (ptrace(PTRACE_SETOPTIONS, child_pid, 0,
                   PTRACE_O_EXITKILL |      // Kill the child if the parent dies
                       PTRACE_O_TRACEFORK | // Generate a trace event on fork
                       PTRACE_O_TRACEVFORK | // Generate a trace event on vfork
                       PTRACE_O_TRACECLONE | // Generate a trace event on clone
                       PTRACE_O_TRACEEXEC    // Generate a trace event on exec
                   ) == -1) {
            perror("ptrace(PTRACE_SETOPTIONS)");
            return 1;
        }

        // Insert the child process into the hash table
        insert_process(child_pid);

        // Continue tracing the child process and stop it at the next system
        // call
        if (ptrace(PTRACE_SYSCALL, child_pid, NULL, NULL) == -1) {
            perror("ptrace(PTRACE_SYSCALL child_pid)");
            return 1;
        }

        // Event loop to handle child processes
        handle_event();

    } else {
        perror("fork");
        return 1;
    }
}
