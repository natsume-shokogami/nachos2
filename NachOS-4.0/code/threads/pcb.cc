#include "main.h"
#include "debug.h"
#include "pcb.h"
#include "utility.h"
#include "thread.h"
#include "addrspace.h"
#include "synch.h"


void StartProcess_1(void *pid){
    int id;
    id = *((int*) pid);
    DEBUG(dbgThread, "DEBUG PCB: pid of process " << id << "\n");
    char* fileName = kernel->pTab->GetFileName(id);
    AddrSpace *space;
    space = new AddrSpace(fileName);
    if (space == NULL){
        DEBUG(dbgAddr, "DEBUG PCB: Cannot create address space from file name: " << fileName << "\n");
        return;
    }
    DEBUG(dbgAddr, "New address space for process id: " << id << " is created successfully");
    DEBUG(dbgThread, "New address space for process id: " << id << " is created successfully");
    kernel->currentThread->space = space;
    space->Execute();
    
}

int WriteStrToMem(int virtAddr, char *buffer, int size){
    if (size <= 0){
        return -1;
    }

    int i = 0;
    char ch;
    while (i < size) {
        ch = buffer[i];
        if (ch == '\0')
            break;
        kernel->machine->WriteMem(virtAddr+i, sizeof(char), ch);
        i++;
    }
    kernel->machine->WriteMem(virtAddr+i+1, sizeof(char), 0); //Terminal character '\0'
    return i + 1;

}

void StartProcess_2(void *args){

    //For some reasons, argc and argv get here aren't correct as they were passed to StartProcess_2
    //Maybe because x86 is little endian and this MIPS emulator in NachOS is big endian but doesn't always convert all pointers
    char **argv = (char**)args;
    AddrSpace *space;

    int argc = 0;
    char* filename = argv[0];

    DEBUG(dbgThread, "File name to execute: " << filename << " and argc: " << argc);

    OpenFile *executable = kernel->fileSystem->Open(filename);
    if (executable == NULL){
        DEBUG(dbgThread, "DEBUG PCB: Unable to open file " << filename << "\n");
        DEBUG(dbgAddr, "DEBUG PCB: Unable to open file " << filename << "\n");
        return;
    }
    space = new AddrSpace(filename);
    if (space == NULL){
        DEBUG(dbgAddr, "DEBUG PCB: Cannot create address space from file name: " << filename << "\n");
        return;
    }

    int stackBottom;
    int argHead;

    while(argv[argc] != NULL){
        argc++; //Get number of argc
    }
    DEBUG(dbgThread, "Argc: " << argc);

    stackBottom = kernel->machine->ReadRegister(StackReg)
        + 16 - UserStackSize + 16;

    argHead = stackBottom + 4 * argc;

    int i = 0;

    while(i < argc)
    {   
        DEBUG(dbgThread, "stackBottom of argv[" << i << "]: " << stackBottom + (i * 4));
        if (kernel->machine->WriteMem(stackBottom + (i * 4),
                4, argHead) == FALSE){
            return;
        }
        DEBUG(dbgThread, "argHead of argv[" << i << "]: " << argHead);

        int len = WriteStrToMem(argHead, argv[i], strlen(argv[i]));
        DEBUG(dbgThread, "Length written to bottom of stack " << len);
        DEBUG(dbgThread, "argv[" << i << "] written to memory successfully: " << argv[i]);

        if (len == -1)
            return;
        
        argHead += len;
        i++;
    }

    DEBUG(dbgAddr, "New address space for filename: " << filename << " is created successfully");
    DEBUG(dbgThread, "New address space for filename: " << filename << " is created successfully");
    kernel->currentThread->space = space;
    // Since we need to pass arguments,
    // we do not use AddrSpace::Execute directly.
    kernel->currentThread->space->InitRegisters();
    kernel->currentThread->space->RestoreState();
    // Pass arguments
    kernel->machine->WriteRegister(4, argc);
    kernel->machine->WriteRegister(5, stackBottom);

    kernel->machine->Run();
    //space->Execute();
    
}

PCB::PCB(int id)
{
    if (id == 0)
    {
        this->parentID = -1;
    }else{
        
        this->parentID = kernel->currentThread->processID;
    }
    this->processName = NULL;
    this->processID = id;
    this->numwait = 0;
    this->thread = NULL;
    this->exitcode = 0;

    this->joinsem = new Semaphore("joinsem", 0);
    this->exitsem  = new Semaphore("exitsem", 0);
    this->multex = new Semaphore("multex", 1);

}

PCB::~PCB() { 

    DEBUG(dbgThread, "Delete process with pid " << this->processID << "and parent process ID " << this->parentID << "\n");
    delete this->joinsem;
    delete this->exitsem;
    delete this->multex;
    delete[] this->processName;
    if (thread != NULL) {
        if (kernel->currentThread->processID == thread->processID) {
            thread->FreeSpace();
            thread->Finish();
            thread = NULL;
        }
        //schedular will delete thread later
    }
}

int PCB::GetID(){
    return this->processID;
}

void PCB::IncNumWait(){
    this->multex->P();
    this->numwait++;
    this->multex->V();
}

void PCB::DecNumWait(){
    this->multex->P();
    if (this->numwait > 0)
    {
        this->numwait--;
    }
    this->multex->V();
}

void PCB::ExitRelease(){
    this->exitsem->V();
}

void PCB::ExitWait(){
    this->exitsem->P();
}

int PCB::GetNumWait(){
    return this->numwait;
}

void PCB::SetExitCode(int exit_code){
    this->exitcode = exit_code;
}

int PCB::GetExitCode(){ return this->exitcode; }



void PCB::JoinWait(){
    this->joinsem->P();
}

void PCB::JoinRelease(){
    this->joinsem->V();
}

void PCB::SetFileName(char* file_name)
{
    if (this->processName != NULL){
        delete[] this->processName;
    }
    this->processName = new char[strlen(file_name) + 1];
    strncpy(this->processName, file_name, strlen(file_name));
}

char* PCB::GetFileName(){
    char* file_name = new char[strlen(this->processName)+1];
    strncpy(file_name, this->processName, strlen(this->processName));
    return file_name;
}

void PCB::RestoreState(){
    if (this->thread != NULL && this->thread->space != NULL){
        this->thread->RestoreUserState();
        this->thread->space->RestoreState();
    }
}

int PCB::Exec(char* name, int pid){
    this->multex->P();
    
    DEBUG(dbgThread, "Create new thread with pid " << pid);
    DEBUG(dbgThread, "Create new thread with name " << name);
    this->thread = new Thread(name);

    DEBUG(dbgThread, "Create new thread with name " << name << "successfully");

    if (this->thread == NULL){
        DEBUG(dbgThread, "DEBUG: Not enough memory to initiate a new thread");
        this->multex->V();
        return -1;
    }

    this->thread->processID = pid;
    this->parentID = kernel->currentThread->processID;

    DEBUG(dbgThread, "Assign thread's process and parent ID successfully");

    //Save parent thread's states 
    /*if (kernel->currentThread->space != NULL){
        kernel->currentThread->SaveState();
        kernel->currentThread->SaveUserState();
    }*/
    this->thread->Fork(StartProcess_1, &(this->processID));
    DEBUG(dbgThread, "Fork child thread successfully");
    this->multex->V();
    return this->thread->processID;
}

int PCB::ExecV(int argc, char **argv){
    this->multex->P();
    DEBUG(dbgThread, "DEBUG: PCB's multex semaphore locked successfully");
    char* thread_name = new char[strlen(argv[0])+1];
    strncpy(thread_name, argv[0], strlen(argv[0]));

    this->thread = new Thread(argv[0]);
    DEBUG(dbgThread, "DEBUG: Assign this process' thread successfully");

    if (this->thread == NULL){
        DEBUG(dbgAddr, "DEBUG: Not enough memory to initiate a new thread");
        this->multex->V();
        return -1;
    }

    DEBUG(dbgThread, "DEBUG: This process now has parent ID: " << kernel->currentThread->processID);
    this->thread->processID = this->processID;
    this->parentID = kernel->currentThread->processID;

    char** args_argv = new char*[argc + 1];
    args_argv[argc] = NULL;
    int args_argc = argc; 

    

    for (int i = 0; i < argc; i++){
        DEBUG(dbgThread, "argv[" << i << "] before child thread fork: " << argv[i]);
        int argv_i_len = strlen(argv[i]);
        args_argv[i] = new char[argv_i_len + 1];
        strncpy(args_argv[i], argv[i], argv_i_len);
        args_argv[i][argv_i_len] = '\0'; 
        DEBUG(dbgThread, "args[" << i << "]: " << args_argv[i]);
    }


    
    this->thread->Fork(StartProcess_2, (void*)args_argv);
    DEBUG(dbgThread, "Fork child thread successfully");
    this->multex->V();
    DEBUG(dbgThread, "DEBUG: PCB's multex semaphore unlocked successfully");

    //Fork will manage memory, no need to delete args_argv here
    return 0;
}

