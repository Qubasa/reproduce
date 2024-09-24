#pragma once

#include <cstd.h>
#include <search.h>
#include <time.h>
#include <unistd.h>
#include <uthash.h>

struct child_process {
    pid_t pid; // Key
    struct timespec gettime_data;
    UT_hash_handle hh; // Makes this structure hashable
};

bool insert_process(pid_t pid);
struct child_process *find_process(pid_t pid);
void delete_process(pid_t pid);
void print_hashtable();
bool is_hash_table_empty();