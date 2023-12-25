// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
#include "synchconsole.h"
#include "filesys.h"
#include "ptable.h"
#include "stable.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions
//	is in machine.h.
//----------------------------------------------------------------------

#define MaxFileLength 32

void IncreasePC()
{
	// set previous program counter to current programcounter
	kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

	/* set next programm counter for brach execution */
	kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
}

void HandleNonSyscallException(ExceptionType which){
	int currentProcessID = kernel->currentThread->processID;
	kernel->pTab->ExitUpdate(which);

	//Restore saved states of parent process
	//kernel->pTab->RestoreParentState(currentProcessID);
}

void ExceptionHandler(ExceptionType which)
{
	int type = kernel->machine->ReadRegister(2);

	DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

	switch (which)
	{
	case SyscallException:
		switch (type)
		{

		case SC_Exec:
		{
			int virtAddr;
			char* filename;
			DEBUG(dbgSys, "SC_Exec called ...\n");
			DEBUG(dbgSys, "Reading virtual address of filename...\n");
			virtAddr = kernel->machine->ReadRegister(4);
			DEBUG(dbgSys, "DEBUG: Virtual address of filename: " << virtAddr << "\n");
			filename = User2System(virtAddr, MaxFileLength + 1);
			if (filename == NULL)
			{
				printf("Not enough memory in system.\n");
				DEBUG(dbgSys, "Not enough memory in system.\n");
				kernel->machine->WriteRegister(2, -1);

				delete[] filename;
				IncreasePC();
				return;
			}
			DEBUG(dbgSys, "DEBUG: File name to exec: " << filename << "\n");
			int id = kernel->pTab->ExecUpdate(filename);
			if (id < 0){
				DEBUG(dbgSys, "Failed to exec file with name " << filename << ".\n");
				kernel->machine->WriteRegister(2, -1);

				delete[] filename;
				IncreasePC();
				return;
			}

			DEBUG(dbgSys, "Exec file name " << filename << " successfully.\n");
			kernel->machine->WriteRegister(2, id);
			delete[] filename;
			IncreasePC();
			return;
			ASSERTNOTREACHED();
			break;
		}
		case SC_ExecV:
		{
			int virtAddr, argc;
			char** argv;
			DEBUG(dbgSys, "SC_Exec called ...\n");
			DEBUG(dbgSys, "Reading virtual address of filename...\n");
			argc = kernel->machine->ReadRegister(4);
			virtAddr = kernel->machine->ReadRegister(5);
			
			argv = new char*[argc];
			for (int i=0; i < argc; i++){
				int argv_i_addr;
				kernel->machine->ReadMem(virtAddr + i*4, 4, &argv_i_addr);
				argv[i] = User2System(argv_i_addr, 1024);
				DEBUG(dbgSys, "DEBUG: argv[" << i << "] value: " << argv[i]);
			}

			int id = kernel->pTab->ExecV(argc, argv);
			if (id < 0){
				DEBUG(dbgSys, "Failed to exec file with name " << argv[0] << ".\n");
				kernel->machine->WriteRegister(2, -1);
			}
			else{
				DEBUG(dbgSys, "Exec file name " << argv[0] << " successfully.\n");
				kernel->machine->WriteRegister(2, id);
			}
			/*for (int i = 0; i < argc; i++){
				delete[] argv[i];
			}
			delete[] argv;*/
			IncreasePC();
			return;
			ASSERTNOTREACHED();
			break;
		}
		case SC_Join:
		{
			int pid;
			DEBUG(dbgSys, "SC_Join called ...\n");
			
			pid = kernel->machine->ReadRegister(4);
			DEBUG(dbgSys, "Join child proc with PID: " << pid);
			int exit_code = kernel->pTab->JoinUpdate(pid);
			kernel->machine->WriteRegister(2, exit_code);
			IncreasePC();
			return;
			ASSERTNOTREACHED();
			break;
		}
		case SC_Exit:
		{
			int ex_code;
			DEBUG(dbgSys, "SC_Exit called ...\n");
			
			ex_code = kernel->machine->ReadRegister(4);
			DEBUG(dbgSys, "Exit code called: " << ex_code);
			int exit_code = kernel->pTab->ExitUpdate(ex_code);

			if (exit_code == 0){
				DEBUG(dbgSys, "Process exits successfully\n");
			}
			else{
				DEBUG(dbgSys, "Process exits with error\n");
			}
			IncreasePC();
			return;
			ASSERTNOTREACHED();
			break;
		}
		case SC_CreateSemaphore:
		{
			int virtAddr, semVal;
			char* filename;
			DEBUG(dbgSys, "SC_CreateSemaphore called ...\n");
			DEBUG(dbgSys, "Reading virtual address of filename...\n");
			virtAddr = kernel->machine->ReadRegister(4);
			semVal = kernel->machine->ReadRegister(5);
			
			filename = User2System(virtAddr, MaxFileLength + 1);
			if (filename == NULL)
			{
				printf("Not enough memory in system.\n");
				DEBUG(dbgSys, "Not enough memory in system.\n");
				kernel->machine->WriteRegister(2, -1);

				delete[] filename;
				IncreasePC();
				return;
			}

			int result = kernel->sTab->Create(filename, semVal);
			kernel->machine->WriteRegister(2, result);
			IncreasePC();
			return;
			ASSERTNOTREACHED();
			break;
		}
		case SC_Signal:
		{
			int virtAddr;
			char* filename;
			DEBUG(dbgSys, "SC_Signal called ...\n");
			DEBUG(dbgSys, "Reading virtual address of filename...\n");
			virtAddr = kernel->machine->ReadRegister(4);
			
			filename = User2System(virtAddr, MaxFileLength + 1);
			if (filename == NULL)
			{
				printf("Not enough memory in system.\n");
				DEBUG(dbgSys, "Not enough memory in system.\n");
				kernel->machine->WriteRegister(2, -1);

				delete[] filename;
				IncreasePC();
				return;
			}

			int result = kernel->sTab->Signal(filename);
			kernel->machine->WriteRegister(2, result);
			IncreasePC();
			return;
			ASSERTNOTREACHED();
			break;
		}
		case SC_Wait:
		{
			int virtAddr;
			char* filename;
			DEBUG(dbgSys, "SC_Wait called ...\n");
			DEBUG(dbgSys, "Reading virtual address of filename...\n");
			virtAddr = kernel->machine->ReadRegister(4);
			
			filename = User2System(virtAddr, MaxFileLength + 1);
			if (filename == NULL)
			{
				printf("Not enough memory in system.\n");
				DEBUG(dbgSys, "Not enough memory in system.\n");
				kernel->machine->WriteRegister(2, -1);

				delete[] filename;
				IncreasePC();
				return;
			}

			int result = kernel->sTab->Wait(filename);
			kernel->machine->WriteRegister(2, result);
			IncreasePC();
			return;
			ASSERTNOTREACHED();
			break;
		}
		case SC_Halt:
		{
			DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

			SysHalt();
			return;
			ASSERTNOTREACHED();
			break;
		}
		case SC_Create:
		{
			int virtAddr;
			char *filename;
			DEBUG(dbgSys, "SC_Create called ...\n");
			DEBUG(dbgSys, "Reading virtual address of filename...\n");

			virtAddr = kernel->machine->ReadRegister(4);
			
			// MaxFileLength = 32
			filename = User2System(virtAddr, MaxFileLength + 1);
			
			if (filename == NULL)
			{
				printf("Not enough memory in system.\n");
				DEBUG(dbgSys, "Not enough memory in system.\n");
				kernel->machine->WriteRegister(2, -1);

				delete[] filename;
				IncreasePC();
				return;
			}
			DEBUG(dbgSys, "Reading filename: " << filename << "\n");
			DEBUG(dbgSys, "Finish reading filename.\n");

			if (!kernel->fileSystem->Create(filename))
			{
				printf("Error creating file '%s'\n", filename);
				kernel->machine->WriteRegister(2, -1);
				delete[] filename;
				IncreasePC();
				return;
			}
			kernel->machine->WriteRegister(2, 0); 
			delete[] filename;
			IncreasePC();
			return;
			ASSERTNOTREACHED();
			break;
		}
		case SC_Add:
		{
			DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");

			/* Process SysAdd Systemcall*/
			int result;
			result = SysAdd(/* int op1 */ (int)kernel->machine->ReadRegister(4),
							/* int op2 */ (int)kernel->machine->ReadRegister(5));

			DEBUG(dbgSys, "Add returning with " << result << "\n");
			/* Prepare Result */
			kernel->machine->WriteRegister(2, (int)result);

			/* Modify return point */
			IncreasePC();
			return;

			ASSERTNOTREACHED();

			break;
		}

		case SC_Open:
		{
			DEBUG(dbgSys, "SC_Open called...\n");
			int bufferVirtAddr = kernel->machine->ReadRegister(4);
			int permission = kernel->machine->ReadRegister(5);
			char *fileName = User2System(bufferVirtAddr, MaxFileLength + 1); //File name
			
			if (fileName == NULL)
			{
				DEBUG(dbgSys, "Empty file name or not enough memory...\n");
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}
			
			DEBUG(dbgSys, "Opening file name: " << fileName << ", permission:" << permission << "\n");
			int OpenFileID = -1;
			for (int index = 2; index < 20; index++)
			{
				if (index < 0) { continue; }
				DEBUG(dbgSys, "Open file pointer at " << kernel->fileSystem->openf[index] << "\n");
				if (kernel->fileSystem->openf[index] == NULL)
				{
					DEBUG(dbgSys, "Set return file ID at " << index << "\n");
					OpenFileID = index;
					break;
				}
			}
			DEBUG(dbgSys, "Index " << OpenFileID << "\n");
			if (OpenFileID == 0 || OpenFileID == 1)
			{
				kernel->machine->WriteRegister(2, 0);
			}
			if (OpenFileID != -1)
			{
				if (permission == 0 or permission == 1)
				{
					int fileNameLength = strlen(fileName);
					DEBUG(dbgSys, "Opening file name: " << fileName << ", file name length:" << fileNameLength << "\n");
					kernel->fileSystem->openf[OpenFileID] = kernel->fileSystem->Open(fileName, 0, permission);
					DEBUG(dbgSys, "File at index " << OpenFileID << " has been occupied with open file pointer" << kernel->fileSystem->openf[OpenFileID] << ".\n");
					DEBUG(dbgSys, "File sector address: " << kernel->fileSystem->openf[OpenFileID] << "\n");
					

					kernel->machine->WriteRegister(2, OpenFileID);

					kernel->fileSystem->tableDescriptor[OpenFileID] = new char[fileNameLength + 1];
					for (int i = 0; i < fileNameLength; i++)
					{
						kernel->fileSystem->tableDescriptor[OpenFileID][i] = fileName[i];
					}

					kernel->fileSystem->tableDescriptor[OpenFileID][fileNameLength] = '\0';
					DEBUG(dbgSys, "Open file successfully ...\n");
					DEBUG(dbgSys, "Return file id: " <<  OpenFileID << "\n");
					kernel->machine->WriteRegister(2, OpenFileID);
					delete[] fileName;
					IncreasePC();
					return;
				}

			}
			DEBUG(dbgSys, "Cannot open file.\n");
			kernel->machine->WriteRegister(2, -1);
			delete[] fileName;
			IncreasePC();
			return;
			break;
		}

		case SC_Close:
		{
			DEBUG(dbgSys, "\n SC_Close calling ...\n");
			int fID = kernel->machine->ReadRegister(4); 
			int index = 0;

			if (fID > 19)
			{
				DEBUG(dbgSys, "Close file at file ID " << fID << "failed. \n");
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				return;
				break;
			}
			else if (fID >= 0) // Chi xu li khi fid nam trong [0, 20]
			{

				if (kernel->fileSystem->openf[fID] != NULL)
				{
					delete kernel->fileSystem->openf[fID]; 
					kernel->fileSystem->openf[fID] = NULL; // Gan vung nho NULL
					kernel->machine->WriteRegister(2, 0);
					// printf("\n Successfully close file ");
					if (kernel->fileSystem->tableDescriptor[fID] != NULL)
					{
						delete kernel->fileSystem->tableDescriptor[fID];
					}
					kernel->fileSystem->tableDescriptor[fID] = NULL;
					IncreasePC();
					return;
					break;
				}
			}
			DEBUG(dbgSys, "Close file at file ID " << fID << "failed. \n");
			kernel->machine->WriteRegister(2, -1);
			IncreasePC();
			return;
			break;
		}

		// int Read(char *buffer, int size, OpenFileId id);
		case SC_Read:
		{
			DEBUG(dbgSys, "\n SC_Read called ...\n");
			int virtAddr = kernel->machine->ReadRegister(4); // buffer virtual address
			int len = kernel->machine->ReadRegister(5);		 // buffer length to read
			int id = kernel->machine->ReadRegister(6);		 // File id to read from
			DEBUG(dbgSys, "Virt address: " << virtAddr << ", Length: " << len << "\n");
			if (id > 19 || id < 0)
			{
				
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				return;
				break;
			}

			if (kernel->fileSystem->openf[id] == NULL)
			{
				printf("Cannot read from non-exist file");
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				return;
				break;
			}

			int oldPos;
			int newPos;
			char *buffer;


			if (id == 1) // Read from console output (id == 1)
			{
				printf("Cannot read console output file.\n");
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				return;
				break;
			}
			oldPos = kernel->fileSystem->openf[id]->GetCurrentPos();
			
			buffer = User2System(virtAddr, len);	

			// Read console input 
			if (id == 0)
			{
				// 
				int size = 0;
				while (true)
				{
					if (size == len - 1)
					{
						break;
					}
					char oneChar = kernel->synchConsoleIn->GetChar();
					buffer[size] = oneChar;
					if (oneChar == '\n')
					{
						break;
					}
					size++;
					
				}
				buffer[size] = '\0';
				System2User(virtAddr, size+1, buffer);		
				kernel->machine->WriteRegister(2, size+1); 
				delete[] buffer;
				IncreasePC();
				return;
				break;
			}
			DEBUG(dbgSys, "Check if current position is at file length\n");
			if (oldPos == kernel->fileSystem->openf[id]->Length() && len > 0)
			{
				DEBUG(dbgSys, "End of file, read nothing\n");
				kernel->machine->WriteRegister(2, 0);
				IncreasePC();
				return;
				break;
			}
			// 
			if ((kernel->fileSystem->openf[id]->Read(buffer, len-1)) > 0)
			{
				//
				buffer[len-1] = '\0';
				newPos = kernel->fileSystem->openf[id]->GetCurrentPos();
				
				System2User(virtAddr, newPos - oldPos, buffer);
				kernel->machine->WriteRegister(2, newPos - oldPos);
				DEBUG(dbgSys, "Have read " << newPos - oldPos << " bytes from files.\n");
			}
			else
			{
				DEBUG(dbgSys, "Cannot read from empty file.\n");
				kernel->machine->WriteRegister(2, -1);
			}
			delete[] buffer;
			IncreasePC();
			return;
			break;
			
		}

		// int Write(char *buffer, int size, OpenFileId id);
		//  int Send(int socketid, char *buffer, int len);
		case SC_Write:
		{
			DEBUG(dbgSys, "\n SC_Write calling ...\n");
			int virtAddr = kernel->machine->ReadRegister(4);
			int len = kernel->machine->ReadRegister(5);
			int openf_id = kernel->machine->ReadRegister(6);
			int index = 0;
			DEBUG(dbgSys, "Virt address: " << virtAddr << ", Length: " << len << "\n");
			if (kernel->fileSystem->openf[openf_id] == NULL)
			{
				DEBUG(dbgSys, "Cannot write to non-exist file.\n");
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				return;
				break;
			}

			if (openf_id != 1 && kernel->fileSystem->openf[openf_id]->getIDType() != 0)
			{
				DEBUG(dbgSys, "Cannot write to socket file.\n");
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				return;
				break;
			}

			if (openf_id > 19 || openf_id < 0 )
			{
				DEBUG(dbgSys, "Index out of range.\n");
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				return;
				break;
			}
			// Read-only file
			if (kernel->fileSystem->openf[openf_id]->permission == 1 )
			{
				DEBUG(dbgSys, "Cannot write to read-only file.\n");
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				return;
				break;
			}

			if (openf_id == 0 )
			{
				DEBUG(dbgSys, "Cannot write to input console.\n");
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				return;
				break;
			}

			
			// write to console
			char *buffer = User2System(virtAddr, len);
			int size = 0;
			if (openf_id == 1)
			{
				
				while (true)
				{
					if (size == len - 1)
					{
						break;
					}
					char oneChar = buffer[size];
					kernel->synchConsoleOut->PutChar(oneChar);
					if (oneChar == '\0')
					{
						break;
					}
					size++;
					
				}
				
				kernel->machine->WriteRegister(2, size); 
				delete[] buffer;
				IncreasePC();
				return;
				break;
			}


			int before = kernel->fileSystem->openf[openf_id]->GetCurrentPos();
			if (buffer[len-1] == '\0') { len--;} //Avoid writing null char to file 
			if (len <= 0) {
				DEBUG(dbgSys, "Cannot write with length <= 0\n");
				kernel->machine->WriteRegister(2, -1);
				delete[] buffer;
				IncreasePC();
				return;
			}
			if ((kernel->fileSystem->openf[openf_id]->Write(buffer, len)) != 0)
			{
				int after = kernel->fileSystem->openf[openf_id]->GetCurrentPos();

				System2User(virtAddr, after - before, buffer);
				kernel->machine->WriteRegister(2, after - before + 1);
				delete[] buffer;
				IncreasePC();
				return;
				break;
			}

			IncreasePC();
			return;
			break;
			
		}

		case SC_SocketTCP:
		{
			DEBUG(dbgSys, "SC_SocketTCP called...\n");
			// Opening socket, return -1 on failure
			int SocketID = socket(AF_INET, SOCK_STREAM, 0);
			if (SocketID == -1)
			{
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				return;
				break;
			}
			else
			{
				int OpenFileID = -1;
				for (int index = 0; index < 20; index++)
				{
					if (kernel->fileSystem->openf[index] == NULL)
					{
						OpenFileID = index;
						break;
					}
				}
				if (OpenFileID != -1) // Found free slot
				{
					kernel->fileSystem->openf[OpenFileID] = kernel->fileSystem->Open(SocketID);
					DEBUG(dbgSys, "Socket opened, current index is:" << OpenFileID <<" \n");
					kernel->machine->WriteRegister(2, OpenFileID);
				}
				else // Unable to find free slot
				{
					kernel->machine->WriteRegister(2, -1);
					IncreasePC();
					return;
					break;
				}
			}
			IncreasePC();
			return;
			break;
		}

		case SC_Connect:
		{
			DEBUG(dbgSys, "\n SC_Connect calling ...");
			int index = kernel->machine->ReadRegister(4);
			int socketID = kernel->fileSystem->openf[index]->getfile();
			int port = kernel->machine->ReadRegister(6);
			int virtAddr = kernel->machine->ReadRegister(5);

			char *ip = User2System(virtAddr, 32);
			struct sockaddr_in server;
			


			if (socketID < 0)
			{
				DEBUG(dbgSys, "\nInvalid Socket ID");
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				return;
				break;
			}
			server.sin_family = AF_INET;
			server.sin_addr.s_addr = inet_addr(ip);
			server.sin_port = htons(port);
			int len = sizeof(server);
			if (connect(socketID, (struct sockaddr *)&server, len) < 0)
			{
				DEBUG(dbgSys, "\nCannot connect to server");
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				return;
				break;
			}
			DEBUG(dbgSys, "\nSuccessfully connect to server");
			kernel->machine->WriteRegister(2, 0);
			IncreasePC();
			return;
			break;
		}

		case SC_Seek:
		{
			DEBUG(dbgSys, "\n SC_Seek calling ...");
			int pos = kernel->machine->ReadRegister(4);
			int id = kernel->machine->ReadRegister(5);

			if (id < 0 || id > 19)
			{
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}
			// Check if file exists
			if (kernel->fileSystem->openf[id] == NULL)
			{
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}
			// Return -1 if seek on consoles
			if (id == 0 || id == 1)
			{
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}
			pos = (pos == -1) ? kernel->fileSystem->openf[id]->Length() : pos;
			if (pos > kernel->fileSystem->openf[id]->Length() || pos < 0)
			{
				kernel->machine->WriteRegister(2, -1);
			}
			else
			{
				kernel->fileSystem->openf[id]->Seek(pos);
				kernel->machine->WriteRegister(2, pos);
			}
			IncreasePC();
			return;
		}
		case SC_Remove:
		{
			DEBUG(dbgSys, "\n SC_Remove calling ...");
			int virtAddr = kernel->machine->ReadRegister(4); 
			char *filename;
			filename = User2System(virtAddr, MaxFileLength); 

			for (int i = 2; i < 20; i++)
			{

				if (kernel->fileSystem->tableDescriptor[i] != NULL && strcmp(filename, kernel->fileSystem->tableDescriptor[i]) == 0)
				{
					DEBUG(dbgSys, "Cannot remove file (file is opening) ...\n");
					kernel->machine->WriteRegister(2, -1);
					IncreasePC();
					return;
				}
			}

			if (kernel->fileSystem->Remove(filename))
			{
				DEBUG(dbgSys, "\n Remove file success...");
				kernel->machine->WriteRegister(2, 0);
			}
			else
			{
				DEBUG(dbgSys, "\n Can not found file ...");
				kernel->machine->WriteRegister(2, -1);
			}
			IncreasePC();
			return;
			break;
		}

		case SC_Receive:
		{
			// User arguments
			DEBUG(dbgSys, "\n SC_Receive calling ...");
			int id = kernel->machine->ReadRegister(4);
			if (kernel->fileSystem->openf[id]== NULL)
			{
				DEBUG(dbgSys, "Socket not exists\n");
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				return;
				break;
			}
			int sockID = kernel->fileSystem->openf[id]->getfile();
			int virtAddr = kernel->machine->ReadRegister(5);
			int len = kernel->machine->ReadRegister(6);
			DEBUG(dbgSys, "Virt address: " << virtAddr << ", Length: " << len << "\n");
			DEBUG(dbgSys, "Sending to socket with ID: "<< sockID << ", length: "<< len<<"\n");
			if (id > 19 || id < 0)
			{
				
				DEBUG(dbgSys, "Index out of range.\n");
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				return;
				break;
			}

			DEBUG(dbgSys, "File type: " << kernel->fileSystem->openf[id]->getIDType() << "\n");
			if (kernel->fileSystem->openf[id]->getIDType() == 1)
			{
				int sockID = kernel->fileSystem->openf[id]->getfile();
				char *buffer = new char[len+1];

				int result = recv(sockID, buffer, len, 0);
				if (result < 0)
				{
					if (errno == 107)
					{
						DEBUG(dbgSys, "\nNo connection detected!");
						kernel->machine->WriteRegister(2, 0);
						delete[] buffer;
						IncreasePC();
						return;
						break;
					}
					DEBUG(dbgSys, "Cannot receive from socket with ID: " << sockID );
					kernel->machine->WriteRegister(2, -1);
					delete[] buffer;
					IncreasePC();
					return;
					break;
				}
				DEBUG(dbgSys, "\nSuccessfully received " << buffer << "\n");
				DEBUG(dbgSys, "Virt address: " << virtAddr << ", Length: " << result << "\n");
				System2User(virtAddr, result, buffer);
				DEBUG(dbgSys, "\nSystem to user successfully\n");
				delete[] buffer;
				kernel->machine->WriteRegister(2, result);
				IncreasePC();
				return;
				break;
			}
			else
			{
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				return;
				break;
			}
			break;
		}
		case SC_Send:
		{
			int sockID;
			int virtAddr;
			int len;
			int id = kernel->machine->ReadRegister(4);

			if (kernel->fileSystem->openf[id]== NULL)
			{
				DEBUG(dbgSys, "Socket not exists\n");
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				return;
				break;
			}

			sockID = kernel->fileSystem->openf[id]->getfile();
			virtAddr = kernel->machine->ReadRegister(5);
			len = kernel->machine->ReadRegister(6);
			int index = kernel->fileSystem->index;

			if (id > index || id < 0)
			{
				
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				return;
				break;
			}

			if (kernel->fileSystem->openf[id]->getIDType() == 1)
			{
				
				int sockID = kernel->fileSystem->openf[id]->getfile();
				char *buffer = User2System(virtAddr, len);

				int result = send(sockID, buffer, len, 0);

				if (errno == 32)
				{
					DEBUG(dbgSys, "No connection detected!\n");
					kernel->machine->WriteRegister(2, 0);
					IncreasePC();
					return;
					break;
				}

				if (result < 0)
				{
					DEBUG(dbgSys, "Cannot send buffer " << *buffer << "to socket with ID" << sockID << "\n");
					kernel->machine->WriteRegister(2, -1);
					IncreasePC();
					return;
					break;
				}
				DEBUG(dbgSys, "Successfully sent");
				kernel->machine->WriteRegister(2, result);
				IncreasePC();
				return;
				break;
			}
			else{
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				return;
				break;
			}
			break;
		}

		case SC_ReadString:
		{
			int bufferVirtAddr = kernel->machine->ReadRegister(4);
			int len = kernel->machine->ReadRegister(5);
			char *buffer = User2System(bufferVirtAddr, len);
			int size = 0;
			while (true)
			{
				if (size == len - 1)
				{
					break;
				}
				char oneChar = kernel->synchConsoleIn->GetChar();
				buffer[size] = oneChar;
				if (oneChar == '\n')
				{
					break;
				}
				size++;
				
			}
			buffer[size] = '\0';
			System2User(bufferVirtAddr, size+1, buffer);		
			kernel->machine->WriteRegister(2, size+1); 
			delete[] buffer;
			IncreasePC();
			return;
			break;
		}

		case SC_PrintString:
		{
			int bufferVirtAddr = kernel->machine->ReadRegister(4);
			int len = kernel->machine->ReadRegister(5);
			char *buffer = User2System(bufferVirtAddr, len);
			int size = 0;
			while (true)
			{
				if (size == len - 1)
				{
					break;
				}
				char oneChar = buffer[size];
				kernel->synchConsoleOut->PutChar(oneChar);
				if (oneChar == '\0')
				{
					break;
				}
				size++;
				
			}	
			kernel->machine->WriteRegister(2, size); 
			delete[] buffer;
			IncreasePC();
			return;
			break;
		}

		default:
			DEBUG(dbgSys, "Unexpected system call " << type << "\n");
			break;
		}
		break;
	case AddressErrorException:
		DEBUG(dbgSys, "Address error exception\n");
		HandleNonSyscallException(AddressErrorException);
		break;

	case ReadOnlyException:
		DEBUG(dbgSys, "Access read-only memory exception\n");
		HandleNonSyscallException(ReadOnlyException);
		break;
	case IllegalInstrException:
		DEBUG(dbgSys, "Illegal instruction exception\n");
		HandleNonSyscallException(IllegalInstrException);
		break;

	case OverflowException:
		DEBUG(dbgSys, "Overflow exception\n");
		HandleNonSyscallException(OverflowException);
		break;

	case BusErrorException:
		DEBUG(dbgSys, "Bus error exception\n");
		HandleNonSyscallException(BusErrorException);
		break;
	default:
		DEBUG(dbgSys, "Unexpected user mode exception" << (int)which << "\n");
		break;
	}
	ASSERTNOTREACHED();
}
