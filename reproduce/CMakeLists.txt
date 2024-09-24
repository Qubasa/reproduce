cmake_minimum_required(VERSION 3.10)
project(reproduce)

# Set the C and C++ compilers to clang
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

# Set the C standard
set(CMAKE_C_STANDARD 23)

# Generate compile_commands.json for language server
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set linker flags to use lld
set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=lld")
set(CMAKE_MODULE_LINKER_FLAGS "-fuse-ld=lld")
set(CMAKE_SHARED_LINKER_FLAGS "-fuse-ld=lld")

# Set the sanitize options
set(SANITIZE_FLAGS
 -fsanitize=address
 -fsanitize=undefined
 -fsanitize=signed-integer-overflow
 -fsanitize=shift
 -fsanitize=integer-divide-by-zero
 -fsanitize=array-bounds
 -fsanitize=implicit-conversion
 -fsanitize=nullability
 -fsanitize=integer
 -fsanitize-address-use-after-return=always
 -fsanitize-address-use-after-scope
)
set(BETTER_STACKTRACE -g -O1 -fno-omit-frame-pointer -fno-optimize-sibling-calls)
set(PERF_FLAGS
    -flto
    -ffast-math
    -funroll-loops
    -finline-functions

    # Only use general purpose registers
    # this makes syscalls faster
    # as Linux doesn't have to save/restore the AVX/SSE registers
    -mgeneral-regs-only
)
set(ADDITIONAL_FLAGS -Wall)


# add header file directory include/macros.h
include_directories(include)



if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Add the debug define for conditional compilation macros
    add_definitions(-DDEBUG_BUILD)
    # Set the compile options
    add_compile_options(${PERF_FLAGS} ${SANITIZE_FLAGS} ${BETTER_STACKTRACE} ${ADDITIONAL_FLAGS})

    # # Add clang-tidy configuration
    # if(NOT CMAKE_CXX_CLANG_TIDY)
    #     find_program(CLANG_TIDY_EXE NAMES clang-tidy)
    #     if(CLANG_TIDY_EXE)
    #         set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_EXE}" "-checks=*,-clang-analyzer-*")
    #         set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}" "-checks=*,-clang-analyzer-*")
    #     endif()
    # endif()
else()
    # Force static linking
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(BUILD_SHARED_LIBS OFF)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

    add_compile_options(${PERF_FLAGS} ${ADDITIONAL_FLAGS})

endif()

# Add the executables
add_executable(printrandom src/printrandom.c)
add_executable(reproduce
    src/main.c
    src/putils.c
    src/vdso.c
    src/proc_init.c
    src/syscalls.c
    src/events.c
    src/hashtable.c
)



if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(WARNING "Debug build. Adding sanitizer flags.")
    # Link the sanitizer libraries
    target_link_libraries(printrandom ${PERF_FLAGS}  ${SANITIZE_FLAGS} ${BETTER_STACKTRACE}  ${ADDITIONAL_FLAGS} pthread)
    target_link_libraries(reproduce ${PERF_FLAGS}  ${SANITIZE_FLAGS} ${BETTER_STACKTRACE}  ${ADDITIONAL_FLAGS})
else()
    target_link_libraries(printrandom ${PERF_FLAGS} ${ADDITIONAL_FLAGS})
    target_link_libraries(reproduce ${PERF_FLAGS} ${ADDITIONAL_FLAGS})
endif()


# Installation targets
install(TARGETS printrandom reproduce RUNTIME DESTINATION bin)


if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Add a custom command to strip the binary after build
    add_custom_command(
        TARGET reproduce
        POST_BUILD
        COMMAND ${CMAKE_STRIP} --strip-all $<TARGET_FILE:reproduce>
        COMMENT "Stripping the binary"
    )
endif()