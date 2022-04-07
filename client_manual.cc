#include <grpcpp/grpcpp.h>
#include <sys/stat.h>
#include <time.h>

#include <iostream>

#include "client.cc"
#include "wifs.grpc.pb.h"

void tester() {
    char buf[BLOCK_SIZE + 1];
    for (int i = 0; i < BLOCK_SIZE; i++) buf[i] = 'c';
    int rc = do_write_wrapper(0, buf, wifs::WriteReq_Crash_NO_CRASH);
    if (rc == -1) std::cout << "WRITE FAIL\n";

    buf[0] = '\0';
    rc = do_read_wrapper(0, buf, wifs::ReadReq_Crash_NO_CRASH);
    if (rc == -1) std::cout << "READ FAIL\n";

    // do_read(0, buf, wifs::ReadReq_Crash_NO_CRASH);
    // if (rc == -1) std::cout << "READ FAIL\n";
    // do_read(0, buf, wifs::ReadReq_Crash_NO_CRASH);
    // if (rc == -1) std::cout << "READ FAIL\n";
    // do_read(0, buf, wifs::ReadReq_Crash_NO_CRASH);
    // if (rc == -1) std::cout << "READ FAIL\n";
}

int main(int argc, char* argv[]) {
    tester();
    return 0;
}