#include "syscall.h"

int main()
{
    char bufferRecv[256];
    char bufferRead[256];
    char filename[33];
    char filename2[33];

    char *ip = "127.0.0.1"; //localhost
    int port = 8080;
    
    int fileID, fileID2; //ID of file to read from and write to
    int byteRead, byteSend, byteRecv, byteWritten, socketID, state = -1;
    int i, createResult;

    socketID = SocketTCP();
    Write("Please input the file you want to read: ", 41, 1);

    Read(filename, 32, 0);
    Write("Please input the file you want to write: ", 43, 1);
    Read(filename2, 32, 0);

    createResult = Create(filename2);
    if (createResult == -1)
    {
        Write("Create file to write failed. Program will stop now.\n", 30, 1);
        Halt();
    }
    fileID2 = Open(filename2, 0);
    fileID = Open(filename, 0);

    if (fileID != -1 && fileID2 != -1)
    {
        byteRead = Read(bufferRead, 255, fileID);
        bufferRead[byteRead] = '\0';

        state = Connect(socketID, ip, port);
        if (state != -1){
            while (byteRead >= 1){
                byteSend = Send(socketID, bufferRead, byteRead);
                byteRecv = Receive(socketID, bufferRecv, byteSend);
                if (byteRecv == 0)
                {
                    break;
                }
                byteWritten = Write(bufferRecv, byteRecv, fileID2);
                if (byteWritten <= 0)
                {
                    break;
                }
                byteRecv = Read(bufferRead, 255, fileID);
            }
            Close(socketID);
        }
        else{
            PrintString("Cannot connect to server.\n",27);
        }
        Close(fileID2);
        Write(bufferRecv, byteRecv, fileID);
        Close(fileID);
    }
    else
    {
        PrintString("Cannot open file.\n",27);
    } 

    Halt();
}
