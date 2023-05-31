// kernel.h
//	Global variables for the Nachos kernel.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef KERNEL_H
#define KERNEL_H

#include "copyright.h"
#include "debug.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "alarm.h"
#include "filesys.h"
#include "machine.h"

#define INT_MAX 2147483647
#define INT_MIN -2147483648
#define MAX_FILE_LENGTH 32

class PostOfficeInput;
class PostOfficeOutput;
class SynchConsoleInput;
class SynchConsoleOutput;
class SynchDisk;

class Kernel
{
public:
  Kernel(int argc, char **argv);
  // Interpret command line arguments
  ~Kernel(); // deallocate the kernel

  void Initialize(); // initialize the kernel -- separated
                     // from constructor because
                     // refers to "kernel" as a global

  void ThreadSelfTest(); // self test of threads and synchronization

  void ConsoleTest(); // interactive console self test

  void NetworkTest(); // interactive 2-machine network test

//------------------------------------------------------------
//
//------------------------------------------------------------

  void IncreasePC();

  char *User2System(int virtAddr, int limit);

  int System2User(int virtAddr, int len, char *buffer);

  void PrintNum(int number); // print interger number

  void EH_ReadNum(); // read inter number 

  void EH_ReadChar(); // read char from console

  void EH_PrintChar(); // print char console

  void ReadString2KeyBoard(int toAddr, char *buffer, int size); // 

  void PrintString2Console(char* buffer);

  void File_Create();

  void File_Open();

  void File_Close();

  void File_Remove();

  void File_Read();

  void File_Write();

  void File_Seek();
  

//------------------------------------------------------------
//
//------------------------------------------------------------

  // These are public for notational convenience; really,
  // they're global variables used everywhere.

  Thread *currentThread; // the thread holding the CPU
  Scheduler *scheduler;  // the ready list
  Interrupt *interrupt;  // interrupt status
  Statistics *stats;     // performance metrics
  Alarm *alarm;          // the software alarm clock
  Machine *machine;      // the simulated CPU
  SynchConsoleInput *synchConsoleIn;
  SynchConsoleOutput *synchConsoleOut;
  SynchDisk *synchDisk;
  FileSystem *fileSystem;
  PostOfficeInput *postOfficeIn;
  PostOfficeOutput *postOfficeOut;

  int hostName; // machine identifier

private:
  bool randomSlice;   // enable pseudo-random time slicing
  bool debugUserProg; // single step user program
  double reliability; // likelihood messages are dropped
  char *consoleIn;    // file to read console input from
  char *consoleOut;   // file to send console output to
#ifndef FILESYS_STUB
  bool formatFlag; // format the disk if this is true
#endif
};

#endif // KERNEL_H
