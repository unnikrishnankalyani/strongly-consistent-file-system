from ctypes import *
import os
from tester import *
import time

import hashlib
import logging
import threading
import sys


num_clients = int(sys.argv[1])

client = [0]*(num_clients+1)

for i in range(num_clients):
    client[i] = Client()    

# buffer, write and read variable initializations
client_buf = [None]*(num_clients+1)
time_writes = [None]*(num_clients+1)
time_reads = [None]*(num_clients+1)
x = [None]*(num_clients+1)
y = [None]*(num_clients+1)

write_array=[]
read_array=[None]*num_clients
for i in range(num_clients):
    write_array.append(get_random_4KB())
#making global random buffer for all threads to write.

#threads definition
def thread_client_write(i):
    client[i].write(4096*i, write_array[i])


def thread_client_read(i):
    read_array[i] = client[i].read(4096*i)


for i in range(num_clients):
    x[i] = threading.Thread(target=thread_client_write, args=(i,))
    y[i] = threading.Thread(target=thread_client_read, args=(i,))
    
starttime = time.time()

for i in range(num_clients):
    x[i].start()  
    
for i in range(num_clients):
    x[i].join()

for i in range(num_clients):
    y[i].start()  
    
for i in range(num_clients):
    y[i].join()

# for i in range(num_clients):
#     read_array[i] = client[i].read_buf

totaltime = time.time() - starttime

print(hashlib.md5(write_array[0].encode()).hexdigest())
print(hashlib.md5(read_array[0]).hexdigest())
for i in range(num_clients):
    assert hashlib.md5(write_array[i].encode()).hexdigest() == hashlib.md5(read_array[i]).hexdigest()

print("Reads and writes at all offsets match!")


b_width = (4096 * num_clients)/ (totaltime * 1024)
print(totaltime)
print(b_width)


# output the total time into a file.
#file1 = open(f"ReadBandwidth_{num_clients}.txt","w+")
print(f"Bandwidth value for {num_clients} clients is {b_width} KB/sec")
