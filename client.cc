#include <grpcpp/grpcpp.h>
#include <time.h>

#include "WifsClient.h"
#include "wifs.grpc.pb.h"

#include <sys/time.h>

extern "C" {

int init() {
    options.wifsclient[0] = new WifsClient(grpc::CreateChannel(ip_server_wifs_1, grpc::InsecureChannelCredentials()));
    options.wifsclient[1] = new WifsClient(grpc::CreateChannel(ip_server_wifs_2, grpc::InsecureChannelCredentials()));
    primary_server = servers[primary_index];
}

void assign_primary() {
    primary_server = servers[primary_index];
    //init();
    //std::cout << "Changed PRIMARY: " << primary_server << std::endl;
}

void switch_primary(int index) {
    primary_index = 1 - index;  // switches between 1 and 0
    primary_server = servers[primary_index];
    //std::cout<<"primary switched to "<<primary_server<<"\n";
}

int do_read(int address, char* buf, wifs::ReadReq_Crash crash_mode) {
    static int rand_index = 0;
    rand_index++;
    if (primary_server == "") assign_primary();
    //std::cout<< "Current value of primary: "<<primary_server<<std::endl; 
    //std::cout << "Current PRIMARY: " << primary_server << std::endl;
    // if(options.wifsclient[0] == NULL){
    //     assign_primary();
    // }
    read_index = single_server ? primary_index : rand_index % 2;
    //std::cout<<"reading from "<<servers[read_index]<<std::endl;
    
    // Start measuring time
    struct timeval begin, end;
    gettimeofday(&begin, 0);
    if(options.wifsclient[0] == NULL or options.wifsclient[1] == NULL){
        std::cout << "Null value for WIFSclient. Exiting" <<std::endl;
        return -1;
    }

    int rc = options.wifsclient[read_index]->wifs_READ(address, buf, crash_mode);

    // Stop measuring time and calculate the elapsed time
    gettimeofday(&end, 0);
    long seconds = end.tv_sec - begin.tv_sec;
    long microseconds = end.tv_usec - begin.tv_usec;
    double elapsed = seconds + microseconds*1e-6;

    //std::cout << elapsed << std::endl;

    //std::cout << "Read Return code: " << rc << std::endl;
    // call goes through, just return
    if (!rc) return 0;

    if (rc < 0) {  // call failed, try other node.
        switch_primary(read_index);
        single_server = 1;
        //std::cout << "Read Call FAILED. Trying other node" << primary_server << std::endl;
        return do_read(address, buf, wifs::ReadReq_Crash_NO_CRASH);
    }

    if (rc == 1) {
        switch_primary(read_index);
        single_server = 0;
        return 0;
    }

    if (rc == 2) {  // current server is running in solo mode
        primary_index = read_index;
        primary_server = servers[primary_index];
        single_server = 1;
        //std::cout << "Server running in SOLO mode. No more read distribution" << primary_server << std::endl;
        return 0;
    }

    // now rc is 3, which means the read will be serviced by the other node, but no change in primary
    // read couldn't be serviced by this node, probably blocking grpc update operation
    // don't swtich primary.
    rc = options.wifsclient[1 - read_index]->wifs_READ(address, buf, crash_mode);
    std::cout << "Read Return code: " << rc << std::endl;

    if (rc < 0) {
        switch_primary(read_index);
        single_server = 1;
        //std::cout << "Read Call FAILED. Trying other node" << primary_server << std::endl;
        return do_read(address, buf, wifs::ReadReq_Crash_NO_CRASH);
    }
    // rc should never be 1 here.
    if (rc == 2) {
        primary_index = read_index;
        primary_server = servers[primary_index];
        single_server = 1;
        return 0;
    }
    return -1;  // this should never happen.
}

int do_write(int address, char* buf, wifs::WriteReq_Crash crash_mode) {
    if (options.wifsclient[0] == NULL) assign_primary();
    struct timeval begin, end;
    gettimeofday(&begin, 0);
      if(options.wifsclient[0] == NULL or options.wifsclient[1] == NULL){
        //std::cout << "Null value for WIFSclient. Exiting" <<std::endl;
        return -1;
    }
    int rc = options.wifsclient[primary_index]->wifs_WRITE(address, buf, crash_mode);
    //std::cout << "Write Return code: " << rc << std::endl;
    // call goes through, just return
    
    long seconds = end.tv_sec - begin.tv_sec;
    long microseconds = end.tv_usec - begin.tv_usec;
    double elapsed = seconds + microseconds*1e-6;
    std::cout << elapsed << std::endl;
    if (!rc) return 0;

    if (rc < 0) {  // call failed
        switch_primary(primary_index);
        single_server = 1;
        //std::cout << "Write Call FAILED. Trying other node" << primary_server << std::endl;
        return do_write(address, buf, wifs::WriteReq_Crash_NO_CRASH);  // repeat operation
    }

    if (rc == 1) {  // primary has changed
        switch_primary(primary_index);
        single_server = 0;
        //std::cout << "Changed PRIMARY: " << primary_server << std::endl;
        //std::cout << "Repeating WRITE" << std::endl;
        return do_write(address, buf, crash_mode);
    }

    if (rc == 2) {  // server running solo
        single_server = 1;
        //std::cout << "Server running in SOLO mode. No more read distribution" << primary_server << std::endl;
        return 0;
    }
    gettimeofday(&end, 0);
    
    return -1;
}
}
