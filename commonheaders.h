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
#include <unistd.h>

#include <iostream>

static struct config {
    std::string ip_master;
    std::string ip_server1;
    std::string ip_server2;
} config;

//fix this
config.ip_master = "localhost:50054";
config.ip_server1 = "localhost:50051";
config.ip_server2 = "localhost:50051";

std::string getServerPath(std::string address, int machine_id) {
    return "/users/oahmed4/.server" + std::to_string(machine_id) + "/file_" + address;
}
