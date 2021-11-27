# Multi-threaded Client Server Model

This operating system networking model allows clients to make requests on the server. The clients are simulated by threads of a single progrem, while the server executes the client requests from distinct user threads employing its own various worker threads.

Hence, it's a system of client-server interfacing, that is optimized by incorporating concurrency and parallelism in linux-based systems that support multi-threading.

## FUNCTIONALITY

**Client program**
- Each user request has 2 main characteristics:
	- The time at which the request has been made
	- The nature of the request/the command issued
- Let’s say there are ‘m’ user requests throughout the course of the simulation.
- In order to simulate different users querying to the same server, you are supposed to create ‘m’ threads: each
representing a single user’s request.
- Each of these user request threads will then try to connect with the server independent of each other (to give
the impression of multiple users from different parts of the world, sending commands to the same server).
- Once the connection to the server has been made, the user request thread can communicate with the
assigned worker thread using a TCP socket.
- Whatever response the user request thread receives from the server, it must output it. (Please see sample for
more details)

**Server program**
- The server will maintain a dictionary. A dictionary is a container that stores mappings of unique keys to values.
- The dictionary values must be common across all clients. The key-value pairs created by one client should be
visible to all other connected clients.
- In order to correctly handle multiple concurrent clients, the server should have a multithreaded architecture
(with a pool of worker threads).
- The server will have ‘n’ worker threads in the thread pool. These ‘n’ threads are the ones which are
supposed to deal with the client requests. As a result, atmax ‘n’ client requests will be handled by the server at
any given instant.
- As soon as the server starts, it must spawn these ‘n’ worker threads.
- Then, the server begins to listen for client requests to connect.
- Whenever a new client’s connection request is accepted (using the accept() system call), the server is
expected to then designate one of the worker threads to deal with the client’s request. [Hint: use a queue in
which the server can push client request entities and worker threads can pop to deal with client requests]
- The worker threads, while dealing with the client requests, might need to perform read/write operations on the
dictionary. They are also expected to send an apt output to the client thread (Please see format below). Also,
whenever a worker thread completes a task, put an artificial sleep of 2 seconds before sending the result back
to the client.

**Commands which can be inputted by the user**
- `insert <key> <value>` : This command is supposed to create a new “key” on the server’s dictionary and set its
value as <value>. Also, in case the “key” already exists on the server, then an appropriate error stating “Key
already exists” should be displayed. If successful, “Insertion successful” should be displayed.
- `delete <key>` : This command is supposed to remove the <key> from the dictionary. If no key with the name
<key> exists, then an error stating “No such key exists” should be displayed. If successful, display the
message “Deletion successful”.
- `update <key> <value>` : This command is supposed to update the value corresponding to <key> on the
server’s dictionary and set its value as <value>. Also, in case the “key” does not exist on the server, then an
appropriate error stating “Key does not exist” should be displayed. If successful, the updated value of the key
should be displayed.
- `concat <key1> <key2>` : Let values corresponding to the keys before execution of this command be {key1:
value_1, key_2:value_2}. Then, the corresponding values after this command's execution should be {key1:
value_1+value_2, key_2: value_2+value_1}. If either of the keys do not exist in the dictionary, an error
message “Concat failed as at least one of the keys does not exist” should be displayed. Else,
the final value of key_2 should be displayed. No input will be provided where <key_1> = <key_2>.
- `fetch <key>` : must display the value corresponding to the key if it exists at the connected server, and an error
“Key does not exist” otherwise.

## INPUT AND RUNNING THE CODE
	
Server: The server will be run by using the following command:
	
		$ ./server <number of worker threads in the thread pool>

Client: The client will be run by using the following command:

		$ ./client
	
- The first line of input would be the total number of user requests throughout the simulation (m).
- The next ‘m’ lines contain description of the user requests in non-decreasing order of the first token
	- `Time in sec after which the request to connect to the server is to be made` `cmd with appropriate arguments`
	
## REPORT

- **CLIENTS.CPP**
	- client spawns `M user threads`, each of which connects to the server individually, on different sockets, to simulate distinct clients. 
	- Then, each locking a mutex, takes a line as input from the terminal which is sent onto the socket.
	- the blocking read system call waits for the server to send the `result` string onto the socket so that the user can print it.

- **SERVER.CPP**
	- server spawns `N worker threads` that wait on the size of the queue of input strings to be > 0, using a conditional variable.
	- Then, after accepting a connection from the a client(user thread), `handle_connection` function pushes the received string on the socket to the queue.
	- A push to the queue is accompanied by 'signal' over the conditional variable to the worker threads, one of which unblocks and pops the string out of the queue
	- The worker threads are capable of operating on client strings and carrying out necessary actions on the `dictionary` simultaneously, while ensuring thread safety by locking only the particularly required keys.
	- once the operation is done, the worker thread sends the `result` string to the respective `client socket` and closes it.
