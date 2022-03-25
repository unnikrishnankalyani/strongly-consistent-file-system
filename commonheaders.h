#define MAX_PATH_LENGTH 1000
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <iostream>
//#include <unistd.h>

#define HTABLESIZE 100

long get_time() {
    struct timespec* ts;
    clock_gettime(CLOCK_MONOTONIC, ts);
    return ts->tv_nsec;
}
std::string getServerPath(std::string address) {
    return "/users/oahmed4/.server/file_" + address;
}
