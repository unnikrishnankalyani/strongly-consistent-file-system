#include <grpcpp/grpcpp.h>
#include <sys/stat.h>
#include <time.h>

#include <iostream>

#include "client.cc"
#include "wifs.grpc.pb.h"

void write_crash(char a, char b) {
    char buf[BLOCK_SIZE + 1];
    for (int i = 0; i < BLOCK_SIZE; i++) buf[i] = a;
    int rc = do_write(0, buf, wifs::WriteReq_Crash_PRIMARY_CRASH_BEFORE_LOCAL_WRITE_AFTER_REMOTE);
    // if (rc == -1) std::cout << "WRITE FAIL\n";

    // for (int i = 0; i < BLOCK_SIZE; i++) buf[i] = b;
    // rc = do_write(1, buf, wifs::WriteReq_Crash_NO_CRASH);
    // if (rc == -1) std::cout << "WRITE FAIL\n";

    // buf[0] = '\0';
    // rc = do_read(0, buf, wifs::ReadReq_Crash_NO_CRASH);
    // if (rc == -1) std::cout << "READ FAIL\n";

    // buf[BLOCK_SIZE] = '\0';
    // printf("read first char - %c\n", buf[0]);

    // rc = do_read(1, buf, wifs::ReadReq_Crash_NO_CRASH);
    // if (rc == -1) std::cout << "READ FAIL\n";

    // buf[BLOCK_SIZE] = '\0';
    // printf("read first char - %c\n", buf[0]);
}

void just_read() {
    char buf[BLOCK_SIZE + 1];
    buf[0] = '\0';
    int rc = do_read(0, buf, wifs::ReadReq_Crash_NO_CRASH);
    if (rc == -1) std::cout << "READ FAIL\n";

    buf[BLOCK_SIZE] = '\0';
    printf("read first char - %c\n", buf[0]);

    // rc = do_read(1, buf, wifs::ReadReq_Crash_NO_CRASH);
    // if (rc == -1) std::cout << "READ FAIL\n";

    // buf[BLOCK_SIZE] = '\0';
    // printf("read first char - %c\n", buf[0]);
}

int main(int argc, char* argv[]) {
    if(!strcmp(argv[1], "1")) write_crash('x', 'y');
    else just_read();
    return 0;
}