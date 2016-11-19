/* calendar.c  1991-08-03 TD,  version 1.1 */
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
! calendar.c
! Copyright (C) 1991, IVT Electronic AB.
*/


/*
!     File: calendar.c
!     
!     Contains code for handling calendar routines
!
!     History     
!     Date        Revision Who  What
!     
!     29-nov-1990     1.0  TD   Initial release of translator
!
!
*/
#include <stdio.h>
#include <time.h>
#include "symtable.h"
#include "calendar.h"
#include "prototype.h"

void dump(a)
struct tm *a;
{
  printf("%d, %d, %d    %d:%d:%d\n",
    a->tm_year, a->tm_mon, a->tm_mday,
    a->tm_hour, a->tm_min, a->tm_sec);
}

/*
! high abstraction inclusive or of two color expressions
*/
long orColorExpression(a, b)
long a, b;
{
  return a | b;
}

/*
!   marks calendar intervall with current state, on/off
*/
void setCalendar(currentCalendar, bt, every, onoffState)
idInfo *currentCalendar;
struct _btTime *bt;
long every;
long onoffState;
{
  long diff1, diff2;
  long i, index;
  unsigned char mask;
  
/*
!   every not implemented yet
*/
  diff1 = difftime(bt->first,
                      currentCalendar->calendarInfo.baseTime) / 3600;
  diff2 = difftime(bt->last, 
                      currentCalendar->calendarInfo.baseTime) / 3600;
  for (i = diff1; i < diff2; i++) {
    index = i / 8;
    mask = (char) (1 << (i % 8));
    if (index < currentCalendar->calendarInfo.size) {
      currentCalendar->calendarInfo.calendar[index] &= ~mask;  /* clear bit */
      if (onoffState)                                          /* if on then */
        currentCalendar->calendarInfo.calendar[index] |= mask; /* set bit */
    }
    else
      vcerror("Set Calendar out of range");
  }
  if (every)
    every = 0;
}

/*
!   allocates a new calendar for the info entry in the symbol table
!   the new calendar will be cleared
*/
idInfo *newCalendar(range, resolution, baseTime)
long range;
long resolution;
long baseTime;
{
  idInfo *tmp;
  long size;
  char *ptr;
  
  size = range / resolution;
  if (size <= 1) {
    vcerror("RANGE must be greater than RESOLUTION operands");
    return (idInfo *) 0;
  }
  else if (size > 5000) {
    vcerror("Too many entries with those RANGE and RESOLUTION operands");
    return (idInfo *) 0;
  }
  tmp = (idInfo *) malloc(sizeof(idInfo));
  tmp->calendarInfo.range = range;
  tmp->calendarInfo.resolution = resolution;
  tmp->calendarInfo.baseTime = baseTime;
  tmp->calendarInfo.size = size;
  tmp->calendarInfo.calendar = (char *) malloc(size);
  for (ptr = tmp->calendarInfo.calendar; size ; size --)
    *ptr++ = '\0';
  return tmp;
}
