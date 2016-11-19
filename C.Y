/* c.y  1993-04-20 TD,  version 1.61 */
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
! c.y
! Copyright (C) 1991-1993 IVT Electronic AB.
*/


/*
!     File: C.y
!     
!     Parser coded for YACC, C-file is 'y.tab.c'
!
!     History     
!     Date        Revision Who  What
!     
!     29-nov-1990     1.00  TD   Initial release of translator
!     14-dec-1990     1.01  TD   changed %d to %ld if not OSK, for long's printf
!     30-apr-1991     1.0x  TD   added type-ref and dotdotdot
!     22-jul-1991     1.03  TD   duration clause is now expression
!                                glitch abs and rel are now expressions
!     26-jul-1991     1.04  TD   constant_and_rel added
!
!     22-feb-1992     1.22  TD   Added assignment operators such as *= += etc
!				 and bitwise operators such as | & ~ >> << etc
!				 and the trinary operator ? :
!
!     26-may-1992     1.30  TD   Added keyword 'alias' such as
!
!                                remote 3 int kalle alias duc3_kalle;
!
!      2-jul-1992     1.40  TD   Added syntax block 'event' such as
!
!                                event <condition> duration_opt do statement
!
!
!      4-sep-1992     1.50  TD   Added internal operator _recList to be used in
!                                1)     screen_order statement
!                                2)     multi dimensional arrays
!
!                                Upon occurrence, the code generator emitts
!                                a)     a start brace,
!                                b)     the right branch
!                                c)     an end brace
!                                The left branch should be terminated.
!
!                                Added _screenIdent
!
!     15-oct-1992     1.60  TD   Added function support, the 'SUBR' keyword
!
!     20-apr-1993     1.61  TD   Added quick i/o support, typeOfTimer[] added
!
*/
%{
#include <stdio.h>
#include <time.h>
#ifdef OSK
#include <strings.h>
#else
#include <stdlib.h>
#include <string.h>
#endif
#include "symtable.h"
#include "calendar.h"
#include "prototype.h"
#include "icode.h"

extern FILE *yyin, *yyout, *fpScreen, *fpScan, *fpMain, *fpSymTable;
extern FILE *fpSubroutine;

FILE *fpCurrent = 0;

int actualDeclare = 0;	/* 1 = id's are being declared as actual parameters */ 
int idDeclare = 0;	/* 1 = id's are being declared */ 
idIdent *cref,          /* current referenced identifier */
        *dref;          /* declared referenced identifier */

char this_string[256], cid[256], screenId[32], cid2[32];
char subr_string[256];  /* current subr "name" */
char includefile[256];
long cval;
double cfval;
int scopeCounter = 0;	/* increments for every new scope and decrements
			   when leaving scope, 0 == global */

int timerCounter = 0;   /* counts each timer AT statement */

#define MAX_NO_OF_TIMERS 100
char typeOfTimer[MAX_NO_OF_TIMERS]; /* 0, fast, 1, slow etc */

char scopeName[100];
int scopeCnt = 0;
int alarmNo, alarmIndex;			/* current no of alarm */
char alarmClass;                                /* current alarm class */
int alarmVector[MAX_NO_OF_ALARMS];
int currentNoOfAlarms = 0;

long nodeNo;                    /* only for extern/remote */
declType currentDeclSpec;	/* current type of var and storage spec */
int moduleNo, channelNo;

btTime betweenTime;

idInfo *currentCalendar;

extern int usePromHooks;

exprTree *screenUsage = 0;

static int eventCounter;

struct {
  int day;
  int month;
  int year;
} cDay;

struct _glitch
{
/*
  time_t abs;
  long rel;
*/
  exprTree *abs;
  exprTree *rel;

  long day;
  int includeFlag;
} glitchSpec;

int systemId, systemDefined = 0;
char systemCid[248];  

struct tm brokenDownTime = {0, 0, 0, 0, 0, 0, 0, 0, 0 };

idInfo currentIdInfo;

int icnt;
static int sp = 0;
static long stack[50];
#define PUSH(xxx) ((stack[sp > 49 ? 49 : sp++] = (xxx)))
#define POP() (sp ? stack[--sp] : 0)

extern int option_DEBUG;

extern int optionGlobalFunctions;

extern int optionSubroutineModule;

extern int nextSubrFcnPtr;
extern idIdent *subrFcn[100];

extern int optionEmitIcode;       /* added 920908 */

#include "coderep.c"
%}
%start program

%union
{
    filterType *filterSpec;
    bindingType *bindingSpec;
    idInfo *infoSpec;
    declType declSpec;
    storageType storageSpec;
    idType typeSpec;
    idIdent *idSpec;
    char *stringPtr;
    exprNode *node;
    exprTree *tree;
    time_t ctid;
    long integer;
    formelList *formelParameters;
}

%token INCLUDE SYSTEMID
%token LEFTPAR RIGHTPAR LEFTBRACE RIGHTBRACE LEFTBRACKET RIGHTBRACKET
%token COLON COMMA SEMICOLON PLUSPLUS MINUSMINUS 
%token MUL DIV MOD PLUS MINUS LT GT LTE GTE ASSIGN AMPERSAND DOTDOTDOT
%token NOTEQUAL AND OR EQUAL
%token NOT
%token CHAR INT FLOAT CONST
%token EXTERN REMOTE ALIAS EVENT SUBR RETURN
%token IF THEN ELSE WHILE 
%token DO WHEN   
%token BREAK CONTINUE SWITCH CASE DEFAULT
%token SCREEN AT ALARM ON CALENDAR RIGHT DOWN HELP MAIN RANGE RESOLUTION BASE
%token MODULE IS CHANNEL FILTER MODULETYPE
/* 
%token DISPLAY PRINT ENTER
*/
%token INT_ID FLOAT_ID CHAR_ID INT_FUNC_ID FLOAT_FUNC_ID CHAR_FUNC_ID 
%token INT_CONST_ID FLOAT_CONST_ID CHAR_CONST_ID
%token INT_VEC_ID FLOAT_VEC_ID CHAR_VEC_ID
%token INT_REF_ID FLOAT_REF_ID CHAR_REF_ID
%token UNKNOWN_ID CALENDAR_ID
%token SCREEN_ID FILTER_ID MODULETYPE_ID SCREEN_ORDER
%token CHARACTER_CONSTANT INTEGER_CONSTANT FLOATING_CONSTANT
%token IDENTIFIER STRING QUOTEDSTRING
%token EVERY DURATION BETWEEN ON OFF
%token RED ORANGE BLACK DECADE YEAR MON MONTH DAY HOUR MIN SEC
%token LEXICALERROR

%token ASSIGN_MUL ASSIGN_DIV ASSIGN_MOD ASSIGN_ADD ASSIGN_SUB
%token ASSIGN_SR ASSIGN_SL ASSIGN_OR ASSIGN_AND ASSIGN_XOR
%token BIT_OR BIT_XOR SR SL QUESTION_MARK CMPL
	
%type <filterSpec>      filterSpec
%type <bindingSpec>     deviceBindingSpec
%type <ctid>            relTime absTime
%type <idSpec>          declarator functionDeclarator
%type <infoSpec>	
%type <declSpec>	declarationSpecifiers parameterDeclaration
%type <formelParameters> parameterTypeList parameterList
%type <formelParameters> actualParameterListOpt
%type <formelParameters> actualParameterList 
%type <declSpec>         actualParameterDeclaration

%type <storageSpec>	storageClassSpecifier
%type <typeSpec>	typeSpecifier
%type <stringPtr>
%type <node>		assignmentOperator unaryOperator
%type <node>		constant identifier string constant_and_rel
%type <tree>		postfixExpression unaryExpression
%type <tree>		multiplicativeExpression additiveExpression
%type <tree>		exclusiveOrExpression andExpression shiftExpression
%type <tree>		relationalExpression equalityExpression
%type <tree>		inclusiveOrExpression logicalAndExpression
%type <tree>		logicalOrExpression conditionalExpression
%type <tree>		assignmentExpression argumentExpressionList
%type <tree>		screenIdExpression screenExpressionList
%type <tree>		screenTotList screenOneList
%type <tree>		primaryExpression expression
%type <tree>        /*    constantList */ calendarName
%type <tree>            durationClause
%type <integer>         timeUnit onOff colorExpression color everySpecOpt

%left BIT_OR BIT_XOR SR SL

%left OR
%left AND LT GT LTE GTE 
%left PLUS MINUS
%left MUL DIV MOD
%left PLUSPLUS MINUSMINUS
%left NOT UMINUS


%left LEFTPAR LEFTBRACKET
%%

program:
	{
	    initSymbolTable();
	    fpCurrent = fpScan;
            emitScanHeadings();
            systemDefined = 0;
            subr_string[0] = 0;
	}
	    translationUnit
	{
	    if (screenUsage)
	      makeScreenUsage(screenUsage);
	    else
  	      makeScreenArray();
            makeGlobalSymTable();
            emitScanBindings();
	    escapeOuterScope();		/* free's symbol table */
            emitScanEndings();
	}


translationUnit
	: externalDeclaration
	| translationUnit externalDeclaration
	;

externalDeclaration
	: moduleDefinition
	| constDeclaration

	| functionDefinition        /* 921006 test ! */

	| declaration
	| alarmDefinition
        | eventDefinition
	| timerDefinition
	| screenDefinition
	| mainDefinition
	| preproc
	;


preproc
	: INCLUDE QUOTEDSTRING { strcpy(includefile, this_string);
				 doSubFile(includefile, 1);
                               }
	| SYSTEMID INTEGER_CONSTANT COMMA QUOTEDSTRING 
          {
            systemId = cval;
            strcpy(systemCid, this_string);
            systemDefined = 1;
          }
	;

moduleDefinition
	: MODULE INTEGER_CONSTANT checkIS checkModuletype_id checkSemicolon
	     {  
                if (cval < 1 || cval > 80)
                    vcerror("illegal module number");
                currentIdInfo.moduleInfo.moduleTypeRef = cref;
                currentDeclSpec.storageSpec = _global;
		currentDeclSpec.type = _module;
		currentDeclSpec.alias = 0;
                sprintf(cid, "%d", cval);
                declareId(cid, currentDeclSpec, &currentIdInfo, 0);
             }
	| MODULETYPE  { idDeclare = 1; } moduleTypeList 
		checkSemicolon { idDeclare = 0; }
	| FILTER { idDeclare = 1; } filterList 
		checkSemicolon { idDeclare = 0; }
	;

moduleTypeList
	: moduleIdentifier
	| moduleTypeList COMMA moduleIdentifier
	;

moduleIdentifier
	: IDENTIFIER { declareGlobalId(cid, _moduletype);}
	;

filterList
	: filterIdentifier
	| filterList COMMA filterIdentifier
	;

filterIdentifier
	: IDENTIFIER { declareGlobalId(cid, _filter);}
	;


declarationListOpt			/* for compound-statement */
	:
	| declarationList 
	;

declarationList
	: declaration
	| declarationList declaration
	;

declaration
/*
	: declarationSpecifiers { idDeclare = 1; currentDeclSpec = $1; } 
		XXXinitDeclaratorList
		{ 
		   idDeclare = 0; 
		} 
	;
*/
	: declarationSpecifiers { idDeclare = 1; currentDeclSpec = $1; } 
		initDeclaratorList SEMICOLON
		{ 
		   idDeclare = 0; 
		} 
	;


XXXinitDeclaratorList
        : /* COLON COLON */ functionDeclarator 
          {
            emitCode("/* declaration list */");
            emitDeclarationList();  /* if any */
          }
          compoundStatement 
          {
            scopeCounter--;
            decScope();
            fpCurrent = fpScan;
          }
        | initDeclaratorList SEMICOLON
        ;

functionDefinition
	: SUBR declarationSpecifiers { idDeclare = 1; currentDeclSpec = $2; } 
              functionDeclarator 
	  compoundStatement
          {
            scopeCounter--;
            decScope();
            fpCurrent = fpScan;
          }
        | SUBR QUOTEDSTRING 
            { 
              strcpy(subr_string, this_string); subr_string[31] = 0;
            }
           LEFTBRACE 
              declarationList 
           RIGHTBRACE     { subr_string[0] = 0; }
	;

functionDeclarator
        : IDENTIFIER 
          { 
	    dref = declareId(cid, currentDeclSpec, 
				0  /* no info */, 0  /* 0 = current scope */);
            dref->declSpec.type = cnv2func(dref->declSpec.type);

            if (optionSubroutineModule) {
              fpCurrent = fpSubroutine;
              subrFcn[nextSubrFcnPtr++] = dref;
            }
            else if (optionGlobalFunctions)
              fpCurrent = fpScan;
            else
              fpCurrent = fpMain;

            scopeCounter++;
            strcpy(scopeName, cid);
            incScope(scopeName, scopeCnt++);
            emitCode("\n");
          }
            LEFTPAR 
          actualParameterListOpt checkRIGHTPAR
          { 
            dref->info = newFormelParameters($4, 0 /* in this module */);
            idDeclare = 0; 
            emitFunctionFkn(dref);
            emitDeclarationList();  /* if any */
          }

declarationSpecifiers
	: storageClassSpecifier typeSpecifier  { $$ = newDeclSpec($1, $2, nodeNo); }
	| typeSpecifier {$$ = newDeclSpec(!scopeCounter ? _global: _local, $1, 0L);}
	;

constDeclaration
	: CONST typeSpecifier 
	    { 
		idDeclare = 1;
		currentDeclSpec.type = cnv2const($2);
		currentDeclSpec.storageSpec = _global;
		currentDeclSpec.alias = 0;
	    } 
	    constInitList checkSemicolon
		{ 
		   idDeclare = 0; 
		} 
	;


constInitList
	: constInit
	| constInitList COMMA constInit
	;

constInit
	: IDENTIFIER checkASSIGN constant_and_rel
	    { 
		currentIdInfo.constInfo.value = cExprTree($3, 0, 0);

		declareId(cid, currentDeclSpec, &currentIdInfo, 0);
	    }
	;

storageClassSpecifier
	: EXTERN	           { nodeNo = 0;    $$ = _extern; 
                   vcwarning("Keyword 'EXTERN' is obsolete; use REMOTE"); }
        | REMOTE             	   { nodeNo = 0;    $$ = _extern; }
        | REMOTE INTEGER_CONSTANT  { nodeNo = cval; $$ = _extern; }
	;

typeSpecifier
	: CHAR		{ $$ = _char; }
	| INT		{ $$ = _int;  }
	| FLOAT		{ $$ = _float;}
	| CALENDAR	{ $$ = _calendar;}
	;

initDeclaratorList
	: initDeclarator
	| initDeclaratorList COMMA { idDeclare = 1; } initDeclarator
	;

parameterTypeList
        : parameterList
        | parameterList COMMA DOTDOTDOT   
          {
/*
            $$ = cFormelList( ???????????? );
*/
          }
        ;

/*
argumentExpressionList
	: assignmentExpression		{ $$ = $1; }
	| argumentExpressionList COMMA assignmentExpression
		{ $$ = cExprTree(cExprNode(_comma, 0, 0, 0), $1, $3); }
	;
*/

parameterList
        : parameterDeclaration     { $$ = cFormelList($1.type, 0, 0, 0); }
        | parameterList COMMA parameterDeclaration
          {
            $$ = cFormelList(0, $1, cFormelList($3.type, 0, 0, 0), 0);
          } 
        ;

actualParameterListOpt
        :                           { $$ = 0; }
        | actualParameterList       { $$ = $1; }
        ;

actualParameterList
        : actualParameterDeclaration     { $$ = cFormelList($1.type, 0, 0, cid); }
        | actualParameterList COMMA actualParameterDeclaration
          {
            $$ = cFormelList(0, $1, cFormelList($3.type, 0, 0, cid), 0);
          } 
        ;

actualParameterDeclaration
        : parameterDeclaration 
          { 
            idDeclare = 1; currentDeclSpec = $1; actualDeclare = 1;
          } 
           declarator
          { 
            if ($3->declSpec.type == _charVec)
              $1.type = _charVec;
            else if ($3->declSpec.type == _intVec)
              $1.type = _intVec;
            else if ($3->declSpec.type == _floatVec)
              $1.type = _floatVec;
            $$ = $1;
            actualDeclare = 0;
          }
        ;

parameterDeclaration
        : declarationSpecifiers /* declarator */  
          {
            $$ = $1;
          }
        | declarationSpecifiers AMPERSAND /* declarator */  
          {
            if ($1.type == _char)
              $1.type = _charRef;
            else if ($1.type == _int)
              $1.type = _intRef;
            else if ($1.type == _float)
              $1.type = _floatRef;
/*
            else if ($1.type == _calendar)
              $1.type = _calendarRef;
*/

            $$ = $1;
          }
        | declarationSpecifiers LEFTBRACKET RIGHTBRACKET /* added 920924 */
          {
            if ($1.type == _char)
              $1.type = _charVec;
            else if ($1.type == _int)
              $1.type = _intVec;
            else if ($1.type == _float)
              $1.type = _floatVec;
            $$ = $1;
          }
        ;

initDeclarator
	: declarator
/*
	| declarator LEFTPAR checkRIGHTPAR
          { 
            $1->declSpec.type = cnv2func($1->declSpec.type);
            $1->info = 0;
          }
	| declarator LEFTPAR parameterTypeList checkRIGHTPAR
          { 
            $1->declSpec.type = cnv2func($1->declSpec.type);
            $1->info = newFormelParameters($3, 0);
          }
*/
	| declarator ASSIGN
          {
            idDeclare = 0;  dref = $1;
	  } initializer
        | IDENTIFIER ALIAS { strncpy(cid2, cid, 31); } declarator
          {
            strcpy($4->declSpec.alias = (char *) malloc(32), cid2);

/*            $3->declSpec.alias = $1; */
/*      $1->declSpec.storageSpec = _empty; */ /* so nobody will find it ! */
          }
	| declarator IS 
          {
            idDeclare = 0;  dref = $1; checkIntOrFloat($1->declSpec.type);
	  }
            filterSpec AT deviceBindingSpec 
          { if ($1->info)
              vcerror("illegal type, no arrays allowed");
            $1->info = newInfoBinding($4, $6);
            /* emitBinding($1, $4, $6); */
          } optInitializer
	| declarator AT
          {
            idDeclare = 0;  dref = $1; checkIntOrFloat($1->declSpec.type);
          }
            deviceBindingSpec
          { if ($1->info)
              vcerror("illegal type, no arrays allowed");
            $1->info = newInfoBinding(0, $4);
            /*            emitBinding($1, 0, $4);       */
          } optInitializer
	;

declarator
	: IDENTIFIER
          {
	    $$ = declareId(cid, currentDeclSpec, 
				0  /* no info */, 0  /* 0 = current scope */);
          }
	| IDENTIFIER LEFTBRACKET RIGHTBRACKET     /* added 921014 */
          {
            declType tmpDeclSpec;   /* copy of type of var and storage spec */
            tmpDeclSpec.storageSpec = currentDeclSpec.storageSpec;
            tmpDeclSpec.type = cnv2vec(currentDeclSpec.type);
            tmpDeclSpec.nodeNo = currentDeclSpec.nodeNo;
            tmpDeclSpec.alias = currentDeclSpec.alias;
            if (actualDeclare == 0)
              vcerror("Must specify size of vector");
            currentIdInfo.vecInfo.size = -1;      /* no specified !! */
	    $$ = declareId(cid, tmpDeclSpec, 
			      &currentIdInfo, 0  /* 0 = current scope */);
          }
	| IDENTIFIER LEFTBRACKET constant RIGHTBRACKET
          {
            declType tmpDeclSpec;   /* copy of type of var and storage spec */
            tmpDeclSpec.storageSpec = currentDeclSpec.storageSpec;
            tmpDeclSpec.type = cnv2vec(currentDeclSpec.type);
            tmpDeclSpec.nodeNo = currentDeclSpec.nodeNo;
            tmpDeclSpec.alias = currentDeclSpec.alias;
            currentIdInfo.vecInfo.size = cval;
	    $$ = declareId(cid, tmpDeclSpec, 
			      &currentIdInfo, 0  /* 0 = current scope */);
          }
	| IDENTIFIER LEFTPAR checkRIGHTPAR
          { 
	    $$ = declareId(cid, currentDeclSpec, 
				0  /* no info */, 0  /* 0 = current scope */);
            $$->declSpec.type = cnv2func($$->declSpec.type);
/*            $$->info = 0;   changed 921015 */
            $$->info = newFormelParameters(0, subr_string);
          }
	| IDENTIFIER LEFTPAR parameterTypeList checkRIGHTPAR
          { 
	    $$ = declareId(cid, currentDeclSpec, 
				0  /* no info */, 0  /* 0 = current scope */);
            $$->declSpec.type = cnv2func($$->declSpec.type);
            $$->info = newFormelParameters($3, subr_string);
          }
	;

optInitializer
        :
	| ASSIGN initializer
        ;

initializer
        : LEFTBRACE argumentExpressionList RIGHTBRACE
          {
/*            checkIntOrFloat(dref->declSpec.type);   */

            dref->expr = cExprTree(0, $2, 0);       /* ????????? */
          }
	| RANGE
          { if (dref->declSpec.type != _calendar)
                vcerror("illegal type specifier, must be calendar");
          }
            timeUnit RESOLUTION timeUnit BASE absTime LEFTBRACE 
                {
                  currentCalendar = newCalendar($3, $5, $7);
                }
            calSpecifications RIGHTBRACE
          {
            dref->info = currentCalendar;
          }
	| assignmentExpression                      /* added 920924 */
          {
            checkIntOrFloat(dref->declSpec.type);
            dref->expr = $1;   /* cExprTree($1, 0, 0); */
          }
/*
	| constant_and_rel
          {
            checkIntOrFloat(dref->declSpec.type);
            dref->expr = cExprTree($1, 0, 0);
          }
*/
	;


filterSpec
	: FILTER_ID
          {
            $$ = newFilterSpec(cref, 0);
          }
	| FILTER_ID { dref = cref; }
                LEFTPAR argumentExpressionList /* constantList */ RIGHTPAR
          {
            $$ = newFilterSpec(dref, $4);
          }
	;
/*
constantList
	: constant    { $$ = cExprTree($1, 0, 0); }
	| constantList COMMA constant
          {
            $$ = cExprTree(cExprNode(_comma, 0, 0, 0), $1, cExprTree($3, 0, 0));
          }
	;
*/
deviceBindingSpec
	: MODULE INTEGER_CONSTANT { moduleNo = cval; }
             channelSpecOpt durationClause
          {
            sprintf(cid, "%d", moduleNo);
            if (!(cref = lookUpIdent(cid))) { char errText[100];
                sprintf(errText, "module no %d not declared", moduleNo);
                vcerror(errText);
            }
            $$ = newBindingSpec(cref, channelNo, $5);
          }
	;

channelSpecOpt
	:                             { channelNo = 0; }
	| CHANNEL INTEGER_CONSTANT    { channelNo = cval; }
	;


timeUnit: DECADE      { $$ = 10*366*24*3600; }
	| YEAR        { $$ = 366*24*3600; }
	| MON         { $$ = 31*24*3600; }
	| DAY         { $$ = 24*3600; }
	| HOUR        { $$ = 3600; }
	| MIN         { $$ = 60; }
	| SEC         { $$ = 1; }
	;

calSpecifications
	: calSpec
	| calSpecifications calSpec
	;

calSpec	: areaSpec everySpecOpt COMMA onOff checkSemicolon
          {
            setCalendar(currentCalendar, &betweenTime, $2, $4);
          }
	;

areaSpec: absTime
            { 
              betweenTime.first = $1; betweenTime.last = $1;
            }
	| BETWEEN absTime checkAND absTime 
            { 
              betweenTime.first = $2; betweenTime.last = $4;
            }
	;

everySpecOpt
	:                           { $$ = colorRed | colorOrange | colorBlack;}
	| EVERY DAY                 { $$ = colorRed | colorOrange | colorBlack;}
	| EVERY colorExpression DAY { $$ = $2; }
	;

onOff	: ON        { $$ = stateOn; }
	| OFF       { $$ = stateOff; }
	;

alarmClassOpt
        :               { alarmClass = 0; /* alarm class A */ }
        | IDENTIFIER
          {
            alarmClass = (cid[0] | 0x20) - 'a';
            if (alarmClass < 0 || alarmClass > 3 || strlen(cid) != 1)
            {
              vcerror("Alarm class expected (a,b,c or d) or keyword WHEN");
            }
          }
        ;

/*
!                                event <condition> duration_opt do statement
*/
eventDefinition
        : EVENT checkLEFTPAR expression RIGHTPAR durationClause checkDO
        {
          eventCounter ++;
          sprintf(scopeName, "event_%02d", eventCounter); scopeCnt = 0;
          emitEventStart(eventCounter, $3, $5);
        }
	    statement
        {
          emitEventMiddle(eventCounter);
        }
	    elseAlarmClause
        {
          emitEventEnd(eventCounter);
        }
      ;

alarmDefinition
	: ALARM INTEGER_CONSTANT 
          { alarmNo = cval; idDeclare = 1; } 
            alarmClassOpt
	    checkWHEN checkLEFTPAR { idDeclare = 0; } expression RIGHTPAR
		 durationClause checkDO
          {
            alarmIndex = checkAndAllocate(alarmNo, alarmVector);
            emitAlarmStart(alarmNo, $8, $10, alarmIndex, alarmClass);
            sprintf(scopeName, "alarm_%02d",alarmNo); scopeCnt = 0;
          }
	    compoundStatement
          {
            emitAlarmMiddle(alarmNo, alarmIndex);
          }
	    elseAlarmClause
          {
            emitAlarmEnd(alarmNo, alarmIndex, alarmClass);
          }
	;

durationClause
	:            
          {         /*  { $$ = 0; }  */
            long temp;
            temp = 0;
            $$ = cExprTree(
                      cExprNode(_int_const, 0, (char *) &temp, _SIZE_OF_INT),
                      0 ,0);
          }
	| DURATION expression  { $$ = $2; }
        | DURATION relTime     
          {
            long temp;
            temp = $2;        /* cast time_t -> long (the same) */
            $$ = cExprTree(
                      cExprNode(_int_const, 0, (char *) &temp, _SIZE_OF_INT),
                      0 ,0);
          }
	;

elseAlarmClause
        :                   { emitCode(";\n"); }
        | ELSE statement  
        ;

timerDefinition
	: AT 
          { 
            glitchSpec.abs = glitchSpec.rel = (exprTree *) 0;
            glitchSpec.day = 0;
            glitchSpec.includeFlag = 0;
          }
            glitchSpecifier checkDO 
	      {
                sprintf(scopeName, "at"); scopeCnt = 0;
                fprintf(fpScan, "void at_%02d()\n{\n", ++timerCounter);
                typeOfTimer[timerCounter] = 0;
                increaseIndent();
                fprintf(fpScan, "  static struct _glitch ");
#ifdef OSK
/*
                fprintf(fpScan, "_glitchSpec = { %d, %d, %d, %d, 0, 0};\n",
                            glitchSpec.abs, glitchSpec.rel, glitchSpec.day,
                            glitchSpec.includeFlag);
*/
                fprintf(fpScan, "_glitchSpec = { 0, 0};\n");
#else
/*
                fprintf(fpScan, "_glitchSpec = { %ld, %ld, %ld, %d, 0, 0};\n",
                            glitchSpec.abs, glitchSpec.rel, glitchSpec.day,
                            glitchSpec.includeFlag);
*/
                fprintf(fpScan, "_glitchSpec = { 0, 0};\n");
#endif
                if (option_DEBUG)
                  emitDebugStatement();
                fprintf(fpScan, "  if (checkGlitchRaise(");
                fprintf(fpScan, "&_glitchSpec, ");
                if (glitchSpec.abs)
                  codeExpression(fpScan, glitchSpec.abs, 0, 0);
                else
                  fprintf(fpScan, "0");
                fprintf(fpScan, ", ");
                if (glitchSpec.rel)
                  codeExpression(fpScan, glitchSpec.rel, 0, 0);
                else
                  fprintf(fpScan, "0");
#ifdef OSK
                fprintf(fpScan, ", %d, %d", 
                            glitchSpec.day, glitchSpec.includeFlag);
#else
                fprintf(fpScan, ", %ld, %d", 
                            glitchSpec.day, glitchSpec.includeFlag);
#endif
                emitCode2(fpScan, "))\n");
                increaseIndent();
              }
	    compoundStatement
              {
                decreaseIndent();
                fprintf(fpScan, "  else if (checkGlitchFall(");
                fprintf(fpScan, "&_glitchSpec, ");
                if (glitchSpec.abs)
                  codeExpression(fpScan, glitchSpec.abs, 0, 0);
                else
                  fprintf(fpScan, "0");
                fprintf(fpScan, ", ");
                if (glitchSpec.rel)
                  codeExpression(fpScan, glitchSpec.rel, 0, 0);
                else
                  fprintf(fpScan, "0");
#ifdef OSK
                fprintf(fpScan, ", %d, %d", 
                            glitchSpec.day, glitchSpec.includeFlag);
#else
                fprintf(fpScan, ", %ld, %d",
                            glitchSpec.day, glitchSpec.includeFlag);
#endif
                emitCode2(fpScan, "))\n");
                increaseIndent();
              }
            elseAtClause
              {
                decreaseIndent();
                decreaseIndent();
                fprintf(fpScan, "}\n\n");
              }
	| AT calendarName checkDO
	      {
                sprintf(scopeName, "at"); scopeCnt = 0;
                fprintf(fpScan, "void at_%02d()\n{\n", ++timerCounter);
                typeOfTimer[timerCounter] = 1;    /* calendar */
                increaseIndent();
                if (option_DEBUG)
                  emitDebugStatement();
/*
void at_01()
{
  if (  checkCalendarLevel(dm->kalle))
  {
    }
  else if (  checkCalendarLevel(dm->kalle))
    ;
}
*/

#define ENTER_CALENDAR

#ifdef ENTER_CALENDAR
                fprintf(fpScan, "  if (checkCalendarLevel(");
                codeExpression(fpScan, $2, 0, 0);      /* & added here ! */
                emitCode2(fpScan, "))\n");
#else
                fprintf(fpScan, "  if (");
                codeExpression(fpScan, $2, 0, 0);
                emitCode2(fpScan, ")\n");
#endif
                increaseIndent();
              }
	    compoundStatement 
              {
                decreaseIndent();
/*
                fprintf(fpScan, "else if (checkCalendarFall(&");
*/
#ifdef ENTER_CALENDAR
                emitCode2(fpScan, "  else\n");
#else
                fprintf(fpScan, "  else if (!");
                codeExpression(fpScan, $2, 0, 0);
                emitCode2(fpScan, ")\n");
#endif
                increaseIndent();
              }
            elseAtClause
              {
                decreaseIndent();
                decreaseIndent();
                fprintf(fpScan, "}\n");
              }
	;

calendarName
	: CALENDAR_ID    { $$ = cExprTree(cExprNode(_ident, cref, 0, 0), 0, 0); }
	;

glitchSpecifier
	: glitch
	| glitchSpecifier glitch
	;

glitch	: absTime                     
          {
            long temp;
            temp = $1;
            glitchSpec.abs = cExprTree(cExprNode(
                                _int_const, 0, (char *) &temp, _SIZE_OF_INT),
                                0, 0);
/*            glitchSpec.abs = $1;      */
            if (glitchSpec.includeFlag & 1)
              vcerror("Absolute expression already specified");
            glitchSpec.includeFlag |= 1;
          }
        | expression
          {
            glitchSpec.abs = $1;
            if (glitchSpec.includeFlag & 1)
              vcerror("Absolute expression already specified");
            glitchSpec.includeFlag |= 1;
          }
        | EVERY expression
          {
            glitchSpec.rel = $2;
            if (glitchSpec.includeFlag & 2)
              vcerror("Relative expression already specified");
            glitchSpec.includeFlag |= 2;
          }
	| EVERY relTime
          {
            long temp;
            temp = $2;
            glitchSpec.rel = cExprTree(cExprNode(
                                _int_const, 0, (char *) &temp, _SIZE_OF_INT),
                                0, 0);
/*            glitchSpec.rel = $2;        */
            if (glitchSpec.includeFlag & 2)
              vcerror("Relative expression already specified");
            glitchSpec.includeFlag |= 2;
          }
	| EVERY DAY
          {
            glitchSpec.day = orColorExpression(colorBlack, 
                                    orColorExpression(colorRed, colorOrange));
            if (glitchSpec.includeFlag & 4)
              vcerror("DAY expression already specified");
            glitchSpec.includeFlag |= 4;
          }
	| EVERY colorExpression DAY
          {
            glitchSpec.day = $2;
            if (glitchSpec.includeFlag & 4)
              vcerror("DAY expression already specified");
            glitchSpec.includeFlag |= 4;
          }
	;

colorExpression
	: color                       { $$ = $1; }
	| colorExpression OR color    { $$ = orColorExpression($1, $3); }
	;

color	: RED               { $$ = colorRed; }
	| ORANGE            { $$ = colorOrange; }
	| BLACK             { $$ = colorBlack; }
	;

relTime	: timeSpec
          {
            $$ = (brokenDownTime.tm_hour * 60 + brokenDownTime.tm_min) * 60 +
                  brokenDownTime.tm_sec;
          }
	;
/*
! abstime parses a day or time specifier or both and returns C-time
! obs ! must have cleared 'brokenDownTime' previously
*/
absTime	: daySpec
          { 
            brokenDownTime.tm_hour = brokenDownTime.tm_min = 
              brokenDownTime.tm_sec = 0;
            $$ = mktime(&brokenDownTime);
          }
	| daySpec timeSpec    { $$ = mktime(&brokenDownTime); }
	| timeSpec           
          { 
/*
            brokenDownTime.tm_mday = brokenDownTime.tm_mon = 
                brokenDownTime.tm_year = 0;
            $$ = mktime(&brokenDownTime);
*/
            $$ = (brokenDownTime.tm_hour * 60 + brokenDownTime.tm_min) * 60 +
                  brokenDownTime.tm_sec;
          }
	;
/*
! daySpec parses a day specifier in format yy-mm-dd and inserts into
! struct 'brokenDownTime'. Does not return anything
*/
daySpec	: numberOpt MINUS 
              { brokenDownTime.tm_mday = (cval > 1900) ? cval - 1900 : cval; }
            monthOpt MINUS 
              { brokenDownTime.tm_mon = cval - 1; } 
            numberOpt
              { brokenDownTime.tm_year = cval; }
	;

timeSpec: numberOpt COLON
              { brokenDownTime.tm_hour = cval; }
            numberOpt COLON
              { brokenDownTime.tm_min = cval; }
            numberOpt
              { brokenDownTime.tm_sec = cval; }
	;

numberOpt
	:                         { cval = 0; }
	| INTEGER_CONSTANT 
	;

monthOpt:
	| MONTH
	;

elseAtClause
	:                               { emitCode(";\n"); }
	| ELSE compoundStatement
	;

screenIdExpression
	: SCREEN_ID	
          { 
            $$ = cExprTree(cExprNode(_screenIdent, cref, 0, 0), 0, 0);
          }
	| constant	{ $$ = cExprTree($1, 0, 0); }
        ;

screenTotList
        : screenOneList                       { $$ = $1; }
        | screenTotList COMMA screenOneList
		{ $$ = cExprTree(cExprNode(_comma, 0, 0, 0), $1, $3); }
        ;

screenOneList
        : LEFTBRACE screenExpressionList RIGHTBRACE   
                { $$ = cExprTree(cExprNode(_recList, 0, 0, 0), 0, $2); }
        ;

screenExpressionList
	: screenIdExpression		{ $$ = $1; }
	| screenExpressionList COMMA screenIdExpression
		{ $$ = cExprTree(cExprNode(_comma, 0, 0, 0), $1, $3); }
	;

screenDefinition
	: SCREEN { idDeclare = 1; currentDeclSpec.storageSpec = _global;
				  currentDeclSpec.type = _screen;
				  currentDeclSpec.alias = 0;
		   currentIdInfo.screenInfo.right[0] = 
		   currentIdInfo.screenInfo.down[0] = 
		   currentIdInfo.screenInfo.help[0] = 0;
		   currentIdInfo.screenInfo.lptr = 
		   currentIdInfo.screenInfo.rptr = 
		   currentIdInfo.screenInfo.uptr = 
		   currentIdInfo.screenInfo.dptr = 
                   currentIdInfo.screenInfo.isHelp = 
		   currentIdInfo.screenInfo.hptr = 0;
		   currentIdInfo.screenInfo.atLine = yylineno;
		 }
	    screenName locationSpec
	    {
		fprintf(fpScreen, "void %s()\n", screenId);
		fpCurrent = fpScreen;
		idDeclare = 0;
		declareId(screenId, currentDeclSpec, &currentIdInfo,
			0  /* 0 = current scope */);

		strcat(strcpy(scopeName, "screen_"), screenId); scopeCnt = 0;
	    }
		compoundStatement
	    {
	        fpCurrent = fpScan;
	    }
        | SCREEN_ORDER ASSIGN LEFTBRACE screenTotList RIGHTBRACE
          {
            screenUsage = cExprTree(0, $4, 0);       /* ????????? */
          }
	;

locationSpec
	:				{ /* terminal screen */ }
	| COLON closeScreenSpec
	;

closeScreenSpec
	: closeScreen
	| closeScreenSpec COMMA closeScreen
	;

closeScreen
	: RIGHT IDENTIFIER	{ strcpy(currentIdInfo.screenInfo.right, cid);}
	| DOWN  IDENTIFIER	{ strcpy(currentIdInfo.screenInfo.down, cid);}
	| HELP  IDENTIFIER	{ strcpy(currentIdInfo.screenInfo.help, cid);}
	;

screenName
	: IDENTIFIER	{ strncpy(screenId, cid, 31); screenId[31] = 0; }
	;

mainDefinition
	: MAIN 
		{ 
			strcpy(scopeName, "main");
			scopeCnt = 0; 
			fpCurrent = fpMain;
			emitCode("main(argc, argv)\n");
			emitCode("int argc;\n");
			emitCode("char *argv[];\n");
                        icnt = 0;
		} 
	  compoundStatement
		{
			fpCurrent = fpScan;
		}
	;

compoundStatement
	: LEFTBRACE 
	    { 
		emitCode("{\n"); 
		scopeCounter++; incScope(scopeName, scopeCnt++);
		increaseIndent();
	    } 
		declarationListOpt
                  {
                    emitDeclarationList();  /* if any ! */
                  }
                statementListOpt 
	  RIGHTBRACE  
	    { 
		decreaseIndent();
		emitCode("}\n"); scopeCounter--; decScope();
	    } 
	;

statementListOpt
	: 
	| statementList
	;

statementList
	: statement
	| statementList statement
	;

           
statement
	: compoundStatement
        | CASE constant COLON { emitCode("case "); 
                    codeExpression(fpCurrent, cExprTree($2, 0, 0), 0, 0); 
                    emitCode(":\n");
              if (option_DEBUG) {
                 emitDebugStatement();
             }
          }
        | {  if (option_DEBUG) {
               emitCode("{\n");
               emitDebugStatement();
             }
          }
            statementOK
          {
            if (option_DEBUG)
               emitCode("}\n");
          }
        ;

statementOK
	: IF checkLEFTPAR expression RIGHTPAR checkTHEN
	    { 
                checkForEqualSign($3);
		emitCode("if ("); codeExpression(fpCurrent, $3, 0, 0);
                    emitCode(")\n");
                if (optionEmitIcode) {
                  emitIcode(_IF);
                  emitIcodeExpression($3);
                  PUSH(getIcodePtr());
                  emitIcode(_dummy);
                }
		increaseIndent();
                freeExpression($3);
                icnt ++;
                PUSH(icnt);
	    }
	      statement 
            {
              decreaseIndent();
              if (icnt == POP()) 
                vcwarning("expression with little effect");
              if (optionEmitIcode) {
                long is;
                is = POP();
                PUSH(getIcodePtr());
                emitIcode(_dummy);
                emitIcode_jmp(is);        /* jump from location 'is' to here */
              }
            }
              elseClause 
            {
              if (optionEmitIcode) {
                long is;
                is = POP();
                emitIcode_jmp(is);        /* jump from location 'is' to here */
              }
            }
	| expressionStatement
	| WHILE checkLEFTPAR expression RIGHTPAR 
	    { 
                checkForEqualSign($3);
		emitCode("while ("); codeExpression(fpCurrent, $3, 0, 0);
                    emitCode(")\n");
                if (optionEmitIcode) {
                  emitIcode(_WHILE);
                  emitIcodeExpression($3);
                  PUSH(getIcodePtr());
                  emitIcode(_dummy);
                }
		increaseIndent();
                freeExpression($3);
                icnt ++;
                PUSH(icnt);
	    }
	     statement 
            {
              decreaseIndent(); 
              if (icnt == POP()) 
                vcwarning("expression with little effect");
              if (optionEmitIcode) {
                long is;
                is = POP();
                emitIcode_jmp(is);        /* jump from location 'is' to here */
              }
            }
	| DO
            {
              emitCode("do\n"); increaseIndent();
              icnt ++;
              PUSH(icnt);
            }
            statement
            WHILE checkLEFTPAR expression RIGHTPAR
	    {
              if (icnt == POP())
                vcwarning("expression with little effect");
              decreaseIndent();
              checkForEqualSign($6);
              emitCode("while ("); codeExpression(fpCurrent, $6, 0, 0);
                    emitCode(")\n");
              freeExpression($6);
            }
	    checkSemicolon  { emitCode(";\n"); }
        | BREAK { emitCode("break"); icnt++; } 
            checkSemicolon  { emitCode(";\n"); }
        | CONTINUE { emitCode("continue"); icnt++; }
            checkSemicolon  { emitCode(";\n"); }
        | DEFAULT COLON { emitCode("default:\n"); }

        | RETURN expression checkSemicolon 
          { 
            icnt++;
            emitCode("return "); 
            codeExpression(fpCurrent, $2, 0, 0);
            freeExpression($2);
            emitCode(";\n");
          }
/*
        | CASE constant COLON { emitCode("case "); 
                    codeExpression(fpCurrent, cExprTree($2, 0, 0), 0, 0); 
                    emitCode(":\n");
          }
*/
        | SWITCH LEFTPAR expression RIGHTPAR 
          {
            emitCode("switch(");
            codeExpression(fpCurrent, $3, 0, 0);
            emitCode(")\n");
          }
          compoundStatement
/*
	| DISPLAY LEFTPAR stringList RIGHTPAR checkSemicolon
	| PRINT LEFTPAR stringList RIGHTPAR checkSemicolon
	| ENTER LEFTPAR string COMMA identifier RIGHTPAR checkSemicolon
*/
	;

elseClause
	:
	| ELSE 
	    {
              emitCode("else \n");
	      increaseIndent();
              icnt ++;
              PUSH(icnt);
	    }
          statement	
            {
              decreaseIndent();
              if (icnt == POP()) 
                vcwarning("expression with little effect");
            }
	;

/* enter("xxxx %8a ", digin3, mask); 		*/
/* on command switch digin3 allow 0x7f */

expressionStatement
	: checkSemicolon		{ emitCode(";\n"); icnt++; }
	| expression checkSemicolon
          {
/*            showExpressionTree($1, 0);    */

            codeExpression(fpCurrent, $1, 0, 0);
            emitCode(";\n");
            freeExpression($1);
          }
	;

expression
	: assignmentExpression		{ $$ = $1; }
	| expression COMMA assignmentExpression	   
		{ $$ = cExprTree(cExprNode(_comma, 0, 0, 0), $1, $3); }
	;

assignmentExpression
	: conditionalExpression		{ $$ = $1; }
	| unaryExpression assignmentOperator assignmentExpression
					{ $$ = cExprTree($2, $1, $3); }
	;

assignmentOperator
	: ASSIGN
          {
            $$ = cExprNode(_assign, 0, 0, 0);
            icnt ++;
          }
        | ASSIGN_MUL { $$ = cExprNode(_assignMul, 0, 0, 0); icnt ++; }
        | ASSIGN_DIV { $$ = cExprNode(_assignDiv, 0, 0, 0); icnt ++; }
        | ASSIGN_MOD { $$ = cExprNode(_assignMod, 0, 0, 0); icnt ++; }
        | ASSIGN_ADD { $$ = cExprNode(_assignAdd, 0, 0, 0); icnt ++; }
        | ASSIGN_SUB { $$ = cExprNode(_assignSub, 0, 0, 0); icnt ++; }
        | ASSIGN_SR  { $$ = cExprNode(_assignSr, 0, 0, 0); icnt ++; }
        | ASSIGN_SL  { $$ = cExprNode(_assignSl, 0, 0, 0); icnt ++; }
        | ASSIGN_AND { $$ = cExprNode(_assignAnd, 0, 0, 0); icnt ++; }
        | ASSIGN_XOR { $$ = cExprNode(_assignXor, 0, 0, 0); icnt ++; }
        | ASSIGN_OR  { $$ = cExprNode(_assignOr, 0, 0, 0); icnt ++; }
	;

conditionalExpression
	: logicalOrExpression		{ $$ = $1; }
	| logicalOrExpression QUESTION_MARK expression COLON conditionalExpression
	  {
	    $$ = cExprTree(cExprNode(_if, 0, 0, 0), $1, 
			cExprTree(0, $3, $5));
	  }
	;

logicalOrExpression
	: logicalAndExpression		{ $$ = $1; }
	| logicalOrExpression OR logicalAndExpression
			{ $$ = cExprTree(cExprNode(_or, 0, 0, 0), $1, $3); }
	;

logicalAndExpression
	: inclusiveOrExpression		{ $$ = $1; }
	| logicalAndExpression AND inclusiveOrExpression
			{ $$ = cExprTree(cExprNode(_and, 0, 0, 0), $1, $3); }
	;

inclusiveOrExpression
	: exclusiveOrExpression		{ $$ = $1; }
	| inclusiveOrExpression BIT_OR exclusiveOrExpression
	  { $$ = cExprTree(cExprNode(_bitOr, 0, 0, 0), $1, $3); }
	;

exclusiveOrExpression
	: andExpression			{ $$ = $1; }
	| exclusiveOrExpression BIT_XOR andExpression
	  { $$ = cExprTree(cExprNode(_bitXor, 0, 0, 0), $1, $3); }
	;

andExpression
	: equalityExpression		{ $$ = $1; }
	| andExpression AMPERSAND equalityExpression
	  { $$ = cExprTree(cExprNode(_bitAnd, 0, 0, 0), $1, $3); }
	;

equalityExpression
	: relationalExpression		{ $$ = $1; }
	| equalityExpression EQUAL relationalExpression
		{ $$ = cExprTree(cExprNode(_equal, 0, 0, 0), $1, $3); }
	| equalityExpression NOTEQUAL relationalExpression
		{ $$ = cExprTree(cExprNode(_notequal, 0, 0, 0), $1, $3); }
	;

relationalExpression
	: shiftExpression		{ $$ = $1; }
	| relationalExpression LT additiveExpression
		{ $$ = cExprTree(cExprNode(_lt, 0, 0, 0), $1, $3); }
	| relationalExpression GT additiveExpression
		{ $$ = cExprTree(cExprNode(_gt, 0, 0, 0), $1, $3); }
	| relationalExpression LTE additiveExpression
		{ $$ = cExprTree(cExprNode(_lte, 0, 0, 0), $1, $3); }
	| relationalExpression GTE additiveExpression
		{ $$ = cExprTree(cExprNode(_gte, 0, 0, 0), $1, $3); }
	;

shiftExpression
	: additiveExpression		{ $$ = $1; }
	| shiftExpression SL additiveExpression
		{ $$ = cExprTree(cExprNode(_sl, 0, 0, 0), $1, $3); }
	| shiftExpression SR additiveExpression
		{ $$ = cExprTree(cExprNode(_sr, 0, 0, 0), $1, $3); }
	;

additiveExpression
	: multiplicativeExpression	{ $$ = $1; }
	| additiveExpression PLUS multiplicativeExpression
		{ $$ = cExprTree(cExprNode(_plus, 0, 0, 0), $1, $3); }
	| additiveExpression MINUS multiplicativeExpression
		{ $$ = cExprTree(cExprNode(_minus, 0, 0, 0), $1, $3); }
	;

multiplicativeExpression
	: unaryExpression		{ $$ = $1; }
	| multiplicativeExpression MUL unaryExpression
		{ $$ = cExprTree(cExprNode(_mul, 0, 0, 0), $1, $3); }
	| multiplicativeExpression DIV unaryExpression
		{ $$ = cExprTree(cExprNode(_div, 0, 0, 0), $1, $3); }
	| multiplicativeExpression MOD unaryExpression
		{ $$ = cExprTree(cExprNode(_mod, 0, 0, 0), $1, $3); }
	;

unaryExpression
	: postfixExpression		{ $$ = $1; }
	| PLUSPLUS unaryExpression
	  { 
            $$ = cExprTree(cExprNode(_plusplus, 0, 0, 0), 0, $2);
            icnt ++;
          }
	| MINUSMINUS unaryExpression
	  {
            $$ = cExprTree(cExprNode(_minusminus, 0, 0, 0), 0, $2);
            icnt ++;
          }
	| unaryOperator unaryExpression        %prec UMINUS
		{ $$ = cExprTree($1, 0, $2); }
	;

unaryOperator
	: NOT		{ $$ = cExprNode(_not, 0, 0, 0); }
	| MINUS		{ $$ = cExprNode(_minus, 0, 0, 0); }
        | CMPL          { $$ = cExprNode(_cmpl, 0, 0, 0); }
	;

postfixExpression
	: primaryExpression	{ $$ = $1; }
	| postfixExpression LEFTBRACKET expression RIGHTBRACKET
		{ $$ = cExprTree(cExprNode(_vector, 0, 0, 0), $1, $3); }
	| postfixExpression LEFTPAR RIGHTPAR
	    {
		checkFunctionId($1);
		$$ = cExprTree(cExprNode(_func, 0, 0, 0), $1, 0);
                icnt ++;
	    }
	| postfixExpression LEFTPAR argumentExpressionList RIGHTPAR
	    {
		checkFunctionId($1);
		$$ = cExprTree(cExprNode(_func, 0, 0, 0), $1, $3);
                icnt ++;
	    }
	| postfixExpression PLUSPLUS
            {
                $$ = cExprTree(cExprNode(_plusplus, 0, 0, 0), $1, 0);
                icnt ++;
            }
	| postfixExpression MINUSMINUS
	    {
                $$ = cExprTree(cExprNode(_minusminus, 0, 0, 0), $1, 0);
                icnt ++;
            }
	;


primaryExpression
	: identifier	{ $$ = cExprTree($1, 0, 0); }
	| constant	{ $$ = cExprTree($1, 0, 0); }
	| string	{ $$ = cExprTree($1, 0, 0); }
	| LEFTPAR expression RIGHTPAR	{ $$ = $2; }
	;

string	: QUOTEDSTRING  {
			    $$ = cExprNode(_string_const, 0,
				(char *) strcpy(malloc(strlen(this_string) + 1),
				    this_string),
                                0 /* tells: use already allocated storage */);
			}
/* 	
	| STRING_CONST_ID	{ $$ = cExprNode(_string, $1, 0, 0); }
*/
	;

argumentExpressionList
	: assignmentExpression		{ $$ = $1; }
	| argumentExpressionList COMMA assignmentExpression
		{ $$ = cExprTree(cExprNode(_comma, 0, 0, 0), $1, $3); }
	;

constant_and_rel
        : constant              { $$ = $1; }
        | relTime               {
                                  long temp;
                                  temp = $1;
                                  $$ = cExprNode(_int_const, 0, (char *) &temp, 
                                              _SIZE_OF_INT);
                                }
        ;

constant: INTEGER_CONSTANT  	{ 
                                  $$ = cExprNode(_int_const, 0, (char *) &cval, 
                                              _SIZE_OF_INT);
                                }
	| CHARACTER_CONSTANT	{
                                  $$ = cExprNode(_char_const, 0, (char*) &cval, 
                                              _SIZE_OF_CHAR);
                                }
	| FLOATING_CONSTANT	{
                                  $$ = cExprNode(_float_const, 0, (char*)&cfval,
                                              _SIZE_OF_FLOAT);
                                }
	| INT_CONST_ID		{ $$ = cExprNode(_ident, cref, 0, 0); }
	| FLOAT_CONST_ID	{ $$ = cExprNode(_ident, cref, 0, 0); }
	| CHAR_CONST_ID		{ $$ = cExprNode(_ident, cref, 0, 0); }
	;

identifier			/* not really IDENTIFIER but known id */
	: INT_ID		{ $$ = cExprNode(_ident, cref, 0, 0); }
	| FLOAT_ID		{ $$ = cExprNode(_ident, cref, 0, 0); }
	| CHAR_ID		{ $$ = cExprNode(_ident, cref, 0, 0); }
	| CALENDAR_ID		{ $$ = cExprNode(_ident, cref, 0, 0); }
	| INT_FUNC_ID		{ $$ = cExprNode(_ident, cref, 0, 0); }
	| FLOAT_FUNC_ID		{ $$ = cExprNode(_ident, cref, 0, 0); }
	| CHAR_FUNC_ID		{ $$ = cExprNode(_ident, cref, 0, 0); }
	| INT_VEC_ID		{ $$ = cExprNode(_ident, cref, 0, 0); }
	| FLOAT_VEC_ID		{ $$ = cExprNode(_ident, cref, 0, 0); }
	| CHAR_VEC_ID		{ $$ = cExprNode(_ident, cref, 0, 0); }
	| INT_REF_ID		{ $$ = cExprNode(_ident, cref, 0, 0); }
	| FLOAT_REF_ID		{ $$ = cExprNode(_ident, cref, 0, 0); }
	| CHAR_REF_ID		{ $$ = cExprNode(_ident, cref, 0, 0); }

	| UNKNOWN_ID		{ $$ = (exprNode *) 0;
					vcerror("identifier not declared");
				}
	| error 		{ $$ = (exprNode *) 0;
					vcerror("expected identifier");
				}
	;

/*
!  just for checking errors
*/

checkSemicolon
	: SEMICOLON
	| error { vcwarning("Added ';' to previous line"); yyerrok; }
	;

checkModuletype_id
	: MODULETYPE_ID
	| error { vcerror("module type not defined"); yyerrok; }
	;

checkLEFTPAR 
	: LEFTPAR
	| error { vcerror("expected a '('"); yyerrok; }
	;

checkRIGHTPAR
	: RIGHTPAR
	| error { vcerror("expected a ')'"); yyerrok; }
	;

checkASSIGN
	: ASSIGN
	| error { vcerror("expected a '='"); yyerrok; }
	;

checkIS
	: IS
	| error { vcerror("expected keyword 'IS'"); yyerrok; }
	;

checkAND 
	: AND
	| error { vcerror("expected keyword 'AND'"); yyerrok; }
	;

checkWHEN 
	: WHEN
	| error { vcerror("expected keyword 'WHEN'"); yyerrok; }
	;

checkDO
	: DO
	| error
          {
	    vcerror("expected keyword 'DO'");
/*            printf("yytext = '%s'\n", yytext);  */
            yyerrok;
          }
	;

checkTHEN
	: 
        | THEN
        /*	| error { vcerror("expected keyword 'THEN'"); yyerrok; }  */
	;

%%
#ifdef OSK
#include "lex.yy.c"
#else
#include "lex_yy.c"
#endif



