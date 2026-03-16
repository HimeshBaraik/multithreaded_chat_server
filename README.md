# Multithreaded Chat Server (C++)

A **TCP chat server built in C++ using Winsock and a Thread Pool**.
The server accepts multiple clients, receives messages from them, and **broadcasts messages to all connected clients**.
Built as a **systems programming project in C++** to explore networking, concurrency, and scalable server architectures.

The project demonstrates:

* Socket Programming
* Multithreading
* Thread Pool Architecture
* Synchronization with Mutex
* Producer–Consumer pattern using Condition Variables

---

# Architecture Overview

The server follows a **thread-pool based architecture**.

```
Clients
   │
   ▼
TCP Connections
   │
   ▼
accept()  (Main Thread)
   │
   ▼
ThreadPool Task Queue
   │
   ▼
Worker Threads
   │
   ▼
Client Handler (AcceptThread)
   │
   ▼
recv() → broadcast → send()
```

## Thread Pool

A custom **thread pool** was implemented to efficiently handle large numbers of tasks. Instead of creating a new thread for every task, a fixed set of worker threads continuously pulls tasks from a queue and executes them.

During stress testing (task spamming), the **thread pool significantly reduced CPU usage—approximately by half** compared to the thread-per-task approach.The main reason for this improvement is **reduced context switching**.
Benchmark results and CPU usage comparisons are shown in the screenshots below.
1.  Without ThreadPool
<img width="404" height="308" alt="1" src="https://github.com/user-attachments/assets/2c84f06c-4ba3-4b21-9962-1ce8c640cd27" />


2. With ThreadPool
<img width="404" height="308" alt="2" src="https://github.com/user-attachments/assets/4e83d44a-65dc-4803-8758-d8f0c851fa9d" />

---

# Project Structure

```
chat-server/
│
├── server/
│   └── server.cpp
│
├── threadpool/
│   └── threadpool.h
│
├── config/
│   └── config.h
│
└── README.md
```


### Workflow

```
Main Thread
    │
    ▼
Push task to queue
    │
    ▼
notify_one()
    │
    ▼
Worker wakes up
    │
    ▼
Executes task
```

---

# Full Request Flow

Server startup:

```
initialize winsock
create threadpool
listen for connections
```

Client connects:

```
accept()
add client to vector
submit task to threadpool
```

Worker thread:

```
handle recv()
broadcast messages
```

Client disconnects:

```
remove from client list
close socket
```

---

# Running the Server

```
./server/main.exe
```

output:

```
server program
Server has started listening on port 12345
Waiting for client to connect...
```

Running the client: on other terminal
```
./client/main.exe
```
output: 

```
client program
Enter your chat name: 
```

output:

---

# Limitations

Current design uses **blocking sockets**.

```
recv() blocks worker thread
```

If a client is idle, the thread remains occupied.
Possible upgrades:
* Non-blocking sockets

---

# Learning Outcomes

This project demonstrates practical understanding of:

* TCP networking
* Multithreaded programming
* Thread pool design
* Synchronization primitives
* Resource sharing in concurrent systems
