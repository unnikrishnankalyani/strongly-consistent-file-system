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

char root_path[MAX_PATH_LENGTH];
std::string server_state = "INIT";

// semaphore for checking if write queue has elements to read
sem_t sem_queue;

// semaphore for checking if log queue has elements to read
sem_t sem_log_queue;

// used to achieve mutual exclusion during enqueue operation on write queue
sem_t mutex_queue;

// used to achieve mutual exclusion during enqueue operation on log queue
sem_t mutex_log_queue;

bool other_node_syncing = false;

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
        const auto path = getServerPath(std::to_string(request->address()));
        std::cout << "WIFS server PATH WRITE TO: " << path << std::endl;

        const int fd = ::open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG);
        if (fd == -1) std::cout << "open failed " << strerror(errno) << "\n";

        int rc = write(fd, (void*)request->buf().c_str(), BLOCK_SIZE);
        if (rc == -1) std::cout << "write failed " << strerror(errno) << "\n";
        node->promise_obj.set_value(rc);
    }
    return;
}

int remote_write(const WriteRequest& write_req) {
    // make grpc call to other node ONLY if (loq_queue.empty() && !other_node_syncing)
    // and if that fails, log write operation.

    return -1;
}

int append_write_request(const WriteReq* request) {
    std::promise<int> promise_obj;
    std::future<int> future_obj = promise_obj.get_future();
    Node node(request, promise_obj);

    sem_wait(&mutex_queue);
    write_queue.push(&node);
    sem_post(&mutex_queue);
    sem_post(&sem_queue);

    WriteRequest write_request;
    write_request.set_blk_address(request->address());
    write_request.set_buffer(request->buf());
    if (remote_write(write_request) == -1) {
        sem_wait(&mutex_log_queue);
        log_queue.push(write_request);
        sem_post(&sem_log_queue);
        sem_post(&mutex_log_queue);
    }
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
        Node node(request, promise_obj);

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
};

class WifsServiceImplementation final : public WIFS::Service {
    Status wifs_WRITE(ServerContext* context, const WriteReq* request,
                      WriteRes* reply) override {
        struct stat info;
        reply->set_status(-1);

        if (append_write_request(request) == -1) return Status::OK;
        reply->set_status(0);
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

void run_wifs_server() {
    std::string address("localhost:50051");
    WifsServiceImplementation service;
    ServerBuilder wifsServer;
    wifsServer.AddListeningPort(address, grpc::InsecureServerCredentials());
    wifsServer.RegisterService(&service);
    std::unique_ptr<Server> server(wifsServer.BuildAndStart());
    std::cout << "Server listening on port: " << address << std::endl;
    server->Wait();
}

void run_pb_server(std::string other_node_address) {
    std::string cur_port = "50052";
    // all this is required when you are trying to run both primary and backup on the same machine.
    // just comment out the below line when running primary and backup on different machines.
    if (other_node_address.substr(other_node_address.length() - 5, 5) == "50052") cur_port = "50053";

    std::string address("localhost:" + cur_port);
    PrimarybackupServiceImplementation service;
    ServerBuilder pbServer;
    pbServer.AddListeningPort(address, grpc::InsecureServerCredentials());
    pbServer.RegisterService(&service);
    std::unique_ptr<Server> server(pbServer.BuildAndStart());
    std::cout << "Server listening on port: " << address << std::endl;
    server->Wait();
}

int main(int argc, char** argv) {
    sem_init(&sem_queue, 0, 0);
    sem_init(&mutex_queue, 0, 0);
    // if (argc == 1) {
    //     std::cout << "address of other node not given\n";
    //     exit(1);
    // }

    // std::string other_node_address(argv[1]);
    // std::cout << "got other node's address as " << other_node_address << "\n";

    // init_connection_with_other_node(other_node_address);
    std::thread writer_thread(local_write);
    run_wifs_server();
    // run_pb_server(other_node_address);
    writer_thread.join();
    return 0;
}
