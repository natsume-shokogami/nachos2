#include "syscall.h"
#define maxlen 32
int main()
{
    int len;
    
    char buffer[1024];
    char filename[32];
    int OpenFileID;
    int CloseStatus;

    int byteRead; //Bytes read from file

    Write("Enter file to read from:\n", 26, 1);
    Read(filename, 32, 0);

    OpenFileID = Open(filename, 0);
    if (OpenFileID != -1)
    {
        Write("Open file successfully\n", 24, 1);
        
        byteRead = Read(buffer, 1023, OpenFileID);
        Write("Content: \n", 11, 1);
        while (byteRead >= 1)
        {
            Write(buffer, byteRead, 1);
            byteRead = Read(buffer, 1023, OpenFileID);
        }

        CloseStatus = Close(OpenFileID);
        if (CloseStatus == 0)
            Write("\nClose file successfully\n", 26, 1);
    }
    Exit(0);
}
