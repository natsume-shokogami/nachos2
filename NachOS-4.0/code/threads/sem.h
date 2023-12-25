#ifndef SEM_H
#define SEM_H
#include "string.h"
class Semaphore;

#define SEMAPHORE_MAX_NAMESIZE 49

class Sem {
private:
    Semaphore* sem;
    char name[SEMAPHORE_MAX_NAMESIZE+1];
public:
    Sem(char* name, int i);
    ~Sem();
    void Wait();
    void Signal();
    char* GetName();
};


#endif