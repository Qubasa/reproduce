#include <dlfcn.h>
#include <libgen.h>
#include <linux/unistd.h>
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

/*
 * removeVDSO() alters the auxiliary table of a newly created process in order
 * to disable VDSO.
 */
void remove_vdso(pid_t child_pid) {
    long int pos;
    int zeroCount;
    long val;
    errno = 0;
    pos = ptrace(PTRACE_PEEKUSER, child_pid, sizeof(long int) * RSP, NULL);
    if (pos == -1) {
        if (errno == NO_SUCH_PROCESS) {
            WARN("Process %d no longer exists", child_pid);
            return;
        }

        perror("ptrace(PTRACE_PEEKUSER)");
        exit(EXIT_FAILURE);
    }

    /* skip to auxiliary vector */
    zeroCount = 0;
    while (zeroCount < 2) {
        errno = 0;
        val = ptrace(PTRACE_PEEKDATA, child_pid, pos += 8, NULL);
        if (val == -1) {
            if (errno == NO_SUCH_PROCESS) {
                WARN("Process %d no longer exists", child_pid);
                return;
            }
            perror("ptrace(PTRACE_PEEKDATA)");
            exit(EXIT_FAILURE);
        }
        if (val == AT_NULL) {
            zeroCount++;
        }
    }

    /* search the auxiliary vector for AT_SYSINFO_EHDR... */
    errno = 0;
    val = ptrace(PTRACE_PEEKDATA, child_pid, pos += 8, NULL);
    if (val == -1) {
        if (errno == NO_SUCH_PROCESS) {
            WARN("Process %d no longer exists", child_pid);
            return;
        }
        perror("ptrace(PTRACE_PEEKDATA)");
        exit(EXIT_FAILURE);
    }
    while (1) {
        if (val == AT_NULL)
            break;
        if (val == AT_SYSINFO_EHDR) {
            /* ... and overwrite it */
            errno = 0;
            if (ptrace(PTRACE_POKEDATA, child_pid, pos, AT_IGNORE) == -1) {
                if (errno == NO_SUCH_PROCESS) {
                    WARN("Process %d no longer exists", child_pid);
                    return;
                }
                perror("ptrace(PTRACE_POKEDATA)");
                exit(EXIT_FAILURE);
            }
            break;
        }
        errno = 0;
        val = ptrace(PTRACE_PEEKDATA, child_pid, pos += 16, NULL);
        if (val == -1) {
            if (errno == NO_SUCH_PROCESS) {
                WARN("Process %d no longer exists", child_pid);
                return;
            }
            perror("ptrace(PTRACE_PEEKDATA)");
            exit(EXIT_FAILURE);
        }
    }
}