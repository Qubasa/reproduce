#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define END_RANGE (1718099106 + 36000)

// Function to be executed by the thread
void *print_message(void *arg) {

    // Execute the command
    system("date +%s");
    return NULL;
}

int main(int argc, char *argv[]) {
    pid_t pid = getpid();
    printf("==Start== pid %d\n", pid);
    int ret = 0;
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }

    // if number of args is more then 1, then dont execute the thread
    if (argc <= 1) {

        pthread_t thread;
        // Create a new thread that runs the print_message function
        if (pthread_create(&thread, NULL, print_message, NULL)) {
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }
        printf("Before join\n");
        // Wait for the thread to finish
        if (pthread_join(thread, NULL)) {
            fprintf(stderr, "Error joining thread\n");
            return 2;
        }
        printf("After join\n");
    }

    const char *rpath = "/dev/urandom";
    // // call openat to open /dev/random
    int fd = open(rpath, 0);
    if (fd == -1) {
        printf("Failed to open %s\n", rpath);
        perror("open");
        return -1;
    }
    printf("%s opened successfully\n", rpath);

    // Perform operations with the file descriptor fd
    // For example, read some random data
    unsigned char buffer[16];
    ssize_t result = read(fd, buffer, sizeof(buffer));
    if (result < 0) {
        perror("Failed to read from /dev/urandom");
        exit(EXIT_FAILURE);
    } else {
        printf("Read %zd bytes from /dev/urandom: ", result);
        for (size_t i = 0; i < sizeof(buffer); ++i) {
            printf("%02x ", buffer[i]);
        }
        printf("\n");
    }

    printf("==Before fork==\n");
    pid = fork();
    if (pid == -1) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        pid = getpid();
        printf("Child process: %d\n", pid);
    } else {
        pid = getpid();
        printf("Parent process: %d\n", pid);
    }

    /* ===== getrandom SYSCALL ======*/
    unsigned char buf[11] = {0};
    buf[0] = 0xBB;
    buf[10] = 0xBB;
    printf("==Before getrandom==\n");
    size_t size = sizeof(buf) - 2;
    int res = getrandom(&buf[1], size, 0);

    // int res = 0;
    if (res == -1) {
        perror("getrandom");
        return -1;
    }

    printf("Buf1 bytes: 0x");
    for (int i = 0; i < sizeof(buf); i++) {
        printf("%02x", buf[i]);
    }
    printf("\n");

    if (buf[1] != 0x22) {
        printf("\033[1;31m%d: getrandom[1] != 0x22\033[0m\n", pid);
        ret = 1;
    }
    if (buf[9] != 0x22) {
        printf("\033[1;31m%d: getrandom[2] != 0x22\033[0m\n", pid);
        ret = 1;
    }
    if (buf[10] != 0xBB) {
        printf("\033[1;31m%d: getrandom[99] != 0xBB\033[0m\n", pid);
        ret = 1;
    }
    if (buf[0] != 0xBB) {
        printf("\033[1;31m%d: getrandom[0] != 0xBB\033[0m\n", pid);
        ret = 1;
    }

    /* ===== getrandom SYSCALL ======*/
    unsigned char buf2[12] = {0};
    buf2[11] = 0xBB;
    buf2[0] = 0xBB;
    size = sizeof(buf2) - 2;
    res = getrandom(&buf2[1], size, 0);

    if (res == -1) {
        perror("getrandom");
        return -1;
    }

    // execute getrandom() and print the result
    printf("Buf2 bytes: 0x");
    for (int i = 0; i < sizeof(buf2); i++) {
        printf("%02x", buf2[i]);
    }
    printf("\n");

    if (buf2[1] != 0x22) {
        printf("\033[1;31m%d: getrandom2[1] != 0x22\033[0m\n", pid);
        ret = 1;
    }
    if (buf2[2] != 0x22) {
        printf("\033[1;31m%d: getrandom2[2] != 0x22\033[0m\n", pid);
        ret = 1;
    }
    if (buf2[11] != 0xBB) {
        printf("\033[1;31m%d: getrandom2[11] != 0xBB\033[0m\n", pid);
        ret = 1;
    }
    if (buf2[0] != 0xBB) {
        printf("\033[1;31m%d: getrandom2[0] != 0xBB\033[0m\n", pid);
        ret = 1;
    }

    /* ===== gettimeofday SYSCALL ======*/
    struct timeval tv;
    gettimeofday(&tv, NULL);
    printf("tv->tv_sec: %ld\n", tv.tv_sec);
    printf("tv->tv_usec: %ld\n", tv.tv_usec);

    if (!(tv.tv_sec >= 1718099106 && tv.tv_sec < END_RANGE)) {
        printf("\033[1;31m%d: gettimeofday not in expected range\033[0m\n",
               pid);
        ret = 1;
    }

    /* ===== TIME SYSCALL ======*/
    time_t mytime_buf = 0;
    time_t mytime = 0;
    mytime = time(&mytime_buf);
    if (mytime == -1) {
        perror("time");
        exit(EXIT_FAILURE);
    }
    if (mytime_buf != mytime) {
        printf("\033[1;31m%d: time_buf != time\033[0m\n", pid);
        printf("mytime_buf: %ld\n", mytime_buf);
        ret = 1;
    }

    printf("mytime: %ld\n", mytime);
    if (!(mytime >= 1718099106 && mytime < END_RANGE)) {
        printf("\033[1;31m%d: time not in expected range\033[0m\n", pid);
        ret = 1;
    }

    /* ===== clock_gettime SYSCALL ======*/
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }
    printf("ts->tv_sec: %ld\n", ts.tv_sec);
    printf("ts->tv_nsec: %ld\n", ts.tv_nsec);

    if (!(ts.tv_sec >= 1718099106 && ts.tv_sec < END_RANGE)) {
        printf("\033[1;31m%d: clock_gettime not in expected range\033[0m\n",
               pid);
        ret = 1;
    }

    printf("%d: Return value: %d\n", pid, ret);
    return ret;
}