
# Inter-Process Communication (IPC) Project

The target of this project is to demonstrate simple Inter Process Communication (IPC) between client and server.
The IPC is based on shared memory mapping and done by using:
*   **SHM API** (`shm_open`, `shm_close`)
*   **Semaphores** (`sem_init`, `sem_wait`, `sem_post`, `sem_destroy`)
*   **Memory mapped files** (`mmap`, `munmap`)

### IPC Schema:
The client and server create their own SHM areas.
### Server SHM:
Used only to accept client connections.
### Client SHM:
The client creates a unique named (including PID) SHM area used for data transfer.
This scheme helps avoid limitations on active client connections.

## Source Files:
*   `libipc.h`, `libipc.c`: Implements the IPC API, produces `build/libipc.so`.
*   `server.c`: Implements server example and produces `build/server` linked with `libipc.so`.
*   `proto.h`, `proto.c`: Implements simple text based protocol as part of server.
*   `client1.c`: Implements client example and produces `build/client1` linked with `libipc.so`.
*   `client2.c`: Implements the same example like `client1` but links `libipc.so` on runtime using `dlopen`, `dlsym` and `dlclose`.
*   `argv.h`, `argv.c`: Command line argument parser (ugly but easy to use). Built into the server and clients.

## How to Get
~~~
git clone https://github.com/agrigomi/es-test.git
~~~

## How to Build
~~~
cd es-test
make all
~~~

## How to Run Server
Server supports two runtime models: fork by default and threads (-t option).

~~~
export LD_LIBRARY_PATH=./build
build/server --ifc="server-1"
~~~

To see available server options use:
~~~
build/server --help
~~~

## How to Run Client 1
~~~
export LD_LIBRARY_PATH=./build
build/client1 --dst=server-1 --prompt='[client-1] > '
~~~

The result should be prompt '[client-1] >' . Use '?' and enter to see available protocol options.

## How to Run Client 2
~~~
build/client2 --dst=server-1 --libipc=./build/libipc.so --prompt='[client-2] > '
~~~
