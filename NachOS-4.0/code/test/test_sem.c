#include "syscall.h"

int main() {
    int createSemaphoreResult, execResult;
    createSemaphoreResult = CreateSemaphore("semaphore", 0);
    if (createSemaphoreResult >= 0){
        Write("Create semaphore successfully.\n", 32, 1);
    }
    else{
        Write("Create semaphore failed.\n", 26, 1);
    }

    execResult = Exec("test_sem_thread");
    createSemaphoreResult = Wait("semaphore");
    Join(execResult);
    if (createSemaphoreResult >= 0){
        Write("Wait semaphore successfully.\n", 30, 1);
    }
    else{
        Write("Wait semaphore failed.\n", 24, 1);
    }
    Exit(0);
}