#include "stable.h"
#include "sem.h"
#include "debug.h"
#include <string.h>

STable::STable(){
    this->bm = new Bitmap(MAX_SEMAPHORE);
    for (int i=0; i<MAX_SEMAPHORE; i++){
        this->semTab[i] = NULL;
    }
}

STable::~STable(){
    for (int i=0; i < MAX_SEMAPHORE; i++){
        delete this->semTab[i];
    }
    delete this->bm;
}

int STable::FindFreeSlot(){
    int id = this->bm->FindAndSet();
    if (id == -1) { return -1; }
    return id;
}

int STable::Create(char *name, int init){
    if (name == NULL || init < 0){
        return -1;
    }
    int id;
    for (id = 0; id < MAX_SEMAPHORE; id++){
        if (semTab[id] == NULL) { continue;}
        //If there's a semaphore with that name, return -1
        if (strncmp(semTab[id]->GetName(), name, SEMAPHORE_MAX_NAMESIZE) == 0){
            DEBUG(dbgSynch, "DEBUG STable: There's already a semaphore with name " << name );
            return -1;
        }
    }
    id = this->FindFreeSlot();
    DEBUG(dbgSynch, "DEBUG STable: Index of new semaphore in STable " << id );
    semTab[id] = new Sem(name, init);
    if (semTab[id] == NULL) {
        //If fail to create to semaphore
        this->bm->Clear(id);
        DEBUG(dbgSynch, "DEBUG STable: Failed to create semaphore " << id );
        return -1;
    }
    else{ return id; }

}

int STable::Wait(char *name){
    DEBUG(dbgSynch, "DEBUG STable: Name of semaphore" << name );
    for (int id = 0; id < MAX_SEMAPHORE; id++){
        if (semTab[id] == NULL) { continue;}
        //If there's a semaphore with that name, return -1
        DEBUG(dbgSynch, "DEBUG STable: Name of current semaphore at index " << id << ":" << semTab[id]->GetName() );
        if (strncmp(semTab[id]->GetName(), name, SEMAPHORE_MAX_NAMESIZE) == 0){
            DEBUG(dbgSynch, "DEBUG STable: Found semaphore with name " << name );
            semTab[id]->Wait();
            return 0;
        }
    }

    DEBUG(dbgSynch, "Failed to wait for semaphore with name " << name );
    return -1;
}

int STable::Signal(char *name){
    DEBUG(dbgSynch, "DEBUG STable: Name of semaphore " << name );
    for (int id = 0; id < MAX_SEMAPHORE; id++){
        if (semTab[id] == NULL) { continue;}
        DEBUG(dbgSynch, "DEBUG STable: Name of current semaphore at index " << id << ":" << semTab[id]->GetName() );
        //If there's a semaphore with that name, return 0, else return -1
        if (strncmp(semTab[id]->GetName(), name, SEMAPHORE_MAX_NAMESIZE) == 0){
            DEBUG(dbgSynch, "DEBUG STable: Found semaphore with name " << name );
            semTab[id]->Signal();
            return 0;
        }
    }
    DEBUG(dbgSynch, "DEBUG STable: Failed to signal semaphore with name " << name );
    return -1;
}

