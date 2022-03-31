#include <time.h>
#include "WifsClient.h"
#include "wifs.grpc.pb.h"
#include <grpcpp/grpcpp.h>


static struct Options {
    WifsClient* wifsclient;
    int show_help;
    std::string primary = "";
} options;

extern "C" {

int init(std::string ip_address){
    options.wifsclient = new WifsClient(grpc::CreateChannel(ip_address, grpc::InsecureChannelCredentials()));
}

int do_read(int address, char* buf) {
    std::cout << "Current PRIMARY: " <<primary_server <<std::endl;
    if (primary_server == ""){
        primary_server = servers[server_index];
        init(primary_server);
    }
    int rc = options.wifsclient->wifs_READ(address, buf);
    if (rc != 0){
        primary_server = servers[1 - server_index]; //switch to other server if rc < 0 (gRPC failure) or rc > 0 (primary IP changed)
        init(primary_server);
        std::cout << "Current PRIMARY: " <<primary_server <<std::endl;
        if (rc<0){ //repeat operation only for failure, not for primary change
            do_read(address,buf);
        }
    }
    return rc;    
}
	

int do_write(int address, char* buf) {
    if (primary_server == ""){
        primary_server = servers[server_index];
        init(primary_server);
        std::cout << "Changed PRIMARY: " <<primary_server <<std::endl;
    }
    int rc =  options.wifsclient->wifs_WRITE(address, buf);
    std::cout << "Write Return code: " << rc <<std::endl;
    if (rc != 0){
        server_index = 1 - server_index; //switches between 1 and 0
        primary_server = servers[server_index]; //switch to other server if rc < 0 (gRPC failure) or rc > 0 (primary IP changed)
        init(primary_server);
        std::cout << "Changed PRIMARY: " <<primary_server <<std::endl; 
        std::cout << "Repeating WRITE" <<std::endl;
        do_write(address,buf);//repeat operation both for failure, and for primary change
    }
    return rc;
}

}
