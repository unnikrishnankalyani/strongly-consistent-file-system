# Strongly Consistent Redundancy-based Utilitarian Block Store (SCRUBS)

This is an implementation of a distributed highly-available file system. All server logic is present in `server.cc` while the client-side library is defined in `client.cc` and `WifsClient.h`. 

## Set-up

To set-up the client, update the `server_1` and `server_2` fields in `commonheaders.h`.

## Compilation

Run the below commands to compile the executables on both servers

  ```
  cd Strongly-Consistent-FS
  mkdir -p cmake/build  
  cd cmake/build  
  cmake -DCMAKE_PREFIX_PATH=$MY_INSTALL_DIR ../..  
  make 
```

## Execution

To run one end-to-end test without any manual steps, skip to the Testing section.

On the server specified in `server_1` earlier, cd into the `cmake/build` directory and run the command `./server 1`
On the server specified in `server_2` earlier, cd into the `cmake/build` directory and run the command `./server 2`

## Testing
We provide a python test suite containing scripts that perform a series of reads and writes on the server and tests the results using checksums. The scripts also insert failures at deterministic points. The scripts can simply be run using python3.
`python3 test1.py`

The failure modes are listed below. To inject a failure, simply add the digit of the mode to the read/write call in the python API.
E.g. `read(1000, 1)` or `write(1000,buffer,2)`
```
NO_CRASH = 0;
PRIMARY_CRASH_BEFORE_LOCAL_WRITE_AFTER_REMOTE = 1;
PRIMARY_CRASH_AFTER_LOCAL_WRITE_AFTER_REMOTE = 2;
PRIMARY_CRASH_BEFORE_LOCAL_WRITE_BEFORE_REMOTE = 3;
PRIMARY_CRASH_AFTER_LOCAL_WRITE_BEFORE_REMOTE = 4;
BACKUP_CRASH_BEFORE_WRITE = 5;
BACKUP_CRASH_AFTER_WRITE = 6;
NODE_CRASH_READ = 7;
```
### End-to-end test

To run an end-to-end test that starts both servers, performs sequences of reads and writes (random data), verifies the checksums of the obtained data and kills the servers - run `python end_to_end_test.py` in the `tests` folder. Remember to kill any running servers on the same node to avoid undefined behavior.
