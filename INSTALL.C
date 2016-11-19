#include <module.h>
#include <stdio.h>
#include <errno.h>

unsigned short int *s = 0x020000;
unsigned short int *max = 0x0a0000;

long int *loadAddress = 0x410;
static int error;

main(c, a)
int c;
char *a[];
{
  int size;
  FILE *fp;
  int all = 0;
  char b[120];
  
  if (c == 2 && a[1][0] == '-' && ((a[1][1] | 0x20) == 'a')) {
    all = 1;
  }
  installBusErrorHandler();  
  s--;
  while (!error && (++s < max)) {
    if (*s == MODSYNC) {
      if (checkHeaderParity(s) != 0) {
        continue;
      }
      if (!all)
        if (checkCrc(s)) {    /* its already ok */
          printf("Found ok module '%s' at address %6x\n", getName(s), s);
          continue;
        }
      sprintf(b,"Trying to update module '%s' at address %6x...", getName(s), s);
      write(1, b, strlen(b));
      size = ((struct modhcom *) s)->_msize;
      if (_setcrc(s) != -1) {
        *loadAddress = (long) s;
        fp = fopen("/mem", "w");
        fwrite(s, size, 1, fp);
        fclose(fp);
        s += size - 2;
        printf("ok\n");
      } else 
        printf("error %d\n", errno);
    }
  }
  if (error) {
    printf("Error #%d at address %6x\n", error, s);
  }
}

#asm
ExcpTbl   dc.w        T_BusErr,Bushandler-*-4,-1
remExcp   dc.w        T_BusErr,0,-1

installBusErrorHandler:
          movem.l     a0-a1,-(a7)
          movea.l     #0,a0
          lea         ExcpTbl(pc),a1
          os9         F$STrap
          movem.l     (a7)+,a0-a1
          rts
          
Bushandler: move.l    #102,error(a6)
          move.l      R$a7(a5),a7         
          move.l      R$pc(a5),-(a7)
          move.w      R$sr(a5),-(a7)
          movem.l     (a5),a0-a6/d0-d7
          rtr
#endasm

getName(s)
struct modhcom *s;
{
  char *p;
  
  p = s;
  p += s->_mname;
  return p;
}

checkCrc(s)
struct modhcom *s;
{
  int accum = -1;
  
  crc(s, s->_msize, &accum);
  accum &= 0x00ffffff;
  return accum == CRCCON;
}

checkHeaderParity(s)
unsigned short int *s;
{
  int i, q;
  
  q = 0xffff;
  for (i = 0; i < 24; i++) {
    q ^= *s++;
  }
  return q;
}
