#include <grpcpp/grpcpp.h>
#include <sys/stat.h>
#include <time.h>
#include <iostream>
#include <chrono>
#include <thread>

#include "commonheaders.h"
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
using wifs::HeartBeat;

std::unique_ptr<WIFS::Stub> master_client_stub_;
std::unique_ptr<WIFS::Stub> heartbeat_client_stub_s1_;
std::unique_ptr<WIFS::Stub> heartbeat_client_stub_s2_;

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
        WriteReq write_request;
        std::cout<< "Received write req" << std::endl;
    	write_request.set_address(request->address());
    	write_request.set_buf(request->buf());	
        WriteRes write_reply;
	    Status status = master_client_stub_->wifs_WRITE(&client_context, write_request, &write_reply); 
        reply->set_status(write_reply.status());
	    return Status::OK;
    }

    Status wifs_READ(ServerContext* context, const ReadReq* request,
                     ReadRes* reply) override {
        //forward read to Primary server - for now
        ClientContext client_context;
        ReadReq read_request;
	    std::cout<< "Received read req" << std::endl;
        read_request.set_address(request->address());
        read_request.set_buf(request->buf());
        ReadRes read_reply;
        Status status = master_client_stub_->wifs_READ(&client_context, read_request, &read_reply);
        reply->set_status(read_reply.status());
        reply->set_buf(read_reply.buf());
	return Status::OK;
    }
};

//Master listens to clients and forwards requests to Primary
void run_master_server() {
    std::string address(ip_master);
    WifsServiceImplementation service;
    ServerBuilder wifsServer;
    wifsServer.AddListeningPort(address, grpc::InsecureServerCredentials());
    wifsServer.RegisterService(&service);
    std::unique_ptr<Server> server(wifsServer.BuildAndStart());
    std::cout << "MASTER listening on port: " << address << std::endl;
    server->Wait();
}

void init_or_change_primary(){
    master_client_stub_ = WIFS::NewStub(grpc::CreateChannel(primary_address, grpc::InsecureChannelCredentials()));
}

void check_heartbeats(){
    HeartBeat request;
    while (true){
        HeartBeat reply1, reply2;
        heartbeat_client_stub_s1_ = WIFS::NewStub(grpc::CreateChannel(ip_server_wifs_1, grpc::InsecureChannelCredentials()));
        heartbeat_client_stub_s2_ = WIFS::NewStub(grpc::CreateChannel(ip_server_wifs_2, grpc::InsecureChannelCredentials()));
        ClientContext context1, context2;
        Status status1 = heartbeat_client_stub_s1_->Ping(&context1, request, &reply1);
        if(reply1.state() == WIFS::HeartBeat_State_READY){
            std::cout << "Server 1 alive!" <<std::endl;
            if(!primary_server){
                primary_address = ip_server_wifs_1;
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

        Status status2 = heartbeat_client_stub_s2_->Ping(&context2, request, &reply2);
        if(reply2.state() == WIFS::HeartBeat_State_READY){
            std::cout << "Server 2 alive!" <<std::endl;
            if(!primary_server){
                primary_address = ip_server_wifs_2;
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
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); //checking heartbeat every second - for now
    }
}

int main(int argc, char** argv) {
    std::thread hb_thread(check_heartbeats);
    //run service for clients only after we have a primary?
    run_master_server();
    return 0;
}
