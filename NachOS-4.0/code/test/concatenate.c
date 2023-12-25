#include "syscall.h"

int main() {

    int len;
    
    char buffer[2][257]; //Buffer for string read from first and second file
    char firstFileName[32], secondFileName[32], resultFileName[32];
    int firstFileID, secondFileID, resultFileID;
    int createResult, deleteResult, writeStatus1, writeStatus2;
    int closeStatus[3];

    int byteRead[2]; //Bytes read from first and second file

    Write("Enter first file to read from:\n", 32, 1);
    Read(firstFileName, 32, 0);

    Write("Enter second file to read from:\n", 33, 1);
    Read(secondFileName, 32, 0);

    Write("Enter result file to write to:\n", 32, 1);
    Read(resultFileName, 32, 0);

    //Writing in truncate mode
    deleteResult = Remove(resultFileName);
    createResult = Create(resultFileName);
    if (createResult == -1)
    {
        Write("Cannot truncate. Program will stop now\n", 40, 1);
        Halt();
    }
    firstFileID = Open(firstFileName, 0);
    secondFileID = Open(secondFileName, 0);
    resultFileID = Open(resultFileName, 0);

    if (firstFileID != -1 && secondFileID != -1 && resultFileID != -1)
    {
        Write("Open file successfully.\n", 25, 1);
        
        byteRead[0] = Read(buffer[0], 256, firstFileID);

        byteRead[1] = Read(buffer[1], 256, secondFileID);
        

        while (byteRead[0] >= 1 && byteRead[1] >= 1)
        {
            writeStatus1 =  Write(buffer[0], byteRead[0], resultFileID);
            writeStatus2 = Write(buffer[1], byteRead[1], resultFileID);

            if (byteRead[0] <= 0 || byteRead[1] <= 0){
            Write("\n Reading source files failed\n", 31, 1);
            }
            if (byteRead[0] <= 0 || byteRead[1] <= 0){
                Write("\n Writing to destination file failed\n", 38, 1);
            }
            byteRead[0] = Read(buffer[0], 256, firstFileID);

            byteRead[1] = Read(buffer[1], 256, secondFileID);

        }
        while (byteRead[0] >= 1)
        {
            Write(buffer[0], byteRead[0], resultFileID);
            byteRead[0] = Read(buffer[0], 256, firstFileID);
        }
        while (byteRead[1] >= 1)
        {
            Write(buffer[1], byteRead[1], resultFileID);
            byteRead[1] = Read(buffer[1], 256, secondFileID);
        }

        closeStatus[0] = Close(firstFileID);
        closeStatus[1] = Close(secondFileID);
        closeStatus[2] = Close(resultFileID);
        if (closeStatus[0] != -1 && closeStatus[1] != -1 && closeStatus[2] != -1)
            Write("\n Concatenate and close all files successfully\n", 48, 1);
        else{
            Write("\n Concatenate and close all files failed\n", 42, 1);
            Exit(0);
        }
    }
    else
    {
        Write("\nOpening files failed\n", 23, 1);
    }
    Exit(0);
}