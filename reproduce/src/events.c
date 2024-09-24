
#include "proc_init.h"
#include <dlfcn.h>
#include <errno.h>
#include <libgen.h>
#include <linux/unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <cstd.h>
#include <events.h>
#include <hashtable.h>
#include <putils.h>
#include <vdso.h>

void print_event(int event, pid_t pid) {
    switch (event) {
    case PTRACE_EVENT_FORK:
        DEBUG("PTRACE_EVENT_FORK %d", pid);
        break;
    case PTRACE_EVENT_VFORK:
        DEBUG("PTRACE_EVENT_VFORK %d", pid);
        break;
    case PTRACE_EVENT_CLONE:
        DEBUG("PTRACE_EVENT_CLONE %d", pid);
        break;
    case PTRACE_EVENT_EXEC:
        DEBUG("PTRACE_EVENT_EXEC %d", pid);
        break;
    case PTRACE_EVENT_EXIT:
        DEBUG("PTRACE_EVENT_EXIT %d", pid);
        break;
    default:
        break;
    }
}

void register_new_child(pid_t pid) {
    // Get the new child's pid
    unsigned long new_child;
    PTRACE_RETURN(PTRACE_GETEVENTMSG, pid, NULL, &new_child);

    // Add the new child to the list
    if (!insert_process(new_child)) {
        WARN("Skipping new child %d as it is already being tracked.", pid);
        return;
    }

    // Wait for the new child to stop
    int new_status;
    if (waitpid(new_child, &new_status, 0) == -1) {
        perror("waitpid");
        exit(EXIT_FAILURE);
    }

    PTRACE_RETURN(PTRACE_SYSCALL, (int)new_child, NULL, NULL);
}

void handle_event() {
    int status = 0;
    pid_t pid;

    while (!is_hash_table_empty()) {
        // Wait for any child process to change state
        pid = waitpid(-1, &status, __WALL);
        if (pid == -1) {
            perror("waitpid");
            // This happens when a side thread calls execve and replaces all
            // threads. ptrace does not see the side thread exiting in this edge
            // case.
            if (errno == ECHILD) {
                WARN("No more child processes to trace");
                print_hashtable();
                break;
            }
            exit(EXIT_FAILURE);
        }

        // If the process has exited or been signaled to exit
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            delete_process(pid);
            continue;
        }

        // If the process has stopped, check if it is a fork or clone event
        if (WIFSTOPPED(status)) {
            int event = (status >> 16) & 0xffff;
            print_event(event, pid);

            // Initialize the new process
            if (event == PTRACE_EVENT_EXEC) {
                proc_init(pid);
                continue;

                // Handle fork, vfork, and clone events
            } else if (event == PTRACE_EVENT_FORK ||
                       event == PTRACE_EVENT_VFORK ||
                       event == PTRACE_EVENT_CLONE) {
                register_new_child(pid);
            }
        }

        // Hook system calls
        {
            // Find the child process in the hash table
            struct child_process *child = find_process(pid);
            if (!child) {
                DEBUG("Unexpected process %d. Add to tracking.", pid);
                proc_init(pid);
                insert_process(pid);
                continue;
            }

            // Read all registers from the child process into regs
            struct user_regs_struct regs;
            PTRACE_DELETE(PTRACE_GETREGS, pid, NULL, &regs);
            // Handle hooked system calls
            handle_syscall(child, &regs);
        }

        // Resume the child process and stop it at the next system call
        PTRACE_DELETE(PTRACE_SYSCALL, pid, NULL, NULL);
    }
}
