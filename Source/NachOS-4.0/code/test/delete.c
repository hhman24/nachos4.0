#include "syscall.h"
int main()
{
    // void ReadString (char* buffer, int length);
    // int Remove(char *name);
    char buffer[225];
    int flag;

    PrintString("-------- Delete Program -------\n");
    PrintString("Enter file name: ");
    ReadString(buffer, 255);
    flag = Remove(buffer);
    if (flag == 0)
    {
        PrintString("Delete file succesfully\n");
    }
    else
    {
        PrintString("Delete file failed\n");
    }
    Halt();
}   