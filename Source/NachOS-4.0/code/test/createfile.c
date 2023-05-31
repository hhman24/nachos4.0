#include "syscall.h"

int main()
{
    char buffer[256];
    int res;
    
    PrintString("---------- Create Program -------\n");
    PrintString("File name: ");
    ReadString(buffer, 255);
    res = Create(buffer);
    if (res = 1)
    {
        PrintString("Create file sucessfully\n");
    }
    else
    {
        PrintString("Create file failed\n");
    }
    Halt();
}