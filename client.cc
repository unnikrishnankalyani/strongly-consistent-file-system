#include <grpcpp/grpcpp.h>
#include <sys/stat.h>
#include <time.h>
#include <iostream>

#include "WifsClient.h"
#include "wifs.grpc.pb.h"

static struct options {
    WifsClient* wifsclient;
    int show_help;
    std::string primary = ip_server_wifs_1;
} options;

extern "C" {

int init_old(){
    for(int i=0;i<sizeof(servers);i++){
	    std::string ip_address = servers[i];
        options.wifsclient = new WifsClient(grpc::CreateChannel(ip_address, grpc::InsecureChannelCredentials()));
        char* ip;
        std::cout<<"Asking server IP of primary " << std::endl;
        int rc = options.wifsclient->wifs_INIT(ip);
        std::cout<<"Server informed that primary ip is "<< std::string(ip) << std::endl;
        if(rc >= 0){
            options.wifsclient = new WifsClient(grpc::CreateChannel(std::string(ip), grpc::InsecureChannelCredentials()));
            break;
        }
    }
}

int init(){
    options.wifsclient = new WifsClient(grpc::CreateChannel(ip_server_wifs_1, grpc::InsecureChannelCredentials()));
}

int do_read(int address, char* buf) {
    int rc = options.wifsclient->wifs_READ(address, buf);
    return rc;    
}
	

int do_write(int address, char* buf) {
    int rc = options.wifsclient->wifs_WRITE(address, buf);
    return rc;
}

}
