/* symtable.c  1993-11-24 TD,  version 1.82 */
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
! symtable.c
! Copyright (C) 1991-1993 IVT Electronic AB.
*/


/*
!     File: symtable.c
!     
!     Contains code for handling routines close to symbol table
!
!     History
!     Date        Revision Who  What
!     
!     29-nov-1990    1.000 TD   Initial release of translator
!     09-dec-1990    1.001 TD   added check for any alarms before packAlarms
!
!     11-may-1991    1.010 TD   service check added, and disable check
!     22-may-1991    1.020 TD   all filters has its previous returned value
!                               as their first argument !
!     22-jul-1991    1.03  TD   functions emitted to symbol table !
!                               vector first release,
!                               durationClause, at- abs&rel time is expressions
!                               18:00:00 is now a legal integer constant !!
!                               (translates to 18*3600 sec)
!     05-aug-1991    1.05  TD   %f changed to %g in codeExpression()
!                               since PCB cross compiler are limited in
!                               the floating point constants they can handle
!                               by their native floating point arithmetic.
!
!      8-sep-1991    1.06  TD   option_T added for update of timers regulary
!
!     28-nov-1991    1.07  TD   emitScanBindings() emits external declaration
!                               of any functions declared to fpSymTable.
!
!     10-dec-1991    1.20  TD   lock2 calculation has an extra () to
!                               correct character addition
!     18-dec-1991    1.21  TD   Service check removed (? request from ctrl)
!     19-dec-1991    1.22  TD   Service check re-inserted !!!(? request..)
!     21-jan-1992    1.23  TD   Added {} blocks arround initialization of
!                               arrays, (initArray).
!     13-mar-1992    1.30  TD   Prom hooks for _initScan, _initMain,
!                               _markAlarm, _unmarkAlarm etc
!
!     17-mar-1992    1.40  TD   BUGFIX !!!!
!                               Previous version couldnt handle:
!                               a = 10/(10*10)        -> a = 10/10*10
!                               a = 10 - (5 - 10);    -> a = 10-5-10;
!                               Now ok, right check for precedence change
!                               from < 0 to <= 0 !!!
!
!     21-may-1992    1.50  TD   Added option -n, reports warning if
!                               variables are used before they are initialized
!
!      4-sep-1992    1.60  TD   Added _recList operator for recursive lists.
!                               Used in the screen_order statement
!                               Added _screenIdent
!
!     14-oct-1992    1.61  TD   Added function support
!
!     20-apr-1993    1.70  TD   Added quick i/o support
!
!     10-nov-1993    1.80  TD   Changed init routines for trap modules
!				added _initScanTraps(); and
!				_initMainTraps();
!
!     23-nov-1993    1.81  TD   Bugfix if formel parameter is a referens
!				then actual parameter must be of same type
!				and a variable. Added error message
!			parameter '%s' must be of type int (%s)"
!
!     24-nov-1993    1.82  TD	Bugfix/enhancement in traverseTree routine.
!				Old routine suffered from stack overflow
!				problem. New algorithm is linear
!				so now can we handle > 128 entries in
!				initialization list for arrays.
*/
#include <errno.h>
#include <stdio.h>
#ifdef OSK
#include <strings.h>
#else
#include <string.h>
#include <stdlib.h>
#endif
#include "symtable.h"
#include "calendar.h"
#include "prototype.h"

#define GLOBAL_ACCESS      "dm->"
#define GLOBAL_ACCESS_NAME "dm"
#define GLOBAL_LOCK_SUFFIX "_lock"
#define OFFSET_SUFFIX      "_OFFSET"
#define NAMEOFDATAMODULE   "VARS"
#define NAMEOFDATAMODULE2  "VAR2"

/* symbol table for identifiers such as variables, objects and stuff */

static symTable  *symbolTable = 0,	/* root scope pointer */
		 *currentScope = 0;  	/* and current scope */

static symTable *xrefSymbolTableRoot = 0,
		*xrefSymbolTableInner = 0,
		*xrefSymbolTableCurrent = 0;

extern int yylineno;			/* used for declaredAt - x-ref */
extern FILE *yyout, *fpScreen, *fpScan, *fpSymTable, *fpMain, *fpCurrent;

extern char generateRandom;  /* true->input scan replaced by random generator */

extern char scopeName[];
extern int scopeCnt;

extern int systemId, systemDefined;
extern char systemCid[248];

extern int notInitialized;            /* added 920521 */

extern int xref;

extern int usePromHooks;

extern int timerCounter;

extern int alarmVector[], currentNoOfAlarms;

extern int optionGlobalFunctions;

extern int option_T;    /* if timers are to be called regulary or 
                            after each scan-cycle */
extern long sizeOfModule_VARS, sizeOfModule_VAR2;

extern int option_Q;    /* quick i/o */

extern char typeOfTimer[];    /* 0 fast, 1 slow */

/* idIdent *iref, *tref;	*/
/* char id[40];	*/

declType newDeclSpec(storage, type, nodeNo)
storageType storage;
idType type;
long nodeNo;
{
    declType tmp;
    tmp.storageSpec = storage;  tmp.type = type;
    tmp.nodeNo = nodeNo;
    tmp.alias = 0;
    return tmp;
}

/* int getSize(idRef)
idIdent *idRef;
{
} */

int emitType(fp, type)
FILE *fp;
idType type;
{
    switch (type) {
        case _charFunc:
        case _charVec:
        case _charRef:
	case _char:	emitCode2(fp, _TYPE_OF_CHAR);	break;
        case _intFunc:
	case _intVec:
        case _intRef:
	case _int:	emitCode2(fp, _TYPE_OF_INT);	break;
	case _floatFunc:
	case _floatVec:
	case _floatRef:
	case _float:	emitCode2(fp, _TYPE_OF_FLOAT);	break;
        case _calendar: emitCode2(fp, _TYPE_OF_CALENDAR);
          emitCode2(fp, " * ");
          break;
	default:
	    fprintf(fp, "/* emitType: type = %d */", type);
	    break;
    }
/*    return getSize(idRef); */
  return 1;
}
    
/* looks up whatever at a specified scope level, i.e. both objects and classes*/

idIdent *lookUpInScope(scope, id)
symTable *scope;
char *id;
{
    register symLink *tmp = scope->thisScope;
    int match;

    while (tmp) {
	if ((match = strcmp(tmp->idRef->IDENT, id)) >= 0) {
            if (match)
  	      return (idIdent *) 0;
  	    else if (tmp->idRef->declSpec.storageSpec != _empty)
  	      return tmp->idRef;
/*
  	      return (!match) ? tmp->idRef : (idIdent *) 0;
*/
	}
	tmp = tmp->next;
    }
    return (idIdent *) 0;
}

void insertLineAccessed(idRef, accessedAtLine)
idIdent *idRef;
int accessedAtLine;
{
    xrefSpec *tmp = idRef->xrefs;
    if (tmp->currentIndex >= MAX_NO_OF_ACCESS_LINES)
    {
	char s[100];
	sprintf(s, 
"too many accesses to a single variable for cross reference facility, ( > %d)", 
	    MAX_NO_OF_ACCESS_LINES);
	vcwarning(s);
    }
    else
	tmp->lines[tmp->currentIndex ++ ] = accessedAtLine;
}

idIdent *lookUpIdent(id)
char *id;
{
    symTable *tmpScope = currentScope;
    idIdent *tmp;
    while (tmpScope) {
	if (tmp = lookUpInScope(tmpScope, id)) {
	    if (xref)
		insertLineAccessed(tmp, yylineno);
	    return tmp;
	}
	tmpScope = tmpScope->outerScope;
    }
    return (idIdent *) 0;
}
    
void declareGlobalId(cid, type)
char *cid;
idType type;
{
    declType declSpec; /* = { _global, type }; */
   
    declSpec.storageSpec = _global;
    declSpec.type = type; 
    declSpec.nodeNo = 0;
    declSpec.alias = 0;
   
    declareId(cid, declSpec, 0, 0);
}

/*
!	declare an identifier
!	all arguments will be copied
!	third argument scope is optional, default is currentScope
*/
idIdent *declareId(id, declSpec, info, scope)
char *id;
declType declSpec;
idInfo *info;
symTable *scope;
{
    idIdent *tmp = (idIdent *) malloc(sizeof(idIdent));

    strncpy(tmp->IDENT, id, 31);    /* only 31 characters significant */
    tmp->IDENT[31] = 0;
    tmp->declSpec = declSpec;	/* struct will be copied */
    tmp->expr = 0;              /* most ids will not have an expression */
    if (info)
	memcpy(tmp->info = (idInfo *) malloc(sizeof(idInfo)), 
				info, sizeof(idInfo));
    else
	tmp->info = 0;
    tmp->initialized = 0;         /* variable has no value yet */

    if (xref) {
	xrefSpec *tmpXref = (xrefSpec *) malloc(sizeof(xrefSpec));
	tmpXref->declaredAt = yylineno;
	tmpXref->currentIndex = 0;
	tmp->xrefs = tmpXref;
    } else
	tmp->xrefs = 0;
    
    insertId(scope ? scope : currentScope, tmp);
    return tmp;
}
/*
!	insert an identifier in ascending order	in current scope
!
*/
idIdent *insertId(scope, idRef)
symTable *scope;
idIdent *idRef;
{
    symLink *tmp = (symLink *) malloc(sizeof(symLink));  /* make another link */

    tmp->idRef = idRef;
    while (((struct _symLink *) scope)->next  &&	/* insert ascending */
	   (strcmp(((struct _symLink *) scope)->next->idRef->IDENT, 
							idRef->IDENT) < 0))
	scope = (struct _symTable *) ((struct _symLink *) scope)->next;

    if (((struct _symLink *) scope)->next  &&	/* check same name */
	   (strcmp(((struct _symLink *) scope)->next->idRef->IDENT, 
							idRef->IDENT) == 0)) {
	char s[180];
	sprintf(s, "Identifier %s already declared", idRef->IDENT);
	vcerror(s);
    }
    tmp->next = ((struct _symLink *) scope)->next;
    ((struct _symLink *) scope)->next = tmp;
    

#ifdef DEBUG
    printf("identifier %s declared\n", idRef->IDENT);
#endif

    return idRef;
}
	
void initSymbolTable()
{
    currentScope = symbolTable = 0;
    incScope("global", 0);
    symbolTable = currentScope;
}
    
void incScope(name, no)	/* allocate a new scope level */
char *name;
int no;				/* allocate a new scope level */
{
    symTable *tmp;

    tmp = (symTable *) malloc(sizeof(symTable));
    tmp->thisScope = 0;
    tmp->outerScope = currentScope;
    tmp->xref = 0;
    currentScope = tmp;
    if (xref) {
	tmp = (symTable *) malloc(sizeof(symTable));
	tmp->thisScope = 0;
	tmp->outerScope = 0;
	sprintf(tmp->name, "%s_%02d", name, no);
	if (xrefSymbolTableInner)
	    xrefSymbolTableInner->outerScope /*should be inner here !!*/
		= tmp;
	else
	    xrefSymbolTableRoot = tmp;
	xrefSymbolTableInner = tmp;
/*	xrefSymbolTableCurrent = tmp;		*/

	currentScope->xref = tmp;
    }
}

void decScope() 	/* return to outer scope and deallocate current scope */
{
    symTable *tmp;
    symLink *tmpLink, *tmpLink2;
    
    if (!currentScope->outerScope) {
	fprintf(stderr, "return from outermost level\n");
	return;
    }
    tmpLink = (tmp = currentScope)->thisScope;
    currentScope = currentScope->outerScope;
    if (xref) {
	tmp->xref->thisScope = tmpLink;
/*	
	
	if (!xrefSymbolTableCurrent->thisScope)
	    free((void *) xrefSymbolTableCurrent);
	else 
	    printf("program fault: decScope\n");
	xrefSymbolTableCurrent = currentScope;
*/	
	
    } else {
	free((void *) tmp);
	while (tmpLink) {
	    tmpLink2 = tmpLink;
	    tmpLink = tmpLink->next;
	    free((void *) tmpLink2);
	}
    }
}

void escapeOuterScope() 	/* return from outetmost scope */
{
    symTable *tmp;
    symLink *tmpLink, *tmpLink2;


    if (!xref)
	return ;

    tmpLink = (tmp = currentScope)->thisScope;
    tmp->xref->thisScope = tmpLink;


    fprintf(yyout, "\ncross reference table\n---------------------\n");
    while (xrefSymbolTableRoot) {
	tmpLink = (tmp = xrefSymbolTableRoot)->thisScope;
	xrefSymbolTableRoot = xrefSymbolTableRoot->outerScope;
	if (tmpLink)
	    fprintf(yyout, "scope: %s\n", tmp->name);
	while (tmpLink) {
	    int i;
	    fprintf(yyout, "%s\t:%d\t:", 
		tmpLink->idRef->IDENT,
		tmpLink->idRef->xrefs->declaredAt);

	    for (i = 0; i < tmpLink->idRef->xrefs->currentIndex - 1; i++)
		fprintf(yyout, "%d,", tmpLink->idRef->xrefs->lines[i]);
	    if (i < tmpLink->idRef->xrefs->currentIndex)
		fprintf(yyout, "%d\n", tmpLink->idRef->xrefs->lines[i]);
	    else
		fprintf(yyout, "\n");
	    free((void *) tmpLink->idRef->xrefs);
	    free((void *) tmpLink->idRef);
	    tmpLink2 = tmpLink;
	    tmpLink = tmpLink->next;
	    free((void *) tmpLink2);
	}
	free((void *) tmp);
    }
}

int alignScreen = 0;
/*
int makeScreenUse(expr)
exprTree *expr;
{
  if (expr->node->operator == _comma) {
    if (expr->left)
      makeScreenUse(expr->left);
    if (expr->right)
      makeScreenUse(expr->right);
  } else {
    char *p;
    if (expr->node->operator == _ident) {
      p = expr->node->operandConst.voidStorage;
      parseCalendar(p);
    } else if (expr->node->operator == _int_const) {
      
      vcwarning("Cannot find string...");
    }
  }
}
*/

void makeScreenUsage(expr)
exprTree *expr;
{
    fprintf(fpScreen, "typedef int (*PFI)();\n");
    fprintf(fpScreen, "struct {\n  short left, right, up, down, help;\n");
    fprintf(fpScreen, "  PFI fcn;\n  char *name;\n} screens[] = {\n");
    fprintf(fpScreen, "    /*left\tright\tup\tdown\thelp\tfcn\tname*/\n");

  codeExpression(fpScreen, expr, 0, 0);

    fprintf(fpScreen, ",\n  { 0, 0, 0, 0, 0, 0}\n};\n");

    fprintf(fpScreen, "int _ROOT_SCREEN_POINTER = 0;\n");
}

#ifdef USAGE_0
void makeScreenUsage(expr)
exprTree *expr;
{
    int screenId, rootPtr;
    char rootName[50];

    alignScreen = 0;

    fprintf(fpScreen, "typedef int (*PFI)();\n");
    fprintf(fpScreen, "struct {\n  short left, right, up, down, help;\n");
    fprintf(fpScreen, "  PFI fcn;\n  char *name;\n} screens[] = {\n");
    fprintf(fpScreen, "    /*left\tright\tup\tdown\thelp\tfcn\tname*/\n");

    while (1) {
      makeOneRow(expr);
      
    screenId = 1;
    rootPtr = 0;
    tmp = symbolTable->thisScope;
    while (tmp && (tmp = (tmpLink = tmp)->next, tmpLink)) {
	if (tmpLink->idRef->declSpec.type != _screen)
	    continue;
	fprintf(fpScreen, "    { %d,\t%d,\t%d,\t%d,\t%d,\t%s,\t\"%s\"},\t/*%d*/\n",
		tmpLink->idRef->info->screenInfo.lptr,
		tmpLink->idRef->info->screenInfo.rptr,
		tmpLink->idRef->info->screenInfo.uptr,
		tmpLink->idRef->info->screenInfo.dptr,
		tmpLink->idRef->info->screenInfo.hptr,
		tmpLink->idRef->IDENT,
		tmpLink->idRef->IDENT, screenId
		);
        if ((!(tmpLink->idRef->info->screenInfo.lptr | 
                tmpLink->idRef->info->screenInfo.uptr)) &&
		(!tmpLink->idRef->info->screenInfo.isHelp))
        {
          if (rootPtr) {           /* root already found */
            char line[80];
            sprintf(line, 
                    "screen '%s' conflicts with previous root screen '%s'\n", 
                    tmpLink->idRef->IDENT, rootName);
            vcerror(line);
/*            vcerror("More than one root screen found (?)"); */
          }
          strcpy(rootName, tmpLink->idRef->IDENT);
          rootPtr = screenId;
        }
	screenId ++;
    }
    if (!rootPtr) {           /* root not found */
      if (screenId == 1)
        vcwarning("No screens were included in source code");
      else
        vcerror("No root screen found (?)");
    }
    fprintf(fpScreen, "    { 0, 0, 0, 0, 0, 0}\n};\n");

    fprintf(fpScreen, "int _ROOT_SCREEN_POINTER = %d;\n", rootPtr);
/*
    fprintf(fpScreen, "#define _ROOT_SCREEN_POINTER %d\n", rootPtr);
    fprintf(fpScreen, "#include \"screen.c\"\n");
*/
}
#endif
void makeScreenArray()
{
    symLink *tmpLink, *tmp;
    int screenId, rootPtr;
    char rootName[50];

    screenId = 1;
    tmp = symbolTable->thisScope;
    while (tmp && (tmp = (tmpLink = tmp)->next, tmpLink)) {
	if (tmpLink->idRef->declSpec.type != _screen)
	    continue;
	if (tmpLink->idRef->info->screenInfo.right[0])
	    tmpLink->idRef->info->screenInfo.rptr = 
		    makeScreen(tmpLink->idRef->info->screenInfo.right,
		    screenId, 1 /*r*/, tmpLink);
	if (tmpLink->idRef->info->screenInfo.down[0])
	    tmpLink->idRef->info->screenInfo.dptr = 
		    makeScreen(tmpLink->idRef->info->screenInfo.down,
		    screenId, 2 /*d*/, tmpLink);
	if (tmpLink->idRef->info->screenInfo.help[0])
	    tmpLink->idRef->info->screenInfo.hptr = 
		    makeScreen(tmpLink->idRef->info->screenInfo.help,
		    screenId, 3 /*h*/, tmpLink);
	screenId ++;
    }

    fprintf(fpScreen, "typedef int (*PFI)();\n");
    fprintf(fpScreen, "struct {\n  short left, right, up, down, help;\n");
    fprintf(fpScreen, "  PFI fcn;\n  char *name;\n} screens[] = {\n");
    fprintf(fpScreen, "    /*left\tright\tup\tdown\thelp\tfcn\tname*/\n");
    screenId = 1;
    rootPtr = 0;
    tmp = symbolTable->thisScope;
    while (tmp && (tmp = (tmpLink = tmp)->next, tmpLink)) {
	if (tmpLink->idRef->declSpec.type != _screen)
	    continue;
	fprintf(fpScreen, "    { %d,\t%d,\t%d,\t%d,\t%d,\t%s,\t\"%s\"},\t/*%d*/\n",
		tmpLink->idRef->info->screenInfo.lptr,
		tmpLink->idRef->info->screenInfo.rptr,
		tmpLink->idRef->info->screenInfo.uptr,
		tmpLink->idRef->info->screenInfo.dptr,
		tmpLink->idRef->info->screenInfo.hptr,
		tmpLink->idRef->IDENT,
		tmpLink->idRef->IDENT, screenId
		);
        if ((!(tmpLink->idRef->info->screenInfo.lptr | 
                tmpLink->idRef->info->screenInfo.uptr)) &&
                (!tmpLink->idRef->info->screenInfo.isHelp))
	{
          if (rootPtr) {           /* root already found */
            char line[80];
            sprintf(line, 
                    "screen '%s' conflicts with previous root screen '%s'\n", 
                    tmpLink->idRef->IDENT, rootName);
            vcerror(line);
/*            vcerror("More than one root screen found (?)"); */
          }
          strcpy(rootName, tmpLink->idRef->IDENT);
          rootPtr = screenId;
        }
	screenId ++;
    }
    if (!rootPtr) {           /* root not found */
      if (screenId == 1)
        vcwarning("No screens were included in source code");
      else
        vcerror("No root screen found (?)");
    }
    fprintf(fpScreen, "    { 0, 0, 0, 0, 0, 0}\n};\n");

    fprintf(fpScreen, "int _ROOT_SCREEN_POINTER = %d;\n", rootPtr);
/*
    fprintf(fpScreen, "#define _ROOT_SCREEN_POINTER %d\n", rootPtr);
    fprintf(fpScreen, "#include \"screen.c\"\n");
*/
}

int makeScreen(name, fromId, dir, ref)
char *name;
int fromId;
int dir;
symLink *ref;
{
    symLink *tmpLink, *tmp;
    idIdent *scr;
    int whatId;
    int storedLineNo = yylineno;
	
    whatId = 1;
    tmp = symbolTable->thisScope;
    while (tmp && (tmp = (tmpLink = tmp)->next, tmpLink)) {
	if (tmpLink->idRef->declSpec.type != _screen)
	    continue;
	if (!strcmp(tmpLink->idRef->IDENT, name)) {
	    if (dir == 1) {			/* right */
		if (tmpLink->idRef->info->screenInfo.lptr) {
		    yylineno = tmpLink->idRef->info->screenInfo.atLine;
		    vcerror("LEFT ambigous direction");
		    yylineno = storedLineNo;
		}
	    	tmpLink->idRef->info->screenInfo.lptr = fromId;
	    } else if (dir == 2) {		/* down */
		if (tmpLink->idRef->info->screenInfo.uptr) {
		    yylineno = tmpLink->idRef->info->screenInfo.atLine;
		    vcerror("UP ambigous direction");
		    yylineno = storedLineNo;
		}
	    	tmpLink->idRef->info->screenInfo.uptr = fromId;
	    } else if (dir == 3) {			/* help */
	    	tmpLink->idRef->info->screenInfo.hptr = -1; /* fromId; */
	    	tmpLink->idRef->info->screenInfo.isHelp = 1;
	    }
	    return whatId;
	}
	whatId ++;
    }
    { 
	char s[100];
	
	yylineno = ref->idRef->info->screenInfo.atLine;
	sprintf(s, "%s not declared", name);
	vcerror(s);
	yylineno = storedLineNo;
    }
    return 0;
}

void tprint(tree, deep)
formelList *tree;
int deep;
{
  if (tree) {
    if (tree->left)
      tprint(tree->left, deep + 1);

    printf("%d: type = %d\n", deep, tree->node);

    if (tree->right)
      tprint(tree->right, deep + 1);
  }
}

void OLDcodeCastedArgumentList(fp, castTree, expr, anyCastList, onlyConstant)
FILE *fp;
formelList *castTree;
exprTree *expr;
int anyCastList;
int onlyConstant;
{
  if (expr->node->operator == _comma) /* fix lower level */
  {
    if (expr->left) 
      codeCastedArgumentList(fp, (castTree) ? castTree->left : 0,
                               expr->left, anyCastList, onlyConstant);
    else if (castTree && castTree->left)
      vcwarning("too few parameters in prototype list");

    emitCode2(fp, ", ");
    if (expr->right)
      codeCastedArgumentList(fp, (castTree) ? castTree->right : 0,
                               expr->right, anyCastList, onlyConstant);
    else if (castTree && castTree->right)
      vcwarning("too few parameters in prototype list");
  } else 
  {
  
    if (castTree) {
      if (!castTree->node)
/*        vcwarning("prototype parameters does not match in numbers"); */
	vcwarning("too many parameters in prototype list");
      else
      {
#define BUGFIX_931123
#ifdef BUGFIX_931123
	switch (castTree->node) {
	  case _intRef:
/*
	if (expr->node->operator != _ident) {
	  vcerror("formel parameter is int&, actual must be a variable!");
	}
*/
	    if (expr->node->operandRef->declSpec.type != _int) {
		char buf[128];
		sprintf(buf, "parameter '%s' must be of type int (%s)",
			expr->node->operandRef->IDENT,
			castTree->name);
		vcerror(buf);
	    }
	    break;
	  case _floatRef:
	    if (expr->node->operandRef->declSpec.type != _float) {
		char buf[128];
		sprintf(buf, "parameter '%s' must be of type float (%s)",
			expr->node->operandRef->IDENT,
			castTree->name);
		vcerror(buf);
	    }
	    break;
	}
#endif
	switch (castTree->node) {
	  case _charRef:
	  case _intRef:
	  case _floatRef:
/*          case _calendarRef:    */
	    emitCode2(fp, "&");
            break;
        }
	emitCode2(fp, "(");
        emitType(fp, castTree->node);
        switch (castTree->node) {
          case _charVec:
          case _intVec:
          case _floatVec:
            emitCode2(fp, "*");
            break;
        }
        emitCode2(fp, ") ");
      }
    }      
    codeExpression(fp, expr, onlyConstant, 0);
  }
}

void castThisArgument(fp, castTree, expr, anyCastList, onlyConstant)
FILE *fp;
formelList *castTree;
exprTree *expr;
int anyCastList;
int onlyConstant;
{
#if 0
  if (expr->node->operator == _comma) /* fix lower level */
  {
    if (expr->left)
      codeCastedArgumentList(fp, (castTree) ? castTree->left : 0,
			       expr->left, anyCastList, onlyConstant);
    else if (castTree && castTree->left)
      vcwarning("too few parameters in prototype list");

    emitCode2(fp, ", ");
    if (expr->right)
      codeCastedArgumentList(fp, (castTree) ? castTree->right : 0,
			       expr->right, anyCastList, onlyConstant);
    else if (castTree && castTree->right)
      vcwarning("too few parameters in prototype list");
  } else
#endif
  {

    if (castTree) {
      if (!castTree->node)
	vcwarning("too many parameters in prototype list");
      else
      {
#define BUGFIX_931123
#ifdef BUGFIX_931123
	switch (castTree->node) {
	  case _intRef:
	    if (expr->node->operandRef->declSpec.type != _int) {
		char buf[128];
		sprintf(buf, "parameter '%s' must be of type int (%s)",
			expr->node->operandRef->IDENT,
			castTree->name);
		vcerror(buf);
	    }
	    break;
	  case _floatRef:
	    if (expr->node->operandRef->declSpec.type != _float) {
		char buf[128];
		sprintf(buf, "parameter '%s' must be of type float (%s)",
			expr->node->operandRef->IDENT,
			castTree->name);
		vcerror(buf);
	    }
	    break;
	}
#endif
	switch (castTree->node) {
	  case _charRef:
	  case _intRef:
	  case _floatRef:
/*          case _calendarRef:    */
	    emitCode2(fp, "&");
	    break;
	}
	emitCode2(fp, "(");
	emitType(fp, castTree->node);
	switch (castTree->node) {
	  case _charVec:
	  case _intVec:
	  case _floatVec:
	    emitCode2(fp, "*");
	    break;
	}
	emitCode2(fp, ") ");
      }
    }
    codeExpression(fp, expr, onlyConstant, 0);
  }
}

void codeCastedArgumentList(fp, castTree, expr, anyCastList, onlyConstant)
FILE *fp;
formelList *castTree;
exprTree *expr;
int anyCastList;
int onlyConstant;
{
  struct _links {
	struct _links *next;
	formelList *castTree;
	exprTree *expr;
  } *linkedList = NULL, *link, *link2, *linkedList2;

#if 0
  while (expr) {
    link = (struct _links *) malloc(sizeof(struct _links));
    link->next = linkedList;
    link->castTree = castTree ?
	((castTree->right) ? castTree->right : castTree) : 0;
    link->expr = (expr->right) ? expr->right : expr;
    linkedList = link;
    expr = expr->left;
    if (castTree)
      castTree = castTree->left;
  }
#else

#if 0
  while (expr) {
    link = (struct _links *) malloc(sizeof(struct _links));
    link->next = linkedList;
    link->castTree = 0;
    link->expr = (expr->right) ? expr->right : expr;
    linkedList = link;
    expr = expr->left;
  }
#else
  while (expr) {
    link = (struct _links *) malloc(sizeof(struct _links));
    link->next = linkedList;
    link->castTree = 0;

    if (expr->node->operator == _comma) {
      link->expr = expr->right;
      expr = expr->left;
    } else {
      link->expr = expr;
      expr = 0;
    }

//    link->expr = (expr->right) ? expr->right : expr;
//    expr = expr->left;

    linkedList = link;
  }
#endif
  linkedList2 = NULL;
  while (castTree) {
    link = (struct _links *) malloc(sizeof(struct _links));
    link->next = linkedList2;
    link->castTree = castTree ?
	((castTree->right) ? castTree->right : castTree) : 0;
    linkedList2 = link;
    if (castTree)
      castTree = castTree->left;
  }

  link = linkedList;
  while (linkedList2) {
    link2 = linkedList2;
    if (link) {
      link->castTree = link2->castTree;
      link = link->next;
    }
    linkedList2 = linkedList2->next;
    free(link2);
  }
#endif
  if (castTree && castTree->left)
    vcwarning("too few parameters in prototype list");

  while (linkedList) {
    castTree = linkedList->castTree;
    expr = linkedList->expr;

    castThisArgument(fp, castTree, expr, anyCastList, onlyConstant);

    link = linkedList;
    linkedList = linkedList->next;
    if (linkedList)
      emitCode2(fp, ", ");

    free(link);
  }
}

void emitFunctionFkn(dref)
idIdent *dref;
{
  FILE *fp;
  int itm;

  fp = fpCurrent;
  itm = 0;
  emitType(fp, dref->declSpec.type);
  emitCode2(fp, " ");
  emitCode2(fp, dref->IDENT);
  emitCode2(fp, "(");
  if (dref->info->funcInfo.formelParameters)
    emitActualList(fp, dref->info->funcInfo.formelParameters, &itm);
  emitCode2(fp, ")\n");
}
  	    
void emitActualList(fp, f, itm)
FILE *fp;
formelList *f;
int *itm;
{
  if (f->left)
    emitActualList(fp, f->left, itm);
  if (f->node) {
    if (*itm)
      emitCode2(fp, ", ");
    emitCode2(fp, f->name);
    (*itm)++;
  }
  if (f->right)
    emitActualList(fp, f->right, itm);
}

formelList *cFormelList(node, left, right, name)
idType node;
formelList *left, *right;
char *name;
{
  formelList *T;
  
  T = (formelList *) malloc(sizeof(formelList));
  T->node = node;
  if (name) {
    strcpy(T->name, name);
  }
  T->left = left;
  T->right = right;
 
  return T;
}

    
exprTree *cExprTree(node, left, right)
exprNode *node;
exprTree *left, *right;
{
    exprTree *T;
    T = (exprTree *) malloc(sizeof(exprTree));
    T->node = node;
    T->left = left;
    T->right = right;
    return(T);
}

exprNode *cExprNode(operator, ref, opConst, opSize)
opCode operator;
idIdent *ref;
char *opConst;
int opSize;
{
    exprNode *node;

    node = (exprNode *) malloc(sizeof(exprNode));
    node->operator = operator;
    node->operandRef = ref;
    if (opConst) {
      if (opSize) 
        memcpy(&node->operandConst, opConst, opSize);
      else
        node->operandConst.voidStorage = (void *) opConst;
    }
    return(node);
}

int precedence(op1, op2)	/* < 0 if op1 == * and op2 == + */
opCode op1, op2;
{
    return priority(op2) - priority(op1);
}

int priority(operator)		/*    * has higher priority than + */
opCode operator;
{
    switch (operator) {
	case _int_const:	return 20;
	case _float_const:	return 20;
	case _char_const:	return 20;
	case _string_const:	return 20;
	case _ident:		return 20;
	case _screenIdent:	return 20;
	case _func:		return 20;
	case _vector:		return 20;
	case _plusplus:	return 14;
	case _minusminus:	return 14;
	case _not:	return 14;
	case _cmpl:     return 14;
	case _mul:	return 13;
	case _div:	return 13;
	case _mod:	return 13;
	case _plus:	return 12;
	case _minus:	return 12;
	case _sr:
	case _sl:       return 11;
	case _lt: 	return 10;
	case _gt: 	return 10;
	case _lte: 	return 10;
	case _gte: 	return 10;
	case _equal:	return 9;
	case _notequal: return 9;
	case _bitXor:   return 8;
        case _bitAnd:   return 7;
        case _bitOr:    return 6;
	case _and:	return 5;
	case _or:	return 4;
	case _if:       return 3;
	case _assign:	return 2;
	case _assignMul:
	case _assignDiv:
	case _assignMod:
	case _assignSr:
	case _assignSl:
	case _assignAdd:
	case _assignSub:
	case _assignAnd:
	case _assignXor:
	case _assignOr:
	  return 2;

	case _recList:	return 2;     /* ??? 1992-09-04, 1 or 2 ? */

	case _comma:	return 1;
	default:
	    printf("program fault: at priority()\n");
	    return 0;
    }
}

int expressionContains(expr, operator)
exprTree *expr;
opCode operator;
{
  if (expr->left) {
    if (expressionContains(expr->left, operator))
      return 1;
  }
  if (expr->node) {
    if (expr->node->operator == operator)
      return 1;
  }
  if (expr->right) {
    if (expressionContains(expr->right, operator))
      return 1;
  }
  return 0;  
}

void freeExpression(expr)
exprTree *expr;
{
  exprNode *node;
  exprTree *right;
  
  if (expr->left)
    freeExpression(expr->left);
    
  node = expr->node;
  right = expr->right;
  free((void *) expr);
  if (node->operator == _string_const) {
    free((void *) node->operandConst.voidStorage);
  }
  free((void *) node);
   
  if (right)
    freeExpression(right);
}

void checkForEqualSign(expr)
exprTree *expr;
{
  if (expressionContains(expr, _assign)) {
/*    vcwarning("assignment used instead of comparision");    */
    vcwarning("possibly incorrect assignment");    
  }
}

void traverseTree_n_init(idRef, t, index, global)
idIdent *idRef;
exprTree *t;
int *index, global;
{
  if (t->left)
    if (t->left->node->operator == _comma)
      traverseTree_n_init(idRef, t->left, index, global);
    else {
      if (global) emitCode(GLOBAL_ACCESS);
      fprintf(fpCurrent, "%s[%d] = ", idRef->IDENT, *index);
      (*index) ++;
      codeExpression(fpCurrent, t->left, 1, 0);
      emitCode(";\n");
    }

  if (t->right) {
    if (global) emitCode(GLOBAL_ACCESS);
    fprintf(fpCurrent, "%s[%d] = ", idRef->IDENT, *index);
    (*index) ++;
    codeExpression(fpCurrent, t->right, 1, 0);
    emitCode(";\n");
  }
}

/*
!  931124. The old straight tree traversing method suffering from
!  the obvious stack overflow problem which occured when handling
!  arrays with ranks greater than ~ 128.
!  The approach is to transfer the recursive algorithm to a linear one.
!  The new algorithm is much nicer. Since the parser makes a left tree
!  we start with counting number of ',' och nodes to the left.
!  Then we repeat the process and emit every right node with index MAX--;
!  The last thing to do is to emit the lefter most node.
*/
void NEWtraverseTree_n_init(idRef, t, index, global)
idIdent *idRef;
exprTree *t;
int *index, global;
{
  int maxNodes = 0, count = 0;
  exprTree *temp;

  temp = t;
  if (temp->node->operator == _comma) {
    maxNodes++;
    while (temp->left->node->operator == _comma) {
      maxNodes++;
      temp = temp->left;
    }
  }
  *index = 1+maxNodes;	/* 1+number of commas */
  //  printf("%d nodes !\n", maxNodes+1);
  emitCode("{\n");
  while (t->node->operator == _comma) {
    if (t->right) {
      count ++;
      if (count > 50) {
	emitCode("}\n");
	emitCode("{\n");
	count = 0;
      }
      if (global) emitCode(GLOBAL_ACCESS);
      fprintf(fpCurrent, "%s[%d] = ", idRef->IDENT, maxNodes--);
      codeExpression(fpCurrent, t->right, 1, 0);
      emitCode(";\n");
    } else
      ; 		// printf("uh?\n");
    t = t->left;
  }
  if (t) {
    if (global) emitCode(GLOBAL_ACCESS);
    fprintf(fpCurrent, "%s[%d] = ", idRef->IDENT, maxNodes);
    codeExpression(fpCurrent, t, 1, 0);
    emitCode(";\n");
  } else
    ; // printf("left missing?\n");

  emitCode("}\n");

  if (maxNodes)
    ; // printf("Please discard your IDCC.EXE file, it has been infected by virus\n");
}

void initArray(idRef, global)
idIdent *idRef;
int global;
{
  exprTree *t;
  int index = 0;

  if (!idRef->expr)
    return ;
  if (!(t = idRef->expr->left))       /* left ??? */
    return ;
  NEWtraverseTree_n_init(idRef, t, &index, global);
  if (index > idRef->info->vecInfo.size)
    vcerror("Too many items in initializer list");
  else if (index < idRef->info->vecInfo.size)
    vcwarning("Too few items in initializer list");
}

isAssign(operator)
opCode operator;
{
  switch (operator) {
    case _assign:
    case _assignMul:
    case _assignDiv:
    case _assignMod:
    case _assignAdd:
    case _assignSub:
    case _assignSr:
    case _assignSl:
    case _assignAnd:
    case _assignXor:
    case _assignOr:
      return 1;
      break;
  }
  return 0;
}

#define ON_CASE_EMIT_OPERATOR(a, b) case a: emitCode(b); break;
void codeExpression(fp, expr, onlyConstant, lhs)
FILE *fp;
exprTree *expr;
int onlyConstant;
int *lhs;              /* 1= leftHandSide, 0 = rightHandSide expression */
{
    int suffix = 0, parentes, ext = 0, cal = 0;
    char s[80];
    int localLHS = 0;
    int emitBraces = 0;
/*
	x = 1*2+3;
	x = 1+2*3;
	x = 1*(2+3);
	x = (1+2)*3;
*/
    
    if (expr->left) {
	if (!expr->left->node)	/* unknown id */
	    return;
	if (precedence(expr->node->operator, expr->left->node->operator) < 0) {
	    parentes = 1;
	    emitCode2(fp, "(");
	} else
	    parentes = 0;
	codeExpression(fp, expr->left, onlyConstant, 
          ((localLHS = (expr->node ? 
			((isAssign(expr->node->operator)) ? 1 : 0) : 0))
              , &localLHS)    /* if localLHS = 2 when back, its update of ext */
	  );
	if (parentes)
	    emitCode2(fp, ")");
    }
    if (!expr->node)	/* unknown id */
	return ;

    switch (expr->node->operator) {
/*	ON_CASE_EMIT_OPERATOR(_equal, "=")	*/

        case _if: emitCode2(fp, " ? ");
          if (expr->right) {
  	    if (expr->right->left)
  	        codeExpression(fp, expr->right->left, onlyConstant, 0);
            emitCode2(fp, " : ");
  	    if (expr->right->right)
  	        codeExpression(fp, expr->right->right, onlyConstant, 0);
          }
	  break;
	case _equal:	emitCode2(fp, " == ");	break;
	case _comma:	emitCode2(fp, ", ");	break;
	case _recList:	emitCode2(fp, "\n{ ");    emitBraces = 1;	break;
	case _assign:	emitCode2(fp, " = ");  break;
	case _assignMul:	emitCode2(fp, " *= ");  break;
	case _assignDiv:	emitCode2(fp, " /= ");  break;
	case _assignMod:	emitCode2(fp, " %= ");  break;
	case _assignAdd:	emitCode2(fp, " += ");  break;
	case _assignSub:	emitCode2(fp, " -= ");  break;
	case _assignSr:         emitCode2(fp, " >>= ");  break;
	case _assignSl:	        emitCode2(fp, " <<= ");  break;
	case _assignAnd:	emitCode2(fp, " &= ");  break;
	case _assignXor:	emitCode2(fp, " ^= ");  break;
	case _assignOr: 	emitCode2(fp, " |= ");  break;
	case _notequal: emitCode2(fp, " != "); break;
	
        case _bitAnd:           emitCode2(fp, " & ");  break;
        case _bitXor:           emitCode2(fp, " ^ ");  break;
        case _bitOr:            emitCode2(fp, " | ");  break;
	case _cmpl:             emitCode2(fp, " ~ ");  break;
        case _sl:               emitCode2(fp, " << ");  break;
        case _sr:               emitCode2(fp, " >> ");  break;

	case _lt: 	emitCode2(fp, " < "); break;
	case _gt: 	emitCode2(fp, " > "); break;
	case _lte: 	emitCode2(fp, " <= "); break;
	case _gte: 	emitCode2(fp, " >= "); break;
	case _plusplus:	emitCode2(fp, "++"); break;
	case _minusminus:	emitCode2(fp, "--"); break;
	case _plus:	emitCode2(fp, " + "); break;
	case _minus:	emitCode2(fp, " - "); break;
	case _mul:	emitCode2(fp, " * "); break;
	case _div:	emitCode2(fp, " / "); break;
	case _mod:	emitCode2(fp, " % "); break;
	case _or:	emitCode2(fp, " || "); break;
	case _and:	emitCode2(fp, " && "); break;
	case _not:	emitCode2(fp, "!"); break;
	case _int_const:
#ifdef OSK	
	    sprintf(s, "%d", expr->node->operandConst.longStorage);
#else
	    sprintf(s, "%ld", expr->node->operandConst.longStorage);
#endif	    
	    emitCode2(fp, s);
	    break;
	case _float_const:    /* changed %f -> %g 910805 (pcb native cannot handle !!!) */
          {
            long tmp;
            double ftmp;
            tmp = (long) expr->node->operandConst.doubleStorage;
            ftmp = tmp;
            if (ftmp == expr->node->operandConst.doubleStorage)
              sprintf(s, "%1.1f", expr->node->operandConst.doubleStorage);
            else
	      sprintf(s, "%g", expr->node->operandConst.doubleStorage);
	    emitCode2(fp, s);
	  }
	    break;
	case _char_const:
	    sprintf(s, "%c", expr->node->operandConst.byteStorage);
	    emitCode2(fp, s);
	    break;
	case _string_const:
	    sprintf(s, "\"%s\"", (char *) expr->node->operandConst.voidStorage);
	    emitCode2(fp, s);
	    break;
	case _screenIdent:                            /* 1992-09-04 */
	    emitCode2(fp, expr->node->operandRef->IDENT); 
	    break;
	case _ident:
	    if (onlyConstant &&
	          ((expr->node->operandRef->declSpec.type != _intConst) &&
		    (expr->node->operandRef->declSpec.type != _floatConst) &&
		    (expr->node->operandRef->declSpec.type != _charConst)))
              vcerror("Only constant operands allowed for 'CONST'");

	    if (((expr->node->operandRef->declSpec.storageSpec == _global) ||
	             (expr->node->operandRef->declSpec.storageSpec == _extern))
		&& (expr->node->operandRef->declSpec.type != _intConst) &&
		    (expr->node->operandRef->declSpec.type != _floatConst) &&
		    (expr->node->operandRef->declSpec.type != _charConst) &&
            /* test 921015, remove this and functions will use dm-> */
		    
		   (optionGlobalFunctions || 
		    (
		     (expr->node->operandRef->declSpec.type != _intFunc) &&
		     (expr->node->operandRef->declSpec.type != _floatFunc) &&
		     (expr->node->operandRef->declSpec.type != _charFunc)
		    )
		   )
	        )
            {
	      if (expr->node->operandRef->declSpec.storageSpec == _extern) {
                if (lhs && *lhs == 1) {    /* update of extern variable */
                  *lhs = 2; 
                  emitCode2(fp, "(");
                } else if (!lhs || *lhs == 0) {/*(getExtern(z_OFFSET), z)*/
#ifdef OBSOLETE
  	          ext = 1;
		  emitCode2(fp, "(getExtern(");
	          emitCode2(fp, expr->node->operandRef->IDENT);
                  emitCode2(fp, OFFSET_SUFFIX);
#ifdef OSK
                  fprintf(fp, ", %d), ", 
                        expr->node->operandRef->declSpec.nodeNo); 
#else
                  fprintf(fp, ", %ld), ", 
                        expr->node->operandRef->declSpec.nodeNo); 
#endif
#endif
                } /* if *lhs == 2 just use local copy in data module */
	      }
              if (expr->node->operandRef->declSpec.type == _calendar)
              {
#define ENTER_CALENDAR
#ifdef ENTER_CALENDAR
                emitCode2(fp, "&");
#else
                emitCode2(fp, "checkCalendarLevel(&");
		cal = 1;
#endif
              }
	      emitCode2(fp, GLOBAL_ACCESS);
            }

            if (notInitialized) {            /* added 920521 */
  	      if (expr->node->operandRef->declSpec.storageSpec 
	                                                    == _local) {
                if (lhs && *lhs == 1) {    /* update of local variable */
                   expr->node->operandRef->initialized = 1;
                } else if ((expr->node->operandRef->initialized == 0) &&
                           (expr->node->operandRef->expr == 0)) {
                  char s[100];
                  sprintf(s, "Local variable '%s' maybe not initialized yet",
                                  expr->node->operandRef->IDENT);
                  vcwarning(s);
                }
              }
            }

            switch (expr->node->operandRef->declSpec.type) {  /* added 921013 */
              case _intRef:
              case _floatRef:
              case _charRef:
                emitCode2(fp, "(*");
                break;
    	    }
	    emitCode2(fp, expr->node->operandRef->IDENT); 	/* constants also !! (defined)*/
            switch (expr->node->operandRef->declSpec.type) {  /* added 921013 */
              case _intRef:
              case _floatRef:
              case _charRef:
                emitCode2(fp, ")");
                break;
    	    }
	   
	    if (ext == 1)
	      emitCode2(fp, ")");
	    if (cal == 1)
	      emitCode2(fp, ")");
	    break;
	case _func:
	    emitCode2(fp, "("); 
/*	    suffix = 1;
*/
/*  --- added ansi-C prototype conventions (only types) */

    {
      idInfo *tmp;
      formelList *t;
      
      if (tmp = expr->left->node->operandRef->info) 
        t = tmp->funcInfo.formelParameters;
      else
        t = 0;

/*  --- */
        if (expr->right) {
          expr = expr->right;
	  codeCastedArgumentList(fp, t, expr, t != 0, onlyConstant);
        }

/*
	    while (expr->right && expr->right->node->operator == _comma) {
	      expr = expr->right;
              if (t) {
                emitCode2(fp, "(");
                if (t->left) 
                  emitType(fp, t->left->node);
                else
                  emitType(fp, t->node);
                emitCode2(fp, ")");
                t = t->right;  
              }
	      if (expr->left)
  	        codeExpression(fp, expr->left, onlyConstant, 0);
	      emitCode2(fp, ", ");
	    }
            if (expr->right) {
	      codeExpression(fp, expr->right, onlyConstant, 0);
            }
*/


    }	    
	    emitCode2(fp, ")"); 
	    return;
	   
/*	    break;        */
	case _vector:
	    emitCode2(fp, "[");
	    suffix = 2;
	    break;
    }
    
/* precedence(op1, op2)	< 0 if op1 == * and op2 == + */

    if (expr->right) {
	if (!expr->right->node)	/* unknown id */
	    return;

        if ((expr->node->operator != _recList) &&   /* added 1992-09-04 */
           (precedence(expr->node->operator, expr->right->node->operator) <= 0))
        {
	    parentes = 1;
	    emitCode2(fp, "(");
	} else
	    parentes = 0;
	codeExpression(fp, expr->right, onlyConstant, 0);
	if (parentes)
	    emitCode2(fp, ")");

	if (emitBraces) {         /* added 1992-09-04 */
	  emitCode2(fp, " }");
	  emitBraces = 0;
	}
	
    }
    if (suffix == 1)
	emitCode2(fp, ")");
    else if (suffix == 2)
	emitCode2(fp, "]");
	
    if (localLHS == 2) {      /* may get 000:102 if none lhs exp at left hand!*/

      emitCode2(fp, ", putExtern(");
      emitCode2(fp, expr->left->node->operandRef->IDENT);
      emitCode2(fp, OFFSET_SUFFIX);
#ifdef OSK
      fprintf(fp, ", %d), ",
                expr->left->node->operandRef->declSpec.nodeNo); 
#else
      fprintf(fp, ", %ld), ",
                expr->left->node->operandRef->declSpec.nodeNo); 
#endif

	codeExpression(fp, expr->left, onlyConstant, &localLHS);
/*
          ((localLHS = (expr->node ? 
			((expr->node->operator == _assign) ? 1 : 0) : 0))
              , &localLHS)
	  );
*/
      emitCode2(fp, ")");
    }
}

void makeConstDefinitions()
{
    symLink *tmpLink, *tmp;

    tmp = symbolTable->thisScope;
    while (tmp && (tmp = (tmpLink = tmp)->next, tmpLink)) {
	if ((tmpLink->idRef->declSpec.storageSpec != _global) &&
	    (tmpLink->idRef->declSpec.storageSpec != _extern))
	    continue;
	switch (tmpLink->idRef->declSpec.type) {
	  case _intConst:
	  case _floatConst:
	  case _charConst:
	    fprintf(fpSymTable, "#define %s ", tmpLink->idRef->IDENT);
	    
	    if (tmpLink->idRef->declSpec.type == _intConst)
	      fprintf(fpSymTable, "(%s) ", _TYPE_OF_INT);
	    else if (tmpLink->idRef->declSpec.type == _floatConst)
	      fprintf(fpSymTable, "(%s) ", _TYPE_OF_FLOAT);
	    else if (tmpLink->idRef->declSpec.type == _charConst)
	      fprintf(fpSymTable, "(%s) ", _TYPE_OF_CHAR);
	  
            codeExpression(fpSymTable, 
                    (exprTree *) tmpLink->idRef->info->constInfo.value, 1, 0);
            fprintf(fpSymTable, "\n");
	    break;
	}
    }
}

unsigned short swapword(w)
unsigned short w;
{
  return ((w & 0xff) << 8) | ((w >> 8) & 0xff);
}

unsigned long swaplong(l)
unsigned long l;
{
  unsigned short w1, w2;
  w1 = (l >> 16) & 0xffff;
  w2 = l & 0xffff;
  return (((unsigned long) swapword(w2)) << 16) | swapword(w1);
}

/*
!     emits a meta module which holds description of the data module 
!     (names, size and types)
*/
void emitMetaModule()
{
  symLink *tmpLink, *tmp;
  int offset, size, index;
  int totsize, i, currentNameOffset, id, odd, hilo;
  FILE *fp;
   
  struct {
    unsigned short sync;
    unsigned short sysrev;
    unsigned long  size;
    unsigned long  owner;
    unsigned long  nameoffset;
    unsigned short access;
    unsigned short typelang;
    unsigned short attrrev;
    unsigned short edition;
    unsigned long  usage;
    unsigned long  symbol;
    unsigned short ident;
    char  spare[12];
    unsigned short parity;
    
    unsigned long  dataptr;
    char  name[12];
  } buff;
  
  struct {
    unsigned short nameOffset,
          size,
          offset,
          lockOffset,
          type;
  } metaEntry;
    
  id = 0;
  currentNameOffset = 0;
  totsize = 0;
  tmp = symbolTable->thisScope;
  while (tmp && (tmp = (tmpLink = tmp)->next, tmpLink)) {
    if ((tmpLink->idRef->declSpec.storageSpec != _global) &&
	    (tmpLink->idRef->declSpec.storageSpec != _extern))
	    continue;
      switch (tmpLink->idRef->declSpec.type) {
          case _intFunc:
          case _floatFunc:
          case _intVec:
          case _floatVec:
          case _int:
          case _float:
          case _calendar:
            if (tmpLink->idRef->declSpec.alias) {
              totsize += strlen(tmpLink->idRef->declSpec.alias) + 1;
            }
            currentNameOffset += 10;
	    totsize += 10 + strlen(tmpLink->idRef->IDENT) + 1;
	    id ++;
	    break;
      }
  }
  totsize += 10;        /* add an extra entry for total size */
  currentNameOffset += 10;
/*
!     create meta module file,
!     emit header
*/
    /* fopen file */

#ifdef OSK
  if ((fp = fopen("metaMod", "w")) == NULL) {
#else
  if ((fp = fopen("metaMod", "wb")) == NULL) {
#endif
    printf("cannot create file metaMod\n");
    printf("error %d\n", errno);
#ifndef OSK
    perror("unable to create file metaMod");
    printf("doserror %d\n", _doserrno);
#endif    
    exit(errno);
  }   

  buff.sync = 0x4afc;
  if (*((char *) &buff.sync) == 0x4a)
    hilo = 1;                             /* like MC68K */
  else
    hilo = 0;                             /* maybe intel */
  buff.sysrev = 1;
  if (totsize & 1) {      /* add pad */
    odd = 1;
    totsize ++;
  } else
    odd = 0;
  buff.size = sizeof(buff) + totsize + 4;                 /* 4 = size of crc */
  buff.owner = 0;
  buff.nameoffset = (int) ( buff.name  - ((int) &buff));
  buff.access = 0x0111;
  buff.typelang = 0x0400;
  buff.attrrev =  0x8000 + MajorVersion;
  buff.edition =  MinorVersion;
  buff.dataptr = sizeof(buff);
  strcpy(buff.name, "METAVAR");
  buff.name[8] = buff.name[9] = buff.name[10] = buff.name[11] = 0;
  buff.usage = 0;
  buff.symbol = 0;
  buff.ident = 0;
  buff.parity = 0;
  *((long *) &buff.spare[0]) = 0;
  *((long *) &buff.spare[4]) = 0;
  *((long *) &buff.spare[8]) = 0;

  if (!hilo) {
    buff.sync   = swapword(buff.sync);
    buff.sysrev = swapword(buff.sysrev);  
    buff.size   = swaplong(buff.size);
    buff.owner  = swaplong(buff.owner);
    buff.nameoffset = swaplong(buff.nameoffset);
    buff.access = swapword(buff.access);
    buff.typelang = swapword(buff.typelang);
    buff.attrrev = swapword(buff.attrrev);
    buff.edition = swapword(buff.edition);
    buff.usage   = swaplong(buff.usage);
    buff.symbol  = swaplong(buff.symbol);
    buff.ident   = swapword(buff.ident);
    buff.parity  = swapword(buff.parity);
    buff.dataptr = swaplong(buff.dataptr);
  }
  if (fwrite(&buff, sizeof(buff), 1, fp) != 1)
    printf("cannot write datamodule");
  

/*
!     emit contents of the meta data module
*/
  metaEntry.nameOffset = id;
  if (!hilo) {
    metaEntry.nameOffset = swapword(metaEntry.nameOffset);
  }
  metaEntry.size = 0;
  metaEntry.offset = 0;
  metaEntry.lockOffset = 0;
  metaEntry.type = 0;

  if (fwrite(&metaEntry, sizeof(metaEntry), 1, fp) != 1)
    printf("cannot write datamodule");

  tmp = symbolTable->thisScope;
  offset = 0;
  while (tmp && (tmp = (tmpLink = tmp)->next, tmpLink)) {
    if ((tmpLink->idRef->declSpec.storageSpec != _global) &&
	    (tmpLink->idRef->declSpec.storageSpec != _extern))
      continue;
    if (tmpLink->idRef->declSpec.storageSpec == _extern) {
      offset += sizeof(struct _remote);
    }
    size = 0;
    switch (tmpLink->idRef->declSpec.type) {
	  case _intVec:
	    size = _SIZE_OF_INT *  tmpLink->idRef->info->vecInfo.size;
	    break;
	  case _floatVec:
	    size = _SIZE_OF_FLOAT *  tmpLink->idRef->info->vecInfo.size;
	    break;
	  case _charVec:
	    size = _SIZE_OF_CHAR *  tmpLink->idRef->info->vecInfo.size;
	    break;
	  case _intFunc:
	  case _floatFunc:            /* new 910719 */
	    size = _SIZE_OF_PTR;
	    break;
	  case _int:
	    size = _SIZE_OF_INT;
	    break;
	  case _float:
	    size = _SIZE_OF_FLOAT;
	    break;
	  case _char:
	    size = _SIZE_OF_CHAR;
	    break;
          case _calendar:
	    size = _SIZE_OF_CALENDAR;       /* new 910719 */
	                /* 16 + tmpLink->idRef->info->calendarInfo.size;  */
	    break;
    }
    if (size == 0)
      continue;
    metaEntry.nameOffset = currentNameOffset;
    metaEntry.size = size;
    metaEntry.offset = offset;
    metaEntry.lockOffset = (int) tmpLink->idRef->lockOffset;
/*
    metaEntry.type = tmpLink->idRef->declSpec.type |
               ((tmpLink->idRef->declSpec.storageSpec == _extern) ? 
                  _REMOTE_MASK : 0);
*/
    metaEntry.type = tmpLink->idRef->declSpec.type |
               ((tmpLink->idRef->declSpec.storageSpec == _extern) ? 
                  _REMOTE_MASK : 0) 
                  | (( tmpLink->idRef->declSpec.nodeNo & 0xff ) << 8) ;

    if (tmpLink->idRef->declSpec.alias) {
      metaEntry.type |= _ALIAS_MASK;
    }
                 
    if (!hilo) {
      metaEntry.nameOffset = swapword(metaEntry.nameOffset);
      metaEntry.size       = swapword(metaEntry.size);
      metaEntry.offset     = swapword(metaEntry.offset);
      metaEntry.lockOffset = swapword(metaEntry.lockOffset);
      metaEntry.type       = swapword(metaEntry.type);
    }
    if (fwrite(&metaEntry, sizeof(metaEntry), 1, fp) != 1)
      printf("cannot write datamodule");
    offset += size;
    currentNameOffset += strlen(tmpLink->idRef->IDENT) + 1;
    if (tmpLink->idRef->declSpec.alias) {
      currentNameOffset += strlen(tmpLink->idRef->declSpec.alias) + 1;
    }
  }
/*
!     Emit names of variables
*/  
  tmp = symbolTable->thisScope;
  while (tmp && (tmp = (tmpLink = tmp)->next, tmpLink)) {
    if ((tmpLink->idRef->declSpec.storageSpec != _global) &&
	    (tmpLink->idRef->declSpec.storageSpec != _extern))
	    continue;
      switch (tmpLink->idRef->declSpec.type) {
          case _char:
          case _charVec:
          
	  case _intVec:
	  case _floatVec:
	  case _intFunc:
	  case _floatFunc:
	  case _int:
	  case _float:
          case _calendar:
        if (fwrite(tmpLink->idRef->IDENT, strlen(tmpLink->idRef->IDENT) + 1, 
                                            1, fp) != 1)
          printf("cannot write datamodule");
        if (tmpLink->idRef->declSpec.alias) {
          if (fwrite(tmpLink->idRef->declSpec.alias, 
                    strlen(tmpLink->idRef->declSpec.alias) + 1, 
                                            1, fp) != 1)
            printf("cannot write datamodule");
        }
      }
  }
  if (odd) {
    char pad = 0;
    if (fwrite(&pad, sizeof(char), 1, fp) != 1)
      printf("cannot write datamodule");
  }
/*
!   emit CRC end,
!   close meta module file
*/   
  {
    long crc;
/*                              for future use
    if (!hilo) 
      crc = swaplong(crc);
*/      
    if (fwrite(&crc, sizeof(long), 1, fp) != 1)
      printf("cannot write datamodule");
  }
  fclose(fp);
}
/*
!	emits the symboltable definitions to file fpSymTable, 
!	i.e. the structure of the data module
*/
void makeGlobalSymTable()
{
    symLink *tmpLink, *tmp;
    int offset, size, index;
    int i, remVar;

    makeConstDefinitions();
    fprintf(fpSymTable, "#define NO_OF_CAL_ENTRIES %d\n", NO_OF_CAL_ENTRIES);
    fprintf(fpSymTable, "typedef struct {\n"); 
    fprintf(fpSymTable, "  unsigned short day[NO_OF_CAL_ENTRIES];\n");
    fprintf(fpSymTable, "  unsigned short stopday[NO_OF_CAL_ENTRIES];\n");
    fprintf(fpSymTable, "  unsigned char  color[NO_OF_CAL_ENTRIES];\n");
    fprintf(fpSymTable, "  unsigned short start[NO_OF_CAL_ENTRIES];\n");
    fprintf(fpSymTable, "  unsigned short stop[NO_OF_CAL_ENTRIES];\n");
    fprintf(fpSymTable, "} CALENDAR;\n");

    fprintf(fpSymTable, "struct _remote {\n"); 
    fprintf(fpSymTable, "  long timeStamp;\n"); 
    fprintf(fpSymTable, "};\n");
  
    fprintf(fpSymTable, "struct _datamodule {\n");
    tmp = symbolTable->thisScope;
    offset = 0;
    remVar = 1;
    while (tmp && (tmp = (tmpLink = tmp)->next, tmpLink)) {
	if ((tmpLink->idRef->declSpec.storageSpec != _global) &&
	    (tmpLink->idRef->declSpec.storageSpec != _extern))
	    continue;
        if (tmpLink->idRef->declSpec.storageSpec == _extern) {
          fprintf(fpSymTable, "    struct _remote _remoterVar_%03d;\n", remVar++);
          size = sizeof(long);
          offset += size;
        }
	switch (tmpLink->idRef->declSpec.type) {
	  case _charVec:
	    fprintf(fpSymTable, "    %s    %s[%d];\n", _TYPE_OF_CHAR,
                                   tmpLink->idRef->IDENT,
                                   tmpLink->idRef->info->vecInfo.size);
	    size = _SIZE_OF_CHAR * tmpLink->idRef->info->vecInfo.size;
	    offset += size;
	    break;
	  case _intVec:
	    fprintf(fpSymTable, "    %s    %s[%d];\n", _TYPE_OF_INT,
                                   tmpLink->idRef->IDENT,
                                   tmpLink->idRef->info->vecInfo.size);
	    size = _SIZE_OF_INT *  tmpLink->idRef->info->vecInfo.size;
	    offset += size;
	    break;
	  case _floatVec:
	    fprintf(fpSymTable, "    %s    %s[%d];\n", _TYPE_OF_FLOAT,
                                   tmpLink->idRef->IDENT,
                                   tmpLink->idRef->info->vecInfo.size);
	    size = _SIZE_OF_FLOAT *  tmpLink->idRef->info->vecInfo.size;
	    offset += size;
	    break;
	  case _intFunc:
	    fprintf(fpSymTable, "    %s    (*%s)();\n", _TYPE_OF_INT, 
	                                             tmpLink->idRef->IDENT);
            size = _SIZE_OF_PTR;
	    offset += size;
            break;
	  case _floatFunc:
	    fprintf(fpSymTable, "    %s    (*%s)();\n", _TYPE_OF_FLOAT,
	                                             tmpLink->idRef->IDENT);
            size = _SIZE_OF_PTR;
	    offset += size;
            break;
	  case _int:
	    fprintf(fpSymTable, "    %s    %s;\n", _TYPE_OF_INT,
	                                             tmpLink->idRef->IDENT);
	    size = _SIZE_OF_INT;
	    offset += size;
	    break;
	  case _float:
	    fprintf(fpSymTable, "    %s %s;\n", _TYPE_OF_FLOAT,
	                                             tmpLink->idRef->IDENT);
	    size = _SIZE_OF_FLOAT;
	    offset += size;
            break;
          case _calendar:
	    fprintf(fpSymTable, "    %s %s;\n", _TYPE_OF_CALENDAR,
	                                             tmpLink->idRef->IDENT);
	    size = _SIZE_OF_CALENDAR;
	    offset += size;
            break;
          case _char:
	    fprintf(fpSymTable, "    %s %s;\n", _TYPE_OF_CHAR,
	                                             tmpLink->idRef->IDENT);
	    size = _SIZE_OF_CHAR;
	    offset += size;
            vcwarning("gee, this part not implemented yet (char in global scope)");
            break;
/*
            vcerror("gee, this part not implemented yet (char in global scope)");
*/
	}
    }
/*
!	and now the lock variables
*/
    tmp = symbolTable->thisScope;
    while (tmp && (tmp = (tmpLink = tmp)->next, tmpLink)) {
	if ((tmpLink->idRef->declSpec.storageSpec != _global) &&
	    (tmpLink->idRef->declSpec.storageSpec != _extern))
	    continue;
	switch (tmpLink->idRef->declSpec.type) {
	  case _intVec:
	  case _floatVec:
	  case _intFunc:
	  case _floatFunc:
	  case _int:
	  case _float:
	  case _calendar:
	    fprintf(fpSymTable, "    char  %s%s;\n", 
				tmpLink->idRef->IDENT, GLOBAL_LOCK_SUFFIX);
	    tmpLink->idRef->lockOffset = offset;
	    size = _SIZE_OF_CHAR;
	    offset += size;
	    break;
	}
    }
/*    
    for (i = 0; i < timerCounter; i++) {
      declType currentDeclSpec;
      idInfo currentIdInfo;
      fprintf(fpSymTable, "    struct _glitch _timer_%02d;\n", i + 1);
*/      
     
    
/*
!      offset += sizeof(struct _glitch); 
*/

/*       
      declareId(cid, currentDeclSpec, &currentIdInfo, 0);

      currentIdInfo.moduleInfo.moduleTypeRef = cref;

      currentDeclSpec.storageSpec = _global;
      currentDeclSpec.type = _timer;
      sprintf(cid, "_timer_%02d.abs", i + 1);
      declareId(cid, currentDeclSpec, &currentIdInfo, 0);

*/

/*    }
*/

    fprintf(fpSymTable, "} *%s;", GLOBAL_ACCESS_NAME);
    fprintf(fpSymTable, "    /* total size of struct = %d */\n\n", offset);
    sizeOfModule_VARS = sizeOfModule_VAR2 = offset;

    emitMetaModule();       /* added 1991-02-01, for version IDC 1.0+ */

/*
!   emit temporary storage initializers for calendars 
*/
    tmp = symbolTable->thisScope;
    while (tmp && (tmp = (tmpLink = tmp)->next, tmpLink)) {
	if ((tmpLink->idRef->declSpec.storageSpec != _global) &&
	    (tmpLink->idRef->declSpec.storageSpec != _extern))
	    continue;
	switch (tmpLink->idRef->declSpec.type) {
	  case _calendar:
	    if (tmpLink->idRef->declSpec.storageSpec == _extern) {
	      vcerror("external calendar not implemented yet");
	      continue;   /* skip extern for now ??? */
	    }
            fprintf(fpSymTable, "CALENDAR _tmp_%s = {\n",tmpLink->idRef->IDENT);
           
            if (tmpLink->idRef->expr) {
              emitCalendarDef(tmpLink->idRef->expr);
            } else { 
              for (i = 0; i < NO_OF_CAL_ENTRIES; i++)
                fprintf(fpSymTable, "%d, ", 0);
              fprintf(fpSymTable, "\n");
              for (i = 0; i < NO_OF_CAL_ENTRIES; i++)
                fprintf(fpSymTable, "%d, ", 0);
              fprintf(fpSymTable, "\n");
              for (i = 0; i < NO_OF_CAL_ENTRIES; i++)
                fprintf(fpSymTable, "%d, ", 0);
              fprintf(fpSymTable, "\n");
              for (i = 0; i < NO_OF_CAL_ENTRIES; i++)
                fprintf(fpSymTable, "%d, ", 0);
              fprintf(fpSymTable, "\n");
              for (i = 0; i < NO_OF_CAL_ENTRIES - 1; i++)
                fprintf(fpSymTable, "%d, ", 0);
              fprintf(fpSymTable, "%d", 0);
              fprintf(fpSymTable, "\n};\n");
            }
	}
    }

    tmp = symbolTable->thisScope;
    offset = 0; index = 1;
    while (tmp && (tmp = (tmpLink = tmp)->next, tmpLink)) {
	if ((tmpLink->idRef->declSpec.storageSpec != _global) &&
	    (tmpLink->idRef->declSpec.storageSpec != _extern))
	    continue;
	switch (tmpLink->idRef->declSpec.type) {
	  case _intVec:
	    size = _SIZE_OF_INT *  tmpLink->idRef->info->vecInfo.size;
   	    if (tmpLink->idRef->declSpec.storageSpec == _extern) {
	      fprintf(fpSymTable,
	      "#define %s%s %d\n", tmpLink->idRef->IDENT, OFFSET_SUFFIX,
	              index /* offset */);
	    }
	    offset += size; index ++;
	    break;
	  case _floatVec:
	    size = _SIZE_OF_FLOAT *  tmpLink->idRef->info->vecInfo.size;
   	    if (tmpLink->idRef->declSpec.storageSpec == _extern) {
	      fprintf(fpSymTable,
	      "#define %s%s %d\n", tmpLink->idRef->IDENT, OFFSET_SUFFIX,
	              index /* offset */);
	    }
	    offset += size; index ++;
	    break;
	  case _int:
	    size = _SIZE_OF_INT;
	    if (tmpLink->idRef->declSpec.storageSpec == _extern) {
	      fprintf(fpSymTable,
	      "#define %s%s %d\n", tmpLink->idRef->IDENT, OFFSET_SUFFIX,
	              index /* offset */);
	    }
	    offset += size; index ++;
	    break;
	  case _float:
	    size = _SIZE_OF_FLOAT;
	    if (tmpLink->idRef->declSpec.storageSpec == _extern) {
	      fprintf(fpSymTable,
	      "#define %s%s %d\n", tmpLink->idRef->IDENT, OFFSET_SUFFIX,
	              index /* offset */);
	    }
	    offset += size; index ++;
	    break;
	  case _intFunc:
	  case _floatFunc:
            index ++;
            break;
	  case _calendar:
	    index ++;
	    break;
	}
    }
    fprintf(fpSymTable, "#define NAMEOFDATAMODULE \"%s\"\n", NAMEOFDATAMODULE);
    fprintf(fpSymTable, "#define NAMEOFDATAMODULE2 \"%s\"\n", NAMEOFDATAMODULE2);
    fprintf(fpSymTable, "#define DATAMODULE_ADDRESS 0x20000\n");
}

idType cnv2const(tp)
idType tp;
{
  if (tp == _int)
    return _intConst;
  else if (tp == _float)
    return _floatConst;
  else if (tp == _char)
    return _charConst;
  else
    vcerror("illegal type for const, must be one of int, float and char");
  return tp;
}    

idType cnv2func(tp)
idType tp;
{
  if (tp == _int)
    return _intFunc;
  else if (tp == _float)
    return _floatFunc;
  else if (tp == _char)
    return _charFunc;
  else
    vcerror("illegal type for function, must be one of int, float and char");
  return tp;
}    

idType cnv2vec(tp)
idType tp;
{
  if (tp == _int)
    return _intVec;
  else if (tp == _float)
    return _floatVec;
  else if (tp == _char)
    return _charVec;
  else
    vcerror("illegal type for vector, must be one of int, float and char");
  return tp;
}    

void checkIntOrFloat(tp)
idType tp;
{
  if ((tp != _int) && (tp != _float))
    vcerror("must be one of int or float");
}

filterType *newFilterSpec(idRef, expr)
idIdent *idRef;
exprTree *expr;
{
    filterType *tmp;
    tmp = (filterType *) malloc(sizeof(filterType));
    tmp->idRef = idRef;
    tmp->parameters = expr;
    return tmp; 
}

bindingType *newBindingSpec(idRef, channelNo, durationExpr)
idIdent *idRef;
int channelNo;
/* long duration; */
exprTree *durationExpr;
{
    bindingType *tmp;
    tmp = (bindingType *) malloc(sizeof(bindingType));
    tmp->idRef = idRef;
    tmp->channelNo = channelNo;
    tmp->durationExpr = durationExpr;
    return tmp;
}

void emitBinding(idRef, filter, binding)
idIdent *idRef;
filterType *filter;
bindingType *binding;
{
  FILE *tmp;
  
  tmp = fpCurrent;
  fpCurrent = fpScan;

  emitCode("if (!");
  emitCode(GLOBAL_ACCESS);
  emitCode(idRef->IDENT); emitCode(GLOBAL_LOCK_SUFFIX);
  emitCode(")\n");
  increaseIndent();
/*
  emitCode("{\n"); increaseIndent();
  emitCode("static long prev;\n");
*/

  if (binding->durationExpr) {
    if ((binding->durationExpr->node->operator != _int_const) ||
        (binding->durationExpr->node->operandConst.longStorage != 0))
    {
      emitCode("{\n"); increaseIndent();
      emitCode("static long store;\n");
    }
  }

  if (strcmp(binding->idRef->info->moduleInfo.moduleTypeRef->IDENT,
          "DIGOUT_1") && 
      strcmp(binding->idRef->info->moduleInfo.moduleTypeRef->IDENT,
          "ANAOUT_1"))
  {
    emitCode(GLOBAL_ACCESS);
    emitCode(idRef->IDENT); emitCode(" = ");
  }
  if (filter) {
    emitCode(filter->idRef->IDENT);
    emitCode("(");
/*
!     to be implemented soon !
!     that is today ! 910522
*/
    emitCode(GLOBAL_ACCESS);
    emitCode(idRef->IDENT);
    emitCode(", ");
/*
!
*/

/*
    if (filter->parameters) {
     codeExpression(fpCurrent, filter->parameters, 0, 0);
     emitCode(", ");
    }
*/

  }
  emitCode(binding->idRef->info->moduleInfo.moduleTypeRef->IDENT);
  emitCode("(");
  emitCode(GLOBAL_ACCESS);
  emitCode(idRef->IDENT);
  emitCode(", ");
  emitCode(binding->idRef->IDENT);
  emitCode(", ");
  fprintf(fpScan, "%d", binding->channelNo);
  emitCode(", ");
/*
!
*/
/*  fprintf(fpScan, "%d", binding->durationExpr); */

  codeExpression(fpScan, binding->durationExpr, 0, 0);


/*   for output routines only, to output when data has changed only
  emitCode(", &prev");
*/
  if (binding->durationExpr) {
    if ((binding->durationExpr->node->operator != _int_const) ||
        (binding->durationExpr->node->operandConst.longStorage != 0))
    {
      emitCode(", ");
      emitCode("&store");
    }
  }
  emitCode(")");
  if (filter) {
    if (filter->parameters) {
     emitCode(", ");
     codeExpression(fpCurrent, filter->parameters, 0, 0);
    }
    emitCode(")");
  }
  emitCode(";\n");
  if (binding->durationExpr) {
    if ((binding->durationExpr->node->operator != _int_const) ||
        (binding->durationExpr->node->operandConst.longStorage != 0))
    {
      decreaseIndent();
      emitCode("}\n");
    }
  }
/*
  decreaseIndent();
  emitCode("}\n");
*/
  decreaseIndent();
  fpCurrent = tmp;
}

int isExpressionAtomVar(expr)
exprTree *expr;
{
  return (expr->left == 0 &&
      expr->right == 0 &&
      expr->node->operator == _ident);
}

int isExpressionIntConst(expr)
exprTree *expr;
{
  return (expr->left == 0 &&
      expr->right == 0 &&
      expr->node->operator == _int_const &&
      expr->node->operandRef == 0);
}

void emitUpdateBinding(idRef, filter, binding)
idIdent *idRef;
filterType *filter;
bindingType *binding;
{
  FILE *tmp;

  tmp = fpCurrent;
  fpCurrent = fpScan;
/*
! Parameters to function update is;
!   char *ptr;
!   int module
!   int channel
!   double durValue;
!   double filtValue;
*/
  if ((isExpressionAtomVar(binding->durationExpr) ||
       isExpressionIntConst(binding->durationExpr)) &&
      (filter == 0 ||
       isExpressionAtomVar(filter->parameters) ||
       isExpressionIntConst(filter->parameters)))
    return;

  emitCode("update(mPtr, ");
  emitCode(binding->idRef->IDENT);        /* module */
  emitCode(", ");
  fprintf(fpScan, "%d, ", binding->channelNo);
  emitCode("(double) ");
  codeExpression(fpScan, binding->durationExpr, 0, 0);
  emitCode(", ");
  emitCode("(double) ");
  if (filter->parameters)
    codeExpression(fpCurrent, filter->parameters, 0, 0);
  else
    emitCode("0");
  emitCode(");\n");
  fpCurrent = tmp;
}

void emitInsertBinding(idRef, filter, binding)
idIdent *idRef;
filterType *filter;
bindingType *binding;
{
  FILE *tmp;

  tmp = fpCurrent;
  fpCurrent = fpScan;
/*
! Parameters to function insert is;
!   int module
!   int channel
!   int cardIOtype
!   int filterFunctionType
!   int varType;
!   void *varPtr;
!   int durType;
!   void *durPtr;
!   int filtType;
!   void *filtPtr;
!   char *lockPtr;
  insert(2, 7, _TEMP_1, _Ni1000LG,
  8, &dm->AS5_GT3_L, durType = -5771, 0,
  8, dm->AS5_GT3_korr, &dm->AS5_GT3_L_lock);

*/
  emitCode("insert(");
  emitCode(binding->idRef->IDENT);        /* module */
  emitCode(", ");
  fprintf(fpScan, "%d, ", binding->channelNo);
  emitCode("_");      /* no conflict with older function calls */
  emitCode(binding->idRef->info->moduleInfo.moduleTypeRef->IDENT);
  emitCode(", ");
  if (filter) {
    emitCode("_");      /* no conflict with older function calls */
    emitCode(filter->idRef->IDENT);
    emitCode(", ");
  } else {
    emitCode("0, ");
  }
  fprintf(fpScan, "%d, ", idRef->declSpec.type);
  emitCode("&");
  emitCode(GLOBAL_ACCESS);
  emitCode(idRef->IDENT);
  emitCode(", ");

  if (binding && binding->durationExpr &&
	(isExpressionAtomVar(binding->durationExpr) ||
	isExpressionIntConst(binding->durationExpr)))
  {
    if (isExpressionIntConst(binding->durationExpr)) {
      if (binding->durationExpr->node->operator == _int_const)
	fprintf(fpScan, "%d /* int const */, ", 20);
      else if (binding->durationExpr->node->operator == _float_const)
	fprintf(fpScan, "%d /* float const */, ", 21);
    } else {
      fprintf(fpScan, "%d, ",
		binding->durationExpr->node->operandRef->declSpec.type);
    }
    codeExpression(fpScan, binding->durationExpr, 0, 0);
    emitCode(", ");
  } else {
    emitCode("0, 0, ");
  }

  if (filter && filter->parameters &&
      (isExpressionAtomVar(filter->parameters) ||
       isExpressionIntConst(filter->parameters)))
  {
    if (isExpressionIntConst(filter->parameters)) {
      if (filter->parameters->node->operator == _int_const)
	fprintf(fpScan, "%d, ",
		filter->parameters->node->operandConst.longStorage);
      else if (filter->parameters->node->operator == _float_const)
	fprintf(fpScan, "%g, ",
		filter->parameters->node->operandConst.doubleStorage);
    } else {
      fprintf(fpScan, "%d, ",
		filter->parameters->node->operandRef->declSpec.type);
    }
      codeExpression(fpCurrent, filter->parameters, 0, 0);
      emitCode(", ");
  } else {
    emitCode("0, 0, ");
  }




  emitCode("&");
  emitCode(GLOBAL_ACCESS);
  emitCode(idRef->IDENT); emitCode(GLOBAL_LOCK_SUFFIX);

  emitCode(");\n");
  fpCurrent = tmp;
}   

idInfo *newInfoBinding(filter, binding)
filterType *filter;
bindingType *binding;
{
  idInfo *tmp;
  tmp = (idInfo *) malloc(sizeof(idInfo));
  tmp->bindingInfo.filter = filter;
  tmp->bindingInfo.binding = binding;
  return tmp;
}

idInfo *newFormelParameters(formelParameters, subr_string)
formelList *formelParameters;
char *subr_string;
{
  idInfo *tmp;
  tmp = (idInfo *) malloc(sizeof(idInfo));
  tmp->funcInfo.formelParameters = formelParameters;
  if (subr_string)
    strcpy(tmp->funcInfo.subrName, subr_string);
  else
    tmp->funcInfo.subrName[0] = 0;
  return tmp;
}

void emitScanBindings()
{
    symLink *tmpLink, *tmp;
    FILE *tmpFile;
    int i, count;
    
    tmpFile = fpCurrent;
    fpCurrent = fpScan;

    if (option_T) {
      fprintf(fpCurrent, "#define _CYCLE_ID               17\n");
      fprintf(fpCurrent, "#define _CYCLE_256_TICKS        256\n");
      emitCode("int timerHandler(s)\n");
      emitCode("int code;\n{\n");
      emitCode("  if (code != _CYCLE_ID)\n");
      emitCode("    exit(0);\n");
      if (lookUpIdent("service"))
          fprintf(fpCurrent, "  if (!%sservice) {\n", GLOBAL_ACCESS);
      for (i = 0; i < timerCounter; i++) {
        fprintf(fpCurrent, "    if (!lock->timerLock[%d])\n", i);
        fprintf(fpCurrent, "      at_%02d();\n", i + 1);
      }
      if (lookUpIdent("service"))
          fprintf(fpCurrent, "  }\n");
      emitCode("}\n");
    }

    emitCode("main(argc, argv)\n");
    emitCode("int argc;\n");
    emitCode("char *argv[];\n");
    emitCode("{\n");	  increaseIndent();
    emitCode("int _restart_after_powerfail = 0;\n");
    if (usePromHooks) {
      emitCode("char *_dest;\n");
    } else {
      emitCode("int _backup_exists = 0;\n");
      emitCode("int _oldAlarmModule = 0, _oldLockModule = 0;\n");
      emitCode("char *_headerPtr1, *_headerPtr2, *_headerPtr3, *_headerPtr4, *_dest;\n");
    }

#define TEST_PID
#ifdef TEST_PID
    emitCode("struct _system *sysVars;\n");
#endif
/*
!     install our traphandlers, i.e PHYIO and IDCIO
*/
    emitCode("_initScanTraps();\n");   	/* new 931110 */

/*    emitCode("initphyio();\n");
    emitCode("initidcio();\n");	*/

    if (systemDefined) {
      fprintf(fpScan, "  strcpy( (char *) 0x3ff00, \"%s\");\n", systemCid);
      fprintf(fpScan, "  *((long *) 0x3fff8) = %d;\n", systemId);
    }
/*
!   destination address for allocation of none existing data modules
*/
    emitCode("_dest = (char *) DATAMODULE_ADDRESS;\n");
/*
!   emit code for checking if '-d' option specifier i.e. debug mode
*/
/*
    emitCode("if ((argv[1][0] == '-') && (argv[1][1] == 'd'))\n");
    emitCode("  DEBUG = 1;\n");
    emitCode("else\n");
    emitCode("  DEBUG = 0;\n");
*/
    
    emitCode("while( argc >= 2  && argv[1][0] == '-' ) {\n");
    emitCode("	while( *++(argv[1]) ) {\n");
    emitCode("    switch( *argv[1] ) {\n");
    emitCode("		case 'd':\n");
    emitCode("		  DEBUG = 1;\n");
    emitCode("		  continue;\n");
    emitCode("		case 'p':\n");
    emitCode("		  LOCAL_PRINTER = 1;\n");
    emitCode("		  continue;\n");
    emitCode("		case 'a':\n");
    emitCode("		  _dest = 0;\n");
    emitCode("		  continue;\n");
    
/*
    		case 'a':
	    if (argv[1][1] == '=') 
  		    sscanf(argv[1] + 2, "%x", &_moduleAddress);
  		  else
  		    _moduleAddress = (char *) 0x35000;
  		   continue;
                case '?':
                    usage();
                    exit(0);
*/                    
  
    emitCode("		default:\n");
    emitCode("		    printf(\"illegal option: %c\", *argv[1]);\n");
    emitCode("		    continue;\n");
    emitCode("		}\n");
    emitCode("	    }\n");
    emitCode("	argv++;\n");
    emitCode("	argc--;\n");
    emitCode("    }\n");

    if (usePromHooks) {
      emitCode("_initScan(&dm, sizeof(struct _datamodule), &aldm, NO_OF_ALARMS,\n");
      emitCode("       sizeof(struct _alarmModule), &aldm2, &lock,\n");
      emitCode("       sizeof(struct _lockModule), NO_OF_TIMERS,\n");
      emitCode("       &_dest, &_restart_after_powerfail);\n");
      emitCode("sysVars = (struct _system *) SYSTEM_AREA;\n");
    } else {
/*
!   allocate data module for global variables
*/  
      emitCode("dm = (struct _datamodule *) \n     createDataModule(");
      emitCode("NAMEOFDATAMODULE, sizeof(struct _datamodule), \n");
      emitCode("         &_restart_after_powerfail, &_headerPtr1, &_dest);\n");
/*
!   allocate data module for backup copy of global variables
*/  
      emitCode("createDataModule(");
      emitCode("NAMEOFDATAMODULE2, sizeof(struct _datamodule), \n");
      emitCode("         &_headerPtr3, &_headerPtr2, &_dest);\n");
      emitCode("createDataModule(");
      emitCode("\"VAR1\", sizeof(struct _datamodule), \n");
      emitCode("         &_headerPtr3, &_headerPtr2, &_dest);\n");
/*
!   if any alarms so try to allocate an alarm module, 
!   that keeps alarm texts
*/
      decreaseIndent();
      emitCode("#if NO_OF_ALARMS > 0\n");
      emitCode("  aldm = (struct _alarmModule *) \n     createDataModule(");
      emitCode("\"ALARM\", sizeof(struct _alarmModule), \n");
      emitCode("          &_oldAlarmModule, &_headerPtr3, &_dest);\n");
      emitCode("  if (!_oldAlarmModule) {\n");
      emitCode("    aldm->alarmListPtr = 0;\n");
      emitCode("    aldm->noOfAlarmEntries = NO_OF_ALARM_ENTRIES;\n");
      emitCode("    aldm->noOfAlarmPts = NO_OF_ALARMS;\n");
      emitCode("  }\n");
      emitCode("  aldm2 = (struct _alarmModule2 *)\n");
      emitCode("            (((char *) aldm) +\n");
      emitCode("              (aldm->noOfAlarmEntries * sizeof(struct _alarmEntry) +\n");
      emitCode("               sizeof(short) + sizeof(long)));\n");
#ifdef TEST_PID
      emitCode("  sysVars = (struct _system *) SYSTEM_AREA;\n");
      emitCode("  sysVars->ptr2AlarmModule = (char *) aldm2;\n");
#else
      emitCode("  { struct _system *sysVars; sysVars = (struct _system *) SYSTEM_AREA; \n");
      emitCode("      sysVars->ptr2AlarmModule = (char *) aldm2; }\n");
#endif
      emitCode("#else\n");
      emitCode("  aldm = 0;\n");
      emitCode("  aldm2 = 0;\n");
      emitCode("#endif\n");
      increaseIndent();
/*
!   if any alarms OR timers, allocate data module for lock variables
*/  
      emitCode("\n");
      decreaseIndent();
      emitCode("#if NO_OF_ALARMS | NO_OF_TIMERS\n");
      emitCode("  lock = (struct _lockModule *) \n     createDataModule(");
      emitCode("\"LOCK\", sizeof(struct _lockModule), \n");
      emitCode("          &_oldLockModule, &_headerPtr4, &_dest);\n");
    /* 911210: added extra () to correct character addition !!!! */
      emitCode("  lock2 = (struct _lockModule2 *) (((char *) lock) +\n");
      emitCode("             (NO_OF_ALARMS * sizeof(int) + sizeof(int)));\n");
      emitCode("  if (!_oldLockModule)\n");
      emitCode("  {\n");
      emitCode("    if (DEBUG) printf(\"new LOCK module created\\n\");\n");
      emitCode("    initAlarm(aldm2, lock);\n");
      emitCode("    initTimer(lock2, NO_OF_TIMERS);\n");
      emitCode("  } else\n");
      emitCode("    if (DEBUG) printf(\"new LOCK module created\\n\");\n");
      emitCode("#else\n");
      emitCode("  lock = 0;\n");
      emitCode("  lock2 = 0;\n");
      emitCode("#endif\n");
      increaseIndent();
    }
/*
!   start scope for initializations done only at STARTUP, not POWER FAIL
*/
    emitCode("if (!_restart_after_powerfail)\n");
    emitCode("{\n"); increaseIndent();
    
/*
    for (i = 0; i < timerCounter; i++) {
      fprintf(fpCurrent, "    %s_timer_%02d.abs = 0;\n", GLOBAL_ACCESS, i + 1);
    }
*/

    
/*
!   initialize any calendars declared globally
*/
    tmp = symbolTable->thisScope;
    while (tmp && (tmp = (tmpLink = tmp)->next, tmpLink)) {
	if (tmpLink->idRef->declSpec.storageSpec != _global)
	    continue;
	if (tmpLink->idRef->declSpec.type != _calendar)
	    continue;
/*
	if (!tmpLink->idRef->info)
	    continue;
*/
	emitCode("memcpy(&");
	emitCode(GLOBAL_ACCESS);
	emitCode(tmpLink->idRef->IDENT); emitCode(", &_tmp_");
	emitCode(tmpLink->idRef->IDENT); emitCode(", sizeof(_tmp_");
	emitCode(tmpLink->idRef->IDENT); emitCode("));\n");
    }
/*
!     initialize all globals that'll need it
*/
    count = 0;
    tmp = symbolTable->thisScope;
    while (tmp && (tmp = (tmpLink = tmp)->next, tmpLink)) {
	if (tmpLink->idRef->declSpec.storageSpec != _global)
	    continue;
	if ((tmpLink->idRef->declSpec.type == _intFunc) ||
  	       (tmpLink->idRef->declSpec.type == _floatFunc))
  	{
  	    /* added 911128 */
	  fprintf(fpSymTable, "extern ");
  	  if (tmpLink->idRef->declSpec.type == _intFunc)
  	    fprintf(fpSymTable, _TYPE_OF_INT);
  	  else if (tmpLink->idRef->declSpec.type == _floatFunc)
  	    fprintf(fpSymTable, _TYPE_OF_FLOAT);
  	  fprintf(fpSymTable, " %s(); \n", tmpLink->idRef->IDENT);

#if 0
if (optionGlobalFunctions) {
  	  emitCode("{ extern ");
	  if (tmpLink->idRef->declSpec.type == _intFunc)
  	    emitCode(_TYPE_OF_INT);
  	  else if (tmpLink->idRef->declSpec.type == _floatFunc)
  	    emitCode(_TYPE_OF_FLOAT);
  	  emitCode(" ");
	  emitCode(tmpLink->idRef->IDENT);
          emitCode("(); ");

  	  emitCode(GLOBAL_ACCESS);
	  emitCode(tmpLink->idRef->IDENT);
	  emitCode(" = ");
    if (tmpLink->idRef->info->funcInfo.subrName[0]) {
/*
!   dm->fkn_X = _initSubrFcn("subr_name", "fkn_X");
*/      
	  emitCode("_initSubrFcn(\"");
	  emitCode(tmpLink->idRef->info->funcInfo.subrName);
	  emitCode("\", \"");
	  emitCode(tmpLink->idRef->IDENT);
	  emitCode("\")");
    } else {
	  emitCode(tmpLink->idRef->IDENT);
    }
	  emitCode(";}\n");
}
#endif
	 
  	  continue;
  	}
/* 
  	  if (tmpLink->idRef->declSpec.type == _intVec) {
            emitCode(".. vec = ");
	codeExpression(fpCurrent, tmpLink->idRef->expr, 0, 0);
	    emitCode("..");
  	  }  
*/

	if (tmpLink->idRef->declSpec.type == _intVec ||
		tmpLink->idRef->declSpec.type == _floatVec)
	{
	  emitCode("{\n");
	  initArray(tmpLink->idRef, 1);
	  emitCode("}\n");
	}

        if (tmpLink->idRef->info)     /* criterium for init after powerfail */
            continue;
	if ((tmpLink->idRef->declSpec.type != _int) && 
  	       (tmpLink->idRef->declSpec.type != _float))
	    continue;
	if (!tmpLink->idRef->expr)
	    continue;
	if (count == 0) {
	  emitCode("{\n");
        }
	emitCode(GLOBAL_ACCESS);
	emitCode(tmpLink->idRef->IDENT);
	emitCode(" = ");
	codeExpression(fpCurrent, tmpLink->idRef->expr, 0, 0);
	emitCode(";\n");
	if (count > 50)
	{
	  emitCode("}\n");
	  count = 0; 
        } else
          count ++;
    }
    if (count != 0) {
      emitCode("}\n");
    }
/*
!	clear all lock flags
*/
    tmp = symbolTable->thisScope;
    count = 0;
    while (tmp && (tmp = (tmpLink = tmp)->next, tmpLink)) {
	if (tmpLink->idRef->declSpec.storageSpec != _global)
	    continue;
	if ((tmpLink->idRef->declSpec.type != _int) && 
  	       (tmpLink->idRef->declSpec.type != _float))
	    continue;
	if (!tmpLink->idRef->info)        /* only locks on i/o bounded vars */
	    continue;
	if (count == 0) {
	  emitCode("{\n");
        }
	emitCode(GLOBAL_ACCESS);
	emitCode(tmpLink->idRef->IDENT); emitCode(GLOBAL_LOCK_SUFFIX);
	emitCode(" = 0;\n");
	if (count > 50)
	{
	  emitCode("}\n");
	  count = 0; 
        } else
          count ++;
    }

    if (count != 0) {
      emitCode("}\n");
    }
/*
!   here ends scope for initializations done only at STARTUP, not POWER FAIL
*/
    decreaseIndent();
    emitCode("}\n");


/*
!     initialize all global functions, added 921016
*/
    count = 0;
    tmp = symbolTable->thisScope;
    while (tmp && (tmp = (tmpLink = tmp)->next, tmpLink)) {
	if (tmpLink->idRef->declSpec.storageSpec != _global)
	    continue;
	if ((tmpLink->idRef->declSpec.type != _intFunc) &&
  	       (tmpLink->idRef->declSpec.type != _floatFunc))
  	    continue;
  	    
        fprintf(fpSymTable, "extern ");
	if (tmpLink->idRef->declSpec.type == _intFunc)
  	    fprintf(fpSymTable, _TYPE_OF_INT);
  	else if (tmpLink->idRef->declSpec.type == _floatFunc)
  	    fprintf(fpSymTable, _TYPE_OF_FLOAT);
  	fprintf(fpSymTable, " %s(); \n", tmpLink->idRef->IDENT);

        if (optionGlobalFunctions) {
  	  emitCode("{ extern ");
  	  if (tmpLink->idRef->declSpec.type == _intFunc)
  	    emitCode(_TYPE_OF_INT);
  	  else if (tmpLink->idRef->declSpec.type == _floatFunc)
  	    emitCode(_TYPE_OF_FLOAT);
  	  emitCode(" ");
	  emitCode(tmpLink->idRef->IDENT);
          emitCode("(); ");

  	  emitCode(GLOBAL_ACCESS);
	  emitCode(tmpLink->idRef->IDENT);
	  emitCode(" = ");
    if (tmpLink->idRef->info->funcInfo.subrName[0]) {
/*
!   dm->fkn_X = _initSubrFcn("subr_name", "fkn_X");
*/      
	  emitCode("_initSubrFcn(\"");
	  emitCode(tmpLink->idRef->info->funcInfo.subrName);
	  emitCode("\", \"");
	  emitCode(tmpLink->idRef->IDENT);
	  emitCode("\")");
    } else {
	  emitCode(tmpLink->idRef->IDENT);
    }
	  emitCode(";}\n");
        }
	 
      continue;
    }
/*
!     end of function initialization
*/

if (!usePromHooks) {
    emitCode("if (_dest != ((char *) DATAMODULE_ADDRESS)) {\n");
    emitCode("  FILE *fp;\n");
    emitCode("  fp = fopen(\"/lcdterm\", \"w\");\n");
    emitCode("  fprintf(fp, \"Please reboot.                          \");\n");
    emitCode("  fprintf(fp, \"reset or cycle power OFF/ON, waiting... \");\n");
    emitCode("  fclose(fp);\n");
    {
      char s[100];
      sprintf(s, 
        "if (os9fork(\"%s\", 0, 0, 0, 0, 0, 0) == -1) {\n",
        "reboot");
      emitCode(s);
      emitCode("  printf(\"cannot fork to reboot module\\n\");\n");
      emitCode("}\n");
    }
    emitCode("  sleep(0);\n");
    emitCode("}\n");
}

/*
!     initialize all outputs that are to be initialized after any startup
!     criterium is if  (tmpLink->idRef->info)  exists
*/
    emitCode("\n/*\n! init for I/O\n*/\n");
    tmp = symbolTable->thisScope;
    while (tmp && (tmp = (tmpLink = tmp)->next, tmpLink)) {
	if (tmpLink->idRef->declSpec.storageSpec != _global)
	    continue;
        if (!tmpLink->idRef->info)
            continue;
	if ((tmpLink->idRef->declSpec.type != _int) &&
  	       (tmpLink->idRef->declSpec.type != _float))
	    continue;
	if (!tmpLink->idRef->expr)
	    continue;
	emitCode(GLOBAL_ACCESS);
	emitCode(tmpLink->idRef->IDENT);
	emitCode(" = ");
	codeExpression(fpCurrent, tmpLink->idRef->expr, 0, 0);
	emitCode(";\n");
    }
/*
!   emit code for forking screen process, so we get pid and can signal to'em
*/
    {
      char s[100];
      sprintf(s, 
        "if (os9fork(\"%s\", 0, 0, 0, 0, 0, 0) == -1) {\n",
        "screen");
      emitCode(s);
      emitCode("  printf(\"cannot fork to screen module\\n\");\n");
      emitCode("}\n");
    }
/*
!   registrate intercept handler
*/
    if (option_T) {
      emitCode("intercept(timerHandler);\n");
      emitCode("if ((alm_cycle(_CYCLE_ID, _CYCLE_256_TICKS | 0x80000000)) == -1 )\n");
      emitCode("  exit(_errmsg(errno, \"can't set alarm - \"));\n");
    }
/*
!     For option_Q, quick I/O, emit declarations of i/o
*/
    if (option_Q) {
      int cnt = 0;
      emitCode("{\n");
      tmp = symbolTable->thisScope;
      while (tmp && (tmp = (tmpLink = tmp)->next, tmpLink)) {
	if (tmpLink->idRef->declSpec.storageSpec != _global)
	    continue;
	if ((tmpLink->idRef->declSpec.type != _int) &&
	       (tmpLink->idRef->declSpec.type != _float))
	    continue;
	if (!tmpLink->idRef->info)
	    continue;
	cnt++;
	if (cnt > 10) {
	  cnt = 0;
	  emitCode("}\n{\n");
	}
	emitInsertBinding(tmpLink->idRef,
			tmpLink->idRef->info->bindingInfo.filter,
			tmpLink->idRef->info->bindingInfo.binding);
      }
      emitCode("}\n");
    }

/*
!	emit the scan loop
*/
    fprintf(fpCurrent, "  while (1) {\n");		  increaseIndent();

    if (lookUpIdent("sysHbLed")) {
      fprintf(fpCurrent,
		"{\nstatic char prev;\n");
      fprintf(fpCurrent,
		"if (prev) *((char*) 0x34001e) = 4; else *((char*)0x34001f) = 4;\n");
      fprintf(fpCurrent, "prev ^= 1;\n}\n");
    }
    if (lookUpIdent("sysScanLoopTime")) {
      fprintf(fpCurrent,
		"{\nstatic int p[3];\n%ssysScanLoopTime = deltatime(p);\n}\n",
		GLOBAL_ACCESS);
    }

#ifdef TEST_PID
    emitCode("sysVars->newFlex++;\n");
#endif
    if (option_Q) {
      tmp = symbolTable->thisScope;
      while (tmp && (tmp = (tmpLink = tmp)->next, tmpLink)) {
	if (tmpLink->idRef->declSpec.storageSpec != _global)
	    continue;
	if ((tmpLink->idRef->declSpec.type != _int) &&
	       (tmpLink->idRef->declSpec.type != _float))
	    continue;
	if (!tmpLink->idRef->info)
	    continue;
	emitUpdateBinding(tmpLink->idRef,
			tmpLink->idRef->info->bindingInfo.filter,
			tmpLink->idRef->info->bindingInfo.binding);
      }

      emitCode("scanAllModules();\n");
    } else {
      tmp = symbolTable->thisScope;
      while (tmp && (tmp = (tmpLink = tmp)->next, tmpLink)) {
	if (tmpLink->idRef->declSpec.storageSpec != _global)
	    continue;
	if ((tmpLink->idRef->declSpec.type != _int) &&
	       (tmpLink->idRef->declSpec.type != _float))
	    continue;
	if (!tmpLink->idRef->info)
	    continue;
	if (generateRandom) {
	  emitCode("{ static long store; ");
	  emitCode("randomizeIt(&");
	  emitCode(GLOBAL_ACCESS);
	  emitCode(tmpLink->idRef->IDENT);
	  emitCode(", &store, ");
	  fprintf(fpCurrent, "%d, ",
	       (tmpLink->idRef->declSpec.type == _float) ? 1 : 0);
	  {
	    bindingType *binding;
	    binding = tmpLink->idRef->info->bindingInfo.binding;
	    emitCode(binding->idRef->IDENT);
	  }
	  emitCode("); }\n");
	} else {
	  emitBinding(tmpLink->idRef,
			tmpLink->idRef->info->bindingInfo.filter,
			tmpLink->idRef->info->bindingInfo.binding);
	}
      }
    }   /* end of if option_Q */
/*
!   last in the scan loop we will check all alarm conditions and timers
*/
    if (option_Q && (1 || currentNoOfAlarms || timerCounter))
      emitCode("if (secondsHasChanged()) {\n");

    if (lookUpIdent("service")) {
      fprintf(fpCurrent, "  if (!%sservice) {\n", GLOBAL_ACCESS);
    }
    for (i = 0; i < currentNoOfAlarms; i++) {
      fprintf(fpCurrent, "    if (!aldm2->alarmPts[%d].disable)\n", i);
      fprintf(fpCurrent, "      alarm_%d();\n", alarmVector[i]);
    }
    if (lookUpIdent("service")) {
      fprintf(fpCurrent, "  }\n");
    }

    if (!option_T) {
      if (option_Q && timerCounter) {
	for (i = 0; i < timerCounter; i++) {
	  if (typeOfTimer[i+1] == 0) {
	    fprintf(fpCurrent, "      if (!lock->timerLock[%d])\n", i);
	    fprintf(fpCurrent, "        at_%02d();\n", i + 1);
	  }
	}
      } else {
	for (i = 0; i < timerCounter; i++) {
	  fprintf(fpCurrent, "    if (!lock->timerLock[%d])\n", i);
	  fprintf(fpCurrent, "      at_%02d();\n", i + 1);
	}
      }
    }

    fprintf(fpCurrent, "#if NO_OF_ALARMS > 0\n");
    if (usePromHooks)
      fprintf(fpCurrent, "    _packAlarms(aldm, aldm2);\n");
    else
      fprintf(fpCurrent, "    packAlarms(aldm, aldm2);\n");
    fprintf(fpCurrent, "#endif\n");

    if (option_Q && (1 || currentNoOfAlarms || timerCounter))
      emitCode("}\n");
/*
! 	check calendars every minute
*/
    if (!option_T) {
      if (option_Q && timerCounter) {
	emitCode("if (minutesHasChanged()) {\n");
	for (i = 0; i < timerCounter; i++) {
	  if (typeOfTimer[i+1] == 1) {
	    fprintf(fpCurrent, "      if (!lock->timerLock[%d])\n", i);
	    fprintf(fpCurrent, "        at_%02d();\n", i + 1);
	  }
	}
	emitCode("}\n");
      }
    }


    fprintf(fpCurrent, "  }\n}\n");
    if (usePromHooks) {
      fprintf(fpCurrent, "\nstatic display(p1, p2, p3, p4, p5, p6, p7, p8)\n");
      fprintf(fpCurrent, "char *p1,*p2,*p3,*p4,*p5,*p6,*p7,*p8;\n");
      fprintf(fpCurrent, "{\n");
      fprintf(fpCurrent, "  _alarmDisplay(p1, p2, p3, p4, p5, p6, p7, p8);\n");
      fprintf(fpCurrent, "}\n");
    }

    if (option_Q) {
fprintf(fpCurrent, "int oldMinute;     \n");
fprintf(fpCurrent, "int oldSecond;     \n");
fprintf(fpCurrent, "int minutesHasChanged()\n");
fprintf(fpCurrent, "{\n");
fprintf(fpCurrent, "  static int minute = -1;\n");
fprintf(fpCurrent, "  int x;\n");
fprintf(fpCurrent, "  x = (_getsys(D_Second, 4) - 1) / 60;\n");
fprintf(fpCurrent, "  if (x == minute)\n");
fprintf(fpCurrent, "    return 0;\n");
fprintf(fpCurrent, "  minute = x;\n");
fprintf(fpCurrent, "  return 1;\n");
fprintf(fpCurrent, "}\n");
fprintf(fpCurrent, "int secondsHasChanged()\n");
fprintf(fpCurrent, "{\n");
fprintf(fpCurrent, "  static int second = -1;\n");
fprintf(fpCurrent, "  int x;\n");
fprintf(fpCurrent, "  x = _getsys(D_Second, 4) % 3600;\n");
fprintf(fpCurrent, "  if (x == second)\n");
fprintf(fpCurrent, "    return 0;\n");
fprintf(fpCurrent, "  second = x;\n");
fprintf(fpCurrent, "  return 1;\n");
fprintf(fpCurrent, "}\n");
    }

    fpCurrent = tmpFile;
}
/*
!	checks to see if primary expression before () are function identifier
*/
void checkFunctionId(expr)
exprTree *expr;
{
    if (!expr || !expr->node)
	return ;			/* already taken care of */
    if (expr->node->operator != _ident) {
	vcerror("must be function identifier");
	printf("expr->node->operator = %d\n", expr->node->operator);
    } else {
	switch (expr->node->operandRef->declSpec.type) {
	    case _intFunc:
	    case _floatFunc:
	    case _charFunc:
		break;
	    default:
		vcerror("must be function identifier");
	        printf("expr->node->operandRef->declSpec.type = %d\n",
	            expr->node->operandRef->declSpec.type);
		break;
	}
    }
}
/*
!	emit declarations for current scope to fpCurrent
*/
void emitDeclarationList()
{
    symLink *tmpLink, *tmp;
    int i;

    tmp = currentScope->thisScope;
    while (tmp && (tmp = (tmpLink = tmp)->next, tmpLink)) {
	switch (tmpLink->idRef->declSpec.type) {
	    case _char:
		emitIndent();
		fprintf(fpCurrent, "%s %s;\n", _TYPE_OF_CHAR,
		                                      tmpLink->idRef->IDENT);
		break;
	    case _int:
		emitIndent();
		fprintf(fpCurrent, "%s %s;\n", _TYPE_OF_INT,
		                                      tmpLink->idRef->IDENT);
		break;
	    case _float:
		emitIndent();
		fprintf(fpCurrent, "%s %s;\n", _TYPE_OF_FLOAT, 
		                                      tmpLink->idRef->IDENT);
		break;
	    case _charVec:
		emitIndent();
		if (tmpLink->idRef->info->vecInfo.size == -1) 
  		  fprintf(fpCurrent, "%s %s[];\n", _TYPE_OF_CHAR, 
					    tmpLink->idRef->IDENT);
		else
  		  fprintf(fpCurrent, "%s %s[%d];\n", _TYPE_OF_CHAR, 
		                            tmpLink->idRef->IDENT,
		        tmpLink->idRef->info->vecInfo.size);
		break;
	    case _intVec:
		emitIndent();
		if (tmpLink->idRef->info->vecInfo.size == -1) 
		  fprintf(fpCurrent, "%s %s[];\n", _TYPE_OF_INT, 
		                            tmpLink->idRef->IDENT);
		else
		  fprintf(fpCurrent, "%s %s[%d];\n", _TYPE_OF_INT, 
					    tmpLink->idRef->IDENT,
			tmpLink->idRef->info->vecInfo.size);
		break;
	    case _floatVec:
		emitIndent();
		if (tmpLink->idRef->info->vecInfo.size == -1) 
		  fprintf(fpCurrent, "%s %s[];\n", _TYPE_OF_FLOAT, 
		                                    tmpLink->idRef->IDENT);
		else
		  fprintf(fpCurrent, "%s %s[%d];\n", _TYPE_OF_FLOAT, 
		                                    tmpLink->idRef->IDENT,
		        tmpLink->idRef->info->vecInfo.size);
		break;
	    case _charRef:
		emitIndent();
		fprintf(fpCurrent, "%s *%s;\n", _TYPE_OF_CHAR, 
		                            tmpLink->idRef->IDENT);
		break;
	    case _intRef:
		emitIndent();
		fprintf(fpCurrent, "%s *%s;\n", _TYPE_OF_INT, 
		                            tmpLink->idRef->IDENT);
		break;
	    case _floatRef:
		emitIndent();
		fprintf(fpCurrent, "%s *%s;\n", _TYPE_OF_FLOAT, 
						    tmpLink->idRef->IDENT);
		break;
            case _calendar:   /* what !!! , cannot automatic initialize 
                                  stack variables in OS9, must place in sym.h
                               */
	      emitIndent();
  	      fprintf(fpCurrent, "%s %s;\n", _TYPE_OF_CALENDAR,
	                                             tmpLink->idRef->IDENT);
/*	      size = _SIZE_OF_CALENDAR;
	      offset += size; */
              break;
/*
	        fprintf(fpSymTable, "struct {\n");
		fprintf(fpSymTable,
			    "  long range, resolution, size, baseTime;\n");
	        fprintf(fpSymTable, "  char calendar[%d];\n",
    	                      tmpLink->idRef->info->calendarInfo.size);
	        fprintf(fpSymTable, "} %s = {\n", tmpLink->idRef->IDENT);
	        fprintf(fpSymTable, " %d, %d, %d, %d, \n", 
	                  tmpLink->idRef->info->calendarInfo.range,
	                  tmpLink->idRef->info->calendarInfo.resolution,
	                  tmpLink->idRef->info->calendarInfo.size,
	                  tmpLink->idRef->info->calendarInfo.baseTime);
	        fprintf(fpSymTable, "{");
                for (i = 0; i<tmpLink->idRef->info->calendarInfo.size-1; i++) {
	          if ((i % 8) == 0)
		    fprintf(fpSymTable, "\n");
		  fprintf(fpSymTable, "%d, ",
	                    tmpLink->idRef->info->calendarInfo.calendar[i]);
                }
                if (i < tmpLink->idRef->info->calendarInfo.size) {
	          fprintf(fpSymTable, "%d ", 
	                    tmpLink->idRef->info->calendarInfo.calendar[i]);
                }
                fprintf(fpSymTable, "\n}\n};\n");
*/
	        break;
	    default:
		vcerror(
    "program fault: type not implemented in routine emitDeclarationList()");
		printf("type = %d\n", tmpLink->idRef->declSpec.type);
	}
    }
/*
!   if this is main, we must here emit code for linkage to datamodule
*/
  if (!strncmp("main", scopeName, 4) && (scopeCnt == 1) && (fpCurrent == fpMain)) {
    if (!usePromHooks) {
      emitCode("char *_headerPtr1, *_headerPtr2;\n");
    }
/*
!   install our traphandler, i.e. IDCIO
*/
    emitCode("_initMainTraps();\n");   	/* new 931110 */
/*    emitCode("initidcio();\n");	*/
    emitCode("if (argv[1][0] == '-' && argv[1][1] == 'd')\n");
    emitCode("  DEBUG = 1;\n");
    if (usePromHooks) {
      emitCode("_initMain(&dm, &aldm, NO_OF_ALARMS, &aldm2);\n");
    } else {
      emitCode("dm = (struct _datamodule *) linkDataModule(NAMEOFDATAMODULE,\n");
      emitCode("        &_headerPtr1);\n");
      emitCode("if (!dm) {\n");
      emitCode("  printf(\"cannot link to datamodule '%s'\\n\", NAMEOFDATAMODULE);\n");
      emitCode("  printf(\"check if process 'scan' is running\\n\");\n");
      emitCode("  return 0;\n");
      emitCode("}\n");

      decreaseIndent();
      emitCode("#if NO_OF_ALARMS > 0\n");
      emitCode("  aldm = (struct _alarmModule *) \n     linkDataModule(");
      emitCode("\"ALARM\", &_headerPtr2);\n");
      emitCode("  aldm2 = (struct _alarmModule2 *)\n");
      emitCode("            (((char *) aldm) +\n");
      emitCode("              (aldm->noOfAlarmEntries * sizeof(struct _alarmEntry) +\n");
      emitCode("               sizeof(short) + sizeof(long)));\n");
      emitCode("#else\n");
      emitCode("  aldm = 0;\n");
      emitCode("  aldm2 = 0;\n");
      emitCode("#endif\n");
      increaseIndent();
    }
  }
/*
!   now initialize those local variables
*/
    tmp = currentScope->thisScope;
    while (tmp && (tmp = (tmpLink = tmp)->next, tmpLink)) {
	if (!tmpLink->idRef->expr)
          continue;
	switch (tmpLink->idRef->declSpec.type) {
	    case _int:
		emitIndent();
		fprintf(fpCurrent, "%s = ", tmpLink->idRef->IDENT);
		codeExpression(fpCurrent, tmpLink->idRef->expr, 0, 0);
		fprintf(fpCurrent, ";\n");
		emitIndent();
		break;
	    case _float:
		emitIndent();
		fprintf(fpCurrent, "%s = ", tmpLink->idRef->IDENT);
		codeExpression(fpCurrent, tmpLink->idRef->expr, 0, 0);
		fprintf(fpCurrent, ";\n");
		emitIndent();
		break;
	    case _intVec:
	    case _floatVec:
    	        emitCode("{\n");
   	        initArray(tmpLink->idRef, 0);
	        emitCode("}\n");
		break;
	    default:
		vcerror(
    "program fault: type not implemented for initialization in routine emitDeclarationList()");
		printf("type = %d\n", tmpLink->idRef->declSpec.type);
		printf("expr = %d\n", tmpLink->idRef->expr);
		break;
	}
    }
}

CALENDAR x;
int entry;

void parseCalendar(line)
char *line;
{
  int w, weekDay = 0, color, stopDay = 0, startTime, stopTime;
  static char day[] = { 'M', 'T', 'W', 'T', 'F', 'S', 'S' };

  for (w = 0; w < 7; w++) 
    if (line[w*3] == day[w])
	weekDay |= (1 << w);
  if (weekDay == 0) {
    weekDay = atoi(&line[0]);
    stopDay = atoi(&line[7]);
  } else
    weekDay |= 2048;

  if (line[23] == 'B') color = 1;
  else if (line[23] == 'O') color = 2;
  else if (line[23] == 'R') color = 4;
  else color = 0;

  if (line[32] == 'H') {
    startTime = 0;     
    stopTime = 2400;
  } else {
    double atof(), d;
    d = 100.0*atof(&line[32]);
    startTime = d;
    d = 100.0*atof(&line[40]);
    stopTime = d;
  }
  x.day[entry] = weekDay;
  x.stopday[entry] = stopDay;
  x.color[entry] = color;
  x.start[entry] = startTime;
  x.stop[entry++] = stopTime;
}

void traverseCalendar(expr)
exprTree *expr;
{
  if (expr->node->operator == _comma) {
    if (expr->left)
      traverseCalendar(expr->left);
    if (expr->right)
      traverseCalendar(expr->right);
  } else {
    char *p;
    if (expr->node->operator == _string_const) {
      p = expr->node->operandConst.voidStorage;
      parseCalendar(p);
    } else {
      vcwarning("Cannot find string...");
    }
  }
}

void emitCalendarDef(expr)
exprTree *expr;
{
  int i;
/* parse tree and build calendar */
  entry = 0;
  traverseCalendar(expr->left);

/* emit calendar */
              for (i = 0; i < NO_OF_CAL_ENTRIES; i++)
                fprintf(fpSymTable, "%d, ", x.day[i]);
              fprintf(fpSymTable, "\n");
              for (i = 0; i < NO_OF_CAL_ENTRIES; i++)
                fprintf(fpSymTable, "%d, ", x.stopday[i]);
              fprintf(fpSymTable, "\n");
              for (i = 0; i < NO_OF_CAL_ENTRIES; i++)
                fprintf(fpSymTable, "%d, ", x.color[i]);
              fprintf(fpSymTable, "\n");
              for (i = 0; i < NO_OF_CAL_ENTRIES; i++)
                fprintf(fpSymTable, "%d, ", x.start[i]);
              fprintf(fpSymTable, "\n");
              for (i = 0; i < NO_OF_CAL_ENTRIES - 1; i++)
                fprintf(fpSymTable, "%d, ", x.stop[i]);
              fprintf(fpSymTable, "%d", x.stop[i]);
              fprintf(fpSymTable, "\n};\n");
}
