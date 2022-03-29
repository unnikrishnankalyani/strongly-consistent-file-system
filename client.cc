#include <grpcpp/grpcpp.h>
#include <sys/stat.h>
#include <time.h>

#include <iostream>

#include "WifsClient.h"
#include "wifs.grpc.pb.h"

static struct Options {
    WifsClient* wifsclient;
    int show_help;
	std::string primary = ip_server_wifs_1;
} options;


int init(std::string server_ip){
	options.wifsclient = new WifsClient(grpc::CreateChannel(server_ip, grpc::InsecureChannelCredentials()));
	//options.backup = new WifsClient((grpc::CreateChannel(ip_server_wifs2, grpc::InsecureChannelCredentials()));
	options.primary = options.wifsclient->wifs_INIT();
	return 0;
}

int read(int address, char* buf){
	return options.wifsclient->wifs_READ(address, buf);
}

int write(int address, char* buf){
	return options.wifsclient->wifs_READ(address, buf);
}


   
