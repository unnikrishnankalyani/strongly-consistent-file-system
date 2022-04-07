#include <grpcpp/grpcpp.h>
#include <sys/stat.h>
#include <time.h>

#include <iostream>

#include "client.cc"
#include "wifs.grpc.pb.h"

void tester() {
    char buf[BLOCK_SIZE + 1];
    for (int i = 0; i < BLOCK_SIZE; i++) buf[i] = 'c';


    struct timeval begin, end;
    gettimeofday(&begin, 0);

    int rc;
    for(int i = 0 ; i < 100 ; i++) do_write_wrapper(0, buf, wifs::WriteReq_Crash_NO_CRASH);
    
    // Stop measuring time and calculate the elapsed time
    gettimeofday(&end, 0);
    long seconds = end.tv_sec - begin.tv_sec;
    long microseconds = end.tv_usec - begin.tv_usec;
    double elapsed = seconds + microseconds*1e-6;
    printf("%lf\n", elapsed);


    gettimeofday(&begin, 0);

    for(int i = 0 ; i < 100 ; i++) do_read_wrapper(0, buf, wifs::ReadReq_Crash_NO_CRASH);
    
    // Stop measuring time and calculate the elapsed time
    gettimeofday(&end, 0);
    seconds = end.tv_sec - begin.tv_sec;
    microseconds = end.tv_usec - begin.tv_usec;
    elapsed = seconds + microseconds*1e-6;

    printf("%lf\n", elapsed);
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