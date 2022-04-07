from ctypes import *
import os
from tester import *
import time
import hashlib
import sys

def write_case():
  client1 = Client()
  client1.init()

  client_buf = get_random_4KB()
  client1.write(0, client_buf)
  print("write succeeded, hash {}".format(hashlib.md5(client_buf.encode()).hexdigest()))

  client_buf = get_random_4KB()
  print("trying to write contents with hash {}".format(hashlib.md5(client_buf.encode()).hexdigest()))
  client1.write(0, client_buf, 4)

def read_case():
  client1 = Client()
  client1.init()

  buf = client1.read(0)
  print("read contents with hash {}".format(hashlib.md5(buf).hexdigest()))


if sys.argv[1] == '1':
  write_case()
else:
  read_case()



