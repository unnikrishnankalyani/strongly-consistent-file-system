#include <grpcpp/grpcpp.h>
#include <sys/stat.h>
#include <time.h>

#include <iostream>

#include "client.cc"
#include "wifs.grpc.pb.h"

void tester() {
    char buf[BLOCK_SIZE + 1];
    for (int i = 0; i < BLOCK_SIZE; i++) buf[i] = 'c';
    int rc = do_write(0, buf);
    if(rc == -1) std::cout << "WRITE FAIL\n";
    
    buf[0] = '\0';
    rc = do_read(0, buf);
    if(rc == -1) std::cout << "READ FAIL\n";

    buf[BLOCK_SIZE] = '\0';
    printf("read first char - %c\n", buf[0]);
}

int main(int argc, char* argv[]) {
    //options.wifsclient = new WifsClient(grpc::CreateChannel("localhost:50055", grpc::InsecureChannelCredentials()));
    tester();
    return 0;
}