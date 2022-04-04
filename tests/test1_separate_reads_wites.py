from ctypes import *
import os
from tester import *
import time

#Client Steps
#Step 1: Init the client with the IP to use for primary
client1 = Client()

#print("Step 2: Perform the writes")

time_writes = [0]*100
time_reads = [0]*100
print("Writes")
for i in range(20):
    client_buf = get_random_4KB()
    client1.write(i * 4096, client_buf)

print("Reads")
for i in range(20):
    client1.read(i * 4096)

