from ctypes import *
import os
from tester import *
import time

# Test 1: Write 4096 bytes to an address and read the bytes

#The helper functions defined in tester.py convert the python objects to c objects that can be used for the functions. 
#Use the same functions for generating ip addresses, write buffers and read buffers


#Client Steps
#Step 1: Init the client with the IP to use for primary
client1 = Client()
#Step 2: Perform the write
client1.write(0, "f" * 4096, 0)

#Step 3: Perform the read
client1.read(0)

#Step 4: Check result
print(client1.read_buf)


#Kill Servers. Move this step around to simulate failures. Use terminate for graceful shutdown and kill for failure
# primary.server.kill()
# secondary.server.kill()
