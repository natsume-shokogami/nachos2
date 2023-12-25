#ifndef STABLE_H
#define STABLE_H
#include "bitmap.h"
#include "sem.h"

#define MAX_SEMAPHORE 10

class STable
{
private:
    Bitmap* bm; // Manage the free slot
    Sem* semTab[MAX_SEMAPHORE];
public:
    // Initial the Sem object, the started value is null
    // Remember to initial the bm object to use
    STable();
    ~STable();
    // Check the semaphore name, create a new one if not already
    int Create(char *name, int init);
    // If the semaphore name already exists, call this->P() to execute it. If not, report an error in Wait, Signal function
    int Wait(char *name);
    int Signal(char *name);
    int FindFreeSlot(); // Find an empty slot
};
#endif STABLE_H