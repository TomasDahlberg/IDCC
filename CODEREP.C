/*
!+
!	CODEREP.C 	
!-
*/
static int currentIndentScreen = 0, 
		currentIndentScan = 0, 
		currentIndentMain = 0;
static int newLine = 0;
extern long sizeOfModule_ALARM;
extern long sizeOfModule_LOCK;

extern int listLineNo;

/* extern int usePromHooks;   */
int usePromHooks2 = 1;

int emitDebugStatement()
{
  fprintf(fpCurrent, "callDbg(%d, %d);\n", listLineNo,
        (fpCurrent == fpScan) ? 1 : 
        ((fpCurrent == fpScreen) ? 2 : ((fpCurrent == fpMain) ? 3 : 0)));
}

void increaseIndent()
{
    if (fpCurrent == fpScreen)
	currentIndentScreen += 2;
    else if (fpCurrent == fpScan)
	currentIndentScan += 2;
    else if (fpCurrent == fpMain)
	currentIndentMain += 2;
}
	
void decreaseIndent()
{
    if (fpCurrent == fpScreen)
	currentIndentScreen -= 2;
    else if (fpCurrent == fpScan)
	currentIndentScan -= 2;
    else if (fpCurrent == fpMain)
	currentIndentMain -= 2;
}

void emitIndent2(fp)
FILE *fp;
{
  int currentIndent, i;
	
  if (fp == fpScreen)
    currentIndent = currentIndentScreen;
  else if (fp == fpScan)
    currentIndent = currentIndentScan;
  else if (fp == fpMain)
    currentIndent = currentIndentMain;
  else
    currentIndent = 0;
  i = 0;
  if (currentIndent > 80) {
    vcwarning("indent > 80, skipping it for this line");
    return ;
  }
  while (i++ < currentIndent)
    fprintf(fp, " ");
}

void emitIndent()
{
  emitIndent2(fpCurrent);  
}

void emitCode2(fp, code)
FILE *fp;
char *code;
{
    int len;

    if (newLine) {
      newLine = 0;
      emitIndent2(fp);
    }
    fprintf(fp, "%s", code);
    if ((len = strlen(code)) > 0) {
	if (code[len - 1] == '\n')
	    newLine = 1;
    }
}
	
void emitCode(code)
char *code;
{
  emitCode2(fpCurrent, code);
}

void emitScanHeadings()
{
    emitCode("#include <stdio.h>\n");
    emitCode("#include <time.h>\n");
/*
    fprintf(fpCurrent, "#include \"%svvsio.h\"\n", includeDirectory);
*/
    emitCode("#include \"vvsio.h\"\n");
    emitCode("int DEBUG = 0;  /* is true if 'scan -d' otherwise false */\n");
    emitCode("int LOCAL_PRINTER = 0;  /* is true if 'scan -p' otherwise false */\n");
    if (usePromHooks2 == 0) {
      emitCode("int _any_sent_to_scan = 0;\n");
      emitCode("int screenPid = 0;\n");
    }
    emitCode("#if NO_OF_ALARMS > 0\n");
    emitCode("#include \"alarm.h\"\n");
    emitCode("#include \"timer.h\"\n");
    emitCode("#include \"lock.h\"\n");
    emitCode("struct _lockModule2 *lock2;\n");
    emitCode("struct _alarmModule *aldm;\n");
    emitCode("struct _alarmModule2 *aldm2;\n");
/*    emitCode("#include \"alarm.c\"\n");   */   /* removed 920107 (.r file) */
    emitCode("#endif\n");
}

void emitScanEndings()
{
  fprintf(fpSymTable, "#define NO_OF_ALARMS %d\n", currentNoOfAlarms ? currentNoOfAlarms : 1);
  fprintf(fpSymTable, "#define NO_OF_TIMERS %d\n", timerCounter ? timerCounter : 1);
  fprintf(fpSymTable, "struct _lockModule {\n");
  fprintf(fpSymTable, "  int noOfAlarms;\n");
  fprintf(fpSymTable, "  int alarmLock[NO_OF_ALARMS];\n");
  fprintf(fpSymTable, "  int noOfTimers;\n");
  fprintf(fpSymTable, "  int timerLock[NO_OF_TIMERS];\n");
  fprintf(fpSymTable, "} *lock; /* total size of struct = %d */\n",
          sizeOfModule_LOCK = 
        4 + 4 * ((currentNoOfAlarms) ? currentNoOfAlarms : 1) + 
        4 + 4 * ((timerCounter) ? timerCounter : 1));
  sizeOfModule_ALARM = 11208 +
        12 * ((currentNoOfAlarms) ? currentNoOfAlarms : 1);
}

void emitComment(cmt)
char *cmt;
{
    char buff[255];
    int i, j, lcmt = strlen(cmt);
    extern int yylineno;

    for(i = j = 0; i < lcmt; i++) {
	if (cmt[i] == '\n')
	    continue;
	if ((cmt[i] != '/' || cmt[i+1] != '*') && 
		(cmt[i] != '*' || cmt[i+1] != '/')) 
	    buff[j++] = cmt[i];
	else
	    i++;
    }
    buff[j] = '\0';
    if (buff[0])
	fprintf(fpCurrent, "/*%5d */ /* %s */\n", yylineno, buff);
    else
	fprintf(fpCurrent, "/*%5d */\n", yylineno);
}

int checkAndAllocate(alarmNo, vector)
int alarmNo;
int vector[];
{
  int i;
  
  if (currentNoOfAlarms >= MAX_NO_OF_ALARMS) {
    char line[80];
    sprintf(line, "Too many alarms declared, > %d", MAX_NO_OF_ALARMS);
    vcerror(line);
    return 0;
  }
  for (i = 0; i < currentNoOfAlarms; i++) {
    if (vector[i] == alarmNo) {
      vcerror("Alarm no already declared");
      return i;
    }
  }
  vector[currentNoOfAlarms] = alarmNo;
  return currentNoOfAlarms++;
}

void emitAlarmStart(alarmNo, expr, durationExpr, index, class)
int alarmNo;
exprTree *expr;
/* long duration; */
exprTree *durationExpr;
int index;
// class;
exprTree *class;
{
  char line[80];
  
  sprintf(line, "/* ------------- alarm no %d ----------------- */\n", alarmNo);
  emitCode(line);
  
  sprintf(line, "void alarm_%d()\n{\n", alarmNo);
  emitCode(line); increaseIndent();

  if (option_DEBUG)
    emitDebugStatement();

  if (usePromHooks2 == 0)
    emitCode("_any_sent_to_scan = 0;\n");
  else 
    emitCode("_clearContext();\n");
  emitCode("if (");
  codeExpression(fpScan, expr, 0, 0);
  emitCode(") {\n");
  increaseIndent();
  if (usePromHooks2 == 0)
    sprintf(line, "if (markAlarm(aldm2, %d,", index);
  else
    sprintf(line, "if (_markAlarm(aldm2, %d,", index);
  emitCode(line);

  emitCode("(int) (");
  codeExpression(fpCurrent, durationExpr, 0, 0);
  emitCode(")");
#if 0
  sprintf(line, ", %d, %d))\n", alarmNo, class);
  emitCode(line);
#else
  sprintf(line, ", %d, ", alarmNo);
  emitCode(line);
  emitCode("(int) (");
  codeExpression(fpCurrent, class, 0, 0);
  emitCode(")))\n");
#endif
  increaseIndent();
}

void emitAlarmMiddle(alarmNo, index)
int alarmNo, index;
{
  char line[80];
  decreaseIndent();
  decreaseIndent(); emitCode("}\n");
  emitCode("else if (");
  if (usePromHooks2 == 0)
    sprintf(line, "unmarkAlarm(aldm, aldm2, %d, %d))\n", index, alarmNo);
  else
    sprintf(line, "_unmarkAlarm(aldm, aldm2, %d, %d))\n", index, alarmNo);
  emitCode(line);
  increaseIndent();
}

void emitAlarmEnd(alarmNo, index, class)
int alarmNo, index;
// class;
exprTree *class;
{
  char line[80];

  decreaseIndent();
  if (usePromHooks2 == 0) {
    emitCode("if (_any_sent_to_scan)\n");
    increaseIndent();
    sprintf(line, "closeConnection(aldm, aldm2, %d, %d, %d);\n",
						    index, alarmNo, class);
    emitCode(line);
    decreaseIndent();
  } else {
#if 0
    sprintf(line, "_closeConnection(aldm, aldm2, %d, %d, %d);\n",
						    index, alarmNo, class);
    emitCode(line);
#else
    sprintf(line, "_closeConnection(aldm, aldm2, %d, %d, ",
						    index, alarmNo);
    emitCode(line);

  emitCode("(int) (");
  codeExpression(fpCurrent, class, 0, 0);
  emitCode("));\n");
#endif
  }
  decreaseIndent();
  emitCode("}\n");
}

void emitEventStart(eventNo, expr, durationExpr)
int eventNo;
exprTree *expr;
exprTree *durationExpr;
{
  char line[80];
  
  sprintf(line, "/* ------------- event no %d ----------------- */\n", eventNo);
  emitCode(line);
  
  sprintf(line, "void event_%d()\n{\n", eventNo);
  emitCode(line); increaseIndent();

  if (option_DEBUG)
    emitDebugStatement();

  emitCode("if (");
  codeExpression(fpScan, expr, 0, 0);
  emitCode(") {\n");
  increaseIndent();

  sprintf(line, "if (_checkFlank(%d, ", eventNo);
  emitCode(line);
  
  emitCode("(int) (");
  codeExpression(fpCurrent, durationExpr, 0, 0); 
  emitCode(")");

  emitCode("))\n");  
  increaseIndent();
}  

void emitEventMiddle(eventNo)
int eventNo;
{
  char line[80];
  decreaseIndent();
  decreaseIndent(); emitCode("}\n");
  emitCode("else if ("); 
  sprintf(line, "_unmarkFlank(%d))\n", eventNo);
  emitCode(line);
  increaseIndent();
}

void emitEventEnd(eventNo)
int eventNo;
{
  decreaseIndent();
  decreaseIndent();
  emitCode("}\n");
}

