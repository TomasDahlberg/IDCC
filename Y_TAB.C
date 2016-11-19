
# line 72 "c.y"
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

# line 176 "c.y"
typedef union 
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
} YYSTYPE;
# define INCLUDE 257
# define SYSTEMID 258
# define LEFTPAR 259
# define RIGHTPAR 260
# define LEFTBRACE 261
# define RIGHTBRACE 262
# define LEFTBRACKET 263
# define RIGHTBRACKET 264
# define COLON 265
# define COMMA 266
# define SEMICOLON 267
# define PLUSPLUS 268
# define MINUSMINUS 269
# define MUL 270
# define DIV 271
# define MOD 272
# define PLUS 273
# define MINUS 274
# define LT 275
# define GT 276
# define LTE 277
# define GTE 278
# define ASSIGN 279
# define AMPERSAND 280
# define DOTDOTDOT 281
# define NOTEQUAL 282
# define AND 283
# define OR 284
# define EQUAL 285
# define NOT 286
# define CHAR 287
# define INT 288
# define FLOAT 289
# define CONST 290
# define EXTERN 291
# define REMOTE 292
# define ALIAS 293
# define EVENT 294
# define SUBR 295
# define RETURN 296
# define IF 297
# define THEN 298
# define ELSE 299
# define WHILE 300
# define DO 301
# define WHEN 302
# define BREAK 303
# define CONTINUE 304
# define SWITCH 305
# define CASE 306
# define DEFAULT 307
# define SCREEN 308
# define AT 309
# define ALARM 310
# define ON 311
# define CALENDAR 312
# define RIGHT 313
# define DOWN 314
# define HELP 315
# define MAIN 316
# define RANGE 317
# define RESOLUTION 318
# define BASE 319
# define MODULE 320
# define IS 321
# define CHANNEL 322
# define FILTER 323
# define MODULETYPE 324
# define INT_ID 325
# define FLOAT_ID 326
# define CHAR_ID 327
# define INT_FUNC_ID 328
# define FLOAT_FUNC_ID 329
# define CHAR_FUNC_ID 330
# define INT_CONST_ID 331
# define FLOAT_CONST_ID 332
# define CHAR_CONST_ID 333
# define INT_VEC_ID 334
# define FLOAT_VEC_ID 335
# define CHAR_VEC_ID 336
# define INT_REF_ID 337
# define FLOAT_REF_ID 338
# define CHAR_REF_ID 339
# define UNKNOWN_ID 340
# define CALENDAR_ID 341
# define SCREEN_ID 342
# define FILTER_ID 343
# define MODULETYPE_ID 344
# define SCREEN_ORDER 345
# define CHARACTER_CONSTANT 346
# define INTEGER_CONSTANT 347
# define FLOATING_CONSTANT 348
# define IDENTIFIER 349
# define STRING 350
# define QUOTEDSTRING 351
# define EVERY 352
# define DURATION 353
# define BETWEEN 354
# define OFF 355
# define RED 356
# define ORANGE 357
# define BLACK 358
# define DECADE 359
# define YEAR 360
# define MON 361
# define MONTH 362
# define DAY 363
# define HOUR 364
# define MIN 365
# define SEC 366
# define LEXICALERROR 367
# define ASSIGN_MUL 368
# define ASSIGN_DIV 369
# define ASSIGN_MOD 370
# define ASSIGN_ADD 371
# define ASSIGN_SUB 372
# define ASSIGN_SR 373
# define ASSIGN_SL 374
# define ASSIGN_OR 375
# define ASSIGN_AND 376
# define ASSIGN_XOR 377
# define BIT_OR 378
# define BIT_XOR 379
# define SR 380
# define SL 381
# define QUESTION_MARK 382
# define CMPL 383
# define UMINUS 384
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern short yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256

# line 1767 "c.y"

#ifdef OSK
#include "lex.yy.c"
#else
#include "lex_yy.c"
#endif



short yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
-1, 132,
	265, 168,
	274, 168,
	-2, 294,
-1, 211,
	261, 332,
	-2, 314,
-1, 325,
	309, 96,
	-2, 97,
-1, 357,
	262, 197,
	-2, 203,
-1, 383,
	262, 198,
	-2, 203,
-1, 426,
	256, 314,
	259, 314,
	263, 314,
	266, 314,
	267, 314,
	268, 314,
	269, 314,
	270, 314,
	271, 314,
	272, 314,
	273, 314,
	274, 314,
	275, 314,
	276, 314,
	277, 314,
	278, 314,
	279, 314,
	280, 314,
	282, 314,
	283, 314,
	284, 314,
	285, 314,
	368, 314,
	369, 314,
	370, 314,
	371, 314,
	372, 314,
	373, 314,
	374, 314,
	375, 314,
	376, 314,
	377, 314,
	378, 314,
	379, 314,
	380, 314,
	381, 314,
	382, 314,
	-2, 316,
	};
# define YYNPROD 335
# define YYLAST 903
short yyact[]={

 145, 126, 215, 384, 489, 455,  46, 133, 127, 385,
 402, 320, 392,  73, 272,  92, 360, 263, 131, 203,
 202, 194, 192, 378, 346, 326, 183, 393, 394, 395,
 264, 396, 397, 398, 399, 219, 220, 221, 426, 233,
 307,  94, 505, 219, 220, 221, 294, 512, 294, 147,
  82,  83, 253, 223, 496,  76,  88, 499,  55, 329,
 492, 300, 353, 352, 139, 351, 149,  74,  87, 153,
  33,  34,  35,  67,  31,  32,  64,  20, 422, 415,
 171, 155, 417, 418, 166, 419, 420, 423, 137, 421,
 223, 513, 453, 367,  58,  36,  56,  42,  45,  38,
 325,  51, 144, 193, 361, 327, 434,  96,  97,  98,
 100, 101, 102, 114, 115, 116, 103, 104, 105, 106,
 107, 108, 109,  99, 182, 514,  77, 293, 112, 111,
 113,  78, 209, 117,  43, 464, 429, 212, 110, 223,
  61,  94, 364, 234, 469, 474, 492, 218, 218, 140,
  82,  83, 237, 301, 302, 303,  88, 502, 230, 456,
 262, 379, 217, 197, 242,  89, 196, 195,  87, 172,
 173, 174, 175, 176, 177, 178, 181, 179, 180, 252,
   8, 114, 115, 116, 114, 115, 116, 114, 115, 116,
 143, 267, 308, 270, 386, 159, 112, 111, 113, 112,
 111, 113, 112, 132, 113,  60, 261,  96,  97,  98,
 100, 101, 102, 114, 115, 116, 103, 104, 105, 106,
 107, 108, 109,  99, 191, 161, 334, 135, 112, 132,
 113, 403, 509, 117, 241,  53, 297, 160, 219, 220,
 221, 376, 259, 333, 445, 214, 487, 309, 110, 504,
 164,  94, 169, 321, 163, 314, 389, 240, 218, 508,
  82,  83, 204, 205, 328,  89,  88, 335, 225, 258,
 148, 336, 134, 337, 158, 157, 110, 224,  87,  94,
 271, 147, 169, 218, 162,  33,  34,  35,  82,  83,
 198, 199, 200, 201,  88, 251, 265, 266, 206, 207,
 208, 350, 343, 148, 332, 470,  87, 331, 349, 322,
  36, 169, 273, 169, 147, 347, 141,  96,  97,  98,
 100, 101, 102, 114, 115, 116, 103, 104, 105, 106,
 107, 108, 109,  99, 439, 468, 362, 245, 112, 111,
 113, 169, 340, 117, 169,  96,  97,  98, 100, 101,
 102, 114, 115, 116, 103, 104, 105, 106, 107, 108,
 109,  99, 114, 115, 116, 391, 112, 111, 113, 342,
 377, 117, 309, 148, 323,  89, 406, 112, 111, 113,
 405, 355, 185, 238, 147, 356, 184, 412, 424, 410,
 400, 186, 187, 225, 259, 467, 425, 381, 228, 407,
 370, 169, 413,  89, 428, 330, 427, 148, 451, 318,
 259, 388, 380, 245, 342, 432, 431, 150, 147, 148,
 339, 169, 440, 411, 441, 317, 448, 437, 438, 146,
 147, 446, 140, 478,  33,  34,  35, 369,  31,  32,
 231, 450, 449, 156, 460, 461, 459, 462, 273, 457,
 458, 452, 435, 454, 138, 341, 463, 344, 169,  36,
 466, 342, 465, 354, 447, 211, 471, 259,  94, 374,
 305, 373, 213, 401, 304, 276, 476,  82,  83, 481,
 316, 169, 479,  88, 486, 482, 491, 483, 130, 244,
 491, 493, 317, 497, 500,  87, 503,  48, 168, 152,
  47, 164, 506, 110, 169, 163,  94, 256, 123, 515,
 134, 255, 517, 510, 516,  82,  83, 125,  95, 477,
   4,  88, 444, 119,  37,  72, 443, 495, 273, 120,
 323,  90,  66,  87,  96,  97,  98, 100, 101, 102,
 114, 115, 116, 103, 104, 105, 106, 107, 108, 109,
  99,  85, 118, 442, 475, 112, 132, 113, 368, 416,
 117, 128,  79, 501,  33,  34,  35, 494,  31,  32,
 480, 473,  96,  97,  98, 100, 101, 102, 114, 115,
 116, 103, 104, 105, 106, 107, 108, 109,  99,  36,
 414, 387,  89, 112, 111, 113, 167, 110, 117, 383,
  94,  33,  34,  35,  63,  31,  32, 382, 357,  82,
  83, 232,  54, 299, 298,  88, 227, 216, 222, 136,
  30,  52, 409, 296, 122, 110,  36,  87,  94, 256,
  89, 408, 345, 255, 295, 129, 348,  82,  83,  41,
 226, 375, 210,  88, 292, 188, 189, 190, 124,  49,
  57, 485, 472, 371, 260,  87,  75, 436, 372, 507,
  33,  34,  35, 490,  31,  32,  96,  97,  98, 100,
 101, 102, 114, 115, 116, 103, 104, 105, 106, 107,
 108, 109,  99, 236, 433,  36, 404, 112, 132, 113,
 365, 488, 117, 128,  96,  97,  98, 100, 101, 102,
 114, 115, 116, 103, 104, 105, 106, 107, 108, 109,
  99, 275, 315, 287, 288, 112, 111, 113, 110, 278,
 117,  94, 311, 274,  89, 484, 279, 280, 216, 363,
  82,  83, 366, 249, 430, 268,  88, 338, 312, 248,
 250, 247, 390, 246, 239, 151, 269, 277,  87,  68,
 243, 235,  89, 216, 269, 269,  70, 269, 269, 269,
 269, 269, 269, 269, 269, 269, 269, 269, 269, 289,
 290, 291, 319,  69,  -1,  -1,  71,  44, 310,  65,
  40,  62,  39, 142,  27,  28,  59,  96,  97,  98,
 100, 101, 102, 114, 115, 116, 103, 104, 105, 106,
 107, 108, 109,  99,  14,  13,  12,  11, 112, 132,
 113,  10,   9, 117,  33,  34,  35,  18,  31,  32,
   7,  22,  19, 281, 282, 283, 284, 285, 286,   6,
   5,   3,   2, 498, 511,  24,  23,  21, 165,  36,
  50,  86, 229,  26, 306,  89,  80,  15, 121,  81,
  17,  16, 313,  93,  91,  84, 170,  29, 359, 358,
 257, 254, 154, 324,   1,   0,   0,   0,   0,   0,
   0,   0,  25,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0, 269 };
short yypact[]={

-1000,-1000, 527, 527,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,-248,-1000,-1000,  -2,-217,
-1000,-249, 241,-240,-1000, -44,-1000,-293,-251,  -2,
-1000,-1000,-253,-1000,-1000,-1000,-1000,-1000,-116,-273,
-276,-1000,-1000,-1000,-282,-1000, 369,-1000,-1000, 341,
 -29,-1000,-261, 193, 171,-1000,  50,-1000,-1000,-154,
-1000,-1000, 163,-1000,-1000, 151,-1000,-1000,-280,-268,
 182,   8,-1000, -84,  -9, 247, 238,-1000,-1000,-199,
-258, 123, 369, 369, 369, -59,-1000,-1000,-1000,-1000,
-356,-1000,-1000,-1000, 369,-358,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-113,-119,
  15,-361, -11,  28, 209,-1000,-1000,  16,-118,-257,
-1000,   3,-1000,-1000,-1000,-1000, 133,-1000, 179,-1000,
-1000,-312,  14,-1000,-1000,-1000,-273,-1000,-1000,-1000,
-276, 117,-1000, -22, 171,-1000, 314,-1000,-1000,-1000,
-1000,-1000,-1000,  31, 373, -96,  16,-1000,-323, 369,
 369,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-1000, 369, 369, 369,  20,-1000,-1000,-1000,-1000,
-1000, 369, 369, 215, 369, 369, 369, 369, 369, 369,
 369, 369, 369, 369, 369, 369, 369, 369, 369,-1000,
-1000,-1000,  16,-1000,-1000,-236,-1000,-1000, 128,-1000,
-1000,-1000,-1000,-1000,-1000,-1000, 171,-1000,-160, 208,
-1000,-150, 314,-1000,-1000,-1000,-1000,-1000,-280,-144,
-1000,-1000,-1000, 221, 147,-1000,-282,  -8,-243,-215,
-290,-1000, 141,-1000, 251,-1000,-1000,  38,-1000, -37,
 241,-1000,-1000, -29, 462,-1000,-1000, 155, -59,-1000,
  78,-1000, 195,-1000,-356,-358,-1000,-113,-119,  15,
  15, -11, -11, -11, -11, -11, -11,  28,  28,-1000,
-1000,-1000, 171,-1000,-313,-338,-257,-1000, 171,  35,
-1000,-284,-286,-287, 179,-1000, 119,-1000,-1000,-1000,
-1000, 314,-1000,-1000,-1000,-1000, 314,-1000,-1000,-1000,
-1000, 369,-1000,-1000,-167,-1000,-1000,-254,-1000, 242,
-1000,-1000, 277,-1000, 136,-1000,-1000,  16,-1000, 369,
-1000,-1000, 369,-1000,-1000, -33,-1000, 105,-138,-1000,
-160,-1000,-1000,-1000,-1000,-1000,-150,-112, 251, -10,
-1000,-1000, 103,-332,-215, 214, -48,-1000,-1000,-1000,
-1000, 369,-112,-1000,-1000,-138,-1000,-1000,-1000, 171,
-1000,-1000, 161,-112,-1000,-1000,-147,-218,-1000, 314,
-290,-1000,-182,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-1000, 369,-1000,  -8,-216, 192,-1000,-1000,-257,-257,
-1000,-1000,-1000,  69,-1000, 241,-1000, 241,-1000,-1000,
-1000, -21, 369, 205,-1000,  47,-1000,-1000,-1000,-332,
 -48, 148,-1000,-323,-255,-323,-140,-1000,-1000,-1000,
 369, 369,-112,  14,  14,-1000,  47, 369,-1000,-184,
-1000,-1000,-1000,-1000, -29,-1000,-112, 135,  75,-156,
-1000,-1000,-1000,  45,-257,-1000,-1000,-153,-1000, 241,
-1000, 172, 171,-1000,-1000,-112, 369, 171,-1000,-1000,
-112,-1000, -14,-1000,-294,-140,-1000,-1000,-208,-1000,
-295,-1000,-257,-1000,-142,  14,-1000,-1000, -17,-321,
 -24,-1000,-1000,-1000,-264,-1000,-238,-257,-1000,-1000,
-112,  14,-1000,-1000,-1000,-1000,-1000,-1000 };
short yypgo[]={

   0, 864, 863,  25, 472,   1,  13, 862,  77, 104,
 861, 860, 859, 858,  16, 857, 620, 856, 855,  15,
 854, 853, 852, 849, 562, 508, 624, 518, 552, 848,
 529, 523, 531, 551, 846, 131, 126,  14,  40, 844,
 842, 158, 841,   8, 840,  17, 838,  12, 834,   2,
 162, 833, 832, 831, 520, 830, 829, 820, 180, 812,
 811, 807, 806, 805, 804, 786, 783,   0, 782, 781,
 780, 779, 604, 532, 778, 489, 777, 776, 775, 774,
   9, 773, 756, 750,  52, 749, 745, 499, 744, 525,
 743, 742, 741,  11, 740, 739, 734,  10, 733, 732,
 729, 725, 691, 690, 686, 684,   4, 663, 659,   6,
   7, 658,   3, 657,   5, 656, 654, 653, 652, 651,
 649, 648, 644, 641,  23, 640, 636, 517, 488, 635,
  18, 634, 632, 631, 623, 622, 621, 619, 616, 614,
 613,  61, 612, 611, 608, 607, 599, 591, 590, 571,
 570, 567, 563, 559, 554, 553, 527, 526, 522, 519,
 513 };
short yyr1[]={

   0,  52,   1,  53,  53,  54,  54,  54,  54,  54,
  54,  54,  54,  54,  54,  64,  64,  55,  68,  55,
  70,  55,  69,  69,  72,  71,  71,  73,  74,  74,
  75,  75,  76,  58,  79,  78,  78,  81,  57,  82,
  57,  83,   7,   8,   8,  85,  56,  86,  86,  87,
  15,  15,  15,  16,  16,  16,  16,  77,  90,  77,
  10,  10,  11,  11,  12,  12,  13,  13,  91,  14,
   9,   9,   9,  89,  92,  89,  94,  89,  95,  96,
  89,  98,  99,  89,   6,   6,   6,   6,   6,  97,
  97,  93, 100, 101,  93,  93,   2, 103,   2, 104,
   3, 105, 105,  47,  47,  47,  47,  47,  47,  47,
 102, 102, 106, 107, 107,  51,  51,  51,  48,  48,
  46,  46,  46, 111, 113,  60, 115, 117, 118, 119,
  59,  45,  45,  45, 114, 114, 120, 122, 123,  61,
 125, 126,  61,  44, 121, 121, 127, 127, 127, 127,
 127, 127,  49,  49,  50,  50,  50,   4,   5,   5,
   5, 131, 133, 129, 134, 135, 128, 130, 130, 132,
 132, 124, 124,  38,  38,  40,  40,  41,  39,  39,
 136, 139,  62,  62, 138, 138, 140, 140, 141, 141,
 141, 137, 142,  63, 143, 144,  80, 145, 145, 146,
 146, 112, 112, 147, 112, 150, 151, 148, 148, 154,
 148, 155, 156, 148, 157, 148, 158, 148, 148, 148,
 159, 148, 152, 160, 152, 153, 153,  43,  43,  36,
  36,  17,  17,  17,  17,  17,  17,  17,  17,  17,
  17,  17,  35,  35,  34,  34,  33,  33,  32,  32,
  27,  27,  28,  28,  31,  31,  31,  30,  30,  30,
  30,  30,  29,  29,  29,  26,  26,  26,  25,  25,
  25,  25,  24,  24,  24,  24,  18,  18,  18,  23,
  23,  23,  23,  23,  23,  42,  42,  42,  42,  21,
  37,  37,  22,  22,  19,  19,  19,  19,  19,  19,
  20,  20,  20,  20,  20,  20,  20,  20,  20,  20,
  20,  20,  20,  20,  20,  67,  67,  66,  66, 109,
 109,  84,  84,  88,  88,  65,  65, 108, 108, 116,
 116, 110, 110, 149, 149 };
short yyr2[]={

   0,   0,   2,   1,   2,   1,   1,   1,   1,   1,
   1,   1,   1,   1,   1,   2,   4,   5,   0,   4,
   0,   4,   1,   3,   1,   1,   3,   1,   0,   1,
   1,   2,   0,   4,   0,   3,   2,   0,   5,   0,
   6,   0,   5,   2,   1,   0,   5,   1,   3,   3,
   1,   1,   2,   1,   1,   1,   1,   1,   0,   4,
   1,   3,   1,   3,   0,   1,   1,   3,   0,   3,
   1,   2,   3,   1,   0,   4,   0,   4,   0,   0,
   8,   0,   0,   6,   1,   3,   4,   3,   4,   0,
   2,   3,   0,   0,  11,   1,   1,   0,   5,   0,
   5,   0,   2,   1,   1,   1,   1,   1,   1,   1,
   1,   2,   5,   1,   4,   0,   2,   3,   1,   1,
   0,   1,   1,   0,   0,  10,   0,   0,   0,   0,
  15,   0,   2,   2,   0,   2,   0,   0,   0,   8,
   0,   0,   7,   1,   1,   2,   1,   1,   2,   2,
   2,   3,   1,   3,   1,   1,   1,   1,   1,   2,
   1,   0,   0,   7,   0,   0,   7,   0,   1,   0,
   1,   0,   2,   1,   1,   1,   3,   3,   1,   3,
   0,   0,   6,   5,   0,   2,   1,   3,   2,   2,
   2,   1,   0,   3,   0,   0,   6,   0,   1,   1,
   2,   1,   3,   0,   2,   0,   0,   9,   1,   0,
   6,   0,   0,   9,   0,   3,   0,   3,   2,   3,
   0,   6,   0,   0,   3,   1,   2,   1,   3,   1,
   3,   1,   1,   1,   1,   1,   1,   1,   1,   1,
   1,   1,   1,   5,   1,   3,   1,   3,   1,   3,
   1,   3,   1,   3,   1,   3,   3,   1,   3,   3,
   3,   3,   1,   3,   3,   1,   3,   3,   1,   3,
   3,   3,   1,   2,   2,   2,   1,   1,   1,   1,
   4,   3,   4,   2,   2,   1,   1,   1,   3,   1,
   1,   3,   1,   1,   1,   1,   1,   1,   1,   1,
   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
   1,   1,   1,   0,   1 };
short yychk[]={

-1000,  -1, -52, -53, -54, -55, -56, -57, -58, -59,
 -60, -61, -62, -63, -64, 320, 324, 323, 290, 295,
  -8, 310, 294, 309, 308, 345, 316, 257, 258, -15,
 -16, 291, 292, 287, 288, 289, 312, -54, 347, -68,
 -70, -16,  -8, 351, -76, 347,-109, 259, 256,-120,
 -44, 341,-136, 279,-142, 351, 347, -16, 347, -65,
 321, 256, -69, -72, 349, -71, -73, 349, -85, -81,
 -82, -77, -89,  -6, 349,-115, -43, -36, -35, -24,
 -34, -23, 268, 269, -18, -33, -42, 286, 274, 383,
 -32, -20, -19, -21, 259, -27, 325, 326, 327, 341,
 328, 329, 330, 334, 335, 336, 337, 338, 339, 340,
 256, 347, 346, 348, 331, 332, 333, 351, -28, -31,
 -30, -29, -26, -25,-121,-127,  -5, -43, 352,-129,
-128,-130, 347,-110, 301, 256,-137, 349, 261, -80,
 261, 266, -66, 344, 256, -67, 266, 267, 256, -67,
 266, -86, -87, 349,  -7, 349, 261, 267, 266, 279,
 321, 309, 293, 263, 259, -46, -43, 349, 260, 266,
 -17, 279, 368, 369, 370, 371, 372, 373, 374, 376,
 377, 375, 382, 284, 263, 259, 268, 269, -24, -24,
 -24, 283, 378, -43, 379, 280, 285, 282, 275, 276,
 277, 278, 381, 380, 273, 274, 270, 271, 272,-110,
-127, 256, -43,  -4, 363, -49,-128, -50,-130, 356,
 357, 358,-128, 347, 274, 265,-125,-138, 265, -40,
 -41, 261,-143, 351, -67, -72, -73, -67, 266, -88,
 279, 256, -80, -83, -75, -58, -90, -92, -95, -98,
 -94, 264, -19, -84, -10, 260, 256, -11,  -9,  -8,
-116, 302, 256, -45, 353, -36, -36, -43, -33, -24,
 -43, 260, -37, -36, -32, -27, 260, -28, -31, -30,
 -30, -26, -26, -26, -26, -26, -26, -25, -25, -24,
 -24, -24,-122, 363, 284,-131,-134, -80,-139,-140,
-141, 313, 314, 315, 266, 262, -39, -38, 342, -19,
 -74, -75, -87, -22, -19,  -4, 259, -58, 262, -89,
 -93, 261, 317, -36,  -2, 343,  -3, 320,  -6, 349,
 264, -84, 266, 280, 263,-109,-110, -43,  -4, 265,
 264, 260, 266, -80, -50,-132, 362,-130,-126, -80,
 266, 349, 349, 349, -41, 262, 266,-144, -12, -13,
 -14,  -9, -37,-100, 309,-103, -99, 347, 281,  -9,
 264,-117,-111, -35, -36,-123, 274, 265,-124, 299,
-141, -38,-145,-146,-112, -80, 306,-147, -84, 266,
 -91, 262, -47, 359, 360, 361, 363, 364, 365, 366,
  -3, 259, -97, 279,-104, -43,-112,-124,-133,-135,
 -80, 262,-112, -19,-148, 297,-153, 300, 301, 303,
 304, 307, 296, 305, -67, -43, 256, -14,  -6, 318,
 -96, -37, -93,-105, 322, 260,-113,-130,-130, 265,
-109,-109,-155,-157,-158, 265, -43, 259, -67, -47,
 -97, 260, -45, 347, -45,-114, 299, -43, -43,-112,
 -67, -67, -67, -43, 319,-110,-112, 260, 260, 300,
 260,  -5,-118,-149, 298,-154,-109,-159, 261, -80,
-150,-112, -43, -80,-101,-119,-112, 260,-102,-106,
-107,  -5, 354,-114,-151,-156, 262,-106, -51, 352,
  -5,-152, 299, -67, 266, 363, -49,-108, 283, 256,
-160, -48, 311, 355, 363,  -5,-112, -67 };
short yydef[]={

   1,  -2,   0,   2,   3,   5,   6,   7,   8,   9,
  10,  11,  12,  13,  14,   0,  18,  20,   0,   0,
  32,   0,   0, 136, 180,   0, 192,   0,   0,   0,
  44,  50,  51,  53,  54,  55,  56,   4,   0,   0,
   0,  45,  37,  39,   0, 126,   0, 319, 320, 167,
   0, 143,   0,   0,   0,  15,   0,  43,  52,   0,
 325, 326,   0,  22,  24,   0,  25,  27,   0,   0,
   0,   0,  57,  73,  84, 120,   0, 227, 229, 268,
 242, 272,   0,   0,   0, 244, 279, 276, 277, 278,
 246, 285, 286, 287,   0, 248, 300, 301, 302, 303,
 304, 305, 306, 307, 308, 309, 310, 311, 312, 313,
 314, 294, 295, 296, 297, 298, 299, 289, 250, 252,
 254, 257, 262, 265, 167, 144, 146, 147, 167, 158,
 160,   0,  -2, 140, 331, 332, 184, 191,   0, 193,
 194,   0,   0, 317, 318,  19,   0, 315, 316,  21,
   0,   0,  47,   0,   0,  41,   0,  33,  58,  74,
  78,  81,  76,   0,   0,   0, 121, 122, 131,   0,
   0, 231, 232, 233, 234, 235, 236, 237, 238, 239,
 240, 241,   0,   0,   0,   0, 283, 284, 273, 274,
 275,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0, 137,
 145,  -2, 148, 149, 150,   0, 157, 152,   0, 154,
 155, 156, 159, 168, 161, 164,   0, 181,   0,   0,
 175,   0,  28,  16,  17,  23,  26,  46,   0, 167,
 323, 324,  38,   0,   0,  30,   0,   0,   0,   0,
   0,  85,   0,  87,   0, 321, 322,  60,  62,  70,
   0, 329, 330,   0, 167, 228, 230,   0, 245, 268,
   0, 281,   0, 290, 247, 249, 288, 251, 253, 255,
 256, 258, 259, 260, 261, 263, 264, 266, 267, 269,
 270, 271,   0, 151,   0, 169, 167, 141,   0, 185,
 186,   0,   0,   0,   0, 183,   0, 178, 173, 174,
 195,  29,  48,  49, 292, 293,  64,  31,  40,  59,
  75,   0,  92,  95,   0,  -2,  82,   0,  77,  84,
  86,  88,   0,  71,   0, 127, 123, 132, 133,   0,
 280, 282,   0, 138, 153,   0, 170,   0, 171, 182,
   0, 188, 189, 190, 176, 177,   0,  -2,   0,  65,
  66,  68,   0,   0,   0,   0,  89,  99,  61,  63,
  72,   0, 203, 243, 291, 171, 162, 165, 142,   0,
 187, 179,   0,  -2, 199, 201,   0,   0,  42,   0,
   0,  91,   0, 103, 104, 105, 106, 107, 108, 109,
  79,   0,  83,   0, 101,   0, 124, 139, 167, 167,
 172, 196, 200,   0, 204,   0, 208,   0, 211, 214,
 216,   0,   0,   0, 225,   0,  -2,  67,  69,   0,
  89,   0,  90, 131,   0, 131, 134, 163, 166, 202,
   0,   0, 203,   0,   0, 218,   0,   0, 226,   0,
  80,  98, 100, 102,   0, 125, 203,   0,   0,   0,
 215, 217, 219,   0, 167, 128, 135, 333, 209,   0,
 220,   0,   0, 205, 334, 203,   0,   0,  93, 129,
 203, 210,   0, 221, 167, 134, 206, 212, 167, 110,
 115, 113, 167, 130, 222,   0,  94, 111,   0,   0,
   0, 207, 223, 213,   0, 116,   0, 167, 327, 328,
 203,   0, 118, 119, 117, 114, 224, 112 };
#ifndef lint
static char yaccpar_sccsid[] = "@(#)yaccpar	4.1	(Berkeley)	2/11/83";
#endif not lint

#
# define YYFLAG -1000
# define YYERROR goto yyerrlab
# define YYACCEPT return(0)
# define YYABORT return(1)

/*	parser for yacc output	*/

#ifdef YYDEBUG
int yydebug = 0; /* 1 for debugging */
#endif
YYSTYPE yyv[YYMAXDEPTH]; /* where the values are stored */
int yychar = -1; /* current input token number */
int yynerrs = 0;  /* number of errors */
short yyerrflag = 0;  /* error recovery flag */

yyparse() {

	short yys[YYMAXDEPTH];
	short yyj, yym;
	register YYSTYPE *yypvt;
	register short yystate, *yyps, yyn;
	register YYSTYPE *yypv;
	register short *yyxi;

	yystate = 0;
	yychar = -1;
	yynerrs = 0;
	yyerrflag = 0;
	yyps= &yys[-1];
	yypv= &yyv[-1];

 yystack:    /* put a state and value onto the stack */

#ifdef YYDEBUG
	if( yydebug  ) printf( "state %d, char 0%o\n", yystate, yychar );
#endif
		if( ++yyps> &yys[YYMAXDEPTH] ) { yyerror( "yacc stack overflow" ); return(1); }
		*yyps = yystate;
		++yypv;
		*yypv = yyval;

 yynewstate:

	yyn = yypact[yystate];

	if( yyn<= YYFLAG ) goto yydefault; /* simple state */

	if( yychar<0 ) if( (yychar=yylex())<0 ) yychar=0;
	if( (yyn += yychar)<0 || yyn >= YYLAST ) goto yydefault;

	if( yychk[ yyn=yyact[ yyn ] ] == yychar ){ /* valid shift */
		yychar = -1;
		yyval = yylval;
		yystate = yyn;
		if( yyerrflag > 0 ) --yyerrflag;
		goto yystack;
		}

 yydefault:
	/* default state action */

	if( (yyn=yydef[yystate]) == -2 ) {
		if( yychar<0 ) if( (yychar=yylex())<0 ) yychar = 0;
		/* look through exception table */

		for( yyxi=yyexca; (*yyxi!= (-1)) || (yyxi[1]!=yystate) ; yyxi += 2 ) ; /* VOID */

		while( *(yyxi+=2) >= 0 ){
			if( *yyxi == yychar ) break;
			}
		if( (yyn = yyxi[1]) < 0 ) return(0);   /* accept */
		}

	if( yyn == 0 ){ /* error */
		/* error ... attempt to resume parsing */

		switch( yyerrflag ){

		case 0:   /* brand new error */

			yyerror( "syntax error" );
		yyerrlab:
			++yynerrs;

		case 1:
		case 2: /* incompletely recovered error ... try again */

			yyerrflag = 3;

			/* find a state where "error" is a legal shift action */

			while ( yyps >= yys ) {
			   yyn = yypact[*yyps] + YYERRCODE;
			   if( yyn>= 0 && yyn < YYLAST && yychk[yyact[yyn]] == YYERRCODE ){
			      yystate = yyact[yyn];  /* simulate a shift of "error" */
			      goto yystack;
			      }
			   yyn = yypact[*yyps];

			   /* the current yyps has no shift onn "error", pop stack */

#ifdef YYDEBUG
			   if( yydebug ) printf( "error recovery pops state %d, uncovers %d\n", *yyps, yyps[-1] );
#endif
			   --yyps;
			   --yypv;
			   }

			/* there is no state on the stack with an error shift ... abort */

	yyabort:
			return(1);


		case 3:  /* no shift yet; clobber input char */

#ifdef YYDEBUG
			if( yydebug ) printf( "error recovery discards char %d\n", yychar );
#endif

			if( yychar == 0 ) goto yyabort; /* don't discard EOF, quit */
			yychar = -1;
			goto yynewstate;   /* try again in the same state */

			}

		}

	/* reduction by production yyn */

#ifdef YYDEBUG
		if( yydebug ) printf("reduce %d\n",yyn);
#endif
		yyps -= yyr2[yyn];
		yypvt = yypv;
		yypv -= yyr2[yyn];
		yyval = yypv[1];
		yym=yyn;
			/* consult goto table to find next state */
		yyn = yyr1[yyn];
		yyj = yypgo[yyn] + *yyps + 1;
		if( yyj>=YYLAST || yychk[ yystate = yyact[yyj] ] != -yyn ) yystate = yyact[yypgo[yyn]];
		switch(yym){
			
case 1:
# line 269 "c.y"
{
	    initSymbolTable();
	    fpCurrent = fpScan;
            emitScanHeadings();
            systemDefined = 0;
            subr_string[0] = 0;
	} break;
case 2:
# line 277 "c.y"
{
	    if (screenUsage)
	      makeScreenUsage(screenUsage);
	    else
  	      makeScreenArray();
            makeGlobalSymTable();
            emitScanBindings();
	    escapeOuterScope();		/* free's symbol table */
            emitScanEndings();
	} break;
case 15:
# line 311 "c.y"
{ strcpy(includefile, this_string);
				 doSubFile(includefile, 1);
                               } break;
case 16:
# line 315 "c.y"
{
            systemId = cval;
            strcpy(systemCid, this_string);
            systemDefined = 1;
          } break;
case 17:
# line 324 "c.y"
{  
                if (cval < 1 || cval > 80)
                    vcerror("illegal module number");
                currentIdInfo.moduleInfo.moduleTypeRef = cref;
                currentDeclSpec.storageSpec = _global;
		currentDeclSpec.type = _module;
		currentDeclSpec.alias = 0;
                sprintf(cid, "%d", cval);
                declareId(cid, currentDeclSpec, &currentIdInfo, 0);
             } break;
case 18:
# line 334 "c.y"
{ idDeclare = 1; } break;
case 19:
# line 335 "c.y"
{ idDeclare = 0; } break;
case 20:
# line 336 "c.y"
{ idDeclare = 1; } break;
case 21:
# line 337 "c.y"
{ idDeclare = 0; } break;
case 24:
# line 346 "c.y"
{ declareGlobalId(cid, _moduletype);} break;
case 27:
# line 355 "c.y"
{ declareGlobalId(cid, _filter);} break;
case 32:
# line 378 "c.y"
{ idDeclare = 1; currentDeclSpec = yypvt[-0].declSpec; } break;
case 33:
# line 380 "c.y"
{ 
		   idDeclare = 0; 
		} break;
case 34:
# line 388 "c.y"
{
            emitCode("/* declaration list */");
            emitDeclarationList();  /* if any */
          } break;
case 35:
# line 393 "c.y"
{
            scopeCounter--;
            decScope();
            fpCurrent = fpScan;
          } break;
case 37:
# line 402 "c.y"
{ idDeclare = 1; currentDeclSpec = yypvt[-0].declSpec; } break;
case 38:
# line 405 "c.y"
{
            scopeCounter--;
            decScope();
            fpCurrent = fpScan;
          } break;
case 39:
# line 411 "c.y"
{ 
              strcpy(subr_string, this_string); subr_string[31] = 0;
            } break;
case 40:
# line 416 "c.y"
{ subr_string[0] = 0; } break;
case 41:
# line 421 "c.y"
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
          } break;
case 42:
# line 442 "c.y"
{ 
            dref->info = newFormelParameters(yypvt[-1].formelParameters, 0 /* in this module */);
            idDeclare = 0; 
            emitFunctionFkn(dref);
            emitDeclarationList();  /* if any */
          } break;
case 43:
# line 450 "c.y"
{ yyval.declSpec = newDeclSpec(yypvt[-1].storageSpec, yypvt[-0].typeSpec, nodeNo); } break;
case 44:
# line 451 "c.y"
{yyval.declSpec = newDeclSpec(!scopeCounter ? _global: _local, yypvt[-0].typeSpec, 0L);} break;
case 45:
# line 456 "c.y"
{ 
		idDeclare = 1;
		currentDeclSpec.type = cnv2const(yypvt[-0].typeSpec);
		currentDeclSpec.storageSpec = _global;
		currentDeclSpec.alias = 0;
	    } break;
case 46:
# line 463 "c.y"
{ 
		   idDeclare = 0; 
		} break;
case 49:
# line 476 "c.y"
{ 
		currentIdInfo.constInfo.value = cExprTree(yypvt[-0].node, 0, 0);

		declareId(cid, currentDeclSpec, &currentIdInfo, 0);
	    } break;
case 50:
# line 484 "c.y"
{ nodeNo = 0;    yyval.storageSpec = _extern; 
                   vcwarning("Keyword 'EXTERN' is obsolete; use REMOTE"); } break;
case 51:
# line 486 "c.y"
{ nodeNo = 0;    yyval.storageSpec = _extern; } break;
case 52:
# line 487 "c.y"
{ nodeNo = cval; yyval.storageSpec = _extern; } break;
case 53:
# line 491 "c.y"
{ yyval.typeSpec = _char; } break;
case 54:
# line 492 "c.y"
{ yyval.typeSpec = _int;  } break;
case 55:
# line 493 "c.y"
{ yyval.typeSpec = _float;} break;
case 56:
# line 494 "c.y"
{ yyval.typeSpec = _calendar;} break;
case 58:
# line 499 "c.y"
{ idDeclare = 1; } break;
case 61:
# line 505 "c.y"
{
/*
            $$ = cFormelList( ???????????? );
*/
          } break;
case 62:
# line 521 "c.y"
{ yyval.formelParameters = cFormelList(yypvt[-0].declSpec.type, 0, 0, 0); } break;
case 63:
# line 523 "c.y"
{
            yyval.formelParameters = cFormelList(0, yypvt[-2].formelParameters, cFormelList(yypvt[-0].declSpec.type, 0, 0, 0), 0);
          } break;
case 64:
# line 529 "c.y"
{ yyval.formelParameters = 0; } break;
case 65:
# line 530 "c.y"
{ yyval.formelParameters = yypvt[-0].formelParameters; } break;
case 66:
# line 534 "c.y"
{ yyval.formelParameters = cFormelList(yypvt[-0].declSpec.type, 0, 0, cid); } break;
case 67:
# line 536 "c.y"
{
            yyval.formelParameters = cFormelList(0, yypvt[-2].formelParameters, cFormelList(yypvt[-0].declSpec.type, 0, 0, cid), 0);
          } break;
case 68:
# line 543 "c.y"
{ 
            idDeclare = 1; currentDeclSpec = yypvt[-0].declSpec; actualDeclare = 1;
          } break;
case 69:
# line 547 "c.y"
{ 
            if (yypvt[-0].idSpec->declSpec.type == _charVec)
              yypvt[-2].declSpec.type = _charVec;
            else if (yypvt[-0].idSpec->declSpec.type == _intVec)
              yypvt[-2].declSpec.type = _intVec;
            else if (yypvt[-0].idSpec->declSpec.type == _floatVec)
              yypvt[-2].declSpec.type = _floatVec;
            yyval.declSpec = yypvt[-2].declSpec;
            actualDeclare = 0;
          } break;
case 70:
# line 561 "c.y"
{
            yyval.declSpec = yypvt[-0].declSpec;
          } break;
case 71:
# line 565 "c.y"
{
            if (yypvt[-1].declSpec.type == _char)
              yypvt[-1].declSpec.type = _charRef;
            else if (yypvt[-1].declSpec.type == _int)
              yypvt[-1].declSpec.type = _intRef;
            else if (yypvt[-1].declSpec.type == _float)
              yypvt[-1].declSpec.type = _floatRef;
/*
            else if ($1.type == _calendar)
              $1.type = _calendarRef;
*/

            yyval.declSpec = yypvt[-1].declSpec;
          } break;
case 72:
# line 580 "c.y"
{
            if (yypvt[-2].declSpec.type == _char)
              yypvt[-2].declSpec.type = _charVec;
            else if (yypvt[-2].declSpec.type == _int)
              yypvt[-2].declSpec.type = _intVec;
            else if (yypvt[-2].declSpec.type == _float)
              yypvt[-2].declSpec.type = _floatVec;
            yyval.declSpec = yypvt[-2].declSpec;
          } break;
case 74:
# line 606 "c.y"
{
            idDeclare = 0;  dref = yypvt[-1].idSpec;
	  } break;
case 76:
# line 609 "c.y"
{ strncpy(cid2, cid, 31); } break;
case 77:
# line 610 "c.y"
{
            strcpy(yypvt[-0].idSpec->declSpec.alias = (char *) malloc(32), cid2);

/*            $3->declSpec.alias = $1; */
/*      $1->declSpec.storageSpec = _empty; */ /* so nobody will find it ! */
          } break;
case 78:
# line 617 "c.y"
{
            idDeclare = 0;  dref = yypvt[-1].idSpec; checkIntOrFloat(yypvt[-1].idSpec->declSpec.type);
	  } break;
case 79:
# line 621 "c.y"
{ if (yypvt[-5].idSpec->info)
              vcerror("illegal type, no arrays allowed");
            yypvt[-5].idSpec->info = newInfoBinding(yypvt[-2].filterSpec, yypvt[-0].bindingSpec);
            /* emitBinding($1, $4, $6); */
          } break;
case 81:
# line 627 "c.y"
{
            idDeclare = 0;  dref = yypvt[-1].idSpec; checkIntOrFloat(yypvt[-1].idSpec->declSpec.type);
          } break;
case 82:
# line 631 "c.y"
{ if (yypvt[-3].idSpec->info)
              vcerror("illegal type, no arrays allowed");
            yypvt[-3].idSpec->info = newInfoBinding(0, yypvt[-0].bindingSpec);
            /*            emitBinding($1, 0, $4);       */
          } break;
case 84:
# line 640 "c.y"
{
	    yyval.idSpec = declareId(cid, currentDeclSpec, 
				0  /* no info */, 0  /* 0 = current scope */);
          } break;
case 85:
# line 645 "c.y"
{
            declType tmpDeclSpec;   /* copy of type of var and storage spec */
            tmpDeclSpec.storageSpec = currentDeclSpec.storageSpec;
            tmpDeclSpec.type = cnv2vec(currentDeclSpec.type);
            tmpDeclSpec.nodeNo = currentDeclSpec.nodeNo;
            tmpDeclSpec.alias = currentDeclSpec.alias;
            if (actualDeclare == 0)
              vcerror("Must specify size of vector");
            currentIdInfo.vecInfo.size = -1;      /* no specified !! */
	    yyval.idSpec = declareId(cid, tmpDeclSpec, 
			      &currentIdInfo, 0  /* 0 = current scope */);
          } break;
case 86:
# line 658 "c.y"
{
            declType tmpDeclSpec;   /* copy of type of var and storage spec */
            tmpDeclSpec.storageSpec = currentDeclSpec.storageSpec;
            tmpDeclSpec.type = cnv2vec(currentDeclSpec.type);
            tmpDeclSpec.nodeNo = currentDeclSpec.nodeNo;
            tmpDeclSpec.alias = currentDeclSpec.alias;
            currentIdInfo.vecInfo.size = cval;
	    yyval.idSpec = declareId(cid, tmpDeclSpec, 
			      &currentIdInfo, 0  /* 0 = current scope */);
          } break;
case 87:
# line 669 "c.y"
{ 
	    yyval.idSpec = declareId(cid, currentDeclSpec, 
				0  /* no info */, 0  /* 0 = current scope */);
            yyval.idSpec->declSpec.type = cnv2func(yyval.idSpec->declSpec.type);
/*            $$->info = 0;   changed 921015 */
            yyval.idSpec->info = newFormelParameters(0, subr_string);
          } break;
case 88:
# line 677 "c.y"
{ 
	    yyval.idSpec = declareId(cid, currentDeclSpec, 
				0  /* no info */, 0  /* 0 = current scope */);
            yyval.idSpec->declSpec.type = cnv2func(yyval.idSpec->declSpec.type);
            yyval.idSpec->info = newFormelParameters(yypvt[-1].formelParameters, subr_string);
          } break;
case 91:
# line 692 "c.y"
{
/*            checkIntOrFloat(dref->declSpec.type);   */

            dref->expr = cExprTree(0, yypvt[-1].tree, 0);       /* ????????? */
          } break;
case 92:
# line 698 "c.y"
{ if (dref->declSpec.type != _calendar)
                vcerror("illegal type specifier, must be calendar");
          } break;
case 93:
# line 702 "c.y"
{
                  currentCalendar = newCalendar(yypvt[-5].integer, yypvt[-3].integer, yypvt[-1].ctid);
                } break;
case 94:
# line 706 "c.y"
{
            dref->info = currentCalendar;
          } break;
case 95:
# line 710 "c.y"
{
            checkIntOrFloat(dref->declSpec.type);
            dref->expr = yypvt[-0].tree;   /* cExprTree($1, 0, 0); */
          } break;
case 96:
# line 726 "c.y"
{
            yyval.filterSpec = newFilterSpec(cref, 0);
          } break;
case 97:
# line 729 "c.y"
{ dref = cref; } break;
case 98:
# line 731 "c.y"
{
            yyval.filterSpec = newFilterSpec(dref, yypvt[-1].tree);
          } break;
case 99:
# line 745 "c.y"
{ moduleNo = cval; } break;
case 100:
# line 747 "c.y"
{
            sprintf(cid, "%d", moduleNo);
            if (!(cref = lookUpIdent(cid))) { char errText[100];
                sprintf(errText, "module no %d not declared", moduleNo);
                vcerror(errText);
            }
            yyval.bindingSpec = newBindingSpec(cref, channelNo, yypvt[-0].tree);
          } break;
case 101:
# line 758 "c.y"
{ channelNo = 0; } break;
case 102:
# line 759 "c.y"
{ channelNo = cval; } break;
case 103:
# line 763 "c.y"
{ yyval.integer = 10*366*24*3600; } break;
case 104:
# line 764 "c.y"
{ yyval.integer = 366*24*3600; } break;
case 105:
# line 765 "c.y"
{ yyval.integer = 31*24*3600; } break;
case 106:
# line 766 "c.y"
{ yyval.integer = 24*3600; } break;
case 107:
# line 767 "c.y"
{ yyval.integer = 3600; } break;
case 108:
# line 768 "c.y"
{ yyval.integer = 60; } break;
case 109:
# line 769 "c.y"
{ yyval.integer = 1; } break;
case 112:
# line 778 "c.y"
{
            setCalendar(currentCalendar, &betweenTime, yypvt[-3].integer, yypvt[-1].integer);
          } break;
case 113:
# line 784 "c.y"
{ 
              betweenTime.first = yypvt[-0].ctid; betweenTime.last = yypvt[-0].ctid;
            } break;
case 114:
# line 788 "c.y"
{ 
              betweenTime.first = yypvt[-2].ctid; betweenTime.last = yypvt[-0].ctid;
            } break;
case 115:
# line 794 "c.y"
{ yyval.integer = colorRed | colorOrange | colorBlack;} break;
case 116:
# line 795 "c.y"
{ yyval.integer = colorRed | colorOrange | colorBlack;} break;
case 117:
# line 796 "c.y"
{ yyval.integer = yypvt[-1].integer; } break;
case 118:
# line 799 "c.y"
{ yyval.integer = stateOn; } break;
case 119:
# line 800 "c.y"
{ yyval.integer = stateOff; } break;
case 120:
# line 804 "c.y"
{ 
            long temp;
            alarmClass = 0; /* alarm class A */ 
            temp = alarmClass;
            yyval.tree = cExprTree(
                      cExprNode(_int_const, 0, (char *) &temp, _SIZE_OF_INT),
                      0 ,0);
          } break;
case 121:
# line 812 "c.y"
{ yyval.tree = yypvt[-0].tree; } break;
case 122:
# line 814 "c.y"
{
            long temp;

            alarmClass = (cid[0] | 0x20) - 'a';
            temp = alarmClass;
            yyval.tree = cExprTree(
                      cExprNode(_int_const, 0, (char *) &temp, _SIZE_OF_INT),
                      0 ,0);

            if (alarmClass < 0 || alarmClass > 3 || strlen(cid) != 1)
            {
              vcerror("Alarm class expected (a,b,c or d) or keyword WHEN");
            }
          } break;
case 123:
# line 835 "c.y"
{
          eventCounter ++;
          sprintf(scopeName, "event_%02d", eventCounter); scopeCnt = 0;
          emitEventStart(eventCounter, yypvt[-3].tree, yypvt[-1].tree);
        } break;
case 124:
# line 841 "c.y"
{
          emitEventMiddle(eventCounter);
        } break;
case 125:
# line 845 "c.y"
{
          emitEventEnd(eventCounter);
        } break;
case 126:
# line 852 "c.y"
{ alarmNo = cval; idDeclare = 1; } break;
case 127:
# line 854 "c.y"
{ idDeclare = 0; } break;
case 128:
# line 856 "c.y"
{
            alarmIndex = checkAndAllocate(alarmNo, alarmVector);
            emitAlarmStart(alarmNo, yypvt[-3].tree, yypvt[-1].tree, alarmIndex, yypvt[-7].tree);
            sprintf(scopeName, "alarm_%02d",alarmNo); scopeCnt = 0;
          } break;
case 129:
# line 862 "c.y"
{
            emitAlarmMiddle(alarmNo, alarmIndex);
          } break;
case 130:
# line 866 "c.y"
{
            emitAlarmEnd(alarmNo, alarmIndex, yypvt[-11].tree);
          } break;
case 131:
# line 873 "c.y"
{         /*  { $$ = 0; }  */
            long temp;
            temp = 0;
            yyval.tree = cExprTree(
                      cExprNode(_int_const, 0, (char *) &temp, _SIZE_OF_INT),
                      0 ,0);
          } break;
case 132:
# line 880 "c.y"
{ yyval.tree = yypvt[-0].tree; } break;
case 133:
# line 882 "c.y"
{
            long temp;
            temp = yypvt[-0].ctid;        /* cast time_t -> long (the same) */
            yyval.tree = cExprTree(
                      cExprNode(_int_const, 0, (char *) &temp, _SIZE_OF_INT),
                      0 ,0);
          } break;
case 134:
# line 892 "c.y"
{ emitCode(";\n"); } break;
case 136:
# line 898 "c.y"
{ 
            glitchSpec.abs = glitchSpec.rel = (exprTree *) 0;
            glitchSpec.day = 0;
            glitchSpec.includeFlag = 0;
          } break;
case 137:
# line 904 "c.y"
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
              } break;
case 138:
# line 949 "c.y"
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
              } break;
case 139:
# line 973 "c.y"
{
                decreaseIndent();
                decreaseIndent();
                fprintf(fpScan, "}\n\n");
              } break;
case 140:
# line 979 "c.y"
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
                codeExpression(fpScan, yypvt[-1].tree, 0, 0);      /* & added here ! */
                emitCode2(fpScan, "))\n");
#else
                fprintf(fpScan, "  if (");
                codeExpression(fpScan, yypvt[-1].tree, 0, 0);
                emitCode2(fpScan, ")\n");
#endif
                increaseIndent();
              } break;
case 141:
# line 1011 "c.y"
{
                decreaseIndent();
/*
                fprintf(fpScan, "else if (checkCalendarFall(&");
*/
#ifdef ENTER_CALENDAR
                emitCode2(fpScan, "  else\n");
#else
                fprintf(fpScan, "  else if (!");
                codeExpression(fpScan, yypvt[-3].tree, 0, 0);
                emitCode2(fpScan, ")\n");
#endif
                increaseIndent();
              } break;
case 142:
# line 1026 "c.y"
{
                decreaseIndent();
                decreaseIndent();
                fprintf(fpScan, "}\n");
              } break;
case 143:
# line 1034 "c.y"
{ yyval.tree = cExprTree(cExprNode(_ident, cref, 0, 0), 0, 0); } break;
case 146:
# line 1043 "c.y"
{
            long temp;
            temp = yypvt[-0].ctid;
            glitchSpec.abs = cExprTree(cExprNode(
                                _int_const, 0, (char *) &temp, _SIZE_OF_INT),
                                0, 0);
/*            glitchSpec.abs = $1;      */
            if (glitchSpec.includeFlag & 1)
              vcerror("Absolute expression already specified");
            glitchSpec.includeFlag |= 1;
          } break;
case 147:
# line 1055 "c.y"
{
            glitchSpec.abs = yypvt[-0].tree;
            if (glitchSpec.includeFlag & 1)
              vcerror("Absolute expression already specified");
            glitchSpec.includeFlag |= 1;
          } break;
case 148:
# line 1062 "c.y"
{
            glitchSpec.rel = yypvt[-0].tree;
            if (glitchSpec.includeFlag & 2)
              vcerror("Relative expression already specified");
            glitchSpec.includeFlag |= 2;
          } break;
case 149:
# line 1069 "c.y"
{
            long temp;
            temp = yypvt[-0].ctid;
            glitchSpec.rel = cExprTree(cExprNode(
                                _int_const, 0, (char *) &temp, _SIZE_OF_INT),
                                0, 0);
/*            glitchSpec.rel = $2;        */
            if (glitchSpec.includeFlag & 2)
              vcerror("Relative expression already specified");
            glitchSpec.includeFlag |= 2;
          } break;
case 150:
# line 1081 "c.y"
{
            glitchSpec.day = orColorExpression(colorBlack, 
                                    orColorExpression(colorRed, colorOrange));
            if (glitchSpec.includeFlag & 4)
              vcerror("DAY expression already specified");
            glitchSpec.includeFlag |= 4;
          } break;
case 151:
# line 1089 "c.y"
{
            glitchSpec.day = yypvt[-1].integer;
            if (glitchSpec.includeFlag & 4)
              vcerror("DAY expression already specified");
            glitchSpec.includeFlag |= 4;
          } break;
case 152:
# line 1098 "c.y"
{ yyval.integer = yypvt[-0].integer; } break;
case 153:
# line 1099 "c.y"
{ yyval.integer = orColorExpression(yypvt[-2].integer, yypvt[-0].integer); } break;
case 154:
# line 1102 "c.y"
{ yyval.integer = colorRed; } break;
case 155:
# line 1103 "c.y"
{ yyval.integer = colorOrange; } break;
case 156:
# line 1104 "c.y"
{ yyval.integer = colorBlack; } break;
case 157:
# line 1108 "c.y"
{
            yyval.ctid = (brokenDownTime.tm_hour * 60 + brokenDownTime.tm_min) * 60 +
                  brokenDownTime.tm_sec;
          } break;
case 158:
# line 1118 "c.y"
{ 
            brokenDownTime.tm_hour = brokenDownTime.tm_min = 
              brokenDownTime.tm_sec = 0;
            yyval.ctid = mktime(&brokenDownTime);
          } break;
case 159:
# line 1123 "c.y"
{ yyval.ctid = mktime(&brokenDownTime); } break;
case 160:
# line 1125 "c.y"
{ 
/*
            brokenDownTime.tm_mday = brokenDownTime.tm_mon = 
                brokenDownTime.tm_year = 0;
            $$ = mktime(&brokenDownTime);
*/
            yyval.ctid = (brokenDownTime.tm_hour * 60 + brokenDownTime.tm_min) * 60 +
                  brokenDownTime.tm_sec;
          } break;
case 161:
# line 1140 "c.y"
{ brokenDownTime.tm_mday = (cval > 1900) ? cval - 1900 : cval; } break;
case 162:
# line 1142 "c.y"
{ brokenDownTime.tm_mon = cval - 1; } break;
case 163:
# line 1144 "c.y"
{ brokenDownTime.tm_year = cval; } break;
case 164:
# line 1148 "c.y"
{ brokenDownTime.tm_hour = cval; } break;
case 165:
# line 1150 "c.y"
{ brokenDownTime.tm_min = cval; } break;
case 166:
# line 1152 "c.y"
{ brokenDownTime.tm_sec = cval; } break;
case 167:
# line 1156 "c.y"
{ cval = 0; } break;
case 171:
# line 1165 "c.y"
{ emitCode(";\n"); } break;
case 173:
# line 1171 "c.y"
{ 
            yyval.tree = cExprTree(cExprNode(_screenIdent, cref, 0, 0), 0, 0);
          } break;
case 174:
# line 1174 "c.y"
{ yyval.tree = cExprTree(yypvt[-0].node, 0, 0); } break;
case 175:
# line 1178 "c.y"
{ yyval.tree = yypvt[-0].tree; } break;
case 176:
# line 1180 "c.y"
{ yyval.tree = cExprTree(cExprNode(_comma, 0, 0, 0), yypvt[-2].tree, yypvt[-0].tree); } break;
case 177:
# line 1185 "c.y"
{ yyval.tree = cExprTree(cExprNode(_recList, 0, 0, 0), 0, yypvt[-1].tree); } break;
case 178:
# line 1189 "c.y"
{ yyval.tree = yypvt[-0].tree; } break;
case 179:
# line 1191 "c.y"
{ yyval.tree = cExprTree(cExprNode(_comma, 0, 0, 0), yypvt[-2].tree, yypvt[-0].tree); } break;
case 180:
# line 1195 "c.y"
{ idDeclare = 1; currentDeclSpec.storageSpec = _global;
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
		 } break;
case 181:
# line 1210 "c.y"
{
		fprintf(fpScreen, "void %s()\n", screenId);
		fpCurrent = fpScreen;
		idDeclare = 0;
		declareId(screenId, currentDeclSpec, &currentIdInfo,
			0  /* 0 = current scope */);

		strcat(strcpy(scopeName, "screen_"), screenId); scopeCnt = 0;
	    } break;
case 182:
# line 1220 "c.y"
{
	        fpCurrent = fpScan;
	    } break;
case 183:
# line 1224 "c.y"
{
            screenUsage = cExprTree(0, yypvt[-1].tree, 0);       /* ????????? */
          } break;
case 184:
# line 1230 "c.y"
{ /* terminal screen */ } break;
case 188:
# line 1240 "c.y"
{ strcpy(currentIdInfo.screenInfo.right, cid);} break;
case 189:
# line 1241 "c.y"
{ strcpy(currentIdInfo.screenInfo.down, cid);} break;
case 190:
# line 1242 "c.y"
{ strcpy(currentIdInfo.screenInfo.help, cid);} break;
case 191:
# line 1246 "c.y"
{ strncpy(screenId, cid, 31); screenId[31] = 0; } break;
case 192:
# line 1251 "c.y"
{ 
			strcpy(scopeName, "main");
			scopeCnt = 0; 
			fpCurrent = fpMain;
			emitCode("main(argc, argv)\n");
			emitCode("int argc;\n");
			emitCode("char *argv[];\n");
                        icnt = 0;
		} break;
case 193:
# line 1261 "c.y"
{
			fpCurrent = fpScan;
		} break;
case 194:
# line 1268 "c.y"
{ 
		emitCode("{\n"); 
		scopeCounter++; incScope(scopeName, scopeCnt++);
		increaseIndent();
	    } break;
case 195:
# line 1274 "c.y"
{
                    emitDeclarationList();  /* if any ! */
                  } break;
case 196:
# line 1279 "c.y"
{ 
		decreaseIndent();
		emitCode("}\n"); scopeCounter--; decScope();
	    } break;
case 202:
# line 1298 "c.y"
{ emitCode("case "); 
                    codeExpression(fpCurrent, cExprTree(yypvt[-1].node, 0, 0), 0, 0); 
                    emitCode(":\n");
              if (option_DEBUG) {
                 emitDebugStatement();
             }
          } break;
case 203:
# line 1305 "c.y"
{  if (option_DEBUG) {
               emitCode("{\n");
               emitDebugStatement();
             }
          } break;
case 204:
# line 1311 "c.y"
{
            if (option_DEBUG)
               emitCode("}\n");
          } break;
case 205:
# line 1319 "c.y"
{ 
                checkForEqualSign(yypvt[-2].tree);
		emitCode("if ("); codeExpression(fpCurrent, yypvt[-2].tree, 0, 0);
                    emitCode(")\n");
                if (optionEmitIcode) {
                  emitIcode(_IF);
                  emitIcodeExpression(yypvt[-2].tree);
                  PUSH(getIcodePtr());
                  emitIcode(_dummy);
                }
		increaseIndent();
                freeExpression(yypvt[-2].tree);
                icnt ++;
                PUSH(icnt);
	    } break;
case 206:
# line 1335 "c.y"
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
            } break;
case 207:
# line 1348 "c.y"
{
              if (optionEmitIcode) {
                long is;
                is = POP();
                emitIcode_jmp(is);        /* jump from location 'is' to here */
              }
            } break;
case 209:
# line 1357 "c.y"
{ 
                checkForEqualSign(yypvt[-1].tree);
		emitCode("while ("); codeExpression(fpCurrent, yypvt[-1].tree, 0, 0);
                    emitCode(")\n");
                if (optionEmitIcode) {
                  emitIcode(_WHILE);
                  emitIcodeExpression(yypvt[-1].tree);
                  PUSH(getIcodePtr());
                  emitIcode(_dummy);
                }
		increaseIndent();
                freeExpression(yypvt[-1].tree);
                icnt ++;
                PUSH(icnt);
	    } break;
case 210:
# line 1373 "c.y"
{
              decreaseIndent(); 
              if (icnt == POP()) 
                vcwarning("expression with little effect");
              if (optionEmitIcode) {
                long is;
                is = POP();
                emitIcode_jmp(is);        /* jump from location 'is' to here */
              }
            } break;
case 211:
# line 1384 "c.y"
{
              emitCode("do\n"); increaseIndent();
              icnt ++;
              PUSH(icnt);
            } break;
case 212:
# line 1391 "c.y"
{
              if (icnt == POP())
                vcwarning("expression with little effect");
              decreaseIndent();
              checkForEqualSign(yypvt[-1].tree);
              emitCode("while ("); codeExpression(fpCurrent, yypvt[-1].tree, 0, 0);
                    emitCode(")\n");
              freeExpression(yypvt[-1].tree);
            } break;
case 213:
# line 1400 "c.y"
{ emitCode(";\n"); } break;
case 214:
# line 1401 "c.y"
{ emitCode("break"); icnt++; } break;
case 215:
# line 1402 "c.y"
{ emitCode(";\n"); } break;
case 216:
# line 1403 "c.y"
{ emitCode("continue"); icnt++; } break;
case 217:
# line 1404 "c.y"
{ emitCode(";\n"); } break;
case 218:
# line 1405 "c.y"
{ emitCode("default:\n"); } break;
case 219:
# line 1408 "c.y"
{ 
            icnt++;
            emitCode("return "); 
            codeExpression(fpCurrent, yypvt[-1].tree, 0, 0);
            freeExpression(yypvt[-1].tree);
            emitCode(";\n");
          } break;
case 220:
# line 1422 "c.y"
{
            emitCode("switch(");
            codeExpression(fpCurrent, yypvt[-1].tree, 0, 0);
            emitCode(")\n");
          } break;
case 223:
# line 1438 "c.y"
{
              emitCode("else \n");
	      increaseIndent();
              icnt ++;
              PUSH(icnt);
	    } break;
case 224:
# line 1445 "c.y"
{
              decreaseIndent();
              if (icnt == POP()) 
                vcwarning("expression with little effect");
            } break;
case 225:
# line 1456 "c.y"
{ emitCode(";\n"); icnt++; } break;
case 226:
# line 1458 "c.y"
{
/*            showExpressionTree($1, 0);    */

            codeExpression(fpCurrent, yypvt[-1].tree, 0, 0);
            emitCode(";\n");
            freeExpression(yypvt[-1].tree);
          } break;
case 227:
# line 1468 "c.y"
{ yyval.tree = yypvt[-0].tree; } break;
case 228:
# line 1470 "c.y"
{ yyval.tree = cExprTree(cExprNode(_comma, 0, 0, 0), yypvt[-2].tree, yypvt[-0].tree); } break;
case 229:
# line 1474 "c.y"
{ yyval.tree = yypvt[-0].tree; } break;
case 230:
# line 1476 "c.y"
{ yyval.tree = cExprTree(yypvt[-1].node, yypvt[-2].tree, yypvt[-0].tree); } break;
case 231:
# line 1481 "c.y"
{
            yyval.node = cExprNode(_assign, 0, 0, 0);
            icnt ++;
          } break;
case 232:
# line 1485 "c.y"
{ yyval.node = cExprNode(_assignMul, 0, 0, 0); icnt ++; } break;
case 233:
# line 1486 "c.y"
{ yyval.node = cExprNode(_assignDiv, 0, 0, 0); icnt ++; } break;
case 234:
# line 1487 "c.y"
{ yyval.node = cExprNode(_assignMod, 0, 0, 0); icnt ++; } break;
case 235:
# line 1488 "c.y"
{ yyval.node = cExprNode(_assignAdd, 0, 0, 0); icnt ++; } break;
case 236:
# line 1489 "c.y"
{ yyval.node = cExprNode(_assignSub, 0, 0, 0); icnt ++; } break;
case 237:
# line 1490 "c.y"
{ yyval.node = cExprNode(_assignSr, 0, 0, 0); icnt ++; } break;
case 238:
# line 1491 "c.y"
{ yyval.node = cExprNode(_assignSl, 0, 0, 0); icnt ++; } break;
case 239:
# line 1492 "c.y"
{ yyval.node = cExprNode(_assignAnd, 0, 0, 0); icnt ++; } break;
case 240:
# line 1493 "c.y"
{ yyval.node = cExprNode(_assignXor, 0, 0, 0); icnt ++; } break;
case 241:
# line 1494 "c.y"
{ yyval.node = cExprNode(_assignOr, 0, 0, 0); icnt ++; } break;
case 242:
# line 1498 "c.y"
{ yyval.tree = yypvt[-0].tree; } break;
case 243:
# line 1500 "c.y"
{
	    yyval.tree = cExprTree(cExprNode(_if, 0, 0, 0), yypvt[-4].tree, 
			cExprTree(0, yypvt[-2].tree, yypvt[-0].tree));
	  } break;
case 244:
# line 1507 "c.y"
{ yyval.tree = yypvt[-0].tree; } break;
case 245:
# line 1509 "c.y"
{ yyval.tree = cExprTree(cExprNode(_or, 0, 0, 0), yypvt[-2].tree, yypvt[-0].tree); } break;
case 246:
# line 1513 "c.y"
{ yyval.tree = yypvt[-0].tree; } break;
case 247:
# line 1515 "c.y"
{ yyval.tree = cExprTree(cExprNode(_and, 0, 0, 0), yypvt[-2].tree, yypvt[-0].tree); } break;
case 248:
# line 1519 "c.y"
{ yyval.tree = yypvt[-0].tree; } break;
case 249:
# line 1521 "c.y"
{ yyval.tree = cExprTree(cExprNode(_bitOr, 0, 0, 0), yypvt[-2].tree, yypvt[-0].tree); } break;
case 250:
# line 1525 "c.y"
{ yyval.tree = yypvt[-0].tree; } break;
case 251:
# line 1527 "c.y"
{ yyval.tree = cExprTree(cExprNode(_bitXor, 0, 0, 0), yypvt[-2].tree, yypvt[-0].tree); } break;
case 252:
# line 1531 "c.y"
{ yyval.tree = yypvt[-0].tree; } break;
case 253:
# line 1533 "c.y"
{ yyval.tree = cExprTree(cExprNode(_bitAnd, 0, 0, 0), yypvt[-2].tree, yypvt[-0].tree); } break;
case 254:
# line 1537 "c.y"
{ yyval.tree = yypvt[-0].tree; } break;
case 255:
# line 1539 "c.y"
{ yyval.tree = cExprTree(cExprNode(_equal, 0, 0, 0), yypvt[-2].tree, yypvt[-0].tree); } break;
case 256:
# line 1541 "c.y"
{ yyval.tree = cExprTree(cExprNode(_notequal, 0, 0, 0), yypvt[-2].tree, yypvt[-0].tree); } break;
case 257:
# line 1545 "c.y"
{ yyval.tree = yypvt[-0].tree; } break;
case 258:
# line 1547 "c.y"
{ yyval.tree = cExprTree(cExprNode(_lt, 0, 0, 0), yypvt[-2].tree, yypvt[-0].tree); } break;
case 259:
# line 1549 "c.y"
{ yyval.tree = cExprTree(cExprNode(_gt, 0, 0, 0), yypvt[-2].tree, yypvt[-0].tree); } break;
case 260:
# line 1551 "c.y"
{ yyval.tree = cExprTree(cExprNode(_lte, 0, 0, 0), yypvt[-2].tree, yypvt[-0].tree); } break;
case 261:
# line 1553 "c.y"
{ yyval.tree = cExprTree(cExprNode(_gte, 0, 0, 0), yypvt[-2].tree, yypvt[-0].tree); } break;
case 262:
# line 1557 "c.y"
{ yyval.tree = yypvt[-0].tree; } break;
case 263:
# line 1559 "c.y"
{ yyval.tree = cExprTree(cExprNode(_sl, 0, 0, 0), yypvt[-2].tree, yypvt[-0].tree); } break;
case 264:
# line 1561 "c.y"
{ yyval.tree = cExprTree(cExprNode(_sr, 0, 0, 0), yypvt[-2].tree, yypvt[-0].tree); } break;
case 265:
# line 1565 "c.y"
{ yyval.tree = yypvt[-0].tree; } break;
case 266:
# line 1567 "c.y"
{ yyval.tree = cExprTree(cExprNode(_plus, 0, 0, 0), yypvt[-2].tree, yypvt[-0].tree); } break;
case 267:
# line 1569 "c.y"
{ yyval.tree = cExprTree(cExprNode(_minus, 0, 0, 0), yypvt[-2].tree, yypvt[-0].tree); } break;
case 268:
# line 1573 "c.y"
{ yyval.tree = yypvt[-0].tree; } break;
case 269:
# line 1575 "c.y"
{ yyval.tree = cExprTree(cExprNode(_mul, 0, 0, 0), yypvt[-2].tree, yypvt[-0].tree); } break;
case 270:
# line 1577 "c.y"
{ yyval.tree = cExprTree(cExprNode(_div, 0, 0, 0), yypvt[-2].tree, yypvt[-0].tree); } break;
case 271:
# line 1579 "c.y"
{ yyval.tree = cExprTree(cExprNode(_mod, 0, 0, 0), yypvt[-2].tree, yypvt[-0].tree); } break;
case 272:
# line 1583 "c.y"
{ yyval.tree = yypvt[-0].tree; } break;
case 273:
# line 1585 "c.y"
{ 
            yyval.tree = cExprTree(cExprNode(_plusplus, 0, 0, 0), 0, yypvt[-0].tree);
            icnt ++;
          } break;
case 274:
# line 1590 "c.y"
{
            yyval.tree = cExprTree(cExprNode(_minusminus, 0, 0, 0), 0, yypvt[-0].tree);
            icnt ++;
          } break;
case 275:
# line 1595 "c.y"
{ yyval.tree = cExprTree(yypvt[-1].node, 0, yypvt[-0].tree); } break;
case 276:
# line 1599 "c.y"
{ yyval.node = cExprNode(_not, 0, 0, 0); } break;
case 277:
# line 1600 "c.y"
{ yyval.node = cExprNode(_minus, 0, 0, 0); } break;
case 278:
# line 1601 "c.y"
{ yyval.node = cExprNode(_cmpl, 0, 0, 0); } break;
case 279:
# line 1605 "c.y"
{ yyval.tree = yypvt[-0].tree; } break;
case 280:
# line 1607 "c.y"
{ yyval.tree = cExprTree(cExprNode(_vector, 0, 0, 0), yypvt[-3].tree, yypvt[-1].tree); } break;
case 281:
# line 1609 "c.y"
{
		checkFunctionId(yypvt[-2].tree);
		yyval.tree = cExprTree(cExprNode(_func, 0, 0, 0), yypvt[-2].tree, 0);
                icnt ++;
	    } break;
case 282:
# line 1615 "c.y"
{
		checkFunctionId(yypvt[-3].tree);
		yyval.tree = cExprTree(cExprNode(_func, 0, 0, 0), yypvt[-3].tree, yypvt[-1].tree);
                icnt ++;
	    } break;
case 283:
# line 1621 "c.y"
{
                yyval.tree = cExprTree(cExprNode(_plusplus, 0, 0, 0), yypvt[-1].tree, 0);
                icnt ++;
            } break;
case 284:
# line 1626 "c.y"
{
                yyval.tree = cExprTree(cExprNode(_minusminus, 0, 0, 0), yypvt[-1].tree, 0);
                icnt ++;
            } break;
case 285:
# line 1634 "c.y"
{ yyval.tree = cExprTree(yypvt[-0].node, 0, 0); } break;
case 286:
# line 1635 "c.y"
{ yyval.tree = cExprTree(yypvt[-0].node, 0, 0); } break;
case 287:
# line 1636 "c.y"
{ yyval.tree = cExprTree(yypvt[-0].node, 0, 0); } break;
case 288:
# line 1637 "c.y"
{ yyval.tree = yypvt[-1].tree; } break;
case 289:
# line 1640 "c.y"
{
			    yyval.node = cExprNode(_string_const, 0,
				(char *) strcpy(malloc(strlen(this_string) + 1),
				    this_string),
                                0 /* tells: use already allocated storage */);
			} break;
case 290:
# line 1652 "c.y"
{ yyval.tree = yypvt[-0].tree; } break;
case 291:
# line 1654 "c.y"
{ yyval.tree = cExprTree(cExprNode(_comma, 0, 0, 0), yypvt[-2].tree, yypvt[-0].tree); } break;
case 292:
# line 1658 "c.y"
{ yyval.node = yypvt[-0].node; } break;
case 293:
# line 1659 "c.y"
{
                                  long temp;
                                  temp = yypvt[-0].ctid;
                                  yyval.node = cExprNode(_int_const, 0, (char *) &temp, 
                                              _SIZE_OF_INT);
                                } break;
case 294:
# line 1667 "c.y"
{ 
                                  yyval.node = cExprNode(_int_const, 0, (char *) &cval, 
                                              _SIZE_OF_INT);
                                } break;
case 295:
# line 1671 "c.y"
{
                                  yyval.node = cExprNode(_char_const, 0, (char*) &cval, 
                                              _SIZE_OF_CHAR);
                                } break;
case 296:
# line 1675 "c.y"
{
                                  yyval.node = cExprNode(_float_const, 0, (char*)&cfval,
                                              _SIZE_OF_FLOAT);
                                } break;
case 297:
# line 1679 "c.y"
{ yyval.node = cExprNode(_ident, cref, 0, 0); } break;
case 298:
# line 1680 "c.y"
{ yyval.node = cExprNode(_ident, cref, 0, 0); } break;
case 299:
# line 1681 "c.y"
{ yyval.node = cExprNode(_ident, cref, 0, 0); } break;
case 300:
# line 1685 "c.y"
{ yyval.node = cExprNode(_ident, cref, 0, 0); } break;
case 301:
# line 1686 "c.y"
{ yyval.node = cExprNode(_ident, cref, 0, 0); } break;
case 302:
# line 1687 "c.y"
{ yyval.node = cExprNode(_ident, cref, 0, 0); } break;
case 303:
# line 1688 "c.y"
{ yyval.node = cExprNode(_ident, cref, 0, 0); } break;
case 304:
# line 1689 "c.y"
{ yyval.node = cExprNode(_ident, cref, 0, 0); } break;
case 305:
# line 1690 "c.y"
{ yyval.node = cExprNode(_ident, cref, 0, 0); } break;
case 306:
# line 1691 "c.y"
{ yyval.node = cExprNode(_ident, cref, 0, 0); } break;
case 307:
# line 1692 "c.y"
{ yyval.node = cExprNode(_ident, cref, 0, 0); } break;
case 308:
# line 1693 "c.y"
{ yyval.node = cExprNode(_ident, cref, 0, 0); } break;
case 309:
# line 1694 "c.y"
{ yyval.node = cExprNode(_ident, cref, 0, 0); } break;
case 310:
# line 1695 "c.y"
{ yyval.node = cExprNode(_ident, cref, 0, 0); } break;
case 311:
# line 1696 "c.y"
{ yyval.node = cExprNode(_ident, cref, 0, 0); } break;
case 312:
# line 1697 "c.y"
{ yyval.node = cExprNode(_ident, cref, 0, 0); } break;
case 313:
# line 1699 "c.y"
{ yyval.node = (exprNode *) 0;
					vcerror("identifier not declared");
				} break;
case 314:
# line 1702 "c.y"
{ yyval.node = (exprNode *) 0;
					vcerror("expected identifier");
				} break;
case 316:
# line 1713 "c.y"
{ vcwarning("Added ';' to previous line"); yyerrok; } break;
case 318:
# line 1718 "c.y"
{ vcerror("module type not defined"); yyerrok; } break;
case 320:
# line 1723 "c.y"
{ vcerror("expected a '('"); yyerrok; } break;
case 322:
# line 1728 "c.y"
{ vcerror("expected a ')'"); yyerrok; } break;
case 324:
# line 1733 "c.y"
{ vcerror("expected a '='"); yyerrok; } break;
case 326:
# line 1738 "c.y"
{ vcerror("expected keyword 'IS'"); yyerrok; } break;
case 328:
# line 1743 "c.y"
{ vcerror("expected keyword 'AND'"); yyerrok; } break;
case 330:
# line 1748 "c.y"
{ vcerror("expected keyword 'WHEN'"); yyerrok; } break;
case 332:
# line 1754 "c.y"
{
	    vcerror("expected keyword 'DO'");
/*            printf("yytext = '%s'\n", yytext);  */
            yyerrok;
          } break;
		}
		goto yystack;  /* stack new state and value */

	}

