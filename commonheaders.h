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

// fix this
std::string ip_master = "localhost:50051";
std::string ip_server_pb_1 = "localhost:50052";
std::string ip_server_pb_2 = "localhost:50053";
std::string ip_server_wifs_1 = "localhost:50054";
std::string ip_server_wifs_2 = "localhost:50055";

string servers [2] = {ip_server_wifs_1, ip_server_wifs_2}

std::string getServerDir(int machine_id){
        return "/users/oahmed4/.server" +  std::to_string(machine_id);
}

std::string getServerPath(std::string address, int machine_id) {
    return getServerDir(machine_id) + "/file_" + address;
}

