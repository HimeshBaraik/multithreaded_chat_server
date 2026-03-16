#include<iostream>
#include<winsock2.h>
#include<ws2tcpip.h>
#include<tchar.h>
#include<thread>
using namespace std;
#pragma comment(lib, "ws2_32.lib")

/*
1. initialize winsock
2. create a socket
3. connect to the server
4. send and receive data
5. close the socket
*/

bool Initialize(){
    WSADATA data;
    return WSAStartup(MAKEWORD(2,2), &data) == 0;
}

void MessageSend(SOCKET connectSocket){
    cout<<"Enter your chat name: ";
    string chatName;
    getline(cin, chatName);

    while (1)
    {
        string sendbuf;
        cout<<"Enter message to send to server: ";
        getline(cin, sendbuf);
        sendbuf = chatName + ": " + sendbuf;

        int bytesSent;
        bytesSent = send(connectSocket, sendbuf.c_str(), sendbuf.size(), 0);
        if(bytesSent == SOCKET_ERROR){
            cout <<"Error at send():"<< WSAGetLastError() <<endl;
            closesocket(connectSocket);
            WSACleanup();
            return;
        }

        if(sendbuf == "exit"){
            cout<<"Exiting chat..."<<endl;
            closesocket(connectSocket);
            WSACleanup();
            return;

        }
        else{
            cout<<"Successfully sent message"<<endl;
        }
    }
}   

void MessageReceive(SOCKET connectSocket){
    int recvbuflen = 4096;
    char recvbuf[recvbuflen];

    while (1)
    {
        int bytesReceived = recv(connectSocket, recvbuf, recvbuflen, 0);
        if(bytesReceived == SOCKET_ERROR){
            cout<<"Server disconnected."<<endl;
            break;
        }
        if(bytesReceived < 0){
            cout <<"Error at recv():"<< WSAGetLastError() <<endl;
            return;
        }
        else{
            string receivedMessage(recvbuf, bytesReceived);
            cout<<endl<<"Received "<<bytesReceived<<" bytes from server: "<<receivedMessage<<endl;
        }
    }

    closesocket(connectSocket);
    WSACleanup();
}

int main(){

    cout<<"client program"<<endl;

    // 1. initialize winsock
    if(!Initialize()){
        cout<<"Initialization failed"<<endl;
        return -1;
    }

    // 2. create a socket
    SOCKET connectSocket;
    connectSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(connectSocket == INVALID_SOCKET){
        cout <<"Error at socket():"<< WSAGetLastError() <<endl;
        WSACleanup();
        return -1;
    }

    // 3. connect to the server
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET; // IPv4
    int port = 12345;
    serverAddr.sin_port = htons(port); // host to network byte order
    string serverIp = "127.0.0.1";
    if(inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr) == 0){
        cout <<"Error at InetPton():"<< WSAGetLastError() <<endl;
        closesocket(connectSocket);
        WSACleanup();
        return -1;
    }

    // connect to the server
    if(connect(connectSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR){
        cout <<"Error at connect():"<< WSAGetLastError() <<endl;
        closesocket(connectSocket);
        WSACleanup();
        return -1;
    }

    thread sendThread(MessageSend, connectSocket);
    thread recvThread(MessageReceive, connectSocket);

    sendThread.join();
    recvThread.join();

    WSACleanup();
    return 0;
}