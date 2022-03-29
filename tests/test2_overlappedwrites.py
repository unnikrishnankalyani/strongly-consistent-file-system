from ctypes import *
import os
from tester import *
# Test 1: Write 4096 bytes to an address and read the bytes


#The helper functions defined in tester.py convert the python objects to c objects that can be used for the functions. 
#Use the same functions for generating ip addresses, write buffers and read buffers

#Server steps
print("Step 1. Initialize Primary with Server ID")
primary = Server(1)
primary.run_server()
print("Step 2. Initialize backup with Server ID")
secondary = Server(2)
secondary.run_server()

#Client Steps
print("Step 1: Init the client with the IP to use for primary")
client1 = Client(ip_server_wifs_1)

print("Step 2: Perform the writes")

client_buf1 = get_random_4KB()
client_buf2 = get_random_4KB()
client_buf3 = get_random_4KB()
client_buf4 = get_random_4KB()

client1.write(int(0/4 * 4096), client_buf1)
client1.write(int(1/4 * 4096), client_buf2)
client1.write(int(2/4 * 4096), client_buf3)
client1.write(int(3/4 * 4096), client_buf4)





#write 1
print("\nStep 3: Perform the read 1")
client1.read(0 * 4096)

client_buf_final = client_buf1
client_buf_final = client_buf1[ : int(1/4 * 4096)] + client_buf2
client_buf_final = client_buf1[ : int(2/4 * 4096)] + client_buf3
client_buf_final = client_buf1[ : int(3/4 * 4096)] + client_buf4

print("Step 4: Check result")
print(client1.read_buf == client_buf1[:4096])
print(client1.read_buf)
print(client_buf_final[:4096])





print("Kill Servers. Move this step around to simulate failures. Use terminate for graceful shutdown and kill for failure")

primary.server.kill()
secondary.server.kill()



