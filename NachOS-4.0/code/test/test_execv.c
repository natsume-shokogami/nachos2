#include "syscall.h"

int main(){
    char* argv[4];
    int newProc, argc;
    argc = 4;
    argv[0] = "./test_execv_child_thread";
    argv[1] = "This is a test program to test ExecV syscall";
    argv[2] = "Child thread is writing this sentence";
    argv[3] = "End of child process";

    newProc = ExecV(argc, argv);
    if (newProc >= 0){
        Join(newProc);
    }
    Exit(0);
}