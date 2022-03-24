#include <string>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <grpcpp/grpcpp.h>
#include "wifs.grpc.pb.h"
#include "commonheaders.h"
#include <grpc/impl/codegen/status.h>
#include <grpcpp/impl/codegen/status_code_enum.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;
using grpc::StatusCode;

using wifs::WIFS;
using wifs::ReadReq;
using wifs::ReadRes;
using wifs::WriteReq;
using wifs::WriteRes;

char root_path[MAX_PATH_LENGTH];

class WifsServiceImplementation final : public WIFS:: Service{

      Status wifs_WRITE(ServerContext* context, const WriteReq* request,
               WriteRes* reply) override {

        struct stat info;

        const auto path = getServerPath(std::to_string(request->address()));
        std::cout << "WIFS server PATH WRITE TO: "<< path <<std::endl;

        const int fd = ::open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG);

        int rc = write(fd, (void*) request->buf().c_str(), 4096);
 	    reply->set_status(rc);
	    return Status::OK;
    }

    Status wifs_READ(ServerContext* context, const ReadReq* request,
               ReadRes* reply) override {

       
        struct stat info;

        const auto path = getServerPath(std::to_string(request->address()));
	std::cout << "WIFS server PATH READ: "<< path <<std::endl;

        const int fd = ::open(path.c_str(), O_RDWR);
	char* buf;
        int rc = read(fd, (void*) buf, 4096);
        std::string buffer(buf);
        reply->set_status(rc);	
        reply->set_buf(buffer);
        return Status::OK;
    
    }
};

void RunWifsServer(std::string ipadd) {
    //create port on localhost 5000
    std::string address("localhost:50051"); //HARDCODED NOT USING IPADD
    WifsServiceImplementation service;
    std::cout << "1" << std::endl;
    ServerBuilder wifsServer; //server name
    std::cout << "2" << std::endl;
    wifsServer.AddListeningPort(address, grpc::InsecureServerCredentials());
    std::cout << "3" << std::endl;
    wifsServer.RegisterService(&service);
    std::cout << "4" << std::endl;
    std::unique_ptr<Server> server(wifsServer.BuildAndStart());
    std::cout << "Server listening on port: " << address << std::endl;

    server->Wait();
}


int main(int argc, char** argv) {
    std::string ipadd = "localhost";

    RunWifsServer(ipadd);
    
    return 0;
}
