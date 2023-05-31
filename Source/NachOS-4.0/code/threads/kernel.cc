// kernel.cc
//	Initialization and cleanup routines for the Nachos kernel.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "debug.h"
#include "main.h"
#include "kernel.h"
#include "sysdep.h"
#include "synch.h"
#include "synchlist.h"
#include "libtest.h"
#include "string.h"
#include "synchconsole.h"
#include "synchdisk.h"
#include "post.h"

//----------------------------------------------------------------------
// Kernel::Kernel
// 	Interpret command line arguments in order to determine flags
//	for the initialization (see also comments in main.cc)
//----------------------------------------------------------------------

Kernel::Kernel(int argc, char **argv)
{
    randomSlice = FALSE;
    debugUserProg = FALSE;
    consoleIn = NULL;  // default is stdin
    consoleOut = NULL; // default is stdout
#ifndef FILESYS_STUB
    formatFlag = FALSE;
#endif
    reliability = 1; // network reliability, default is 1.0
    hostName = 0;    // machine id, also UNIX socket name
                     // 0 is the default machine id
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-rs") == 0)
        {
            ASSERT(i + 1 < argc);
            RandomInit(atoi(argv[i + 1])); // initialize pseudo-random
                                           // number generator
            randomSlice = TRUE;
            i++;
        }
        else if (strcmp(argv[i], "-s") == 0)
        {
            debugUserProg = TRUE;
        }
        else if (strcmp(argv[i], "-ci") == 0)
        {
            ASSERT(i + 1 < argc);
            consoleIn = argv[i + 1];
            i++;
        }
        else if (strcmp(argv[i], "-co") == 0)
        {
            ASSERT(i + 1 < argc);
            consoleOut = argv[i + 1];
            i++;
#ifndef FILESYS_STUB
        }
        else if (strcmp(argv[i], "-f") == 0)
        {
            formatFlag = TRUE;
#endif
        }
        else if (strcmp(argv[i], "-n") == 0)
        {
            ASSERT(i + 1 < argc); // next argument is float
            reliability = atof(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "-m") == 0)
        {
            ASSERT(i + 1 < argc); // next argument is int
            hostName = atoi(argv[i + 1]);
            i++;
        }
        else if (strcmp(argv[i], "-u") == 0)
        {
            cout << "Partial usage: nachos [-rs randomSeed]\n";
            cout << "Partial usage: nachos [-s]\n";
            cout << "Partial usage: nachos [-ci consoleIn] [-co consoleOut]\n";
#ifndef FILESYS_STUB
            cout << "Partial usage: nachos [-nf]\n";
#endif
            cout << "Partial usage: nachos [-n #] [-m #]\n";
        }
    }
}

//----------------------------------------------------------------------
// Kernel::Initialize
// 	Initialize Nachos global data structures.  Separate from the
//	constructor because some of these refer to earlier initialized
//	data via the "kernel" global variable.
//----------------------------------------------------------------------

void Kernel::Initialize()
{
    // We didn't explicitly allocate the current thread we are running in.
    // But if it ever tries to give up the CPU, we better have a Thread
    // object to save its state.
    currentThread = new Thread("main");
    currentThread->setStatus(RUNNING);

    stats = new Statistics();       // collect statistics
    interrupt = new Interrupt;      // start up interrupt handling
    scheduler = new Scheduler();    // initialize the ready queue
    alarm = new Alarm(randomSlice); // start up time slicing
    machine = new Machine(debugUserProg);
    synchConsoleIn = new SynchConsoleInput(consoleIn);    // input from stdin
    synchConsoleOut = new SynchConsoleOutput(consoleOut); // output to stdout
    synchDisk = new SynchDisk();                          //
#ifdef FILESYS_STUB
    fileSystem = new FileSystem();
#else
    fileSystem = new FileSystem(formatFlag);
#endif // FILESYS_STUB
    postOfficeIn = new PostOfficeInput(10);
    postOfficeOut = new PostOfficeOutput(reliability);

    interrupt->Enable();
}

//----------------------------------------------------------------------
// Kernel::~Kernel
// 	Nachos is halting.  De-allocate global data structures.
//----------------------------------------------------------------------

Kernel::~Kernel()
{
    delete stats;
    delete interrupt;
    delete scheduler;
    delete alarm;
    delete machine;
    delete synchConsoleIn;
    delete synchConsoleOut;
    delete synchDisk;
    delete fileSystem;
    delete postOfficeIn;
    delete postOfficeOut;

    Exit(0);
}

//----------------------------------------------------------------------
// Kernel::ThreadSelfTest
//      Test threads, semaphores, synchlists
//----------------------------------------------------------------------

void Kernel::ThreadSelfTest()
{
    Semaphore *semaphore;
    SynchList<int> *synchList;

    LibSelfTest(); // test library routines

    currentThread->SelfTest(); // test thread switching

    // test semaphore operation
    semaphore = new Semaphore("test", 0);
    semaphore->SelfTest();
    delete semaphore;

    // test locks, condition variables
    // using synchronized lists
    synchList = new SynchList<int>;
    synchList->SelfTest(9);
    delete synchList;
}

//----------------------------------------------------------------------
// Kernel::ConsoleTest
//      Test the synchconsole
//----------------------------------------------------------------------

void Kernel::ConsoleTest()
{
    char ch;

    cout << "Testing the console device.\n"
         << "Typed characters will be echoed, until ^D is typed.\n"
         << "Note newlines are needed to flush input through UNIX.\n";
    cout.flush();

    do
    {
        ch = synchConsoleIn->GetChar();
        if (ch != EOF)
            synchConsoleOut->PutChar(ch); // echo it!
    } while (ch != EOF);

    cout << "\n";
}

//----------------------------------------------------------------------
// Kernel::NetworkTest
//      Test whether the post office is working. On machines #0 and #1, do:
//
//      1. send a message to the other machine at mail box #0
//      2. wait for the other machine's message to arrive (in our mailbox #0)
//      3. send an acknowledgment for the other machine's message
//      4. wait for an acknowledgement from the other machine to our
//          original message
//
//  This test works best if each Nachos machine has its own window
//----------------------------------------------------------------------

void Kernel::NetworkTest()
{

    if (hostName == 0 || hostName == 1)
    {
        // if we're machine 1, send to 0 and vice versa
        int farHost = (hostName == 0 ? 1 : 0);
        PacketHeader outPktHdr, inPktHdr;
        MailHeader outMailHdr, inMailHdr;
        char *data = "Hello there!";
        char *ack = "Got it!";
        char buffer[MaxMailSize];

        // construct packet, mail header for original message
        // To: destination machine, mailbox 0
        // From: our machine, reply to: mailbox 1
        outPktHdr.to = farHost;
        outMailHdr.to = 0;
        outMailHdr.from = 1;
        outMailHdr.length = strlen(data) + 1;

        // Send the first message
        postOfficeOut->Send(outPktHdr, outMailHdr, data);

        // Wait for the first message from the other machine
        postOfficeIn->Receive(0, &inPktHdr, &inMailHdr, buffer);
        cout << "Got: " << buffer << " : from " << inPktHdr.from << ", box "
             << inMailHdr.from << "\n";
        cout.flush();

        // Send acknowledgement to the other machine (using "reply to" mailbox
        // in the message that just arrived
        outPktHdr.to = inPktHdr.from;
        outMailHdr.to = inMailHdr.from;
        outMailHdr.length = strlen(ack) + 1;
        postOfficeOut->Send(outPktHdr, outMailHdr, ack);

        // Wait for the ack from the other machine to the first message we sent
        postOfficeIn->Receive(1, &inPktHdr, &inMailHdr, buffer);
        cout << "Got: " << buffer << " : from " << inPktHdr.from << ", box "
             << inMailHdr.from << "\n";
        cout.flush();
    }

    // Then we're done!
}

//----------------------------------------------------------------------

void Kernel::IncreasePC()
{
    /* set previous programm counter (debugging only)*/
    kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

    /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
    kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

    /* set next programm counter for brach execution */
    kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
    return;
}

char *Kernel::User2System(int virtAddr, int limit)
{
    int i; // index
    int oneChar;
    char *kernelBuf = NULL;

    kernelBuf = new char[limit + 1]; // need for terminal string
    if (kernelBuf == NULL)
        return kernelBuf;
    memset(kernelBuf, 0, limit + 1);

    // printf("\n Filename u2s:");
    for (i = 0; i < limit; i++)
    {
        kernel->machine->ReadMem(virtAddr + i, 1, &oneChar);
        kernelBuf[i] = (char)oneChar;
        // printf("%c",kernelBuf[i]);
        if (oneChar == 0)
            break;
    }
    return kernelBuf;
}

int Kernel::System2User(int virtAddr, int len, char *buffer)
{
    if (len < 0)
        return -1;
    if (len == 0)
        return len;
    int i = 0;
    int oneChar = 0;
    do
    {
        oneChar = (int)buffer[i];
        kernel->machine->WriteMem(virtAddr + i, 1, oneChar);
        i++;
    } while (i < len && oneChar != 0);
    return i;
}

void Kernel::ReadString2KeyBoard(int toAddr, char *buffer, int size)
{
    if (size < 0 || size == 0) // check error size
        return;

    int i = 0;

    for (; i < (size - 1); i++)
    {
        buffer[i] = kernel->synchConsoleIn->GetChar();
        if (buffer[i] == '\n' || buffer[i] == EOF)
        {
            break;
        }
    }
    buffer[i] = '\0';
    kernel->System2User(toAddr, i, buffer);
}

void Kernel::PrintString2Console(char *buffer)
{
    int len = 0;
    while (buffer[len] != '\0')
    {
        kernel->synchConsoleOut->PutChar(buffer[len]);
        len++;
    }
}

void Kernel::EH_ReadNum()
{
    double re = 0;
    int maxSize = 20;
    char *value = new char[maxSize];
    /*
     * Enter a string
     */
    int index = 0;
    do
    {
        char c = kernel->synchConsoleIn->GetChar();
        if (c == '\n')
            break;
        value[index] = c;
        index++;
    } while (true);

    int i = 0, n = 1;
    int countDot = 0; // count Dot in string
    bool isNegative = false;

    // Check positive or negative number
    if (value[i] == '-')
    {
        i++;               // Increace index and ignore the sign '-', get second element
        isNegative = true; // Number maybe is negative number
    }

    // Browse Aray, we
    for (i; i < strlen(value); i++)
    {
        // If string exist only digits and I have 1 dau cham (if real number)
        // Then exception, otherwise return 0
        if (value[i] > '9' || value[i] < '0')
            if (value[i] != '.' || (value[i] == '.' && countDot == 1))
            {
                /*return value 0, If there is more Dot or difference char not digits*/
                machine->WriteRegister(2, 0);
                delete[] value;
                return;
            }
            else
                countDot++;
        else // Count Dot, convert to number
            if (countDot == 0)
                re = re * 10 + (value[i] - '0');
            else
            {
                n *= 10;
                re += (value[i] - '0') / n;
            }
    }
    // if negative, change sign
    if (isNegative)
        re = -re;

    machine->WriteRegister(2, (int)re); // return number for register 2
    // cout << (int)re << endl; // neu ma no qua gioi han thi se stack overflow
    delete[] value;
}

void Kernel::PrintNum(int number)
{
    char buffer[12];
    sprintf(buffer, "%d", number); // Write formatted data to string
    kernel->PrintString2Console(buffer);
}

void Kernel::EH_ReadChar()
{
    int maxSize = 10;
    char *value = new char[maxSize];
    int byte = 0;

    //
    do
    {
        value[byte] = kernel->synchConsoleIn->GetChar();
        byte++;

    } while (byte < maxSize && value[byte - 1] != '\0' && value[byte - 1] != '\n');

    value[byte - 1] = '\0';

    // Because 1 char = 1 byte
    // --> We consider all cases :
    //  1. Input only 1 char
    //  2. No input (Null String)
    //  3. Input more 1 char(ex: 2,3,4,5... char)

    byte = byte - 1; // String has '\n' and '\0'
    // cout << "byte: " << byte << endl;

    switch (byte)
    {
    case 1:
    {
        char character = value[0];
        machine->WriteRegister(2, character);
        // cout << "Char: " << character << endl;
        break;
    }
    case 0:
    {
        printf("No input ! \n");
        machine->WriteRegister(2, 0);
        break;
    }
    default:
    {
        printf("More chacter !!!! Vui long thu lai \n");
        machine->WriteRegister(2, 0);
        break;
    }
    }
    delete[] value;
}

void Kernel::EH_PrintChar()
{
    // Read from Register 4
    char character = (char)kernel->machine->ReadRegister(4);
    // Out put the char Console
    kernel->synchConsoleOut->PutChar(character);
}

// Project 2 /-------------------------------------------------------------------

void Kernel::File_Create()
{
    int virtAddr;
    char *fileName;

    virtAddr = kernel->machine->ReadRegister(4);

    fileName = kernel->User2System(virtAddr, MAX_FILE_LENGTH + 1); // MaxFileLength là = 32

    if (fileName == NULL)
    {
        cout << "\n Not enough memory in system";
        DEBUG(dbgFile, "\n Not enough memory in system");
        kernel->machine->WriteRegister(2, -1); // trả về lỗi cho chương trình người dùng
        delete fileName;
        return;
    }
    DEBUG(dbgFile, "\n Finish reading filename.");

    if (!kernel->fileSystem->Create(fileName, 0))
    {
        cout << "\n Error create file " << fileName;
        kernel->machine->WriteRegister(2, -1);
        delete fileName;
        return;
    }

    kernel->machine->WriteRegister(2, 0); 
    delete fileName;
    return;
}

void Kernel::File_Open()
{
    int virtual_add = kernel->machine->ReadRegister(4);    // đọc thanh ghi số 4 để lấy ra địa chỉ , xong từ địa chỉ ->biết chuối
    char *fileName = kernel->User2System(virtual_add, 32); // hàm copy chuỗi từ user sang kernel
    // Bản chất của việc file : hệ thống sẽ cũng 1 bảng chứa những file mở (bảng file)
    //  Mục tiêu code 1 bảng chứa toàn bộ những file mình muốn mở .
    //  Mình muốn mở file mình sẽ thư viện hỗ trợ của hệ điều hành Linux
    //  bảng chứa file , mình tìm ra vị trí còn trống để nó có thể mở file

    for (int i = 2; i < 20; i++) //  kiểm tra 1 file đã có đang mở hay không
    {
        if (kernel->fileSystem->ListFile[i] != NULL)
        {
            if (strcmp(fileName, kernel->fileSystem->ListFile[i]->fileName) == 0)
            {
                kernel->machine->WriteRegister(2, i);
                delete[] fileName;
                return;
            }
        }
    }

    int freeIndex = kernel->fileSystem->FindFreeSlot(); // tìm slot còn trống 

    if (freeIndex == -1) // không còn chỗ trống 
    {
        DEBUG(dbgSys, "\nBo nho day ");
        cout << "Bo nho day " << endl;
        kernel->machine->WriteRegister(2, -1);
        delete[] fileName;
        return;
    }

    // gán id và lưu trữ openfile vào bảng mô tả
    kernel->fileSystem->ListFile[freeIndex] = kernel->fileSystem->Open(fileName);
    kernel->fileSystem->ListFile[freeIndex]->fileName = new char[strlen(fileName)]; // cấp phát bộ nhớ

    strcpy(kernel->fileSystem->ListFile[freeIndex]->fileName, fileName); // đặt tên cho id mô tả bằng tên file

    if (!kernel->fileSystem->ListFile[freeIndex]) // kiểm tra file có tồn tại hay không
    {
        DEBUG(dbgSys, "\nFile not exist");
        cout << "File not exist" << endl;
        kernel->machine->WriteRegister(2, -1);
    }
    else
    {
        DEBUG(dbgSys, "\nOpen file successfully!!!");
        cout << "Open file successfully!!!" << endl;
        kernel->machine->WriteRegister(2, freeIndex);
    }

    delete[] fileName;
    return;
}

void Kernel::File_Close()
{
    int OpenFileID = kernel->machine->ReadRegister(4);

    if (OpenFileID < 2 || OpenFileID > 20) // kiểm tra file có nằm trong bảng mô tả hahy không
    {
        cout << "Error. Cannot open file" << endl;
        DEBUG(dbgSys, "\nError. Cannot open file");
        kernel->machine->WriteRegister(2, -1);
        return;
    }

    if (!kernel->fileSystem->ListFile[OpenFileID]) // kiểm tra file có tồn tại
    {
        DEBUG(dbgSys, "\nFile not exist!");
        cout << "File not exist!" << endl;
        kernel->machine->WriteRegister(2, -1);
        return;
    }
    else
    {
        // delete[] kernel->fileSystem->ListFile[OpenFileID]->fileName;
        delete kernel->fileSystem->ListFile[OpenFileID];
        cout << "\nClose file sucessfully!!!" << endl;
        kernel->fileSystem->ListFile[OpenFileID] = NULL;
        kernel->machine->WriteRegister(2, 1);
        return;
    }
}

void Kernel::File_Remove()
{
    int virAddr;
    char *fileName;

    // check for exception
    virAddr = kernel->machine->ReadRegister(4);

    fileName = kernel->User2System(virAddr, MAX_FILE_LENGTH); // MaxFileLength là = 32

    if (fileName == NULL)
    {
        printf("\n Not enough memory in system");
        DEBUG(dbgFile, "\n Not enough memory in system");
        kernel->machine->WriteRegister(2, -1); // trả về lỗi cho chương trình người dùng
        delete fileName;
        return;
    }

    for (int i = 2; i < 20; i++) // kiem tra file co dang mo hay khong nếu đang mở trả về -1
    {
        if (kernel->fileSystem->ListFile[i] != NULL)
        {
            if (strcmp(fileName, kernel->fileSystem->ListFile[i]->fileName) == 0)
            {
                kernel->machine->WriteRegister(2, -1);
                delete[] fileName;
                return;
            }
        }
    }
    if (!kernel->fileSystem->Remove(fileName))
    {
        printf("\n Error delete file '%s'", fileName);
        kernel->machine->WriteRegister(2, -1);
        delete fileName;
        return;
    }

    kernel->machine->WriteRegister(2, 0); // trả về cho chương trình người dùng thành công
    delete fileName;
}

void Kernel::File_Read()
{

    int virtAdr = kernel->machine->ReadRegister(4);
    int bufferSize = kernel->machine->ReadRegister(5);
    int fileID = kernel->machine->ReadRegister(6);

    int n_buf = 0;
    char *buffer;
    char c;
    int i = 0;

    // Kiem tra id cua file truyen vao co nam ngoai bang mo ta file khong ?
    if (fileID < 0 || fileID > 20)
    {
        printf("\nCannot read file cause ID does not belong in the description table");
        machine->WriteRegister(2, -1);
        return;
    }
    // Kiem tra file co ton tai khong
    if (kernel->fileSystem->ListFile[fileID] == NULL)
    {
        printf("\nCannot read file cause this file does not exist");
        machine->WriteRegister(2, -1);
        return;
    }

    if (fileID == 0) // Nếu là file stdin thì tiến hành đọc từ màn hình //kernel->fileSystem->ListFile[fileID] == 0
    {
        while (i < bufferSize)
        {
            c = kernel->synchConsoleIn->GetChar();
            if (c == '\n' || c == '\001') // finish input
            {
                buffer[i] = '\0';
                break;
            }
            buffer[i++] = c;
        }
        buffer[i] = '\0';

        kernel->System2User(virtAdr, i, buffer);
        machine->WriteRegister(2, i);

        delete buffer;
        return;
    }

    buffer = kernel->User2System(virtAdr, bufferSize); // truyền địa chỉ từ user space đến kernel
    
    n_buf = kernel->fileSystem->ListFile[fileID]->Read(buffer, bufferSize); // đọc file

    if (n_buf > 0) // nếu đọc được file với số byte lớn hơn 0
    {
        
        System2User(virtAdr, n_buf, buffer);
        machine->WriteRegister(2, n_buf);
    }
    else
    {
        machine->WriteRegister(2, INT_MIN); // các trường hợp bị lỗi sai
    }

    delete buffer;
    return;
}

void Kernel::File_Write()
{
    int virAddr = machine->ReadRegister(4);
    int bufferSize = machine->ReadRegister(5);
    int fileID = machine->ReadRegister(6);

    int n_buf = 0;
    int i = 0;
    char *buffer = new char[bufferSize];

    // Kiem tra id cua file truyen vao co nam ngoai bang mo ta file khong
    if (fileID < 0 || fileID > 20)
    {
        printf("\nCannot write file cause ID does not belong in the description table");
        kernel->machine->WriteRegister(2, -1);
        return;
    }

    // Kiem tra file co ton tai khong
    if (kernel->fileSystem->ListFile[fileID] == NULL)
    {
        printf("\nCannot write file cause this file does not exist");
        machine->WriteRegister(2, -1);
        return;
    }

    // file stdin  tra ve -1
    if (fileID == 0)
    {
        printf("\nCannot write file stdin or file only read.");
        kernel->machine->WriteRegister(2, -1);
        return;
    }

    // truyền user space xuống kernel
    buffer = kernel->User2System(virAddr, bufferSize);

    n_buf = kernel->fileSystem->ListFile[fileID]->Write(buffer, bufferSize); // bắt đầu ghi vô file

    //  ghi file  thi tra ve so byte thuc su
    if (n_buf > 0)
    {
        kernel->machine->WriteRegister(2, n_buf);
        delete buffer;
        return;
    }

    if (fileID == 1) // Xet truong hop con lai ghi file stdout
    {
        while (buffer[i] != 0 && buffer[i] != '\n') // Vong lap de write den khi gap ky tu '\n'
        {
            kernel->synchConsoleOut->PutChar(buffer[i]);
            i++;
        }
        buffer[i] = '\n';
        kernel->synchConsoleOut->PutChar(buffer[i]); // Write ky tu '\n'
        kernel->machine->WriteRegister(2, i - 1);    // Tra ve so byte thuc su write duoc
        delete buffer;
        return;
    }
}

void Kernel::File_Seek()
{
    
    int pos = kernel->machine->ReadRegister(4);    
    int fileID = kernel->machine->ReadRegister(5); 

    // Kiem tra id cua file truyen vao co nam ngoai bang mo ta file khong
    if (fileID < 0 || fileID > 20)
    {
        printf("\nCannot seek file cause ID does not belong in the description table");
        kernel->machine->WriteRegister(2, -1);
        return;
    }

    // Kiem tra file co ton tai khong
    if (kernel->fileSystem->ListFile[fileID] == NULL)
    {
        printf("\nCannot seek file cause this file does not exist");
        kernel->machine->WriteRegister(2, -1);
        return;
    }

    // Kiem tra co goi Seek tren file stdin va stdout
    if (fileID == 0 || fileID == 1)
    {
        printf("\nCannot seek file on file console.");
        kernel->machine->WriteRegister(2, -1);
        return;
    }

    // Neu pos = -1 thi gan pos = Length nguoc lai thi giu nguyen pos
    if (pos == -1)
    {
        pos = kernel->fileSystem->ListFile[fileID]->Length();
    }

    if (pos > kernel->fileSystem->ListFile[fileID]->Length() || pos < 0) // trường hợp seek quá độ dài file hoặc vị trí không đúng
    {
        cout << "\n Cannot seek file to this location";
        kernel->machine->WriteRegister(2, -1);
    }
    else
    {
        // trả lại vị trí thực sự của file
        kernel->fileSystem->ListFile[fileID]->Seek(pos);
        machine->WriteRegister(2, pos);
    }
    return;
}