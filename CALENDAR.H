/* calendar.h  1991-08-03 TD,  version 1.1 */
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
! calendar.h
! Copyright (C) 1991, IVT Electronic AB.
*/


typedef struct 
{
  long range;
  long resolution;
  long baseTime;
  int size;
  char *calendar;
} calendar;

typedef struct _btTime
{
  long first, last;
} btTime;

#define stateOn  1
#define stateOff 0
#define colorBlack  1
#define colorOrange 2
#define colorRed    4

idInfo *newCalendar();
