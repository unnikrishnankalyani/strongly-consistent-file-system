#include <grpcpp/grpcpp.h>
#include <sys/stat.h>
#include <time.h>

#include <iostream>

#include "WifsClient.h"
#include "wifs.grpc.pb.h"

static struct options {
    WifsClient* wifsclient;
    int show_help;
} options;

int init(std::string ip_address){
    options.wifsclient = new WifsClient(grpc::CreateChannel(ip_address, grpc::InsecureChannelCredentials()));
}

int do_read(int address, char* buf) {
    std::cout << "Current PRIMARY: " <<primary_server <<std::endl;
    if (primary_server == ""){
        primary_server = servers[server_index];
        init(primary_server);
        std::cout << "Changed PRIMARY: " <<primary_server <<std::endl;
    }
    int rc = options.wifsclient->wifs_READ(address, buf);
    std::cout << "Read Return code: " << rc <<std::endl;
    if (rc != 0){
        server_index = 1 - server_index; //switches between 1 and 0
        primary_server = servers[server_index]; //switch to other server if rc < 0 (gRPC failure) or rc > 0 (primary IP changed)
        init(primary_server);
        std::cout << "Changed PRIMARY: " <<primary_server <<std::endl;
        if (rc<0){ //repeat operation only for failure, not for primary change
            do_read(address,buf);
        }
    }
    return rc;    
}

int do_write(int address, char* buf) {
    if (primary_server == ""){
        primary_server = servers[server_index];
        init(primary_server);
        std::cout << "Changed PRIMARY: " <<primary_server <<std::endl;
    }
    int rc =  options.wifsclient->wifs_WRITE(address, buf);
    std::cout << "Write Return code: " << rc <<std::endl;
    if (rc != 0){
        server_index = 1 - server_index; //switches between 1 and 0
        primary_server = servers[server_index]; //switch to other server if rc < 0 (gRPC failure) or rc > 0 (primary IP changed)
        init(primary_server);
        std::cout << "Changed PRIMARY: " <<primary_server <<std::endl; 
        std::cout << "Repeating WRITE" <<std::endl;
        do_write(address,buf);//repeat operation both for failure, and for primary change
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
    printf("read %s", buf);
}

int main(int argc, char* argv[]) {
    //options.wifsclient = new WifsClient(grpc::CreateChannel("localhost:50055", grpc::InsecureChannelCredentials()));
    tester();
    return 0;
}