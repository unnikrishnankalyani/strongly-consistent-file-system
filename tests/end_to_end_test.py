from ctypes import *
import os
from tester import *
import time
import hashlib

#Server steps
primary = Server(1)
primary.run_server()

secondary = Server(2)
secondary.run_server()

#Client Steps
#Step 1: Init the client with the IP to use for primary
client1 = Client()
client1.init()
#print("Step 2: Perform the writes")

write_array=[]
read_array = []
for i in range(20):
    write_array.append(get_random_4KB())

for i in range(20):
    client1.write(i * 4096, write_array[i])


for i in range(20):
    read_array.append(client1.read(i * 4096))

for i in range(20):
    print("Write hash: " + str(hashlib.md5(write_array[i].encode()).hexdigest()))
    print("Read hash: " + str(hashlib.md5(read_array[i]).hexdigest()))

primary.server.kill()
secondary.server.kill()

