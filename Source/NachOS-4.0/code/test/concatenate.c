#include "syscall.h"

int main()
{
    char filename_scr1[255], a[100];
    char filename_scr2[255];
    char filename_des[255];
    int ID_SrcFile1, ID_SrcFile2, ID_DesFile, lengthFile1 = 0, lengthFile2 = 0, lengthFileDes = 0;

    PrintString("-------------- Concatenate Program --------------\n");
    PrintString("Enter source file 1: ");
    ReadString(filename_scr1,255);
    PrintString("Enter source file 2: ");
    ReadString(filename_scr2,255);

    ID_SrcFile1 = Open(filename_scr1);
    ID_SrcFile2 = Open(filename_scr2);
    if(ID_SrcFile1 == -1 || ID_SrcFile2 == -1)
    {
        PrintString("Cannot open file source!\n");
    }
    else
    {
        //int Create(char *name);
        PrintString("Enter destination file : ");
        ReadString(filename_des,255);
        Create(filename_des);
       // PrintString(filename_des);
        ID_DesFile = Open(filename_des);
        // //lấy chiều dài của file nguồn số 2
        lengthFile1 = Seek(-1, ID_SrcFile1);
        lengthFile2 = Seek(-1, ID_SrcFile2);

        lengthFileDes = lengthFile1 + lengthFile2;

        Seek(0, ID_SrcFile1);
        Seek(0, ID_SrcFile2);

        Seek(0, ID_DesFile);

        Read(a, lengthFile1, ID_SrcFile1);
        Write(a, lengthFile1, ID_DesFile);

        Seek(lengthFile1, ID_DesFile);

        Read(a, lengthFile2, ID_SrcFile2);
        Write(a, lengthFile2, ID_DesFile);

        PrintString("Concatenate successfully!!!");

    }

    Close(ID_DesFile);
    Close(ID_SrcFile1);
    Close(ID_SrcFile2);

    Halt();
}