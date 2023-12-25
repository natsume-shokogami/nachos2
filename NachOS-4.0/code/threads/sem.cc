#include "sem.h"
#include "synch.h"

Sem::Sem(char* name, int i){
    strncpy(this->name, name, SEMAPHORE_MAX_NAMESIZE);
    sem = new Semaphore(this->name, i);
}

Sem::~Sem(){
    if ( sem != NULL) {
        delete sem;
    }
}

char* Sem::GetName() {
    return name;
}

void Sem::Wait() {
    sem->P();
}
void Sem::Signal() {
    sem->V();
}