/* symtable.h  1992-09-04 TD,  version 1.2 */
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
! symtable.h
! Copyright (C) 1991, IVT Electronic AB.
*/

/*
!     File: symtable.h
!     
!     Contains code for handling routines close to symbol table
!
!     History     
!     Date        Revision Who  What
!     
!     29-nov-1990    1.000 TD   Initial release of translator
!
!      4-sep-1992    1.20  TD   Added _recList operator for recursive lists.
!                               Used in the screen_order statement
!                               Added _screenIdent
*/

typedef struct _symLink;
typedef struct _idIdent;
typedef struct _filterType;
typedef struct _bindingType;
typedef struct _exprTree;
typedef struct _formelList;

#define NO_OF_CAL_ENTRIES 10

typedef struct {
  unsigned short day[NO_OF_CAL_ENTRIES];
  unsigned short stopday[NO_OF_CAL_ENTRIES];
  unsigned char color[NO_OF_CAL_ENTRIES];
  unsigned short start[NO_OF_CAL_ENTRIES];
  unsigned short stop[NO_OF_CAL_ENTRIES];
} CALENDAR;

struct _remote {
  long timeStamp;
};
/*#define _REMOTE_MASK 0x8000 */
#define _REMOTE_MASK 0x80
#define _ALIAS_MASK 0x40

/* these should be placed in a new header file such as C-definitions */
#define _SIZE_OF_CHAR   sizeof(char)
#define _SIZE_OF_INT    sizeof(long)
#define _SIZE_OF_FLOAT  sizeof(double)
#define _SIZE_OF_PTR    sizeof(char*)
#define _SIZE_OF_CALENDAR sizeof(CALENDAR)

#define _TYPE_OF_CHAR    "char"
#define _TYPE_OF_INT    "int"
#define _TYPE_OF_FLOAT  "double"
#define _TYPE_OF_CALENDAR "CALENDAR"

struct _idIdent *insertId();

typedef enum {	_equal, _comma, _assign, _notequal, _lt, _gt, _lte, _gte,
		_plusplus, _plus, _minusminus, _minus, _mul, _div, _mod,
		_int_const, _float_const, _char_const, _string_const,
		_ident, _func, _vector,
		_or, _and, _not,
/*
!	The following are new 920222 to conform to ANSI C standard ...
*/
		_bitOr, _bitXor, _bitAnd, _cmpl, _sl, _sr, _if,
		_assignMul, _assignDiv, _assignMod, _assignSr, 
		_assignSl, _assignAdd, _assignSub, _assignAnd,
		_assignOr, _assignXor, 
/*
!       1992-09-04 'recList' added for recursive lists (multi dimensional).
!       Used for screen_order and will also be used in the future to 
!       implement the long gone multi dimensional arrays !
*/		
		_recList, _screenIdent
	} opCode;

typedef enum {
    _intConst, _floatConst, _charConst, _stringConst,
    _intVec, _floatVec, _charVec,
    _int, _float, _char, _string, 	/* _const,	*/
    _intFunc, _floatFunc, _charFunc,
    _screen, _calendar, _filter, _moduletype, _module,
    _intRef, _floatRef, _charRef
} idType;

typedef enum {  _local,  /* defined inside a compound statement */
		_global, /* defined in the outermost scope of a file */
		_extern,  /* will be retrieved from another site */
		_empty    /* not used */
} storageType;

typedef union {
	struct {
	    struct _exprTree *value;
/* 	    long value;       */
	} constInfo;
	struct {
/*	    int dummy;  */
            struct _formelList *formelParameters;
            char subrName[32];
	} funcInfo;
	struct {
	    struct _idIdent *moduleTypeRef;
	} moduleInfo;
	struct {
	    int size;
	} vecInfo;
	struct {
	    long range, resolution, size, baseTime;
	    char *calendar;
	} calendarInfo;
        struct {
            struct _filterType *filter;
            struct _bindingType *binding;
        } bindingInfo;
	struct {
	    int atLine;
	    char right[30], down[30], help[30];
/*	    struct _idIdent *lptr, *rptr, *uptr, *dptr, *hptr;  */
	    short lptr, rptr, uptr, dptr, hptr;
	    short isHelp;
	} screenInfo;
/*
	struct {
          time_t abs;
          long rel;
          long day;
          int includeFlag;
	} timerInfo;
*/
} idInfo;
    
typedef struct {
    storageType storageSpec;
    idType type;
    long nodeNo;                   /* only for extern */
    char *alias;        /* pointer to real name (name at remote site) */
} declType;

#define MAX_NO_OF_ACCESS_LINES 100
	   
typedef struct _xrefSpec
{
    int declaredAt;
    short lines[MAX_NO_OF_ACCESS_LINES];  /* or a linked list if less than 30
					     or more than 100 vars */
    char currentIndex;
} xrefSpec;
	   
typedef struct _idIdent
{
    char IDENT[32];
    idInfo *info;
    declType declSpec;
    xrefSpec *xrefs;
    long lockOffset;
    struct _exprTree *expr;       /* identifier initialized to this */
    char initialized;             /* 0 == variable has no value yet */
} idIdent;

typedef struct _symLink
{
    struct _symLink *next;
    idIdent *idRef;
} symLink;

typedef struct _symTable
{
    struct _symLink *thisScope;
    struct _symTable *outerScope;
    struct _symTable *xref;
    char name[100];
} symTable;

typedef struct
{
    opCode operator;
    idIdent *operandRef;
    union {
      char byteStorage;
      long longStorage;
      double doubleStorage;
      void *voidStorage;
    } operandConst;
} exprNode;

typedef struct _exprTree
{
    struct _exprTree *left, *right;
    exprNode *node;
} exprTree;

typedef struct _filterType
{
    idIdent *idRef;
    exprTree *parameters;
} filterType;


typedef struct _bindingType
{
  idIdent *idRef;
  int channelNo;
  exprTree *durationExpr;   /* long durationTime; */
} bindingType;

/* used for formel list in function prototypes */

typedef struct _formelList
{
  char name[32];
  idType node;
  struct _formelList *left, *right;
} formelList;

#define MAX_NO_OF_ALARMS 100

