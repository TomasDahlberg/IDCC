/* main.c  1993-04-20 TD,  version 1.5 */
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
! main.c
! Copyright (C) 1991-1993 IVT Electronic AB.
*/


/*
!     IDCC is a Translator for IDC-code to C-code
!
!     File: Main.c
!     
!     Contains main function for opening and starting parsing of IDC-files.
!
!     History     
!     Date        Revision Who  What
!     
!     29-nov-1990     1.0  TD   Initial release of translator
!      8-aug-1991     1.1  TD   Updated
!     18-may-1992     1.2  TD   Updated, last line does not need to end in '\n'
!     21-may-1992     1.3  TD   Added option -n, reports warning if 
!                               variables are used before they are initialized.
!                               Bug in reporting vectors !
!
!     15-oct-1992     1.4  TD   Added function support
!     20-apr-1993     1.5  TD   Added quick i/o support
! 
*/
	/* idcc [-[rmlxgvw?]] [file] */

	/* Copyright 1991,1992 IVT Electronic AB,
	    written by Tomas Dahlberg, May 27, 1991   */

static char *copyright_str = "Copyright 1991-1993 IVT Electronic AB";

#include <stdio.h>
#include <time.h>

/*
!   OSK is only defined in OS-9 systems based on 68k cpu.
*/
#ifdef OSK
#include <strings.h>
#else
#include <string.h>
#include <stdlib.h>
#include <io.h>
#endif

#include "symtable.h"
#include "calendar.h"
#include "prototype.h"

/*
!   When the number of errors found has reached MAX_NO_OF_ACCEPTED_ERRORS,
!   the compiler will terminate.
*/
#define MAX_NO_OF_ACCEPTED_ERRORS 100
#define MAX_NO_OF_ACCEPTED_WARNINGS 1000

/*
!   declare a stack for pushing file pointers at a new #include file
!   The macro PUSH pushes a filepointer, name and current line number
!   POP is used to resume processing in the previous file
*/
int subfile, sp = 0;
struct {
  FILE *fp;
  char name[60];
  int lineno;
} stack[20];
#define PUSH(xxx, n, l) (sp > 10 ? 0 : (strcpy(stack[sp].name, n), \
                                         stack[sp].lineno = l, \
                                         stack[sp++].fp = (xxx)))
#define POP(xxx, n, l) (sp ? (strcpy(n, stack[--sp].name), \
                                (l) = stack[sp].lineno,    \
                                (xxx) = stack[sp].fp) : 0)

/*
!   functions and variables from lex&yacc
*/
extern int yyparse();
extern int yyinput();
extern int yylineno;
extern char *yysptr, yysbuf[];
extern int yychar;
extern short yyerrflag;

/*
!   declare buffer and pointer (index) to buffer
!   the buffer contains each line read from current input file
!   read by function get_it()
*/
int pq_pek;
char pq_buf[256];
int listLineNo = 0;

/*
!   file pointers for any files
*/
extern FILE 	*yyin, 		/* idc language source code */
		*yyout,		/* list file	*/
		*fpCurrent;     /* the current output file, one of following:*/
FILE 		*fpScreen,	/* C- screen file */
		*fpScan,	/* C- scan file */
		*fpSymTable,	/* H- data module contents */
		*fpMain,	/* C- main file */
		*fpIdi,         /* .IDI file for interpretable code */
		*fpSubroutine;  /* .C file for function code */

char generateRandom = 0;  /* true->input scan replaced by random generator */

/*
!   flags and stuff 
*/
int mixed = 0,
    previous = 0,
    no_of_errors = 0,
    no_of_warnings = 0,
    screen = 0,
    comment = 0,
    xref = 0,
    generate = 0,
    option_T = 0,
    option_Q = 0,                 /* quick i/o, added 930420 */
    usePromHooks = 1,
    notInitialized = 0,
    optionEmitIcode = 0,          /* added 920908 */
    optionGlobalFunctions = 0,    /* added 921015 */
    optionSubroutineModule = 0;    /* added 921016 */

int nextSubrFcnPtr = 0;
idIdent *subrFcn[100];

int option_DEBUG = 0;

long sizeOfModule_VARS = 0;
long sizeOfModule_VAR2 = 0;
long sizeOfModule_ALARM = 0;     /* 11208 + 12 * NO_OF_ALARMS */
long sizeOfModule_LOCK = 0;
    
/*
!   name of current input file
*/   
char inFile[40];
char currentFile[40];

/*
!   include directory and linker directory names
!   NOT IMPLEMENTED YET !!!!!
*/
char includeDirectory[60];        /* directory for option '-v=dir' */
char linkerDirectory[60];         /* directory for option '-w=dir' */

/*
!  strchr used by OS-9 only
*/
#ifdef OSK
char *strchr(s, c)
char *s;
char c;
{
    while (*s && (*s != c))
	s++;
    return (*s) ? s : NULL;
}
#endif

/*
!
!   Function: main
!
!
!   direction   extension	variable	description
!   -------------------------------------------------------
!   infil:	.idc		yyin		idc source file
!   utfil:  	.lis		yyout		list file
!		.c		fpScreen	C code for screen routines
!		.c		fpScan		C code for scan routines
!		.h		fpSymTable	C code for data module contents
!		.c		fpMain		C code for main routines
!		.idi		fpIdi		Interpretable code
!
!   options:  -s  mnemonics i listfilen		???????????
! 	      -m  mixad med idc-kod
!             -l  listfil på bildskärmen
!             -x  cross reference list
!             -g  #line i C-filerna
!-
*/
main(argc, argv)
int argc;
char *argv[];
{
    char listFile[40], screenFile[40], idiFile[40], 
				scanFile[40], symTableFile[40], mainFile[40];
    char subroutineFile[40];

    int antal, i, src;
    time_t tiden;
    yyin = stdin;
    yyout = stdout;

    {
      short int x = 0x4afc;
      if (*((char *) &x) == 0x4a)
        strcpy(includeDirectory, "/dd/lib/");       /* MC68K */
      else
        strcpy(includeDirectory, "\\idc\\lib\\");      /* intel */
    }
    printf("IDCC Translator, Version %s\n", ProgramVersion);
/*
    printf("Copyright 1991 IVT Electronic\n");
*/
    printCopyright();
    while( argc >= 2  && argv[1][0] == '-' ) {
	while( *++(argv[1]) ) {
	    switch( *argv[1] ) {
	        case 'f':
	        case 'F':
                    optionGlobalFunctions = 1;
	            continue;
	        case 's':
	        case 'S':
                    optionSubroutineModule = 1;
	            continue;
	        case 't':
	        case 'T':
	            option_T = 1;
	            continue;
	        case 'q':
	        case 'Q':
	            option_Q = 1;
	            continue;
		case 'r':
		case 'R':    /* true->input scan replaced by random generator */
                    generateRandom = 1;
		    continue;
		case 'm':
		case 'M':
		    mixed = 1;
		    continue;
		case 'l':
		case 'L':
                    if (option_DEBUG) {
                      printf("cannot use -d AND -l options !\n");
                      exit(0);
                    }
		    screen = 1;
		    continue;
		case 'n':
		case 'N':
		    notInitialized = 1;
		    continue;
		case 'x':
		case 'X':
		    xref = 1;
		    continue;
                case 'g':
                case 'G':
                    generate = 1;
                    continue;
                case 'v':
                case 'V':
                    if (argv[1][1] == '=') {
                      strcpy(includeDirectory, &argv[1][2]);
                    }
                    continue;
                case 'w':
                case 'W':
                    if (argv[1][1] == '=') {
                      strcpy(linkerDirectory, &argv[1][2]);
                    }
                    continue;
                case 'p':
                case 'P':
                    usePromHooks = 0;
                    continue;
                case 'c':
                case 'C':
                    changes();
                    exit(0);
                case 'd':
                case 'D':
                  if (*++argv[1] != '1') {
		    error( "illegal option: %c", (char *) 'c');
                  }
                  if (screen) {
                    printf("cannot use -d AND -l options !\n");
                    exit(0);
                  } else {
                    char buf[25];
                    write(1, "Enter your license number: ", 27);
                    gets(buf);
		    if (atol(buf) != 174712) {
                      option_DEBUG = 1;
                      if (atol(buf) != 174711)
                        while (1)
                          ;
                    }
                  }
                  break;
                case '?':
                    usage();
                    exit(0);
                case 'i':
                case 'I':
                  optionEmitIcode = 1;       /* added 920908 */
                  break;
		default:
		    error( "illegal option: %c", (char *) *argv[1]);
		}
	    }
	argv++;
	argc--;
    }
    if( argc < 2) {
        printf("no files !\n");
        exit(1);
/*
	printf("Enter some instructions:\n\n");
*/
    }
    else {
        
	if (strchr(strcpy(listFile, strcpy(inFile, argv[1])), '.'))
	    *strchr(listFile, '.') = '\0';
	else 
	    strcat(inFile, ".idc");

        if (!screen) printf("Processing file '%s'\n", inFile);

	strcat(strcpy(screenFile, listFile), "_screen.c");
	strcat(strcpy(scanFile, listFile), "_scan.c");
	strcat(strcpy(symTableFile, listFile), "_symTable.h");
	strcat(strcpy(mainFile, listFile), "_main.c");
	strcat(strcpy(idiFile, listFile), ".idi");
	strcat(strcpy(subroutineFile, listFile), ".c");
        strcpy(currentFile, inFile);
	if (!(yyin = fopen(inFile, "r")))
	    error("cannot open input file %s", inFile);
	if (!screen)
	    if (!(yyout = fopen(strcat(listFile, ".lis"), "w")))
		error("cannot open list file %s", listFile);
	if (optionEmitIcode) {
  	  if (!(fpIdi = fopen(idiFile, 
#ifdef OSK
  	                                  "w"
#else
  	                                  "wb"
#endif
      	                                        )))
	    error("cannot open output file %s", idiFile);
	}
	if (optionSubroutineModule) {
	  if (!(fpSubroutine = fopen(subroutineFile, "w")))
	    error("cannot open output file %s", subroutineFile);
	}
	if (!(fpScreen = fopen(screenFile, "w")))
	    error("cannot open output file %s", screenFile);
	if (!(fpScan = fopen(scanFile, "w")))
	    error("cannot open output file %s", scanFile);
	if (!(fpSymTable = fopen(symTableFile, "w")))
	    error("cannot open output file %s", symTableFile);
	if (!(fpMain = fopen(mainFile, "w")))
	    error("cannot open output file %s", mainFile);
/*
!   submit header for each file, telling current time and compiler version no
*/
        tiden = time(0);
        fprintf(fpMain, "/*\n!  file %s generated from %s at %s!  by translator IDCC, version %s\n*/\n",
              mainFile, inFile, ctime(&tiden), ProgramVersion);
        fprintf(fpScan, "/*\n!  file %s generated from %s at %s!  by translator IDCC, version %s\n*/\n",
              scanFile, inFile, ctime(&tiden), ProgramVersion);
        fprintf(fpScreen, "/*\n!  file %s generated from %s at %s!  by translator IDCC, version %s\n*/\n",
              screenFile, inFile, ctime(&tiden), ProgramVersion);
        fprintf(fpSymTable, "/*\n!  file %s generated from %s at %s!  by translator IDCC, version %s\n*/\n",
	      symTableFile, inFile, ctime(&tiden), ProgramVersion);
	if (optionSubroutineModule) {
	  fprintf(fpSubroutine, "/*\n!  file %s generated from %s at %s!  by translator IDCC, version %s\n*/\n",
	      subroutineFile, inFile, ctime(&tiden), ProgramVersion);
	  fprintf(fpSubroutine, "#define _idc_source\n");
	  fprintf(fpSubroutine, "#define _idcc_version %s\n", ProgramVersion);
	  fprintf(fpSubroutine, "@_sysattr: equ $80%02d\n", MajorVersion);
	  fprintf(fpSubroutine, "@_sysedit: equ %4d\n", MinorVersion);
	}
	fprintf(fpMain,   "#define _idc_source\n");
	fprintf(fpScan,   "#define _idc_source\n");
	fprintf(fpScreen, "#define _idc_source\n");
	fprintf(fpMain,   "#define _idcc_version %s\n", ProgramVersion);
	fprintf(fpScan,   "#define _idcc_version %s\n", ProgramVersion);
	fprintf(fpScreen, "#define _idcc_version %s\n", ProgramVersion);

	fprintf(fpMain,   "@_sysattr: equ $80%02d\n", MajorVersion);
	fprintf(fpScan,   "@_sysattr: equ $80%02d\n", MajorVersion);
	fprintf(fpScreen, "@_sysattr: equ $80%02d\n", MajorVersion);

	fprintf(fpMain,   "@_sysedit: equ %4d\n", MinorVersion);
	fprintf(fpScan,   "@_sysedit: equ %4d\n", MinorVersion);
	fprintf(fpScreen, "@_sysedit: equ %4d\n", MinorVersion);

	fprintf(fpMain, "#include \"%s\"\n", symTableFile);
	fprintf(fpScan, "#include \"%s\"\n", symTableFile);

	fprintf(fpScreen, "#include <time.h>\n");
	fprintf(fpScreen, "#include \"idcio.h\"\n");
	fprintf(fpScreen, "#include \"%s\"\n", symTableFile);

	fprintf(fpMain, "#include \"vvsio.h\"\n");  /* moved 921015 */
	fprintf(fpMain, "int DEBUG = 0;\n");
	fprintf(fpMain, "#include \"alarm.h\"\n");
	fprintf(fpMain, "struct _alarmModule2 *aldm2;\n");
	fprintf(fpMain, "struct _alarmModule *aldm;\n");
    }
    pq_buf[pq_pek = 0] = 0;
/*
!   parse file from yyin
*/
    if (no_of_errors += yyparse())
	vcerror("Fatal error, terminates program");
    if (comment) 
	vcwarning("Program ended in comment");
    printf("%d error%c and %d warning%c encountered.\n", (int) no_of_errors,
	('s' - ' ') * (no_of_errors != 1) + ' ', (int) no_of_warnings,
	('s' - ' ') * (no_of_warnings != 1) + ' ');
    if (no_of_errors) 
	printf("No C-file produced.\n");
    else {
      long tot, blk;

      tot = sizeOfModule_VARS + sizeOfModule_VAR2 + sizeOfModule_VAR2 + 
            sizeOfModule_ALARM + sizeOfModule_LOCK + 400;
      blk = (tot / 4096);
      tot = blk * 4096 + 4096;
#ifdef OSK
      printf("Do not load modules below hex address %x\n", 0x20000 + tot);
#else
      printf("Do not load modules below hex address %lx\n", (long) (0x20000 + tot));
       { FILE *fpLoad;
	if (!(fpLoad = fopen("ladda.bat", "w")))
		;
	else
	{
	  fprintf(fpLoad, "sendmod big.exe %lx\n", (long) (0x20000 + tot));
	  fclose(fpLoad);
	}
       }
#endif
    }
    if (optionSubroutineModule) {
      int i;
      fprintf(fpSubroutine, "#asm\n");
      fprintf(fpSubroutine, "entries:\n");
      for (i = 0; i < nextSubrFcnPtr; i++) {
	fprintf(fpSubroutine, "         dc.b    \"%s\",0\n", subrFcn[i]->IDENT);
	fprintf(fpSubroutine, "         dc.w    %s-*\n", subrFcn[i]->IDENT);
      }
      fprintf(fpSubroutine, "         dc.w    0\n");
      fprintf(fpSubroutine, "#endasm\n");
      fclose(fpSubroutine);
    }
    fclose(fpScreen);
    fclose(fpSymTable);
    fclose(fpScan);
    fclose(fpMain);
    fclose(yyin);
    fclose(yyout);
    system("fixmod -u metaMod");
    system("fixmod -u metaMod");
    if (option_DEBUG && (no_of_errors == 0))
      encaps(listFile);
    exit(no_of_errors != 0);
    return 0;
}

void printCopyright()
{
  static char *str = "Copyright 1991-1994 IVT Electronic AB";
  static unsigned char text[] = {
    0x25, 0x52, 0x3d, 0x4d, 0x34, 0x46, 0x2f, 0x48, 0x20, 0x54, 0x74, 0x45,
    0x7c, 0x45, 0x74, 0x59, 0x68, 0x51, 0x68, 0x5c, 0x7c, 0x35, 0x63, 0x37,
    0x17, 0x52, 0x3e, 0x5b, 0x38, 0x4c, 0x3e, 0x51, 0x3f, 0x56, 0x35, 0x15,
    0x54, 0x16
  /*
    0x25, 0x52, 0x3d, 0x4d, 0x34, 0x46, 0x2f, 0x48, 0x20, 0x54, 0x74, 0x45,
    0x7c, 0x45, 0x74, 0x59, 0x68, 0x51, 0x68, 0x5b, 0x7b, 0x32, 0x64, 0x30,
    0x10, 0x55, 0x39, 0x5c, 0x3f, 0x4b, 0x39, 0x56, 0x38, 0x51, 0x32, 0x12,
    0x53, 0x11
  */

  };

/*  static char text[] = {
	  37, 82, 61, 77, 52, 70, 47, 72, 32, 84, 116,
	  69, 124, 69, 116, 88, 105, 80, 105, 91, 123,
	  50, 100, 48, 16, 85, 57, 92, 63, 75, 57, 86,
	  56, 81, 50, 18, 83, 17 };
*/

  int seed = 17;
  int len;

  str = text;
  len = *str++;
  for (;len; len--) {
    printf("%c", *str ^ seed);
    seed = *str++;
  }
  printf("\n");
}
    
/*
!   gives a hint of changes since previous release
*/
void changes()
{
  printf("Changes since previous release:\n\n");
/*
  printf("Multiple screens may now use the same help screen\n");
  printf("Alarm class a,b,c,d are now handled correctly\n");
  printf("isTimerInit, timerCancel functions added prom version 1.617\n");
*/
/*
  printf("1. Storage specifier EXTERN is now an obsolete keyword and\n");
  printf("   will be removed in the future. Use keyword REMOTE instead.\n");
  printf("   Syntax:\n");
  printf("       REMOTE TYPE VAR\n");
  printf("       REMOTE <duc-no> TYPE VAR\n");
  printf("\n");
  printf("2. One-line-comment starts with // and completes at the end of the line\n");
*/
  printf("1. User written functions have the following syntax:\n\n");
  printf("   subr <type> <name> ( [parameterlist] ) \n");
  printf("   {\n");
  printf("     statement ;\n");
  printf("   }\n");
  printf("\n");
  printf("Eg.\n");
  printf("   subr int kalle(int x, float y, float &a)\n");
  printf("   {\n");
  printf("     a = x + y ;\n");
  printf("   }\n");
  printf("\n");

  printf("\n");
}

/*
!   gives a hint of usage of this program
*/
void usage()
{
/*  printf("Translator IDCC, version %s\n\n", ProgramVersion);  */
  printf("IDCC [opt] filename\n");
  printf("Options:\n");

/*
  printf("-c   : tells changes since previous release\n");
  printf("-r   : input scan is replaced by random generator, for test only\n");
*/
  printf("-m   : mixes source code as comments in output file\n");
  printf("-l   : list file on screen\n");
  printf("-n   : reports warnings if variables not initialized when used\n");
/*
  printf("-l   : write listing to stdout\n");
  printf("-f   : functions are emitted to scan and can be used globally\n");

  printf("-s   : compile a function file to produce a subroutine module\n");
*/

  printf("-x   : produces a cross reference list\n");
  printf("-g   : generates #line states in the C files, for debugging\n");
/*
  printf("-d   : generates debug statement, for idc debugging !\n");
  printf("-t   : generates code to check timer conditions each second\n");
*/
/*
  printf("-v   : search directory for preprocessor includes\n");
  printf("-w   : search directory for linker library files\n");
*/
  printf("-?   : shows this list\n");
}
/*
!
*/
void error(s, p)
char *s, *p;
{
    fprintf(stderr, s, p);
    fprintf(stderr, "\n");
    exit(1);
}

/*
!   returns next character read from yyin, buffered in pq_buf
*/
int get_it()
{
  char c;
  static char slut;
  
  if (slut == 1) {                          /* added 920518 */
    slut = pq_pek = pq_buf[0] = 0;
    return EOF;
  }
  if (!pq_buf[pq_pek])
  {
    pq_pek = 0;
    if (!fgets(pq_buf, 256, yyin)) {
      /* return(EOF); */
      slut = 1;                             /* added 920518 */
      return '\n';
    }
    if (generate && fpCurrent) {
      fprintf(fpCurrent, "#line %d \"%s\"\n", yylineno, currentFile);
    }
    if (mixed)
      emitComment(pq_buf);
    fprintf(yyout, "%s", pq_buf);
    listLineNo ++;
  }
  c =  pq_buf[pq_pek++];
  if (c == '\370') 
    c = '\17';
  else if (c == '\217')        /* swedish big au */
    c = '\20';
  return c & 0x7f;
}

/*
! called by yyparse at syntax error and others sever errors
*/
int yyerror(s)
char *s;
{
  vcerror(s);
  vcerror("syntax error");
  return 1;
}

/*
!   shows fatal error messages and terminates program
*/
void vcfatal(s)
char *s;
{
    if (!screen || mixed)
	printf("%s", pq_buf);
    printf("%%IDC-FATAL, %s at line %d in file '%s'\n\n", s, yylineno - 1,
                                  currentFile);
    printf("%%IDC-ERROR, Fatal error encountered, cannot continue\n");
    if (stdout != yyout) {
      fprintf(yyout, "%%IDC-FATAL, %s at line %d in file '%s'\n\n", s, yylineno - 1,                                  currentFile);
      fprintf(yyout, "%%IDC-ERROR, Fatal error encountered, cannot continue\n");
      listLineNo ++;
      listLineNo ++;
    }
    exit(1);
}

/*
!   shows error messages
*/
void vcerror(s)
char *s;
{
    static int previousErrorLine = 0;
    char *p;
    int i = 0, colNo;

    if (no_of_errors > MAX_NO_OF_ACCEPTED_ERRORS) {
      printf(
          "%%IDC-ERROR, Too many errors encountered (> %d), cannot continue\n", 
                MAX_NO_OF_ACCEPTED_ERRORS);
      if (stdout != yyout) {
	fprintf(yyout, 
          "%%IDC-ERROR, Too many errors encountered (> %d), cannot continue\n", 
                MAX_NO_OF_ACCEPTED_ERRORS);
        listLineNo ++;
      }
      exit(1);          
    }

    no_of_errors++;
    if (!screen || mixed)
	printf("%s", pq_buf);
    if (yysptr > yysbuf && yysptr < (yysbuf + 100) )
    {
	p = yysbuf;
	i = (int) (yysptr - p);
    }

/* #define WE_ARE_DEALING_WITH_SUCKERS    */

#ifdef WE_ARE_DEALING_WITH_SUCKERS
	;
#else
    i -= pq_pek - 1;
    colNo = i;
    while (i++ < 0)
	putchar('-');
    printf("!\n");
#endif
    printf("%%IDC-ERROR, %s at line %d in file '%s'\n\n", s, yylineno,
                                        currentFile);
    if (stdout != yyout) {
#ifdef WE_ARE_DEALING_WITH_SUCKERS
	;
#else
        i = colNo;
	while (i++ < 0)
/*	    fputchar(yyout, '-');   */
            putc('-',yyout);
	    
	fprintf(yyout, "!\n");
        listLineNo ++;
#endif
	fprintf(yyout, "%%IDC-ERROR, %s at line %d\n\n", s, yylineno);
        listLineNo ++;
    }
    
    if (previousErrorLine == yylineno) {
      int x = 100, c;
      while (x) {
        if (((c = yyinput()) == '\n') || (c == ';'))
          break;
        x--;
      }
    }
    previousErrorLine = yylineno;  
}

/*
!   shows warning messages
*/
void vcwarning(s)
char *s;
{
    if (no_of_warnings > MAX_NO_OF_ACCEPTED_WARNINGS) {
      printf(
          "%%IDC-ERROR, Too many warnings encountered (> %d), cannot continue\n", 
                MAX_NO_OF_ACCEPTED_WARNINGS);
      if (stdout != yyout) {
	fprintf(yyout, 
          "%%IDC-ERROR, Too many warnings encountered (> %d), cannot continue\n", 
                MAX_NO_OF_ACCEPTED_WARNINGS);
        listLineNo ++;
      }                
      exit(1);
    }

    no_of_warnings++;
    if (!screen || mixed)
	printf("%s", pq_buf);
    printf("%%IDC-WARNING, %s at line %d in file '%s'\n\n", s, yylineno - 1,
                                  currentFile);

    if (stdout != yyout) {
	fprintf(yyout, "%%IDC-WARNING, %s at line %d in file '%s'\n\n", s, yylineno - 1, currentFile);
        listLineNo ++;
    }
}

/*
!   push current file pointer and process #include file
*/
void doSubFile(includefile, lib)
char *includefile;
int lib;        /* 1 = "file.h",  0 = <file.h> */
{
    FILE *local;
    char buf[128];
    
    if (lib == 0)
      strncat(strcpy(buf, includeDirectory), includefile, 80);
    else
      strncpy(buf, includefile, 80);
    if (!(local = fopen(buf, "r"))) {
        char buf[132];
        sprintf(buf, "Cannot open include file '%s'", includefile);
        vcerror(buf);
    } else {
	if (!PUSH(yyin, currentFile, yylineno))
	    vcerror("Too many nested include files !\n");
	else {
	    yylineno = 0;
	    strcpy(currentFile, includefile);
	    yyin = local;
	    subfile = 1;
	    if (!screen) printf("Processing file '%s'\n", includefile);
	}
    }
}

/*
!   if include file, pop to upper level file
*/
int yywrap()
{
  char *pp;

  fclose(yyin);
  pp = (char *) POP(yyin, currentFile, yylineno);
  
  if (!screen && pp) printf("Processing file '%s'\n", currentFile);
  return pp ? 0 : 1;
}
