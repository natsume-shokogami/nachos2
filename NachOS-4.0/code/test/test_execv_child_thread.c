#include "syscall.h"

int main(int argc, char* argv[]){
    int i;
    Write("Child process to test ExecV.\n", 30, 1);
    Write(argv[0], 60, 1);
    for (i = 0; i < argc; i++){
        Write("--", 3, 1);
        Write(argv[i], 128, 1);
    }
    Exit(0);
}