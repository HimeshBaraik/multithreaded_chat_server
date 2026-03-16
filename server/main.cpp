#include<iostream>
#include<winsock2.h>
#include<ws2tcpip.h>
#include<tchar.h>
#include<thread>
#include<vector>
#include<algorithm>
#include<mutex>
using namespace std;

#include"config.h"
#include "../threadpool/threadpool.h"

#pragma comment(lib, "ws2_32.lib")

/*
1. initialize winsock
2. create a socket
3. bind the socket
4. listen on the socket
5. accept a connection - blocks until a client connects
6. receive and send data - recv is blocking, it will wait until data is received from the client
7. close the socket
*/

bool Initialize(){
    WSADATA data;
    return WSAStartup(MAKEWORD(2,2), &data) == 0;
}

mutex clientsMutex;

void AcceptThread(SOCKET clientSocket, vector<SOCKET>& clientSockets){
    cout<<"Client connected. Starting communication thread..."<<endl;

    // 6. receive and send

    // receive data from the client
    int recvbuflen = 4096;
    char recvbuf[recvbuflen];

    while (1)
    {
        // recv is blocking, it will wait until data is received from the client
        int bytesReceived = recv(clientSocket, recvbuf, recvbuflen, 0);
        if(bytesReceived == SOCKET_ERROR){
            cout<<"Client disconnected."<<endl;
            break;
        }
        if(bytesReceived < 0){
            cout <<"Error at recv():"<< WSAGetLastError() <<endl;
            return;
        }
        else{
            cout<<"recv() is OK!"<<endl;
            string receivedMessage(recvbuf, bytesReceived);
            cout<<"Received "<<bytesReceived<<" bytes from"<<receivedMessage<<endl;
        }

        lock_guard<mutex> lock(clientsMutex);

        for(auto& client : clientSockets){
            if(client != clientSocket){
                int sendResult = send(client, recvbuf, bytesReceived, 0);
                if(sendResult == SOCKET_ERROR){
                    cout <<"Error at send():"<< WSAGetLastError() <<endl;
                    return;
                }
                else{
                    cout<<"send() is OK!"<<endl;
                    cout<<"Sent "<<sendResult<<" bytes to all the connected clients."<<endl;
                }
            }
        }
    }
    
    // remove the client socket from the list and close it
    // NEW - protect vector modification
    {
        lock_guard<mutex> lock(clientsMutex);

        auto it = find(clientSockets.begin(), clientSockets.end(), clientSocket);
        if(it != clientSockets.end()){
            clientSockets.erase(it);
        }
    }

    closesocket(clientSocket);
}

int main(){
    ServerConfig config;
    cout<<"server program"<<endl;

    // 1. initialize winsock
    if(!Initialize()){
        cout<<"Initialization failed"<<endl;
        return -1;
    }


    // 2. create a socket
    SOCKET listenSocket;
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(listenSocket == INVALID_SOCKET){
        cout <<"Error at socket():"<< WSAGetLastError() <<endl;
        WSACleanup();
        return -1;
    }

    // 3. bind the socket

    // address structure for bind
    int port = config.port;
    string ip = config.ip;

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET; // IPv4
    serverAddr.sin_port = htons(port); // host to network byte order

    // inet_pton converts the IP address from string format to binary format and stores it in serverAddr.sin_addr
    if(inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) == 0){
        cout <<"Error at InetPton():"<< WSAGetLastError() <<endl;
        closesocket(listenSocket);
        WSACleanup();
        return -1;
    }

    // bind the socket to the address and port
    int bindingResult = bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if(bindingResult == SOCKET_ERROR){
        cout <<"Error at bind():"<< WSAGetLastError() <<endl;
        closesocket(listenSocket);
        WSACleanup();
        return -1;
    }

    // 4. listen on the socket
    // SOMAXCONN is the maximum number of pending connections the queue will hold (backlog)
    if(listen(listenSocket, SOMAXCONN) == SOCKET_ERROR){
        cout <<"Error at listen():"<< WSAGetLastError() <<endl;
        closesocket(listenSocket);
        WSACleanup();
        return -1;
    }

    cout<<"Server has started listening on port "<<port<<". Waiting for client to connect..."<<endl;

    vector<SOCKET> clientSockets;

    Threadpool pool(16);

    // 5. accept a connection -> blocks until a client connects
    while (true)
    {
        // accept returns a new socket for the client connection, the listenSocket continues to listen for new connections
        SOCKET clientSocket;
        clientSocket = accept(listenSocket, NULL, NULL);
        if(clientSocket == INVALID_SOCKET){
            cout <<"Error at accept():"<< WSAGetLastError() <<endl;
            closesocket(listenSocket);
            WSACleanup();
            return -1;
        }

        {
            lock_guard<mutex> lock(clientsMutex);
            clientSockets.push_back(clientSocket);
        }

        pool.ExecuteTask(AcceptThread, clientSocket, ref(clientSockets)); // NEW - use threadpool
    }
    

    WSACleanup();
    return 0;
}