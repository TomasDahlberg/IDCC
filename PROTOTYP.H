/* prototype.h  1992-10-14 TD,  version 1.3 */
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
! prototype.h
! Copyright (C) 1991,1992 IVT Electronic AB.
*/


#ifdef OSK
#undef ANSI_C
#else
#define ANSI_C
#endif

#define ProgramVersion "1.66"
#define MajorVersion 1
#define MinorVersion 66

#ifdef ANSI_C
/*
!   symtable.c
*/
isAssign(opCode operator);

void traverseTree_n_init(idIdent *idRef, exprTree *t, int *index, int global);

void emitCalendarDef(exprTree *expr);

void emitFunctionFkn(idIdent *dref);

void makeScreenUsage(exprTree *expr);

void parseCalendar(char *line);

void initArray(idIdent *idRef, int global);

void traverseCalendar(exprTree *expr);


declType newDeclSpec(storageType storage, idType type, long nodeNo);

/* int getSize(idIdent *idRef);  */

int emitType(FILE *fp, idType type);

idIdent *lookUpInScope(symTable *scope, char *id);

void insertLineAccessed(idIdent *idRef, int accessedAtLine);

idIdent *lookUpIdent(char *id);

void declareGlobalId(char *cid, idType type);

idIdent *declareId(char *id, declType declSpec, idInfo *info, symTable *scope);

idIdent *insertId(symTable *scope, idIdent *idRef);

void initSymbolTable(void);

void incScope(char *name, int no);

void decScope(void);

void escapeOuterScope(void);

void makeScreenArray(void);

int makeScreen(char *name, int fromId, int dir, symLink *ref);

void tprint(formelList *tree, int deep);

void codeCastedArgumentList(FILE *fp, formelList *castTree, exprTree *expr, 
                int anyCastList, int onlyConstant);

formelList *cFormelList(idType node, formelList *left, formelList *right, char*);

exprTree *cExprTree(exprNode *node, exprTree *left, exprTree *right);

exprNode *cExprNode(opCode operator, idIdent *ref, char *opConst, int opSize);

int precedence(opCode op1, opCode op2);

int priority(opCode operator);

int expressionContains(exprTree *expr, opCode operator);

void freeExpression(exprTree *expr);

void checkForEqualSign(exprTree *expr);

void codeExpression(FILE *fp, exprTree *expr, int onlyConstant, int *lhs);

void makeConstDefinitions(void);

void makeGlobalSymTable(void);

idType cnv2const(idType tp);

idType cnv2func(idType tp);

idType cnv2vec(idType tp);

void checkIntOrFloat(idType tp);

filterType *newFilterSpec(idIdent *idRef, exprTree *expr);

bindingType *newBindingSpec(idIdent *idRef, int channelNo, exprTree *duration);

void emitBinding(idIdent *idRef, filterType *filter, bindingType *binding);

idInfo *newInfoBinding(filterType *filter, bindingType *binding);

idInfo *newFormelParameters(formelList *formelParameters, char *name);

void emitScanBindings(void);

void checkFunctionId(exprTree *expr);

void emitDeclarationList(void);

unsigned short swapword(unsigned short w);
unsigned long  swaplong(unsigned long  w);
void emitMetaModule(void);

int emitDebugStatement(void);

void emitActualList(FILE *fp, formelList *f, int *itm);
/*
!     emitCode.c 
*/
void increaseIndent(void);
void decreaseIndent(void);
void emitIndent2(FILE *fp);
void emitIndent(void);
void emitCode2(FILE *fp, char *code);
void emitCode(char *code);
void emitScanHeadings(void);
void emitScanEndings(void);
void emitComment(char *cmt);
int  checkAndAllocate(int alarmNo, int vector[]);
void emitAlarmStart(int alarmNo, exprTree *expr, exprTree *duration, int index, exprTree *class);
void emitAlarmMiddle(int alarmNo, int index);
void emitAlarmEnd(int alarmNo, int index, exprTree *class);
void emitEventStart(int EventNo, exprTree *expr, exprTree *duration);
void emitEventMiddle(int EventNo);
void emitEventEnd(int EventNo);

/*
!   calendar.c
*/
long orColorExpression(long a, long b);
void setCalendar(idInfo *currentCalendar, struct _btTime *bt, 
                  long every, long onoffState);
idInfo *newCalendar(long range, long resolution, long baseTime);

/*
!   main.c
*/
void changes(void);
void usage(void);
void error(char *s, char *p);
int get_it(void);
int yyerror(char *s);
void vcfatal(char *s);
void vcerror(char *s);
void vcwarning(char *s);
void doSubFile(char *includefile, int lib);
int yywrap(void);

char *getMem(long s);
void returnMem(char *s);
void printCopyright(void);
/*
!   y_tab.c
*/
int yyparse(void);
int yyinput(void);
int yylook(void);
int yyback(int *p, int m);
int yylex(void);

/*
!   icode.c
*/
void insertJmpAdr(long where, long what);
long getIcodePtr(void);
void emitIcode(int typ);
void emitIcode_jmp(int loc);
void emitIcodeExpression(exprTree *expr);

/*
!   encaps.c
*/
encaps(char *listFile);
double swapdouble(double d);
updateSize(FILE *fp, long siz);
emitHeader(FILE *fp);
emitTrailer(FILE *fp);

#else
/*
!   symtable.c
*/
void emitActualList();
int isAssign();
void traverseTree_n_init();
void emitCalendarDef();
void emitFunctionFkn();
void makeScreenUsage();
void parseCalendar();
void initArray();
void traverseCalendar();

declType newDeclSpec();
/* int getSize(); */
int emitType();
idIdent *lookUpInScope();
void insertLineAccessed();
idIdent *lookUpIdent();
void declareGlobalId();
idIdent *declareId();
idIdent *insertId();
void initSymbolTable();
void incScope();
void decScope();
void escapeOuterScope();
void makeScreenArray();
int makeScreen();
void tprint();
void codeCastedArgumentList();
formelList *cFormelList();
exprTree *cExprTree();
exprNode *cExprNode();
int precedence();
int priority();
int expressionContains();
void freeExpression();
void checkForEqualSign();
void codeExpression();
void makeConstDefinitions();

void makeGlobalSymTable();
idType cnv2const();
idType cnv2func();
idType cnv2vec();
void checkIntOrFloat();
filterType *newFilterSpec();
bindingType *newBindingSpec();
void emitBinding();
idInfo *newInfoBinding();
idInfo *newFormelParameters();
void emitScanBindings();
void checkFunctionId();
void emitDeclarationList();

unsigned short swapword();
unsigned long  swaplong();
void emitMetaModule();
/* 
!     emitCode.c 
*/
void increaseIndent();
void decreaseIndent();
void emitIndent2();
void emitIndent();
void emitCode2();
void emitCode();
void emitScanHeadings();
void emitScanEndings();
void emitComment();
int  checkAndAllocate();
void emitAlarmStart();
void emitAlarmMiddle();
void emitAlarmEnd();

/*
!   calendar.c
*/
void deceaseIndent();
long orColorExpression();
void setCalendar();
idInfo *newCalendar();

/*
!   main.c
*/
void changes();
void usage();
void error();
int get_it();
int yyerror();
void vcerror();
void vcwarning();
void doSubFile();
int yywrap();
void printCopyright();

/*
!   y_tab.c
*/
int yyparse();
int yyinput();
int yylook();
int yyback();
int yylex();

char *getMem();
void returnMem();


#endif



#define malloc    getMem
#define free      returnMem
