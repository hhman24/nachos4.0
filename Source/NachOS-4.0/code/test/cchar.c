#include"syscall.h"

int main()
{
    char *c;
    ReadString(c, 223);
    PrintString(c);

    // char c = ReadChar();
    // PrintChar(c);
    Halt();
}