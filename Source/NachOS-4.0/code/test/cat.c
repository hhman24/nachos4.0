#include "syscall.h"
#define MAX_LENGTH 32

int main()
{
    int openFileId;
    int fileSize;
    char c; 
    char *fileName;
    int i; 
    PrintString("------------ Cat Program -----------\n");
    PrintString("Enter File Name: ");
    ReadString(fileName, MAX_LENGTH);

    openFileId = Open(fileName);
    
    if (openFileId >= 0) // == -1 error
    {

        fileSize = Seek(-1, openFileId);

        PrintNum(fileSize);

        i = 0;
        Seek(0, openFileId);
        PrintString("-> File content: \n");

        for (; i < fileSize; i++) 
        {
            Read(&c, 1, openFileId); 
            PrintChar(c);            
        }

        Close(openFileId); 
    }
    else
    {
        PrintString("-> Error!! Cannot open file!!\n");
    }

    Halt();
}