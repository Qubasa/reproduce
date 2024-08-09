#include <hashtable.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <cstd.h>

// Define the child_process struct

// Head of the hash table
static struct child_process *process_table = NULL;

// Global counter to track the number of entries
static unsigned int entry_count = 0;

// Find a process by pid
__attribute__((no_sanitize("shift", "integer"))) struct child_process *
find_process(pid_t pid) {
    struct child_process *proc;
    HASH_FIND_INT(process_table, &pid, proc);
    return proc;
}

// Insert a process into the hash table
__attribute__((no_sanitize("shift", "integer"))) bool
insert_process(pid_t pid) {
    if (find_process(pid)) {
        WARN("Process with pid: %d already exists", pid);
        return false;
    }
    DEBUG("Inserting process with pid: %d", pid);
    struct child_process *proc = malloc(sizeof(struct child_process));
    if (!proc) {
        ERROR("Failed to allocate memory for child process");
        exit(EXIT_FAILURE);
    }
    proc->pid = pid;
    proc->gettime_data.tv_sec = 1718099106;
    proc->gettime_data.tv_nsec = 516453925;

    HASH_ADD_INT(process_table, pid, proc);
    entry_count++; // Increment the counter when inserting an entry
    return true;
}

// Delete a process by pid
__attribute__((no_sanitize("shift", "integer"))) void
delete_process(pid_t pid) {
    DEBUG("Deleting process with pid: %d", pid);
    struct child_process *proc = find_process(pid);
    if (proc) {
        HASH_DEL(process_table, proc);
        free(proc);    // Free the memory allocated for the struct
        entry_count--; // Decrement the counter when deleting an entry
    }
}

// print hashtable
void print_hashtable() {
    struct child_process *proc, *tmp;
    HASH_ITER(hh, process_table, proc, tmp) { DEBUG("pid: %d", proc->pid); }
}

// Check if the hash table is empty
bool is_hash_table_empty() { return entry_count == 0; }
