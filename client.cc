#include <time.h>
#include <sys/stat.h>
#include <iostream>
#include <grpcpp/grpcpp.h>
#include "wifs.grpc.pb.h"
#include "WifsClient.h"

static struct options {
	WifsClient* wifsclient;
	int show_help;
} options;

int do_read(int address){
	return options.wifsclient->wifs_READ(address);
}

int do_write(int address, char buf[4096]){
	return options.wifsclient->wifs_WRITE(address, buf);
}

void tester(){
	char buf[4096];
	for (int i=0; i<4096;i++){
		buf[i] = 'a';
	}
	do_write(0, buf);
	do_read(0);
}

int main(int argc, char* argv[]){

	options.wifsclient = new WifsClient(grpc::CreateChannel(
  "localhost:50051", grpc::InsecureChannelCredentials()));
    tester();
    return 0;
}
