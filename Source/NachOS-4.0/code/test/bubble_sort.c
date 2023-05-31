#include "syscall.h"
int main()
{
    int i, j;
    int n;
    int temp;
    int mode = 0;
    int arr[100];
    PrintString("\n=============== SORT PROGRAM =============\n");
    // User input
    PrintString("||\tMode 1: ASC  \t\t\t||\n");
    PrintString("||\tMode 2: DESC \t\t\t||\n");
    PrintString("==========================================\n");
    PrintString("\tYour mode: ");
    while(1)
    {
        mode = ReadNum();
        if((mode != 1) && (mode != 2))
        {
            PrintString("Try again: ");
        }
        else 
        {
            break;
        }
    }
    
    PrintString("==> Input the number of elements  (0 <= n <= 100):\nn = ");
    n = ReadNum();
    while(n < 0 || n > 100)
    {
        PrintString("!!! n Has to be an integer between 1 and 100 (inclusive), please ry again\n");
        PrintString("n = ");
        n = ReadNum();
    }

    PrintString("Array: \n");
    for (i = 0; i < n; i++)
    {
        PrintString("Element [");
        PrintNum(i);
        PrintString("]");
        PrintString(": ");
        arr[i] = ReadNum();
    }

    // Bubble Sort
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n - 1; j++)
        {
            if (mode == 1)
            {
                if (arr[j] > arr[j + 1])
                {
                    // Swap arr[j] and arr[j+1]
                    temp = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = temp;
                }
            }
            else if (mode == 2)
            {
                if (arr[j] < arr[j + 1])
                {
                    // Swap arr[j] and arr[j+1]
                    temp = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = temp;
                }
            }
        }
    }
    // Print new order
    PrintString("\nSORTED ARRAY: ");
    for (i = 0; i < n; i++)
    {
        if (i != 0)
        {
            PrintString(", ");
        }
        PrintNum(arr[i]);
    }
    PrintString("\n==================== SORT SUCCESSFULLY !!! =========================\n");
    Halt();
}