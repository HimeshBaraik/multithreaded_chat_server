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

### Key Idea

Instead of creating **one thread per client**, the server uses a **fixed number of worker threads**.

Example:

```
1000 clients
     ↓
16 worker threads
```

This significantly improves scalability.


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
./server.exe
```

Example output:

```
server program
Server has started listening on port 12345
Waiting for client to connect...
```

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
