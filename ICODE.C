#include <stdio.h>
#include "icode.h"
#include "symtable.h"
#include "prototype.h"
extern FILE *fpIdi;

#define SEEK_SET 0

void insertJmpAdr(where, what)
long where, what;
{
  long w;
  w = ftell(fpIdi);
  fseek(fpIdi, where, SEEK_SET);
  fwrite(&what, sizeof(long), 1, fpIdi);
  fseek(fpIdi, w, SEEK_SET);        /* restore */
}

long getIcodePtr()
{
  return ftell(fpIdi);
}

void emitIcode(typ)
int typ;
{
  char b;
/*  mem[iStreamPtr] = typ;    */
  b = typ;
  fwrite(&b, 1, 1, fpIdi);
}

      /* jump from location 'is' to here */
void emitIcode_jmp(loc)
int loc;
{
/*   mem[loc] = iStreamPtr;  */    /* or relative ? */
  insertJmpAdr(loc, getIcodePtr());
}

void emitIcodeExpression(expr)
exprTree *expr;
{
}
