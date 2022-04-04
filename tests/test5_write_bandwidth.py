from ctypes import *
import os
from tester import *
import time

import logging
import threading


# Test 4: Checking read and write bandwidth for multiple clients.



#Server steps: for this test initialize primary and backup on different machines.
#+++++++++++++++++++++++++++++++++++++++++++++++++++
# print("Step 1. Initialize Primary with Server ID")
# primary = Server(1)
# primary.run_server()
# print("Step 2. Initialize backup with Server ID")
# secondary = Server(2)
# secondary.run_server()
#+++++++++++++++++++++++++++++++++++++++++++++++++++


#Client Steps
#Step 1: Init the clients.

num_clients = 8192

client = [0]*(num_clients+1)

for i in range(num_clients):
    client[i] = Client()    

# buffer, write and read variable initializations
client_buf = [None]*(num_clients+1)
time_writes = [None]*(num_clients+1)
time_reads = [None]*(num_clients+1)
x = [None]*(num_clients+1)
y = [None]*(num_clients+1)


#making global random buffer for all threads to write.
client_buf_main = get_random_4KB()

#threads definition
def thread_client_write(i):
    client[i].write(4096*i, client_buf_main)
    pass


def thread_client_read(i):
    client[i].read(4096)


# client[0].init()

# Perform the writes.++++++++++++++++++++++++++++++++++++++++++++++
for i in range(num_clients):
    #print("reached this place 1")
    client_buf[i] = get_random_4KB()    
    x[i] = threading.Thread(target=thread_client_write, args=(i,))
    
starttime = time.time()

for i in range(num_clients):
    #print("reached this place 2")    
    x[i].start()
    
# waiting for all write to finish in main thread.
for i in range(num_clients):
    #print("reached this place 3")
    x[i].join()

#print("reached this place 3.5")
totaltime = time.time() - starttime


b_width = (4096 * num_clients)/ (totaltime * 1024)
#print("reached this place 4")

# output the total time into a file.
file1 = open(f"Bandwidth_{num_clients}.txt","w+")
print(f"Bandwidth value for {num_clients} clients is {b_width} KB/sec", file = file1)
file1.close()
#print("Separate writes : Average write time", sum(time_writes)/len(time_writes))
#print("reached this place 5")



# Perform the reads.++++++++++++++++++++++++++++++++++++++++++++++
# for i in range(k):
#      # client read from same locations.
#     y[i] = threading.Thread(target=thread_client_read, args=(1,i))
#     y[i].start()


# for i in range(k):
#     y[i].join()

    #prints for each client
    # print("checksum value is", client[i].read_buf == client_buf[i])    
    # print("Write time taken is ", time_writes[i])
    # print("Read time taken is ", time_reads[i])


# print average times.
#print("Separate reads : Average read time", sum(time_reads)/len(time_reads))







# print("Kill Servers. Move this step around to simulate failures. Use terminate for graceful shutdown and kill for failure")
# primary.server.kill()
# secondary.server.kill()


