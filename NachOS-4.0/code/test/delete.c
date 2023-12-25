#include "syscall.h"
#define MAX_FILE_NAME_LENGTH 32

int main(int argc, char* argv[])
{
	int deleteFileId = 0;
	int result = -1;

	char fileName[MAX_FILE_NAME_LENGTH + 1];

	Write("Enter file name to delete (max 32 chars): \n", 44, 1);
	Read(fileName, MAX_FILE_NAME_LENGTH + 1, 0);
	result = Remove(fileName);

	if (result == -1)
	{
		Write("Remove file failed.\n", 21, 1);
	}
	else
	{
		Write("Remove file successfully.\n", 27, 1);
	}
    Exit(0);
}