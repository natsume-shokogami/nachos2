#include "ptable.h"
#include "pcb.h"
#include "synch.h"
#include "debug.h"
#include "main.h"

PTable::PTable(int size){
    this->bmsem = new Semaphore("bmsem", 1);
    char *mainthread_name = "main";

    this->psize = size;
    this->bm = new Bitmap(size);

    for (int i = 0; i < size; i++){
        this->pcb[i] = NULL;
    }

    this->pcb[0] = new PCB(0);
    this->bm->Mark(0);
    this->pcb[0]->SetFileName(mainthread_name);
    this->pcb[0]->parentID = -1;

}

PTable::~PTable(){
    delete this->bmsem;
    delete this->bm;
    for (int i = 0; i < this->psize; i++){
        delete this->pcb[i];
    }
}

int PTable::JoinUpdate(int pid){
    //Kiem tra xem pid co hop le va tien trinh goi join update
    DEBUG(dbgThread, "PID of process wants to join: " << pid);
    if (pid < 0 || pid >= this->psize || this->pcb[pid] == NULL 
    || this->pcb[pid]->parentID != kernel->currentThread->processID){
        //Invalid pid
        DEBUG(dbgThread, "DEBUG: Cannot join");
        return -1;

    }

    //Tang so tien trinh con phai cho cua tien trinh cha
    this->pcb[this->pcb[pid]->parentID]->IncNumWait();

    DEBUG(dbgThread, "DEBUG PCB: This process' parent PID: "<<this->pcb[this->pcb[pid]->parentID]->processID << "\n" );
    DEBUG(dbgThread, "Number of children threads need to wait: "<<this->pcb[this->pcb[pid]->parentID]->GetNumWait() << "\n" );
    DEBUG(dbgThread, "DEBUG PCB: This process' name: "<<this->pcb[pid]->GetFileName() << "\n" );
    this->pcb[pid]->JoinWait();
    DEBUG(dbgThread, "Join wait successfully" );

    int exitCode = this->pcb[pid]->GetExitCode();
    DEBUG(dbgThread, "Exit code: "<< exitCode << "\n");
    this->pcb[pid]->ExitRelease();
    return exitCode;
}

int PTable::ExecUpdate(char *name){
    DEBUG(dbgAddr, "DEBUG: Execute execupdate\n");
    bmsem->P();

    if (name == NULL){
        DEBUG(dbgThread, "DEBUG: Cannot exec process if name is NULL\n");
        bmsem->V();
        return -1;
    }

    DEBUG(dbgThread, "DEBUG: Process name to exec: " << name << "\n");

    if (kernel->fileSystem->Open(name) == NULL){
        //For some reasons, executing nonexist file will crash NachOS 
        //(Likely because of incomplete Addrspace object in the PCB's thread), so I'll check it
        DEBUG(dbgThread, "DEBUG: File does not exist");
        bmsem->V();
        return -1;
    }
    int name_len = strlen(name);
    // Since name char array is also owned by ExceptionHandler function, it'll be deleted at the end of Exec syscall
    // Don't ask me why I knew this :P
    char *threadname_copy = new char[name_len + 1];
    strncpy(threadname_copy, name, name_len);

    if (strcmp(name, kernel->currentThread->getName())== 0){
        DEBUG(dbgThread, "DEBUG: Cannot execute itself\n");
        bmsem->V();
        return -1;
    }

    int index = this->GetFreeSlot();
    DEBUG(dbgThread, "DEBUG: Index of process in pTable: " << index << "\n");
    if (index < 0){
        //No free slot
        DEBUG(dbgThread, "DEBUG: Not free slot in pTable\n");
        bmsem->V();
        return -1;
    }

    pcb[index] = new PCB(index);
    DEBUG(dbgThread, "DEBUG: Create PCB for new process successfully\n");
    pcb[index]->SetFileName(threadname_copy);

    DEBUG(dbgThread, "DEBUG: Execute thread with name: " << name << " and PID " << index << "\n");
    pcb[index]->parentID = kernel->currentThread->processID;
    DEBUG(dbgThread, "DEBUG: Parent process ID: " << kernel->currentThread->processID << "\n");
    int pid = pcb[index]->Exec(threadname_copy, index);
    bmsem->V();
    return pid;
}

int PTable::ExecV(int argc, char** argv){
    bmsem->P();

    char* name = argv[0];
    if (name == NULL){
        DEBUG(dbgThread, "DEBUG: Cannot exec process if name is NULL\n");
        bmsem->V();
        return -1;
    }

    DEBUG(dbgThread, "DEBUG: Process name to exec: " << name << "\n");
    if (strcmp(name, kernel->currentThread->getName())== 0){
        DEBUG(dbgThread, "DEBUG: Cannot execute itself\n");
        bmsem->V();
        return -1;
    }
    if (kernel->fileSystem->Open(name) == NULL){
        //For some reasons, executing nonexist file will crash NachOS 
        //(Likely because of incomplete Addrspace object in the PCB's thread), so I'll check it
        DEBUG(dbgThread, "DEBUG: Cannot exec process if file does not exist");
        bmsem->V();
        return -1;
    }

    int index = this->GetFreeSlot();
    DEBUG(dbgThread, "DEBUG: Index of process in pTable: " << index << "\n");
    if (index < 0){
        //No free slot
        DEBUG(dbgThread, "DEBUG: Not free slot in pTable\n");
        bmsem->V();
        return -1;
    }

    pcb[index] = new PCB(index);
    pcb[index]->SetFileName(name);

    DEBUG(dbgThread, "DEBUG: Execute thread with name: " << name << " and PID " << index << "\n");
    pcb[index]->parentID = kernel->currentThread->processID;
    DEBUG(dbgThread, "DEBUG: Create PCB for new process successfully\n");
    int pid = pcb[index]->ExecV(argc, argv);
    bmsem->V();
    return index;
}

int PTable::ExitUpdate(int exit_code){
    int id = kernel->currentThread->processID;
    if (pcb[id] == NULL){
        DEBUG(dbgThread, "DEBUG: PCB of the current thread has been deleted or lost unexpectedly.");
    }
    DEBUG(dbgThread, "ID of process wants to exit: " << id);
    DEBUG(dbgThread, "ID of parent process of process wants to exit: " << pcb[id]->parentID);

    //If the exit process is the main process (init)
    if (id == 0){
        std::cout << "\nInit process exit code:" <<  exit_code << std::endl;
        delete kernel->currentThread->space;
        kernel->currentThread->space = NULL;
        kernel->interrupt->Halt();
        return exit_code;
    }

    // Dat exit code cho tien trinh con 
    pcb[id]->SetExitCode(exit_code);

    //Giam so luong tien trinh con phai cho cua tien trinh cha
    pcb[this->pcb[id]->parentID]->DecNumWait();
    DEBUG(dbgThread, "This process' parent number of waiting child processes after exit this process: " << pcb[this->pcb[id]->parentID]->GetNumWait());

    //Giai phong cac semaphore de tien trinh cha co the tiep tuc, dong thoi cho tien trinh cha nhan duoc exit code
    this->pcb[id]->JoinRelease();
    
    this->pcb[id]->ExitWait();
    //Sau khi tien trinh cha nhan exit code, xoa PCB tien trinh con
    this->Remove(id);
    return exit_code;
    
}

void PTable::RestoreParentState(int id){
    if (!this->IsExist(id)){
        return;
    }
    int parent = this->pcb[id]->parentID;
    if (!this->IsExist(parent))
    {
        return;
    }
    this->pcb[parent]->RestoreState();
}

void PTable::Remove(int pid){
    this->bm->Clear(pid);
    if (this->pcb[pid] != NULL){
        PCB* del = this->pcb[pid];
        this->pcb[pid] = NULL;
        delete del;
        
    }
}

int PTable::GetFreeSlot(){
    /* for (int i = 0; i < this->psize; i++){
        if (this->pcb[i] != NULL){
            return i;
        }
    }
    return -1; */
    return this->bm->FindAndSet();
}

bool PTable::IsExist(int pid){
    /* if (pid >= this->psize || pid < 0 || this->pcb[pid] == NULL){
        //Process not exists
        return false; 
    }
    return true; */
    return this->bm->Test(pid);
}

char* PTable::GetFileName(int id){
    if (!this->IsExist(id)){
        //Process not exists
        return NULL; 
    }
    char *filename = this->pcb[id]->GetFileName();
    return filename;
}