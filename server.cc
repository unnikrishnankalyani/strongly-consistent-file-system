#include <bits/stdc++.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grpc/impl/codegen/status.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/impl/codegen/status_code_enum.h>
#include <limits.h>
#include <semaphore.h>
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
#include <time.h> 

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
using wifs::ClientInitReq;
using wifs::ClientInitRes;

using primarybackup::HeartBeat;
using primarybackup::PrimaryBackup;
using primarybackup::WriteRequest;
using primarybackup::WriteResponse;
using primarybackup::InitReq;
using primarybackup::InitRes;

char root_path[MAX_PATH_LENGTH];
std::string server_state = "INIT";
std::string election_state = "INIT";

std::string other_node_address;
std::string this_node_address;
std::string cur_node_wifs_address;
std::string other_node_wifs_address;
std::string primary = "";

int server_id = 0;

// semaphore for checking if write queue has elements to read
sem_t sem_queue;

// semaphore for checking if log queue has elements to read
sem_t sem_log_queue;

// used to achieve mutual exclusion during enqueue operation on write queue as well as log queue
sem_t mutex_queue;
sem_t mutex_log_queue;

sem_t mutex_pending_grpc_write;
//used to ensure node doesn't respond to candidate request while it is a candidate itself
sem_t mutex_election;

//ensure that concensus happens only after the PB interfaces are up
sem_t sem_concensus;

int pending_write_address = -1;
bool other_node_syncing = false;

std::unique_ptr<PrimaryBackup::Stub> client_stub_;

void init_connection_with_other_node() {
    client_stub_ = PrimaryBackup::NewStub(grpc::CreateChannel(other_node_address, grpc::InsecureChannelCredentials()));
}

class Node {
   public:
    Node(const WriteReq* req, std::promise<int>& promise_obj) : req(req), promise_obj(promise_obj) {
    }

    std::promise<int>& promise_obj;
    const WriteReq* req;
};

#define BLOCK_SIZE 4096

std::queue<Node*> write_queue;
std::queue<WriteRequest> log_queue;

void local_write(void) {
    while (true) {
        sem_wait(&sem_queue);
        Node* node = write_queue.front();
        write_queue.pop();

        const WriteReq* request = node->req;
        const auto path = getServerPath(std::to_string(request->address()), server_id);
        std::cout << "WIFS server PATH WRITE TO: " << path << std::endl;

        const int fd = ::open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG);
        if (fd == -1) std::cout << "open failed " << strerror(errno) << "\n";

        int rc = pwrite(fd, (void*)request->buf().c_str(), BLOCK_SIZE, request->address());
        if (rc == -1) std::cout << "write failed " << strerror(errno) << "\n";
        node->promise_obj.set_value(rc);
    }
    return;
}

int remote_write(const WriteRequest& write_req) {
    // make grpc call to other node ONLY if (loq_queue.empty() && !other_node_syncing)
    // and if that fails, log write operation.
    int pending_writes = 0;
    sem_getvalue(&sem_log_queue, &pending_writes);
    if (pending_writes || other_node_syncing) return -1;

    WriteResponse reply;
    ClientContext context;
    Status status = client_stub_->Write(&context, write_req, &reply);
    // assuming the write never fails when the connection goes through.
    // if call fails, try one more time
    if (!status.ok()) {
        init_connection_with_other_node();
        ClientContext context;
        status = client_stub_->Write(&context, write_req, &reply);
    }
    return status.ok() ? 0 : -1;
}

int append_write_request(const WriteReq* request) {
    std::promise<int> promise_obj;
    std::future<int> future_obj = promise_obj.get_future();
    Node node(request, promise_obj);

    sem_wait(&mutex_queue);

    write_queue.push(&node);
    sem_post(&sem_queue);

    // write request to other node
    WriteRequest write_request;
    write_request.set_blk_address(request->address());
    write_request.set_buffer(request->buf());

    sem_wait(&mutex_pending_grpc_write);
    pending_write_address = request->address();
    sem_post(&mutex_pending_grpc_write);

    if (remote_write(write_request) == -1) {
        std::cout << "appending to failure log\n";
        log_queue.push(write_request);
        sem_post(&sem_log_queue);
    }
    sem_post(&mutex_queue);

    sem_wait(&mutex_pending_grpc_write);
    pending_write_address = -1;
    sem_post(&mutex_pending_grpc_write);

    return future_obj.get();
}

class PrimarybackupServiceImplementation final : public PrimaryBackup::Service {
    Status Ping(ServerContext* context, const HeartBeat* request, HeartBeat* reply) {
        reply->set_state(server_state == "INIT" ? primarybackup::HeartBeat_State_INIT : primarybackup::HeartBeat_State_READY);
        return Status::OK;
    }

    Status Write(ServerContext* context, const WriteRequest* request, WriteResponse* reply) {
        std::promise<int> promise_obj;
        std::future<int> future_obj = promise_obj.get_future();

        // should have used the same proto to share write request.
        Node node((WriteReq*)request, promise_obj);

        sem_wait(&mutex_queue);
        write_queue.push(&node);
        sem_post(&mutex_queue);
        sem_post(&sem_queue);

        reply->set_status(future_obj.get() == -1 ? primarybackup::WriteResponse_Status_FAIL : primarybackup::WriteResponse_Status_PASS);
        return Status::OK;
    }

    Status Sync(ServerContext* context, const HeartBeat* request, ServerWriter<WriteRequest>* writer) {
        int pending_writes = 0;
        sem_getvalue(&sem_log_queue, &pending_writes);
        if (pending_writes) other_node_syncing = true;
        while (pending_writes) {
            for (int i = 0; i < pending_writes; i++) {
                sem_wait(&sem_log_queue);
                writer->Write(log_queue.front());
                log_queue.pop();
            }
            sem_getvalue(&sem_log_queue, &pending_writes);
        }
        return Status::OK;
    }

    Status CheckSync(ServerContext* context, const HeartBeat* request, HeartBeat* reply) {
        int pending_writes = 0;
        sem_getvalue(&sem_log_queue, &pending_writes);
        reply->set_state(pending_writes ? primarybackup::HeartBeat_State_INIT : primarybackup::HeartBeat_State_READY);
        if (!pending_writes) other_node_syncing = false;
        return Status::OK;
    }

    Status Init(ServerContext* context, const InitReq* request, InitRes* reply){
        // int pending_writes = 0;
        // sem_getvalue(&sem_log_queue, &pending_writes);
        int sem_value;
        sem_getvalue(&mutex_election,&sem_value);
        if (sem_value == 0){
            election_state = "CANDIDATE";
            reply->set_status(0);
            return Status::OK;
        }
        sem_wait(&mutex_election);
        std::cout << "Election RPC Received!" <<std::endl;
        if (request->role() == primarybackup::InitReq_Role_LEADER) {
            if(election_state == "INIT" or election_state=="BACKUP"){
                std::cout << "Other candidate accepted as PRIMARY. This server BACKUP" << std::endl;
                reply->set_role(primarybackup::InitRes_Role_BACKUP);
                reply->set_status(1);
                election_state = "BACKUP";
            }
            else if (election_state == "PRIMARY"){
                reply->set_status(1);
                std::cout << "Other candidate rejected as PRIMARY. This server PRIMARYç" << std::endl;
                reply->set_role(primarybackup::InitRes_Role_PRIMARY);
            }
            else if (election_state == "CANDIDATE"){
                reply->set_status(0);
            }
        }
        sem_post(&mutex_election);
        return Status::OK;     
    }
};

class WifsServiceImplementation final : public WIFS::Service {

    Status wifs_WRITE(ServerContext* context, const WriteReq* request,
                      WriteRes* reply) override {
        // check if this is primary or not, and then only do the write.
        // make this change after merging with Adil's branch.
        sem_wait(&mutex_election);
        if (election_state != "PRIMARY"){
            reply->set_status(wifs::WriteRes_Status_RETRY); //retry with the primary IP provided in the next line
            reply->set_primary_ip(primary);
            sem_post(&mutex_election);
            return Status::OK;
        }
        sem_post(&mutex_election);
        

        reply->set_status(wifs::WriteRes_Status_FAIL);
        if (append_write_request(request) == -1) return Status::OK;
        reply->set_status(wifs::WriteRes_Status_PASS);

        

        return Status::OK;
    }

    Status wifs_INIT(ServerContext* context, const ClientInitReq* request,
                      ClientInitRes* reply) override {
        sem_wait(&mutex_election);
        reply->set_primary_ip(primary);
        sem_post(&mutex_election);
        return Status::OK;
    }

    Status wifs_READ(ServerContext* context, const ReadReq* request,
                     ReadRes* reply) override {
        bool is_grpc_write_pending = false;
        sem_wait(&mutex_pending_grpc_write);
        is_grpc_write_pending = pending_write_address == request->address();
        sem_post(&mutex_pending_grpc_write);

        sem_wait(&mutex_election);
        reply->set_primary_ip(primary);
        std::cout<< "Primary IP is " << primary << std::endl;
        sem_post(&mutex_election);

        if (is_grpc_write_pending) {
            // if the other node is down, then the client library should make a thrird call to the
            // earlier node, it will most probably be served.
            // this will happen when the other node is down, is_grpc_write_pending is set, and a read is called
            // before pushing the write to failure log and resetting is_grpc_write_pending.
            reply->set_status(wifs::ReadRes_Status_RETRY);
            reply->set_node_ip(cur_node_wifs_address == ip_server_wifs_1 ? ip_server_wifs_2 : ip_server_wifs_1);
            return Status::OK;
        }

        const auto path = getServerPath(std::to_string(request->address()), server_id);
        std::cout << "WIFS server PATH READ: " << path << std::endl;

        const int fd = ::open(path.c_str(), O_RDONLY);
        if (fd == -1) {
            reply->set_status(wifs::ReadRes_Status_FAIL);
            return Status::OK;
        }

        std::string buffer(BLOCK_SIZE, ' ');
        std::ifstream file_inp(path);
        buffer.assign((std::istreambuf_iterator<char>(file_inp)), std::istreambuf_iterator<char>());
        reply->set_status(wifs::ReadRes_Status_PASS);
        reply->set_buf(buffer);
        return Status::OK;
    }
};

void run_wifs_server() {
    std::string node_address;
    if (server_id == 1) {
        node_address = ip_server_wifs_1;
        other_node_wifs_address = ip_server_wifs_2;
    } else {
        node_address = ip_server_wifs_2;
        other_node_wifs_address = ip_server_wifs_1;
    }
    std::string address(node_address);
    WifsServiceImplementation service;
    ServerBuilder wifsServer;
    wifsServer.AddListeningPort(address, grpc::InsecureServerCredentials());
    wifsServer.RegisterService(&service);
    std::unique_ptr<Server> server(wifsServer.BuildAndStart());
    std::cout << "WIFS Server listening on port: " << address << std::endl;
    sem_wait(&mutex_election);
    if(election_state == "PRIMARY"){
        primary = node_address;

    }
    if(election_state == "BACKUP"){
        primary = other_node_wifs_address;
    }
    sem_post(&mutex_election);

    server->Wait();
}

void run_pb_server(int server_id) {
    std::string node_address;
    if (server_id == 1) {
        node_address = ip_server_pb_1;
    } else {
        node_address = ip_server_pb_2;
    }
    cur_node_wifs_address = node_address;
    std::string address(node_address);
    PrimarybackupServiceImplementation service;
    ServerBuilder pbServer;
    pbServer.AddListeningPort(address, grpc::InsecureServerCredentials());
    pbServer.RegisterService(&service);
    std::unique_ptr<Server> server(pbServer.BuildAndStart());
    sem_post(&sem_concensus);
    std::cout << "PB Server listening on port: " << address << std::endl;
    server->Wait();
}

void init_connection_with_other_node(std::string other_node_address) {
    client_stub_ = PrimaryBackup::NewStub(grpc::CreateChannel(other_node_address, grpc::InsecureChannelCredentials()));
}

void concensus(){
    sem_wait(&sem_concensus);
    std::cout << "Election begins. Waiting for mutex release" <<std::endl;
    sem_wait(&mutex_election);
    ClientContext context;
    InitReq request;
    InitRes reply;
    //Start elections if PRIMARY (other server) has no heartbeat or during INIT.
    if (election_state == "INIT" or election_state == "CANDIDATE"){
        std::cout << "No heartbeat received. Server is a candidate" <<std::endl;
        election_state = "CANDIDATE";
        request.set_role(primarybackup::InitReq_Role_LEADER);
        Status status = client_stub_->Init(&context, request, &reply);
        if(status.ok()) {
            std::cout <<"Status is OK" <<std::endl;
            if(reply.status() == 0) {
                std::cout << "Both servers are candidates simultaneously! Retrying Election" <<std::endl;
                sem_post(&mutex_election);
                sem_post(&sem_concensus);
                int randTime = 10000 + rand() % 100000;
                usleep(randTime);
                return concensus();
            }
            else if (reply.status() == 1){
                if (reply.role() == primarybackup::InitRes_Role_PRIMARY){
                    std::cout << "Other server is Primary. This server is now backup" <<std::endl;
                    election_state = "BACKUP";
                }
                else if (reply.role() == primarybackup::InitRes_Role_BACKUP){
                    std::cout << "Other server is Backup. This server is now Primary" <<std::endl;
                    election_state = "PRIMARY";
                }
            }
        }
        else {
            election_state = "PRIMARY";
            std::cout << "This server is a primary!" <<std::endl; 
        }
        
    }
    // if(election_state == "PRIMARY"){
    //     primary = cur_node_wifs_address;

    // }
    // if(election_state == "BACKUP"){
    //     primary = other_node_wifs_address;
    // }
    sem_post(&mutex_election);
    sem_post(&sem_concensus);
}

void check_heartbeat() {
	std::cout << "Heartbeats" <<std::endl;
    HeartBeat request;
    while (true) {
        if(election_state == "BACKUP"){
            HeartBeat reply;
            ClientContext context;
            Status status = client_stub_->Ping(&context, request, &reply);
            if(!status.ok() || reply.state() != primarybackup::HeartBeat_State_READY){
                std::cout << "No heartbeat on Primary, Start elections" << std::endl;
                // Try to be Primary
                election_state = "CANDIDATE";
                concensus();
            }
            else {
                std::cout << "Primary alive " << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(HEARTBEAT_TIMER));  
    }
}

void update_state_to_latest(int retry_count) {
    HeartBeat request;
    ClientContext context;
    std::unique_ptr<ClientReader<WriteRequest> > reader(client_stub_->Sync(&context, request));
    WriteRequest reply;
    while (reader->Read(&reply)) {
        // don't use the write queue since that will add an overhead of populating a promise obj each time.
        // sync write should be fast, and not like write in normal operation
        const auto path = getServerPath(std::to_string(reply.blk_address()), server_id);
        std::cout << "WIFS server PATH WRITE TO: " << path << std::endl;

        const int fd = ::open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG);
        if (fd == -1) std::cout << "sync open failed " << strerror(errno) << "\n";

        int rc = pwrite(fd, (void*)reply.buffer().c_str(), BLOCK_SIZE, reply.blk_address());
        if (rc == -1) std::cout << "sync write failed " << strerror(errno) << "\n";
    }

    Status status = reader->Finish();
    if (!status.ok()) {
        // implies that the other node is not up
        std::cout << "not able to contant other node\n";
        server_state = "READY";
        if (!retry_count) return update_state_to_latest(1);
        return;
    } else
        std::cout << "was able to contant other node\n";

    // now check if there are any pending log entries that the other node received when we were busy doing the above sync.
    HeartBeat pending_writes;
    ClientContext new_context;
    client_stub_->CheckSync(&new_context, request, &pending_writes);
    if (!status.ok() || pending_writes.state() == primarybackup::HeartBeat_State_READY) {
        // implies that the other node crashed in between when status not okay
        server_state = "READY";
        return;
    }

    // still has writes pending
    return update_state_to_latest(0);
}

int main(int argc, char** argv) {

    srand(time(NULL));

    sem_init(&sem_queue, 0, 0);
    sem_init(&mutex_queue, 0, 1);
    sem_init(&mutex_log_queue, 0, 1);
    sem_init(&mutex_election,0,1);
    sem_init(&sem_concensus,0,0);

    sem_init(&mutex_pending_grpc_write, 0, 1);
    if (argc < 2) {
        std::cout << "Machine id not given\n";
        exit(1);
    }

    server_id = atoi(argv[1]);
    std::cout << "got machine id as " << server_id << "\n";

    if (server_id == 1) {
        other_node_address = ip_server_pb_2;
        this_node_address = ip_server_pb_1;
    } else {
        other_node_address = ip_server_pb_1;
        this_node_address = ip_server_pb_2;
    }
    std::cout << "got other node's address as " << other_node_address << "\n";
    
    init_connection_with_other_node(other_node_address);
    
    update_state_to_latest(0);
    
    std::cout << "synced to latest state\n";
    std::thread writer_thread(local_write);
    std::thread internal_server(run_pb_server, server_id);
    std::thread hb_thread(check_heartbeat);
    concensus();
    //Create server path if it doesn't exist
    DIR* dir = opendir(getServerDir(server_id).c_str());
    if (ENOENT == errno) {
        mkdir(getServerDir(server_id).c_str(), 0777);
    }
    run_wifs_server();
    // internal_server.join();
    // writer_thread.join();
    return 0;
}
