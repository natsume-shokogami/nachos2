#include "syscall.h"

int main(){
    char *argv[2];
    int argc, result, newProc, newProc2;
    argc = 2;
    argv[0] = "./createfile";
    argv[1] = "./cat";

    newProc = Exec(argv[0]);
    
    Join(newProc);
    newProc2 = Exec(argv[1]);
    Join(newProc2);
    Exit(0);
}