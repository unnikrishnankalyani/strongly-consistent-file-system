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

int init(const char* server_ip = ip_server_wifs_1.c_str()){
	std::string ip_address(server_ip);
    options.wifsclient = new WifsClient(grpc::CreateChannel(ip_address, grpc::InsecureChannelCredentials()));
}

int do_read(int address, char* buf) {
    int rc = options.wifsclient->wifs_READ(address, buf);

    if(rc < 0){
		init(ip_server_wifs_2.c_str());
		do_read(address, buf);
    }
    return rc;    
}

int do_write(int address, char* buf) {
    int rc = options.wifsclient->wifs_WRITE(address, buf);
    if(rc<0){
		init(ip_server_wifs_2.c_str());
        do_write(address, buf);
    }
    return rc;
}

}
	

