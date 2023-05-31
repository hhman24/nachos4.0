#include "syscall.h"

int main()
{
    char filename_des[255], a[100];
    char filename_sou[255];
    int ID_DesFile, ID_SouFile, lengthFile = 0;

    PrintString("--------- Copy Program --------\n");
    PrintString("Enter destination file: ");
    ReadString(filename_des, 255);
    PrintString("Enter source file: ");
    ReadString(filename_sou, 255);

    ID_SouFile = Open(filename_sou);
    if (ID_SouFile == -1)
    {
        PrintString("Cannot open source file!\n");
    }
    else
    {
        //Close(ID_DesFile);
        ID_DesFile = Open(filename_des);
        if(ID_DesFile == -1)
        {
            PrintString("Cannot open destination file!\n");
            
        }
        else
        {
            //lấy chiều dài của file nguồn
            //int Seek(int position, OpenFileId id);
            lengthFile = Seek(-1, ID_SouFile);
            
            Seek(0, ID_SouFile);
            Seek(0, ID_DesFile);
           // PrintNum(lengthFile);
            // int Read(char *buffer, int size, OpenFileId id);
            Read(a, lengthFile, ID_SouFile);
               
            //int Write(char *buffer, int size, OpenFileId id);
            Write(a, lengthFile, ID_DesFile);
            PrintString("Copy file successfully");

        }
    }
  
    Close(ID_DesFile);
    Close(ID_SouFile);
    Halt();
}