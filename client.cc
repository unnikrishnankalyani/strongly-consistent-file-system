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

int do_read(int address, char* buf) {
    return options.wifsclient->wifs_READ(address, buf);
}

int do_write(int address, char* buf) {
    return options.wifsclient->wifs_WRITE(address, buf);
}

void tester() {
    char buf[BLOCK_SIZE + 1];
    for (int i = 0; i < BLOCK_SIZE; i++) buf[i] = 'a';
    do_write(0, buf);
    buf[0] = '\0';
    do_read(0, buf);
    buf[BLOCK_SIZE] = '\0';
    printf("read %s", buf);
}

int main(int argc, char* argv[]) {
    options.wifsclient = new WifsClient(grpc::CreateChannel(config.ip_master, grpc::InsecureChannelCredentials()));
    tester();
    return 0;
}
