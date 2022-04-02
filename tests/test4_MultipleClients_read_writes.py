from ctypes import *
import os
from tester import *
import time

import logging
import threading


# Test 1: Checking read and write bandwidth for multiple clients.

#The helper functions defined in tester.py convert the python objects to c objects that can be used for the functions. 
#Use the same functions for generating ip addresses, write buffers and read buffers

#Server steps
print("Step 1. Initialize Primary with Server ID")
primary = Server(1)
primary.run_server()
print("Step 2. Initialize backup with Server ID")
secondary = Server(2)
secondary.run_server()

time.sleep(2)
#Client Steps
#Step 1: Init the client with the IP to use for primary

k = 10

client = [0]*k
for i in range(k):
    client[i] = Client()    

client_buf = [0]*k
time_writes = [0]*k
time_reads = [0]*k
x = [0]*k
y = [0]*k

def thread_client_write(name, i):
    
    # calculating write times.q
    starttime = time.time()
    client[i].write(i * 4096, client_buf[i])
    time_writes[i] = time.time() - starttime


def thread_client_read(name, i):
    
    # calculating write times.q
    starttime = time.time()
    client[i].read(i * 4096, client_buf[i])
    time_writes[i] = time.time() - starttime


print("Step 2: Perform the writes")
for i in range(k):
    client_buf[i] = get_random_4KB()
    
    x[i] = threading.Thread(target=thread_client_write, args=(1,i))
    x[i].start()
    
# waiting for all write to finish in main thread.
for i in range(k):
    x[i].join()


# print average times.
print("Separate writes : Average write time", sum(time_writes)/len(time_writes))




for i in range(k):
     # client read from same locations.
    y[i] = threading.Thread(target=thread_client_read, args=(1,i))
    y[i].start()


for i in range(k):
    y[i].join()

    #prints for each client
    print("checksum value is", client[i].read_buf == client_buf[i])    
    print("Write time taken is ", time_writes[i])
    print("Read time taken is ", time_reads[i])


# print average times.
print("Separate reads : Average read time", sum(time_reads)/len(time_reads))







print("Kill Servers. Move this step around to simulate failures. Use terminate for graceful shutdown and kill for failure")

primary.server.kill()
secondary.server.kill()


