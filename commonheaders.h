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



#define HEARTBEAT_TIMER 1000



std::string ip_server_pb_1 = "10.10.1.2:50052";
std::string ip_server_pb_2 = "10.10.1.3:50053";
std::string ip_server_wifs_1 = "10.10.1.2:50054";
std::string ip_server_wifs_2 = "10.10.1.3:50055";
std::string primary_server = "";
std::string servers [2] = {ip_server_wifs_1, ip_server_wifs_2};

int read_index = 0;
int primary_index = 0;

int single_server = 0;

std::string getServerDir(int machine_id){
        return "/users/oahmed4/.server" +  std::to_string(machine_id);
}

std::string getServerPath(std::string address, int machine_id) {
    return getServerDir(machine_id) + "/bs" ;//"/file_" + address;
}