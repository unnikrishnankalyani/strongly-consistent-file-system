#include "commonheaders.h"
extern "C" {
int do_read(int address, char* buf);
int do_write(int address, char* buf);
int init(const char* server_ip);
}