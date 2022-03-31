#include <grpc++/grpc++.h>
#include <grpc/impl/codegen/status.h>
#include <grpcpp/impl/codegen/status_code_enum.h>

#include <chrono>
#include <thread>

#include "commonheaders.h"
#include "wifs.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;
using grpc::StatusCode;

using wifs::ReadReq;
using wifs::ReadRes;
using wifs::WIFS;
using wifs::WriteReq;
using wifs::WriteRes;
using wifs::ClientInitReq;
using wifs::ClientInitRes;

#define BLOCK_SIZE 4096

class WifsClient {
   public:
    WifsClient(std::shared_ptr<Channel> channel) : stub_(WIFS::NewStub(channel)) {}

    int interval = 1000;
    int retries = 1;

    int wifs_READ(int address, char buf[BLOCK_SIZE]) {
        // makes changes here to try out from the servers list, and retry based on the status
        // if(reply.status == wifs::ReadRes_Status_RETRY) retry;
        ClientContext context;
        ReadReq request;
        ReadRes reply;
        request.set_address(address);
        Status status = stub_->wifs_READ(&context, request, &reply);
        strncpy(buf, reply.buf().c_str(), BLOCK_SIZE);
        return status.ok() ? 0 : -1;
    }

    int wifs_WRITE(int address, char buf[BLOCK_SIZE]) {
        // makes changes here to try out from the servers list, and retry based on the status
        // if(reply.status == wifs::WriteRes_Status_RETRY) retry;
        ClientContext context;
        WriteReq request;
        WriteRes reply;
        request.set_address(address);
        request.set_buf(std::string(buf));
        Status status = stub_->wifs_WRITE(&context, request, &reply);
        return status.ok() ? 0 : -1;
    }

    int wifs_INIT(const char buf[MAX_PATH_LENGTH]){
        ClientContext context;
        ClientInitReq request;
        ClientInitRes reply;
        Status status = stub_->wifs_INIT(&context, request, &reply);
        buf = reply.primary_ip().c_str();
        std::cout<<"Printing from WifsClient: IP address: " << buf <<std::endl;
        return status.ok() ? 0 : -1;
    }

   private:
    std::unique_ptr<WIFS::Stub> stub_;
};
