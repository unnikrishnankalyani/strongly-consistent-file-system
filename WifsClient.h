#include <grpc++/grpc++.h>
#include "wifs.grpc.pb.h"
#include "commonheaders.h"
#include <grpc/impl/codegen/status.h>
#include <grpcpp/impl/codegen/status_code_enum.h>
#include <chrono>
#include <thread>

using grpc::Channel;
using grpc::Status;
using grpc::StatusCode;
using grpc::ClientContext;
using grpc::ClientReader;

using wifs::WIFS;
using wifs::ReadReq;
using wifs::ReadRes;
using wifs::WriteReq;
using wifs::WriteRes;

class WifsClient {
    public:
        WifsClient(std::shared_ptr<Channel> channel) : stub_(WIFS::NewStub(channel)) {}

    int interval = 1000;
    int retries = 1;

    int wifs_READ(int address)
    {
        ClientContext context;
        ReadReq request;
        ReadRes reply;
        request.set_address(address);
        Status status = stub_->wifs_READ(&context, request, &reply);
        const auto ret = reply.buf();
        std::cout << ret << std::endl;
        if (status.ok())
                    return 0;
            else
                    return -ENOENT;
    }

    int wifs_WRITE(int address, char buf[4096])
    {
        ClientContext context;
        WriteReq request;
        WriteRes reply;
        request.set_address(address);
        request.set_buf(s);
        Status status =  stub_->wifs_WRITE(&context, request ,&reply);
        if (status.ok())
            return 0;
        else
            return -ENOENT;
        }
    
    private:
        std::unique_ptr<WIFS::Stub> stub_;
};
