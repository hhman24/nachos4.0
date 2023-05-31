#include "syscall.h"

void printASCII()
{
    int i;
    for (i = 33; i < 127; i++)
    {
        PrintChar(i);
        PrintString(" --> ");
        PrintNum(i);
        PrintChar('\n');
    }
}

int main()
{
    PrintString("-----------------------------------\n");
    PrintString("            TABLE ASCII\n");
    printASCII();
    PrintString("-----------------------------------\n");
    Halt();
}