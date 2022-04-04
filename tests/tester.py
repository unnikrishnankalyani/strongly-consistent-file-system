from ctypes import *
import os
from subprocess import Popen, PIPE, STDOUT, DEVNULL
import threading
import enum

import random
import string

ip_master = "localhost:50051"
ip_server_pb_1 = "localhost:50052"
ip_server_pb_2 = "localhost:50053"
ip_server_wifs_1 = "localhost:50054"
ip_server_wifs_2 = "localhost:50055"

servers = [ip_server_wifs_1, ip_server_wifs_2]

class OPS(enum.Enum):
    READ = 1
    WRITE = 2

primary_ip = c_char_p(ip_server_wifs_1.encode('utf-8'))
backup_ip = c_char_p(ip_server_wifs_2.encode('utf-8'))

# wifsClient = CDLL(os.path.abspath("../cmake/build/libclient.so"))
# init = wifsClient.init
# do_read = wifsClient.do_read
# do_write = wifsClient.do_write
#The helper functions defined here convert the python objects to c objects that can be used for the functions. 

def get_write_buffer(buffer):
    return c_char_p(buffer.encode('utf-8'))

def get_offset(offset):
    return c_int(offset)

def get_read_buffer(size):
    read_buf = create_string_buffer(size)
    return read_buf


class Server():
    def __init__(self, server_id):
        self.server_id = server_id

    def run_server(self):
        self.server = Popen([os.path.abspath('../cmake/build/server'), \
            str(self.server_id)], shell=False, close_fds=True)#, stdout=DEVNULL, stderr=STDOUT)
        #self.server.communicate()

class Client():
    def __init__(self):
        self.libclient = CDLL(os.path.abspath("../cmake/build/libclient.so"))
        self.libclient.init()

    def read(self, address, crash_mode=0):
        self.address = get_offset(address)
        self.read_buf = get_read_buffer(4096)
        self.libclient.do_read(self.address, self.read_buf, crash_mode)
        self.read_buf = self.read_buf.value.decode("utf-8")
    
    def write(self, address, buffer,crash_mode=0):
        address = get_offset(address)
        write_buf = get_write_buffer(buffer)
        self.libclient.do_write(address, write_buf,crash_mode)    

    def init(self):
        self.libclient.init()


def get_random_4KB():
    letters = string.ascii_uppercase + string.ascii_lowercase + string.ascii_letters + string.digits + string.punctuation
    temp = ''.join(random.choice(letters) for i in range(4096)) 
    return(temp)
