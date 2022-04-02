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

using wifs::ClientInitReq;
using wifs::ClientInitRes;
using wifs::ReadReq;
using wifs::ReadRes;
using wifs::WIFS;
using wifs::WriteReq;
using wifs::WriteRes;

#define BLOCK_SIZE 4096

class WifsClient {
   public:
    WifsClient(std::shared_ptr<Channel> channel) : stub_(WIFS::NewStub(channel)) {}

    int interval = 1000;
    int retries = 1;

    int wifs_READ(int address, char buf[BLOCK_SIZE]) {
        ClientContext context;
        ReadReq request;
        ReadRes reply;
        request.set_address(address);
        Status status = stub_->wifs_READ(&context, request, &reply);
        strncpy(buf, reply.buf().c_str(), BLOCK_SIZE);
        if (!status.ok()) return -1;

        if (reply.status() == wifs::ReadRes_Status_SOLO) {
            return 2;
        }
        if (reply.status() == wifs::ReadRes_Status_RETRY) {
            // no need to switch primary here.
            return 3;
        }
        return reply.primary_ip() == primary_server ? 0 : 1;
    }

    int wifs_WRITE(int address, char buf[BLOCK_SIZE]) {
        ClientContext context;
        WriteReq request;
        WriteRes reply;
        request.set_address(address);
        request.set_buf(std::string(buf));
        Status status = stub_->wifs_WRITE(&context, request, &reply);
        if (!status.ok()) return -1;

        if (reply.status() == wifs::WriteRes_Status_RETRY) {
            // need to swtich primary and retry operation
            return 1;
        }
        if (reply.status() == wifs::WriteRes_Status_SOLO) {
            return 2;
        }
        return 0;
    }

   private:
    std::unique_ptr<WIFS::Stub> stub_;
};
