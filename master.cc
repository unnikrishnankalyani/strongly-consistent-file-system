#include <grpcpp/grpcpp.h>
#include <sys/stat.h>
#include <time.h>
#include <iostream>
#include<dos.h>

#include "WifsClient.h"
#include "commonheaders.h"
#include "primarybackup.grpc.pb.h"
#include "wifs.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;
using grpc::StatusCode;

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;

using wifs::ReadReq;
using wifs::ReadRes;
using wifs::WIFS;
using wifs::WriteReq;
using wifs::WriteRes;

using primarybackup::HeartBeat;
using primarybackup::PrimaryBackup;
using primarybackup::WriteRequest;
using primarybackup::WriteResponse;

std::unique_ptr<WIFS::Stub> master_client_stub_;
std::unique_ptr<PrimaryBackup::Stub> heartbeat_client_stub_s1_;
std::unique_ptr<PrimaryBackup::Stub> heartbeat_client_stub_s2_;

//Get Primary and Backup addresses
std::string primary_address;
std::string backup_address;
int primary_server = 0;

//Wifs service to accept requests from clients and redirect to Primary server
class WifsServiceImplementation final : public WIFS::Service {
    Status wifs_WRITE(ServerContext* context, const WriteReq* request,
                      WriteRes* reply) override {
        //forward write to Primary server
        ClientContext client_context;
        Status status = master_client_stub_->wifs_WRITE(&client_context, request, &reply); 
        return Status::OK;
    }

    Status wifs_READ(ServerContext* context, const ReadReq* request,
                     ReadRes* reply) override {
        //forward read to Primary server - for now
        ClientContext client_context;
        Status status = master_client_stub_->wifs_READ(&client_context, request, &reply); 
        return Status::OK;
    }
};

//Master listens to clients and forwards requests to Primary
void run_master_server() {
    std::string address(config.ip_master);
    WifsServiceImplementation service;
    ServerBuilder wifsServer;
    wifsServer.AddListeningPort(address, grpc::InsecureServerCredentials());
    wifsServer.RegisterService(&service);
    std::unique_ptr<Server> server(wifsServer.BuildAndStart());
    std::cout << "MASTER listening on port: " << address << std::endl;
    server->Wait();
}

void init_or_change_primary(){
    master_client_stub_ = PrimaryBackup::NewStub(grpc::CreateChannel(primary_address, grpc::InsecureChannelCredentials()));
    // run service for clients only after we have a primary?
    run_master_server();
}

void check_heartbeats(){
    heartbeat_client_stub_s1_ = PrimaryBackup::NewStub(grpc::CreateChannel(config.ip_server1, grpc::InsecureChannelCredentials()));
    heartbeat_client_stub_s2_ = PrimaryBackup::NewStub(grpc::CreateChannel(config.ip_server2, grpc::InsecureChannelCredentials()));
    ClientContext context;
    HeartBeat request;
    while (true){
        HeartBeat reply;
        Status status = heartbeat_client_stub_s1_->Ping(&context, request, &reply);
        if(reply.state == READY){
            std::cout << "Server 1 alive!" <<std::endl;
            if(!primary_server)){
                primary_address = config.ip_server1;
                primary_server = 1;
                init_or_change_primary();
                std::cout << "Server 1: set as PRIMARY" <<std::endl;
            }
        }else {
            std::cout << "Server 1: No heartbeat" <<std::endl;
            if(primary_server == 1){
                primary_server = 0;
                std::cout << "Primary (Server 1) crashed" <<std::endl;
            } 
        }

        Status status = heartbeat_client_stub_s2_->Ping(&context, request, &reply);
        if(reply.state == READY){
            std::cout << "Server 2 alive!" <<std::endl;
            if(!primary_server)){
                primary_address = config.ip_server2;
                primary_server = 2;
                init_or_change_primary();
                std::cout << "Server 2: set as PRIMARY" <<std::endl;
            }
        }else {
            std::cout << "Server 2: No heartbeat" <<std::endl;
            if(primary_server == 2){
                primary_server = 0;
                std::cout << "Primary (Server 2) crashed" <<std::endl;
            } 
        }
    delay(1000); //check every second for now
    }
}

int main(int argc, char** argv) {
    
    std::thread check_heartbeats;
    return 0;
}