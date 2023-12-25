#include "syscall.h"
int main()
{
    char filename[32];
    Write("Enter file name to create: ", 28, 1);
    Read(filename, 32, 0);

    if (Create(filename) != -1){
        Write("Successfully create file!\n", 27, 1);
    }
    else{
        Write("Unable to create file!\n", 24, 1);
    }

    Exit(0);
    /* not reached */
}