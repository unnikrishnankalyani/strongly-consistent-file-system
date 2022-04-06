# Strongly Consistent Redundancy-based Advanced Block Store (SCRABS)

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

On the server specificed in `server_1` earlier, cd into the `cmake/build` directory and run the command `./server 1`
On the server specificed in `server_2` earlier, cd into the `cmake/build` directory and run the command `./server 2`

## Testing
We provide a python test suite that performs a series of reads and writes on the server and tests the results using checksums. The scripts also insert failures at deterministic points. The scripts can simply be run using python3.
`python3 test1.py`
