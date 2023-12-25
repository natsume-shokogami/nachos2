#include "syscall.h"

int main()
{   
    
    char bufferRecv[1024];
    char bufferRead[1024];

    char *ip= "127.0.0.1"; //localhost
    
    int port = 8080;
    
    int fileID, byteRead, byteSend, byteRecv, state, i, socketID;
    
    byteRecv = 0;
    socketID = SocketTCP();
    state = Connect(socketID, ip, port);
    if (state != -1){
        Write(" Please input first string to send to server (max 1024 characters): ", 68, 1);
        byteRead = Read(bufferRead, 1024, 0);
        byteSend = Send(socketID, bufferRead, byteRead);

        byteRecv = Receive(socketID, bufferRecv, byteSend);
        
        Write("Received:", 10, 1);
        Write(bufferRecv, byteRecv+1, 1);
        Write("\n", 2, 1);
        Close(socketID);
    }
    else 
    {
        Write("Cannot connect to server... Trying to connect from another socket...\n", 70, 1);
        Close(socketID);
    }

    byteRecv = 0;
    socketID = SocketTCP();
    state = Connect(socketID, ip, port);
    if (state != -1){
        Write(" Please input second string to send to server (max 1024 characters): ", 69, 1);
        byteRead = Read(bufferRead, 1024, 0);

        byteSend = Send(socketID, bufferRead, byteRead);
        byteRecv = Receive(socketID, bufferRecv, byteSend);
        Write("Received:", 10, 1);
        Write(bufferRecv, byteRecv+1, 1);
        Write("\n", 2, 1);
        Close(socketID);
    }
    else 
    {
        Write("Cannot connect to server... Trying to connect from another socket...\n", 70, 1);
        Close(socketID);
    }

    byteRecv = 0;
    socketID = SocketTCP();
    state = Connect(socketID, ip, port);
    if (state != -1){
        Write(" Please input third string to send to server (max 1024 characters): ", 68, 1);
        byteRead = Read(bufferRead, 1024, 0);

        byteSend = Send(socketID, bufferRead, byteRead);
        byteRecv = Receive(socketID, bufferRecv, byteSend);
        Write("Received:", 10, 1);
        Write(bufferRecv, byteRecv+1, 1);
        Write("\n", 2, 1);
        Close(socketID);
    }
    else 
    {
        Write("Cannot connect to server... Trying to connect from another socket...\n", 70, 1);
        Close(socketID);
    }
    
    byteRecv = 0;
    socketID = SocketTCP();
    state = Connect(socketID, ip, port);
    if (state != -1){
        Write(" Please input fourth string to send to server (max 1024 characters): ", 69, 1);
        byteRead = Read(bufferRead, 1024, 0);
        byteSend = Send(socketID, bufferRead, byteRead);
        byteRecv = 0;
        byteRecv = Receive(socketID, bufferRecv, byteSend);
        Write("Received:", 10, 1);
        Write(bufferRecv, byteRecv+1, 1);
        Write("\n", 2, 1);
        Close(socketID);
    }
    else 
    {
        Write("Cannot connect to server... Trying to connect from another socket...\n", 70, 1);
        Close(socketID);
    }
    Halt();
    return 0;
}