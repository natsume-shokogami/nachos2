#include "syscall.h"

int main() {
    int a;
    a = Signal("semaphore");
    if (a == 0) {
        Write("Signal for Semaphore successfully.\n", 36, 1);
    }
    Exit(0);
}