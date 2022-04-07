from ctypes import *
import os
from tester import *
import time
# Test 1: Write 4096 bytes to an address and read the bytes

#The helper functions defined in tester.py convert the python objects to c objects that can be used for the functions. 
#Use the same functions for generating ip addresses, write buffers and read buffers

#Server steps
# print("Step 1. Initialize Primary with Server ID")
# primary = Server(1)
# primary.run_server()
# print("Step 2. Initialize backup with Server ID")
# secondary = Server(2)
# secondary.run_server()

time.sleep(2)
#Client Steps
#Step 1: Init the client with the IP to use for primary
client1 = Client()


#print("Step 2: Perform the writes")

client_buf = [0]
time_writes = [0]*10
time_reads = [0]*10

for i in range(10):
    client_buf = get_random_4KB()
    
    # calculating write times.q
    starttime = time.time()
    client1.write(i * 4096, client_buf)
    time_writes[i] = time.time() - starttime

for i in range(10):
    
    if(i % 4 ==0):
        continue
    # calculating read times.
    starttime = time.time()
    client1.read(int((i / 4) * 4096))
    time_reads[i] = time.time() - starttime

    #prints for each iteration
    # print("checksum value is", client1.read_buf == client_buf)    
    # print("Write time taken is ", time_writes[i])
    # print("Read time taken is ", time_reads[i])

# print average times.
# print("Separate aligned writes : Average write time", sum(time_writes)/len(time_writes))
# print("Separate unaligned reads : Average read time", sum(time_reads)/len(time_reads))







# print("Kill Servers. Move this step around to simulate failures. Use terminate for graceful shutdown and kill for failure")

# primary.server.kill()
# secondary.server.kill()


