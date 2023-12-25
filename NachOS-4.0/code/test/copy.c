#include "syscall.h"
#define FILE_NAME__MAX_LENGTH 32
int main()
{
    int openFileID, destFileID, sourceFileNameLength, destFileNameLength, 
    byteRead, srcCloseStatus, destCloseStatus, createResult, deleteResult;
    char sourceFile[FILE_NAME__MAX_LENGTH];
    char destFile[FILE_NAME__MAX_LENGTH];
    char buffer[1024];

    Write("Enter source file: ", 20, 1);
    sourceFileNameLength = Read(sourceFile, FILE_NAME__MAX_LENGTH, 0);
    Write("\n", 2, 1);
    Write("Enter destination file: ", 25, 1);
    destFileNameLength = Read(destFile, FILE_NAME__MAX_LENGTH, 0);
    Write("\n", 2, 1);

    //Truncate mode
    deleteResult = Remove(destFile);
    createResult = Create(destFile);
    if (createResult == -1)
    {
        Write("Create file to write failed. Program will stop now.\n", 53, 1);
        Exit(0);
    }

    openFileID = Open(sourceFile, 0);
    destFileID = Open(destFile, 0);

    if (openFileID != -1)
    {
        Write("Open file successfully\n", 26, 1);
        
        byteRead = Read(buffer, 1024, openFileID);
        while (byteRead >= 1)
        {
            Write(buffer, byteRead, destFileID);
            byteRead = Read(buffer, 1024, openFileID);
        }

        srcCloseStatus = Close(openFileID);
        destCloseStatus = Close(destFileID);
        if (srcCloseStatus == 0 && destCloseStatus == 0)
            Write("Close files successfully\n", 26, 1);
    }
    Exit(0);
}
