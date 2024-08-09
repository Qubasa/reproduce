#pragma once

#include <libgen.h>
#include <stdio.h>

#define YELLOW_TEXT "\033[1;33m"
#define BLUE_TEXT "\033[1;34m"
#define GRAY_TEXT "\033[1;30m"
#define RED_TEXT "\033[1;31m"
#define RESET_TEXT "\033[0m"

#define ERROR_PRINT_HELPER(fmt, ...)                                           \
    fprintf(stderr, RED_TEXT "%s:%d: " fmt RESET_TEXT "\n",                    \
            basename(__FILE__), __LINE__, ##__VA_ARGS__)
#define ERROR(...) ERROR_PRINT_HELPER(__VA_ARGS__)

// Helper macros to handle zero arguments case
#define WARN_PRINT_HELPER(fmt, ...)                                            \
    fprintf(stderr, YELLOW_TEXT "%s:%d: " fmt RESET_TEXT "\n",                 \
            basename(__FILE__), __LINE__, ##__VA_ARGS__)
#define WARN(...) WARN_PRINT_HELPER(__VA_ARGS__)

#ifdef DEBUG_BUILD
// Helper macros to handle zero arguments case
#define DEBUG_HELPER(fmt, ...)                                                 \
    fprintf(stderr, BLUE_TEXT "%s:%d: " fmt RESET_TEXT "\n",                   \
            basename(__FILE__), __LINE__, ##__VA_ARGS__)
#define DEBUG(...) DEBUG_HELPER(__VA_ARGS__)
#else
#define DEBUG(...) ((void)0)
#endif

#ifdef TRACE_BUILD
// Helper macros to handle zero arguments case
#define TRACE_HELPER(fmt, ...)                                                 \
    fprintf(stderr, GRAY_TEXT "%s:%d: " fmt RESET_TEXT "\n",                   \
            basename(__FILE__), __LINE__, ##__VA_ARGS__)
#define TRACE(...) TRACE_HELPER(__VA_ARGS__)
#else
#define TRACE(...) ((void)0)
#endif

#define BREAKPOINT()                                                           \
    do {                                                                       \
        ERROR("Breakpoint hit!");                                              \
        __asm__("int3; nop " ::: "memory");                                    \
    } while (0)
