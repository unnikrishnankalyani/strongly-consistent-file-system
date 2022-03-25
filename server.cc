#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grpc/impl/codegen/status.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/impl/codegen/status_code_enum.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <streambuf>
#include <string>

#include "commonheaders.h"
#include "wifs.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;
using grpc::StatusCode;

using wifs::ReadReq;
using wifs::ReadRes;
using wifs::WIFS;
using wifs::WriteReq;
using wifs::WriteRes;

char root_path[MAX_PATH_LENGTH];

#define BLOCK_SIZE 4096

class WifsServiceImplementation final : public WIFS::Service {
    Status wifs_WRITE(ServerContext* context, const WriteReq* request,
                      WriteRes* reply) override {
        struct stat info;

        const auto path = getServerPath(std::to_string(request->address()));
        std::cout << "WIFS server PATH WRITE TO: " << path << std::endl;

        const int fd = ::open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG);

        int rc = write(fd, (void*)request->buf().c_str(), BLOCK_SIZE);
        std::cout << "rc: " << rc << std::endl;
        reply->set_status(rc);
        return Status::OK;
    }

    Status wifs_READ(ServerContext* context, const ReadReq* request,
                     ReadRes* reply) override {
        struct stat info;

        const auto path = getServerPath(std::to_string(request->address()));
        std::cout << "WIFS server PATH READ: " << path << std::endl;

        const int fd = ::open(path.c_str(), O_RDONLY);
        if (fd == -1) {
            reply->set_status(-1);
            return Status::OK;
        }

        std::string buffer;
        buffer.reserve(BLOCK_SIZE);
        std::ifstream file_inp(path);
        buffer.assign((std::istreambuf_iterator<char>(file_inp)), std::istreambuf_iterator<char>());
        reply->set_status(0);
        reply->set_buf(buffer);
        return Status::OK;
    }
};

void RunWifsServer(std::string ipadd) {
    // create port on localhost 5000
    std::string address("localhost:50051");  // HARDCODED NOT USING IPADD
    WifsServiceImplementation service;
    ServerBuilder wifsServer;  // server name
    wifsServer.AddListeningPort(address, grpc::InsecureServerCredentials());
    wifsServer.RegisterService(&service);
    std::unique_ptr<Server> server(wifsServer.BuildAndStart());
    std::cout << "Server listening on port: " << address << std::endl;
    server->Wait();
}

int main(int argc, char** argv) {
    std::string ipadd = "localhost";

    RunWifsServer(ipadd);

    return 0;
}
