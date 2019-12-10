/* @date   10 Nov 2019
 * @target lab4-exercise3
 * @brief  test for TLB miss handler
 * */

#include "syscall.h"

int A[64];

int 
main()
{
    int i, j, tmp;

    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < 64; i++)
        A[i] = 64 - i;

    /* then sort! */
    for (i = 0; i < 63; i++)
        for (j = i; j < (63 - i); j++)
           if (A[j] > A[j + 1]) {       /* out of order -> need to swap ! */
              tmp = A[j];
              A[j] = A[j + 1];
              A[j + 1] = tmp;
           }
    
    Halt();
}
