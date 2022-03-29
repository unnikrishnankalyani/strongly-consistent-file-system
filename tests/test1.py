from ctypes import *
import os
from tester import *
# Test 1: Write 4096 bytes to an address and read the bytes


#The helper functions defined in tester.py convert the python objects to c objects that can be used for the functions. 
#Use the same functions for generating ip addresses, write buffers and read buffers

#Server steps
#Step 1. Initialize Primary with Server ID
primary = Server(1)
primary.run_server()
# #Step 2. Initialize backup with Server ID
secondary = Server(2)
secondary.run_server()

#Client Steps
#Step 1: Init the client with the IP to use for primary
init(primary_ip)

#Step 2: Set the offset to read/write from
address = get_offset(0)

#Step 3: Set the write buffer to be written
write_buf = get_write_buffer("e" * 4096)

#Step 4: Set the read buffer to be filled
read_buf = get_read_buffer(4096)

#Step 5: Perform the write
do_write(address,write_buf)

#Step 6: Perform the read
do_read(address, read_buf)

#Step 7: Check result
print(read_buf.value)


#Kill Servers. Move this step around to simulate failures. Use terminate for graceful shutdown and kill for failure

primary.server.kill()
secondary.server.kill()



