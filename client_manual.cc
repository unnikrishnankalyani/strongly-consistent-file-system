#include <grpcpp/grpcpp.h>
#include <sys/stat.h>
#include <time.h>

#include <iostream>

#include "WifsClient.h"
#include "wifs.grpc.pb.h"

static struct options {
    WifsClient* wifsclient[2];
    int show_help;
} options;

int init(){
    options.wifsclient[0] = new WifsClient(grpc::CreateChannel(ip_server_wifs_1, grpc::InsecureChannelCredentials()));
    options.wifsclient[1] = new WifsClient(grpc::CreateChannel(ip_server_wifs_2, grpc::InsecureChannelCredentials()));
}

int do_read(int address, char* buf) {
    std::cout << "Current PRIMARY: " <<primary_server <<std::endl;
    if (primary_server == ""){
        primary_server = servers[primary_index];
        init();
        std::cout << "Changed PRIMARY: " <<primary_server <<std::endl;
    }
    if (!single_server)
        read_index = rand() % 2;
    else
        read_index = primary_index;
    std::cout << "Reading from server " << servers[read_index];
    int rc = options.wifsclient[read_index]->wifs_READ(address, buf);
    std::cout << "Read Return code: " << rc <<std::endl;
    if (rc < 0){ //call failed
        primary_index = 1 - read_index; //switches between 1 and 0
        primary_server = servers[primary_index];
        single_server = 1;
        std::cout << "Read Call FAILED. Trying other node" <<primary_server <<std::endl;
        do_read(address,buf); //repeat operation
    }
    else if (rc == 1){ //primary has changed
        primary_index = 1 - read_index; //switches between 1 and 0
        primary_server = servers[primary_index];
        single_server = 0;
        std::cout << "Changed PRIMARY: " <<primary_server <<std::endl;
    }
    else if (rc == 2){ //current server is running in solo mode
        primary_index = read_index;
        primary_server = servers[primary_index];
        single_server = 1;
        std::cout << "Server running in SOLO mode. No more read distribution" <<primary_server <<std::endl;
    }
    else{
        single_server = 0;
    }
    return rc;    
}

int do_write(int address, char* buf) {
    if (primary_server == ""){
        primary_server = servers[primary_index];
        init();
        std::cout << "Changed PRIMARY: " <<primary_server <<std::endl;
    }
    int rc =  options.wifsclient[primary_index]->wifs_WRITE(address, buf);
    std::cout << "Write Return code: " << rc <<std::endl;
    int count = 0;
    if (rc <0 ){ //call failed
        if (count == 4) return rc;
        primary_index = 1 - primary_index; //switches between 1 and 0
        primary_server = servers[primary_index];
        single_server = 1;
        std::cout << "Write Call FAILED. Trying other node" <<primary_server <<std::endl;
        count++;
        do_write(address,buf); //repeat operation
    }
    else if (rc == 1){ //primary has changed
        primary_index = 1 - primary_index; //switches between 1 and 0
        primary_server = servers[primary_index];
        single_server = 0;
        std::cout << "Changed PRIMARY: " <<primary_server <<std::endl;
        std::cout << "Repeating WRITE" <<std::endl;
        do_write(address,buf);
    }
    else if (rc == 2){ //server running solo
        single_server = 1;
        std::cout << "Server running in SOLO mode. No more read distribution" <<primary_server <<std::endl;
    }
    else {
        single_server = 0;
    }
    return rc;

}

void tester() {
    char buf[BLOCK_SIZE + 1];
    for (int i = 0; i < BLOCK_SIZE; i++) buf[i] = 'c';
    do_write(0, buf);
    buf[0] = '\0';
    do_read(0, buf);
    buf[BLOCK_SIZE] = '\0';
    printf("read successful\n");
    do_read(0, buf);
    printf("read successful\n");
    do_read(0, buf);
    printf("read successful\n");
    do_read(0, buf);
    printf("read successful\n");
    do_read(0, buf);
    printf("read successful\n");
}

int main(int argc, char* argv[]) {
    //options.wifsclient = new WifsClient(grpc::CreateChannel("localhost:50055", grpc::InsecureChannelCredentials()));
    tester();
    return 0;
}