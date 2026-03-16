# Multithreaded Chat Server (C++)

A **TCP chat server built in C++ using Winsock and a Thread Pool**.
The server accepts multiple clients, receives messages from them, and **broadcasts messages to all connected clients**.

The project demonstrates key **systems programming concepts**:

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

This significantly improves scalability and memory usage.

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

---

# Components

## 1. Server (`server.cpp`)

Responsible for:

* Initializing Winsock
* Creating server socket
* Binding to IP and port
* Listening for incoming connections
* Accepting client connections
* Submitting client tasks to the thread pool

Main loop:

```
while(true)
{
    clientSocket = accept(...)
    add client to clientSockets
    pool.ExecuteTask(AcceptThread, clientSocket)
}
```

---

## 2. Thread Pool (`threadpool.h`)

The thread pool maintains a **fixed set of worker threads** that execute submitted tasks.

### Worker Thread Logic

```
while(true)
{
    wait for task
    execute task
}
```

### Components

| Component          | Purpose                            |
| ------------------ | ---------------------------------- |
| threads            | Worker threads                     |
| tasks              | Queue of pending tasks             |
| mutex              | Protects task queue                |
| condition_variable | Notifies workers when tasks arrive |

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

## 3. Client Handler (`AcceptThread`)

Handles communication with one client.

Flow:

```
while(true)
{
    recv message from client
    broadcast to other clients
}
```

If the client disconnects:

```
remove client from list
close socket
```

---

# Client Management

All connected clients are stored in:

```
vector<SOCKET> clientSockets
```

Operations performed on this vector:

* Add client
* Remove client
* Iterate during broadcast

Since **multiple threads access this vector**, synchronization is required.

---

# Synchronization and Locks

A global mutex is used:

```
mutex clientsMutex
```

### 1. Adding a Client

```
lock_guard<mutex> lock(clientsMutex);
clientSockets.push_back(clientSocket);
```

Prevents race conditions when multiple threads add clients.

---

### 2. Broadcasting Messages

```
lock_guard<mutex> lock(clientsMutex);

for(auto& client : clientSockets)
{
    send(client, message)
}
```

Prevents vector modification during iteration.

---

### 3. Removing a Client

```
lock_guard<mutex> lock(clientsMutex);
clientSockets.erase(it);
```

Ensures safe removal.

---

# Why `lock_guard`?

`lock_guard` uses **RAII**.

```
lock_guard<mutex> lock(mtx)
```

Automatically performs:

```
mtx.lock()
...
mtx.unlock()
```

When the scope ends.

This prevents:

* Deadlocks
* Forgotten unlocks
* Exceptions leaving mutex locked

---

# ThreadPool Synchronization

Inside the thread pool another mutex protects the **task queue**.

Workers wait using a **condition variable**:

```
cv.wait(lock, predicate)
```

Workers sleep until tasks arrive.

When a new task is pushed:

```
tasks.push(...)
cv.notify_one()
```

This wakes a worker thread.

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

# Building the Project

Compile using **g++ (MinGW / MSYS2)**.

From project root:

```
g++ server/server.cpp -o server.exe -lws2_32 -pthread -std=c++17
```

Explanation:

| Flag         | Meaning                 |
| ------------ | ----------------------- |
| `-lws2_32`   | Link Winsock library    |
| `-pthread`   | Enable threading        |
| `-std=c++17` | Use modern C++ features |

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

---

# Future Improvements

Possible upgrades:

* Non-blocking sockets
* Event driven architecture (epoll / IOCP)
* Message framing protocol
* Authentication layer
* Persistent connections
* Load balancing

---

# Learning Outcomes

This project demonstrates practical understanding of:

* TCP networking
* Multithreaded programming
* Thread pool design
* Synchronization primitives
* Resource sharing in concurrent systems

---

# Author

Built as a **systems programming project in C++** to explore networking, concurrency, and scalable server architectures.
