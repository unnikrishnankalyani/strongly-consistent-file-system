from ctypes import *
import os
from subprocess import Popen, PIPE, STDOUT, DEVNULL
import threading

ip_master = "localhost:50051"
ip_server_pb_1 = "localhost:50052"
ip_server_pb_2 = "localhost:50053"
ip_server_wifs_1 = "localhost:50054"
ip_server_wifs_2 = "localhost:50055"


primary_ip = c_char_p(ip_server_wifs_1.encode('utf-8'))
backup_ip = c_char_p(ip_server_wifs_2.encode('utf-8'))

wifsClient = CDLL(os.path.abspath("../cmake/build/libclient.so"))

#The helper functions defined here convert the python objects to c objects that can be used for the functions. 

def get_write_buffer(buffer):
    return c_char_p(buffer.encode('utf=8'))

def get_offset(offset):
    return c_int(offset)

def get_read_buffer(size):
    read_buf = create_string_buffer(size)
    return read_buf

init = wifsClient.init
do_read = wifsClient.do_read
do_write = wifsClient.do_write

class Server():
    def __init__(self, server_id):
        self.server_id = server_id

    def run_server(self):
        self.server = Popen([os.path.abspath('../cmake/build/server'), \
            str(self.server_id)], shell=False, close_fds=True, stdout=DEVNULL, stderr = STDOUT)
        #self.server.communicate()