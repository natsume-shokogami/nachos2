#include "syscall.h"
#define MAX_FILE_NAME_LENGTH 32

int main(int argc, char* argv[])
{
	int deleteFileId = 0;
	int result = -1;
    int i;

	for (i = 0; i < argc; i++)
    {
        result = -1;
        PrintString(argv[i], MAX_FILE_NAME_LENGTH+1);
        result = Create(argv[i]);

        if (result == -1)
        {
            Write("Create file ", 13, 1);
            Write(argv[i], MAX_FILE_NAME_LENGTH+1, 1);
            Write(" failed\n", 9, 1);
        }
        else
        {
            Write("Create file ", 13, 1);
            Write(argv[i], MAX_FILE_NAME_LENGTH+1, 1);
            Write(" successfully\n", 15, 1);
        }
    }
       
    Exit(0);
}