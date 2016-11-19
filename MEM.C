/* mem.c  1991-08-03 TD,  version 1.1 */
/*
 * This file contains proprietary information of IVT Electronic AB.
 * Copying or reproduction without prior written approval is prohibited.
 *
 * This file is furnished under a license agreement or nondisclosure
 * agreement. The software may be used or copied only in accordance 
 * with the terms of the agreement.
 *
 * In no event will IVT Electronic AB, be liable for any lost revenue or
 * profits or other special, indirect and consequential damages, even if
 * IVT has been advised of the possibility of such damages.
 *
 * IVT Electronic AB
 * Box 996
 * 191 29 Sollentuna
 * Sweden
 */

/*
! mem.c 
! Copyright (C) 1991, IVT Electronic AB.
*/


#ifdef OSK

#else
#include <alloc.h>
void vcfatal(char *s);
#endif

int totalSize = 0;
int currentTotalSize = 0;
int getMemCalls = 0;
int returnMemCalls = 0;

/*
struct {
  char *address;
  int size;
} array[5000];
*/

char *getMem(s)
int s;
{
  char *a;
  
  currentTotalSize += s;
  if (currentTotalSize > totalSize)
    totalSize = currentTotalSize;

  a = (char *) malloc(s);
  if (a == 0) {
    vcfatal("Out of memory");
  }  
/*
  array[getMemCalls].address = a;
  array[getMemCalls].size = s;
*/
  
  getMemCalls ++;
  return a;
}

void returnMem(s)
char *s;
{
  int i, siz;
  
  returnMemCalls ++;
  free(s);
 
/*
  for (i = 0; i < getMemCalls; i++) {
    if (array[i].address == s) {
      siz = array[i].size;
      break;
    }
  }
  if (i < getMemCalls) {
    currentTotalSize -= siz;
  } else {
    printf("free: address not found (not alloceted here !)\n");
  }
*/
  
}
