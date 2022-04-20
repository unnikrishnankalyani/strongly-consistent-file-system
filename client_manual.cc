#include <grpcpp/grpcpp.h>

#include <iostream>
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/Bucket.h>
#include <aws/s3/model/CreateBucketConfiguration.h>
#include <aws/s3/model/CreateBucketRequest.h>
#include <aws/s3/model/DeleteBucketRequest.h>

#include <sys/stat.h>
#include <time.h>

#include "client.cc"
#include "wifs.grpc.pb.h"


void tester() {
    char buf[BLOCK_SIZE + 1];
    for (int i = 0; i < BLOCK_SIZE; i++) buf[i] = 'c';
    int rc = do_write(0, buf, wifs::WriteReq_Crash_NO_CRASH);
    if (rc == -1) std::cout << "WRITE FAIL\n";

    buf[0] = '\0';
    rc = do_read(0, buf, wifs::ReadReq_Crash_NODE_CRASH_READ);
    if (rc == -1) std::cout << "READ FAIL\n";

    buf[BLOCK_SIZE] = '\0';
    printf("read first char - %c\n", buf[0]);

    // do_read(0, buf, wifs::ReadReq_Crash_NO_CRASH);
    // if (rc == -1) std::cout << "READ FAIL\n";
    // do_read(0, buf, wifs::ReadReq_Crash_NO_CRASH);
    // if (rc == -1) std::cout << "READ FAIL\n";
    // do_read(0, buf, wifs::ReadReq_Crash_NO_CRASH);
    // if (rc == -1) std::cout << "READ FAIL\n";
}

int main(int argc, char* argv[]) {
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        Aws::Client::ClientConfiguration clientConfig;
        clientConfig.region = "us-east-1";
        clientConfig.endpointOverride = "http://c220g1-030604.wisc.cloudlab.us:4566";
        Aws::S3::S3Client client(clientConfig);

        auto outcome = client.ListBuckets();
        if (outcome.IsSuccess()) {
            std::cout << "Found " << outcome.GetResult().GetBuckets().size() << " buckets\n";
            for (auto&& b : outcome.GetResult().GetBuckets()) {
                std::cout << b.GetName() << std::endl;
            }
        }
        else {
            std::cout << "Failed with error: " << outcome.GetError() << std::endl;
        }
    }

    return 0;
}