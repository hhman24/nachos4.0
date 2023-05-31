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

//----
#define MaxFileLength 33 // Max filename length: 32 + null terminator
#define MAX_LENGTH_STRINH_SYS 223
//----

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

//----------------------------------------------------------------------

//----------------------------------------------------------------------

/* set  previous program counter (debugging only)*/
// void IncreasePC()
// {
// 	/* set previous programm counter (debugging only)*/
// 	kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

// 	/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
// 	kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

// 	/* set next programm counter for brach execution */
// 	kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
// 	return;
// }

void ExceptionHandler(ExceptionType which)
{
	int type = kernel->machine->ReadRegister(2);

	DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

	switch (which)
	{
	case NoException: // Everything ok!
		return;
	case PageFaultException: // No valid translation found
	{
		DEBUG(dbgAddr, "No valid translation found\n"); // If flag is enabled, print a message, address spaces
		printf("\n\nNo valid translation found\n");		// ham nay la cua linus hay cua nachos
		SysHalt();
		break;
	}
	case ReadOnlyException: // Write attempted to page marked -.// "read-only"
	{
		DEBUG(dbgAddr, "Write attempted to page marked - read-only\n");
		printf("\n\nWrite attempted to page marked - read-only\n");
		SysHalt();
		break;
	}
	case BusErrorException: // Translation resulted in an invalid physical address
	{
		DEBUG(dbgAddr, "Translation resulted in an invalid physical address\n");
		printf("\n\nTranslation resulted in an invalid physical address\n");
		SysHalt();
		break;
	}
	case AddressErrorException: // Unaligned reference or one that was beyond the end of the address space
	{
		DEBUG(dbgAddr, "Unaligned reference or one that was beyond the end of the address space\n");
		printf("\n\nUnaligned reference or one that was beyond the end of the address space\n");
		SysHalt();
		break;
	}
	case OverflowException: // Integer overflow in add or sub
	{
		DEBUG(dbgAddr, "Integer overflow in add or sub\n");
		printf("\n\nInteger overflow in add or sub\n");
		SysHalt();
		break;
	}
	case IllegalInstrException: // Unimplemented or reserved instr
	{
		DEBUG(dbgAddr, "Unimplemented or reserved instr\n");
		printf("\n\nUnimplemented or reserved instr\n");
		SysHalt();
		break;
	}
	case NumExceptionTypes:
	{
		DEBUG(dbgAddr, "Number exception types\n");
		printf("\n\nNumber exception types\n");
		SysHalt(); // Shut down Nachos cleanly, printing out performance statistics.
		return;
	}
	case SyscallException: // A program executed a system call
		switch (type)
		{
		case SC_Halt:
		{
			DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

			SysHalt();

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
			{
				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			}

			return;

			ASSERTNOTREACHED();

			break;
		}

		case SC_Sub:
		{
			/* Process SysAdd Systemcall */
			int result2;
			result2 = SysSub(/* int op1 */ (int)kernel->machine->ReadRegister(4),
							 /* int op2 */ (int)kernel->machine->ReadRegister(5));
			DEBUG(dbgSys, "Sub returning with" << result2 << "\n");
			/* Prepare Result */
			kernel->machine->WriteRegister(2, (int)result2);

			kernel->IncreasePC();

			// /* Modify return point */ // tăng thanh ghi count
			// {
			// 	/* set previous programm counter (debugging only)*/
			// 	kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

			// 	/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
			// 	kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

			// 	/* set next programm counter for brach execution */
			// 	kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			// }
			return;

			ASSERTNOTREACHED();

			break;
		}

		case SC_ReadNum:
		{
			kernel->EH_ReadNum();
			kernel->IncreasePC(); // ask Nachos to increase the program counter
			break;
		}

		case SC_ReadChar:
		{
			kernel->EH_ReadChar();
			kernel->IncreasePC();
			break;
		}

		case SC_PrintChar:
		{
			kernel->EH_PrintChar();
			kernel->IncreasePC();
			break;
		}

		case SC_RandomNum:
		{
			RandomInit(time(NULL));
			int num = random() % 100;
			kernel->machine->WriteRegister(2, num);
			kernel->IncreasePC();
			break;
		}

		case SC_PrintNum:
		{
			int number = kernel->machine->ReadRegister(4);
			kernel->PrintNum(number);
			kernel->IncreasePC();
			break;
		}

		case SC_ReadString:
		{
			int virAddr = kernel->machine->ReadRegister(4); // read register 4(argument 1)
			int len = kernel->machine->ReadRegister(5);		// read register 5(argument 2)
			char *str = kernel->User2System(virAddr, len);	// copy buffer from User memory space to System memory space

			kernel->ReadString2KeyBoard(virAddr, str, len);

			// cout << str << endl;

			kernel->IncreasePC();
			delete[] str;
			return;
		}

		case SC_PrintString:
		{
			int virAddr = kernel->machine->ReadRegister(4); // read register 4(argument 1)

			char *str_buffer = kernel->User2System(virAddr, MAX_LENGTH_STRINH_SYS); // copy buffer from User memory space to System memory space

			if (str_buffer == NULL) // check not buffer
			{
				printf("\n Memory is invalid\n");
				DEBUG(dbgSys, "\n Memory is invalid\n");
				kernel->IncreasePC();
				return;
			}

			kernel->PrintString2Console(str_buffer);

			delete[] str_buffer;
			kernel->IncreasePC();

			return;
		}

		case SC_Create:
		{
			kernel->File_Create();
			kernel->IncreasePC();
			break;
		}

		case SC_Open:
		{
			kernel->File_Open();
			kernel->IncreasePC();
			break;
		}

		case SC_Remove:
		{
			kernel->File_Remove();
			kernel->IncreasePC();
			break;
		}

		case SC_Close:
		{

			kernel->File_Close();
			kernel->IncreasePC();
			break;
		}

		case SC_Read:
		{
			kernel->File_Read();
			kernel->IncreasePC();
			break;
		}

		case SC_Write:
		{
			kernel->File_Write();
			kernel->IncreasePC();
			break;
		}

		case SC_Seek:
		{
			kernel->File_Seek();
			kernel->IncreasePC();
			break;
		}

		default:
		{
			cerr << "Unexpected system call " << type << "\n";
			break;
		}
		}
		break;
	default:
		cerr << "Unexpected user mode exception" << (int)which << "\n";
		break;
	}
}
