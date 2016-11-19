# include <stdio.h>
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX 200
# define output(c) putc(c,yyout)
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==13?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng; extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin, *yyout;
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
/* c.l  1992-07-02 TD,  version 1.4 */
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
! c.l
! Copyright (C) 1991,1992 IVT Electronic AB.
*/
/*
!	Version 1.2, 920222, Added the following primitives:
!
!		"&"	(BIN_AND) AMPERSAND
!		"|"	BIT_OR
!		"^"	BIT_XOR
!		"~"	CMPL
!		">>"	SR
!		"<<"	SL
!		"?"	QUESTION_MARK
!		*= /= %= += -= >>= <<= &= ^= |=	also added
!
!	also added aliases for
!	
!		"&&"	AND
!		"||"	OR
!
!	Bugfix, uses atol() instead of atoi() since pc int's are 16bits only.
!
!       1.3  920526   Added keyword 'alias'
!       1.4  920702   Added keyword 'event'
*/


extern int get_it();
# undef input
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr): get_it())=='\n'?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
#ifndef vms
#define TRUE 1
#define FALSE 0
#endif

double atof();
extern int comment;
# define C 2
# define NC 4
# define STR 6
# define CCODE 8
# define CC 10
# define YYNEWLINE 13
yylex(){
int nstr; extern int yyprevious;
			   {BEGIN NC;}
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:
 		   { comment = 1; BEGIN C;}
break;
case 2:
			   ;
break;
case 3:
			   ;
break;
case 4:
	 		   { comment = 0; BEGIN NC;}
break;
case 5:
                          { BEGIN CC; }
break;
case 6:
                             ;
break;
case 7:
                            { BEGIN NC; }
break;
case 8:
	 	   { BEGIN CCODE;}
break;
case 9:
		   { emitCode(yytext);  }
break;
case 10:
		   { emitCode(yytext);  }
break;
case 11:
		   { emitCode("\n"); BEGIN NC; }
break;
case 12:
	 		   { strcpy(this_string, ""); BEGIN STR;}
break;
case 13:
		   { strcat(this_string, "\"\""); }
break;
case 14:
		   	   { BEGIN NC; 
					/*strcat(this_string, "\""); */
					return QUOTEDSTRING; 
    				   }
break;
case 15:
                         { strcat(this_string, "\\006"); }
break;
case 16:
                         { strcat(this_string, "\\004"); }
break;
case 17:
                         { strcat(this_string, "\\024"); }
break;
case 18:
                         { strcat(this_string, "\\016"); }
break;
case 19:
                         { strcat(this_string, "\\017"); }
break;
case 20:
			   { strcat(this_string, yytext);  }
break;
case 21:
		return FILTER;
break;
case 22:
		return MODULETYPE;
break;
case 23:
		return MODULE;
break;
case 24:
		return SCREEN;
break;
case 25:
		return ALARM;
break;
case 26:
			return AT;
break;
case 27:
		return MAIN;
break;
case 28:
                      return EVENT;
break;
case 29:
                       return SUBR;
break;
case 30:
                     return RETURN;
break;
case 31:
{ 

/* [^"]* {  */

/* <NC>"#"include[ \t\n]*[\"].*[\"]  */
  char buf[120], *x;
  int y;

#define TRY
#ifdef TRY
  y = 0;
  while ((y < 120) && (buf[y] = input())) {
    if (buf[y] == '"' || buf[y] == 0x0d || buf[y] == 0x0a)
      break;
    y++;
  }
  if (buf[y] != '"')
    vcerror("Non terminated string");
  buf[y] = 0;
  x = &buf[-1];
#else
  if ((y = strlen(yytext)) > 120)
    vcerror("too long string ( > 120)\n");
  input();    /* skip ending quote (") character */
  strcpy(buf, yytext);

/*
  buf[y - 1] = 0;
*/
  x = strchr(buf, '"');
#endif
  if (!x)
    vcfatal("error... c.l: ??? cannot find string start\n");
  else {
    x ++;
    doSubFile(x, 1);
  }
}
break;
case 32:
{ 
  char buf[120], *x;
  int y;

  y = 0;
  while ((y < 120) && (buf[y] = input())) {
    if (buf[y] == '>' || buf[y] == 0x0d || buf[y] == 0x0a)
      break;
    y++;
  }
  if (buf[y] != '>')
    vcerror("Missing '>'");
  else {
    buf[y] = 0;
    doSubFile(buf, 0);
  }
}
break;
case 33:
		return SYSTEMID;
break;
case 34:
		return CHAR;
break;
case 35:
			return INT;
break;
case 36:
		return FLOAT;
break;
case 37:
		return CONST;
break;
case 38:
			return IF;
break;
case 39:
		return THEN;
break;
case 40:
		return ELSE;
break;
case 41:
		return WHILE;
break;
case 42:
                      return BREAK;
break;
case 43:
                   return CONTINUE;
break;
case 44:
                     return SWITCH;
break;
case 45:
                       return CASE;
break;
case 46:
                    return DEFAULT;
break;
case 47:
		return EXTERN;
break;
case 48:
                     return REMOTE;
break;
case 49:
                      return ALIAS;
break;
case 50:
			return DO;
break;
case 51:
		return WHEN;
break;
case 52:
		return CALENDAR;
break;
case 53:
		return RIGHT;
break;
case 54:
		return DOWN;
break;
case 55:
		return HELP;
break;
case 56:
	return SCREEN_ORDER;
break;
case 57:
		return RANGE;
break;
case 58:
		return RESOLUTION;
break;
case 59:
                       return BASE;
break;
case 60:
			return IS;
break;
case 61:
		return CHANNEL;
break;
case 62:
		return EVERY;
break;
case 63:
		return DURATION;
break;
case 64:
		return BETWEEN;
break;
case 65:
			return ON;
break;
case 66:
			return OFF;
break;
case 67:
		return DECADE;
break;
case 68:
		return YEAR;
break;
case 69:
		return MON;
break;
case 70:
			return DAY;
break;
case 71:
		return HOUR;
break;
case 72:
			return MIN;
break;
case 73:
			return SEC;
break;
case 74:
			return NOT;
break;
case 75:
			return NOT;
break;
case 76:
			return OR;
break;
case 77:
		return OR;
break;
case 78:
			return AND;
break;
case 79:
		return AND;
break;
case 80:
			return BIT_OR;
break;
case 81:
			return BIT_XOR;
break;
case 82:
			return CMPL;
break;
case 83:
		 { cval = 1; return TRUE; }
break;
case 84:
		 { cval = 0; return FALSE; }
break;
case 85:
			return RED;
break;
case 86:
	 	return ORANGE;
break;
case 87:
	 	return BLACK;
break;
case 88:
		{ cval = (yytext[0] == 'j') ? 1 :
					 (yytext[0] == 'f') ? 2 : 3;
				  return MONTH; }
break;
case 89:
	{ cval = (yytext[0] == 'a') ? 4 :
					 (yytext[0] == 'm') ? 5 : 6;
				  return MONTH; }
break;
case 90:
		{ cval = (yytext[0] == 'j') ? 7 :
					 (yytext[0] == 'a') ? 8 : 9;
				  return MONTH; }
break;
case 91:
	{ cval = (yytext[0] == 'o') ? 10 :
					 (yytext[0] == 'n') ? 11 : 12;
				return MONTH; }
break;
case 92:
		return EQUAL;
break;
case 93:
			return COMMA;
break;
case 94:
			return SEMICOLON;
break;
case 95:
			return COLON;
break;
case 96:
			return LEFTPAR;
break;
case 97:
			return RIGHTPAR;
break;
case 98:
			return LEFTBRACKET;
break;
case 99:
			return RIGHTBRACKET;
break;
case 100:
			return LEFTBRACE;
break;
case 101:
			return RIGHTBRACE;
break;
case 102:
			return ASSIGN;
break;
case 103:
		return ASSIGN_MUL;
break;
case 104:
		return ASSIGN_DIV;
break;
case 105:
		return ASSIGN_MOD;
break;
case 106:
		return ASSIGN_ADD;
break;
case 107:
		return ASSIGN_SUB;
break;
case 108:
		return ASSIGN_SR;
break;
case 109:
		return ASSIGN_SL;
break;
case 110:
		return ASSIGN_AND;
break;
case 111:
		return ASSIGN_XOR;
break;
case 112:
		return ASSIGN_OR;
break;
case 113:
		return NOTEQUAL;
break;
case 114:
		return NOTEQUAL;
break;
case 115:
			return QUESTION_MARK;
break;
case 116:
			return LT;
break;
case 117:
			return GT;
break;
case 118:
		return LTE;
break;
case 119:
		return GTE;
break;
case 120:
		return SR;
break;
case 121:
		return SL;
break;
case 122:
		return PLUSPLUS;
break;
case 123:
			return PLUS;
break;
case 124:
		return MINUSMINUS;
break;
case 125:
			return MINUS;
break;
case 126:
			return MUL;
break;
case 127:
			return DIV;
break;
case 128:
			return MOD;
break;
case 129:
                        return AMPERSAND;
break;
case 130:
                      return DOTDOTDOT;
break;
case 131:
;
break;
case 132:
	;
break;
case 133:
         {   
                          if (yytext[1] == '\\')  /* <NC>"'".*"'"  */
                          {
                            if (yytext[2] == 'x')
                              sscanf(&yytext[3], "%x", &cval);
                            else
                              sscanf(&yytext[3], "%o", &cval);
                            cval &= 0xff;
                          } else
                            cval = yytext[1];
                          return INTEGER_CONSTANT;
                        }
break;
case 134:
 case 135:
           {
                          sscanf(&yytext[2], "%x", &cval);
                          return INTEGER_CONSTANT;
                        }
break;
case 136:
 case 137:
           {
                          int llop = 1;
                          cval = 0;
                          while (yytext[++llop])
                            cval = (cval << 1) | (yytext[llop] == '1');
                          return INTEGER_CONSTANT;
                        }
break;
case 138:
	{ 
#ifdef OSK
			   cval = atoi(yytext); 
#else
			   long atol();
			   cval = atol(yytext);
#endif
			   return INTEGER_CONSTANT; 
			}
break;
case 139:
 case 140:
	{ 
#ifdef OSK
				cval = atoi(yytext); 
#else
				long atol();
				cval = atol(yytext);
#endif
			  cfval = (double) cval; return FLOATING_CONSTANT; 
			}
break;
case 141:
case 142:
case 143:
	{
				cfval = (double) atof(yytext);
				return FLOATING_CONSTANT;
			}
break;
case 144:
      /* identifier */

{
    strcpy(cid, yytext);
    if (idDeclare)
	return IDENTIFIER;
	
    if (!(cref = lookUpIdent(cid)))
	return UNKNOWN_ID;

	
    if (cref->declSpec.type == _int)
	return INT_ID;
    else if (cref->declSpec.type == _float)
	return FLOAT_ID;
    else if (cref->declSpec.type == _char)
	return CHAR_ID;
    else if (cref->declSpec.type == _intFunc)
	return INT_FUNC_ID;
    else if (cref->declSpec.type == _floatFunc)
	return FLOAT_FUNC_ID;
    else if (cref->declSpec.type == _charFunc)
	return CHAR_FUNC_ID;
    else if (cref->declSpec.type == _intConst)
	return INT_CONST_ID;
    else if (cref->declSpec.type == _floatConst)
	return FLOAT_CONST_ID;
    else if (cref->declSpec.type == _charConst)
	return CHAR_CONST_ID;
    else if (cref->declSpec.type == _intVec)
	return INT_VEC_ID;
    else if (cref->declSpec.type == _floatVec)
	return FLOAT_VEC_ID;
    else if (cref->declSpec.type == _charVec)
	return CHAR_VEC_ID;
    else if (cref->declSpec.type == _intRef)
	return INT_REF_ID;
    else if (cref->declSpec.type == _floatRef)
	return FLOAT_REF_ID;
    else if (cref->declSpec.type == _charRef)
	return CHAR_REF_ID;


    else if (cref->declSpec.type == _calendar)
	return CALENDAR_ID;
    else if (cref->declSpec.type == _screen)
	return SCREEN_ID;
    else if (cref->declSpec.type == _filter)
	return FILTER_ID;			/* pt100 etc */
    else if (cref->declSpec.type == _moduletype)
	return MODULETYPE_ID;			/* ad16c4 etc */
}
break;
case 145:
                     ;
break;
case 146:
	 ;
break;
case -1:
break;
default:
fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */
int yyvstop[] ={
0,

145,
0,

145,
0,

145,
0,

145,
0,

145,
0,

145,
0,

145,
0,

145,
0,

145,
0,

145,
0,

145,
0,

145,
0,

2,
0,

3,
0,

2,
0,

146,
0,

145,
146,
0,

132,
146,
0,

145,
0,

75,
146,
0,

12,
146,
0,

146,
0,

128,
146,
0,

129,
146,
0,

146,
0,

96,
146,
0,

97,
146,
0,

126,
146,
0,

123,
146,
0,

93,
146,
0,

125,
146,
0,

146,
0,

127,
146,
0,

138,
146,
0,

138,
146,
0,

95,
146,
0,

94,
146,
0,

116,
146,
0,

102,
146,
0,

117,
146,
0,

115,
146,
0,

144,
146,
0,

98,
146,
0,

5,
146,
0,

99,
146,
0,

81,
146,
0,

144,
146,
0,

144,
146,
0,

144,
146,
0,

144,
146,
0,

144,
146,
0,

144,
146,
0,

144,
146,
0,

144,
146,
0,

144,
146,
0,

144,
146,
0,

144,
146,
0,

144,
146,
0,

144,
146,
0,

144,
146,
0,

144,
146,
0,

144,
146,
0,

144,
146,
0,

100,
146,
0,

80,
146,
0,

101,
146,
0,

82,
146,
0,

20,
0,

16,
20,
0,

15,
20,
0,

18,
20,
0,

19,
20,
0,

17,
20,
0,

14,
20,
0,

9,
0,

10,
0,

9,
0,

6,
0,

7,
0,

4,
0,

114,
0,

105,
0,

8,
0,

79,
0,

110,
0,

133,
0,

103,
0,

122,
0,

106,
0,

124,
0,

107,
0,

142,
0,

1,
0,

131,
0,

104,
0,

141,
0,

138,
0,

140,
0,

139,
0,

121,
0,

118,
0,

113,
0,

92,
0,

119,
0,

120,
0,

144,
0,

111,
0,

144,
0,

144,
0,

144,
0,

26,
144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

50,
144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

38,
144,
0,

144,
0,

60,
144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

65,
144,
0,

76,
144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

112,
0,

77,
0,

13,
0,

11,
0,

130,
0,

141,
142,
0,

137,
0,

143,
0,

135,
0,

136,
0,

134,
0,

109,
0,

108,
0,

144,
0,

144,
0,

78,
144,
0,

89,
144,
0,

90,
144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

70,
144,
0,

91,
144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

88,
144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

35,
144,
0,

144,
0,

72,
144,
0,

144,
0,

144,
0,

74,
144,
0,

91,
144,
0,

66,
144,
0,

144,
0,

144,
0,

85,
144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

73,
144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

142,
0,

141,
0,

144,
0,

144,
0,

59,
144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

45,
144,
0,

144,
0,

34,
144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

54,
144,
0,

144,
0,

40,
144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

55,
144,
0,

71,
144,
0,

27,
144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

29,
144,
0,

144,
0,

39,
144,
0,

83,
144,
0,

51,
144,
0,

144,
0,

68,
144,
0,

141,
142,
0,

25,
144,
0,

49,
144,
0,

144,
0,

87,
144,
0,

42,
144,
0,

144,
0,

144,
0,

37,
144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

28,
144,
0,

62,
144,
0,

144,
0,

84,
144,
0,

144,
0,

36,
144,
0,

144,
0,

69,
144,
0,

144,
0,

57,
144,
0,

144,
0,

144,
0,

144,
0,

53,
144,
0,

144,
0,

144,
0,

41,
144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

67,
144,
0,

144,
0,

144,
0,

47,
144,
0,

21,
144,
0,

23,
144,
0,

86,
144,
0,

48,
144,
0,

144,
0,

30,
144,
0,

24,
144,
0,

44,
144,
0,

64,
144,
0,

144,
0,

61,
144,
0,

144,
0,

46,
144,
0,

144,
0,

144,
0,

144,
0,

144,
0,

52,
144,
0,

43,
144,
0,

63,
144,
0,

144,
0,

144,
0,

144,
0,

31,
0,

32,
0,

33,
0,

144,
0,

144,
0,

144,
0,

22,
144,
0,

58,
144,
0,

144,
0,

144,
0,

56,
144,
0,
0};
# define YYTYPE int
struct yywork { YYTYPE verify, advance; } yycrank[] ={
0,0,	0,0,	3,13,	0,0,	
0,0,	0,0,	8,69,	0,0,	
8,70,	0,0,	3,13,	3,13,	
0,0,	0,0,	3,14,	8,0,	
8,71,	8,72,	0,0,	17,19,	
88,0,	0,0,	8,73,	17,19,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
8,74,	0,0,	10,77,	74,169,	
3,13,	24,86,	17,19,	3,15,	
3,13,	4,15,	5,16,	29,91,	
15,80,	3,13,	0,0,	3,13,	
31,93,	95,173,	5,17,	5,16,	
33,97,	5,18,	5,19,	102,177,	
102,177,	33,98,	20,81,	28,90,	
24,87,	29,92,	3,13,	23,84,	
31,94,	39,112,	3,13,	46,116,	
3,13,	40,113,	40,114,	33,99,	
106,181,	106,181,	5,20,	5,21,	
5,22,	109,183,	5,23,	5,24,	
5,25,	5,26,	5,27,	5,28,	
5,29,	5,30,	5,31,	5,32,	
5,33,	5,34,	5,35,	5,35,	
114,184,	23,85,	38,109,	38,110,	
38,111,	77,170,	0,0,	5,36,	
5,37,	5,38,	5,39,	5,40,	
5,41,	22,82,	5,42,	55,145,	
62,165,	63,166,	5,42,	82,171,	
5,42,	57,150,	32,95,	22,83,	
32,96,	32,96,	32,96,	32,96,	
32,96,	32,96,	32,96,	32,96,	
32,96,	32,96,	53,140,	55,146,	
83,172,	118,187,	49,126,	119,188,	
5,43,	5,44,	5,45,	5,46,	
53,141,	49,127,	5,47,	5,48,	
5,49,	5,50,	5,51,	5,52,	
49,128,	5,53,	5,54,	5,55,	
6,18,	51,133,	5,56,	5,57,	
5,58,	61,163,	121,189,	5,59,	
5,60,	5,61,	122,190,	51,134,	
5,62,	51,135,	5,63,	61,164,	
5,64,	5,65,	5,66,	5,67,	
65,167,	6,20,	6,21,	6,22,	
123,191,	6,23,	6,24,	124,192,	
6,26,	6,27,	6,28,	125,193,	
6,30,	6,31,	6,32,	6,33,	
47,117,	6,35,	47,118,	50,129,	
47,119,	127,196,	128,197,	50,130,	
47,120,	47,121,	6,36,	6,37,	
6,38,	6,39,	6,40,	6,41,	
48,122,	50,131,	52,136,	129,198,	
48,123,	131,201,	52,137,	50,132,	
54,142,	132,202,	52,138,	48,124,	
56,147,	52,139,	59,156,	117,185,	
54,143,	48,125,	59,157,	133,203,	
56,148,	54,144,	59,158,	117,186,	
134,204,	126,194,	56,149,	6,43,	
6,44,	6,45,	6,46,	65,168,	
126,195,	6,47,	6,48,	6,49,	
6,50,	6,51,	6,52,	135,205,	
6,53,	6,54,	6,55,	136,206,	
137,207,	6,56,	6,57,	6,58,	
138,208,	139,209,	6,59,	6,60,	
6,61,	130,199,	140,210,	6,62,	
130,200,	6,63,	141,211,	6,64,	
6,65,	6,66,	6,67,	7,68,	
143,212,	145,207,	7,69,	146,189,	
7,70,	146,188,	58,151,	7,68,	
7,68,	58,152,	148,214,	7,0,	
7,71,	7,72,	58,153,	9,75,	
149,215,	58,154,	7,73,	151,218,	
60,159,	58,155,	60,160,	9,75,	
9,75,	152,219,	149,216,	9,76,	
150,217,	153,218,	150,218,	155,220,	
7,74,	147,213,	147,188,	11,78,	
156,221,	7,68,	60,161,	158,226,	
60,162,	7,68,	147,207,	11,78,	
11,78,	157,222,	7,68,	11,79,	
7,68,	147,188,	159,227,	9,77,	
160,228,	9,75,	157,223,	161,229,	
162,230,	9,75,	163,231,	164,232,	
157,224,	157,225,	9,75,	7,68,	
9,75,	160,189,	25,88,	7,68,	
165,233,	7,68,	166,235,	171,236,	
165,234,	11,78,	25,88,	25,88,	
172,237,	11,78,	25,0,	9,75,	
175,240,	185,243,	11,78,	9,75,	
11,78,	9,75,	34,100,	186,244,	
34,101,	34,101,	34,101,	34,101,	
34,101,	34,101,	34,101,	34,101,	
34,101,	34,101,	190,245,	11,78,	
191,246,	192,247,	193,248,	11,78,	
25,89,	11,78,	34,102,	194,249,	
25,88,	34,103,	34,104,	195,250,	
175,240,	25,88,	199,255,	25,88,	
35,100,	200,256,	35,101,	35,101,	
35,101,	35,101,	35,101,	35,101,	
35,101,	35,101,	35,101,	35,101,	
34,105,	196,251,	25,88,	197,253,	
197,254,	196,252,	25,88,	201,257,	
25,88,	202,258,	34,106,	35,103,	
35,104,	34,103,	34,107,	103,178,	
203,259,	103,178,	205,262,	206,263,	
103,179,	103,179,	103,179,	103,179,	
103,179,	103,179,	103,179,	103,179,	
103,179,	103,179,	208,264,	209,265,	
34,108,	42,115,	42,115,	42,115,	
42,115,	42,115,	42,115,	42,115,	
42,115,	42,115,	42,115,	35,103,	
35,107,	204,260,	210,266,	211,267,	
213,268,	204,261,	42,115,	42,115,	
42,115,	42,115,	42,115,	42,115,	
42,115,	42,115,	42,115,	42,115,	
42,115,	42,115,	42,115,	42,115,	
42,115,	42,115,	42,115,	42,115,	
42,115,	42,115,	42,115,	42,115,	
42,115,	42,115,	42,115,	42,115,	
215,269,	216,270,	220,271,	221,272,	
42,115,	223,273,	42,115,	42,115,	
42,115,	42,115,	42,115,	42,115,	
42,115,	42,115,	42,115,	42,115,	
42,115,	42,115,	42,115,	42,115,	
42,115,	42,115,	42,115,	42,115,	
42,115,	42,115,	42,115,	42,115,	
42,115,	42,115,	42,115,	42,115,	
96,96,	96,96,	96,96,	96,96,	
96,96,	96,96,	96,96,	96,96,	
96,96,	96,96,	98,98,	224,274,	
225,275,	226,276,	227,277,	229,278,	
230,279,	231,280,	98,98,	98,0,	
232,281,	96,174,	98,98,	100,175,	
100,175,	100,175,	100,175,	100,175,	
100,175,	100,175,	100,175,	100,175,	
100,175,	178,179,	178,179,	178,179,	
178,179,	178,179,	178,179,	178,179,	
178,179,	178,179,	178,179,	233,282,	
100,176,	234,283,	235,284,	236,285,	
98,98,	237,286,	243,289,	244,290,	
98,98,	96,174,	246,291,	247,292,	
248,293,	98,98,	249,294,	98,98,	
251,295,	253,296,	254,297,	255,298,	
105,180,	105,180,	105,180,	105,180,	
105,180,	105,180,	105,180,	105,180,	
105,180,	105,180,	98,98,	256,299,	
100,176,	258,300,	98,98,	260,301,	
98,98,	105,180,	105,180,	105,180,	
105,180,	105,180,	105,180,	108,182,	
108,182,	108,182,	108,182,	108,182,	
108,182,	108,182,	108,182,	108,182,	
108,182,	261,302,	262,303,	263,304,	
264,305,	265,306,	269,307,	270,308,	
108,182,	108,182,	108,182,	108,182,	
108,182,	108,182,	271,309,	272,310,	
273,311,	105,180,	105,180,	105,180,	
105,180,	105,180,	105,180,	174,238,	
274,312,	174,238,	275,313,	276,314,	
174,239,	174,239,	174,239,	174,239,	
174,239,	174,239,	174,239,	174,239,	
174,239,	174,239,	277,315,	279,316,	
108,182,	108,182,	108,182,	108,182,	
108,182,	108,182,	176,241,	283,317,	
176,241,	285,318,	286,319,	176,242,	
176,242,	176,242,	176,242,	176,242,	
176,242,	176,242,	176,242,	176,242,	
176,242,	238,239,	238,239,	238,239,	
238,239,	238,239,	238,239,	238,239,	
238,239,	238,239,	238,239,	240,287,	
291,320,	240,287,	294,321,	295,322,	
240,288,	240,288,	240,288,	240,288,	
240,288,	240,288,	240,288,	240,288,	
240,288,	240,288,	241,242,	241,242,	
241,242,	241,242,	241,242,	241,242,	
241,242,	241,242,	241,242,	241,242,	
287,288,	287,288,	287,288,	287,288,	
287,288,	287,288,	287,288,	287,288,	
287,288,	287,288,	297,323,	298,324,	
299,325,	300,326,	303,327,	305,328,	
307,329,	309,330,	311,331,	312,332,	
313,333,	315,334,	316,335,	318,336,	
319,337,	320,338,	321,339,	322,340,	
323,341,	325,342,	326,343,	329,344,	
332,345,	334,346,	336,347,	337,348,	
339,349,	341,350,	343,351,	344,352,	
345,353,	346,354,	347,347,	348,357,	
352,358,	353,359,	347,347,	354,360,	
358,361,	359,362,	360,363,	363,364,	
364,365,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	347,347,	0,0,	347,355,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	347,356,	0,0,	0,0,	
0,0};
struct yysvf yysvec[] ={
0,	0,	0,
yycrank+0,	0,		yyvstop+1,
yycrank+0,	0,		yyvstop+3,
yycrank+-1,	0,		yyvstop+5,
yycrank+-3,	yysvec+3,	yyvstop+7,
yycrank+-45,	0,		yyvstop+9,
yycrank+-140,	yysvec+5,	yyvstop+11,
yycrank+-266,	0,		yyvstop+13,
yycrank+-2,	yysvec+7,	yyvstop+15,
yycrank+-282,	0,		yyvstop+17,
yycrank+-1,	yysvec+9,	yyvstop+19,
yycrank+-302,	0,		yyvstop+21,
yycrank+0,	yysvec+11,	yyvstop+23,
yycrank+0,	0,		yyvstop+25,
yycrank+0,	0,		yyvstop+27,
yycrank+1,	0,		yyvstop+29,
yycrank+0,	0,		yyvstop+31,
yycrank+10,	0,		yyvstop+33,
yycrank+0,	0,		yyvstop+36,
yycrank+0,	yysvec+17,	yyvstop+39,
yycrank+1,	0,		yyvstop+41,
yycrank+0,	0,		yyvstop+44,
yycrank+4,	0,		yyvstop+47,
yycrank+6,	0,		yyvstop+49,
yycrank+3,	0,		yyvstop+52,
yycrank+-333,	0,		yyvstop+55,
yycrank+0,	0,		yyvstop+57,
yycrank+0,	0,		yyvstop+60,
yycrank+2,	0,		yyvstop+63,
yycrank+4,	0,		yyvstop+66,
yycrank+0,	0,		yyvstop+69,
yycrank+7,	0,		yyvstop+72,
yycrank+72,	0,		yyvstop+75,
yycrank+14,	0,		yyvstop+77,
yycrank+308,	0,		yyvstop+80,
yycrank+338,	0,		yyvstop+83,
yycrank+0,	0,		yyvstop+86,
yycrank+0,	0,		yyvstop+89,
yycrank+38,	0,		yyvstop+92,
yycrank+8,	0,		yyvstop+95,
yycrank+12,	0,		yyvstop+98,
yycrank+0,	0,		yyvstop+101,
yycrank+381,	0,		yyvstop+104,
yycrank+0,	0,		yyvstop+107,
yycrank+0,	0,		yyvstop+110,
yycrank+0,	0,		yyvstop+113,
yycrank+10,	0,		yyvstop+116,
yycrank+80,	yysvec+42,	yyvstop+119,
yycrank+107,	yysvec+42,	yyvstop+122,
yycrank+37,	yysvec+42,	yyvstop+125,
yycrank+94,	yysvec+42,	yyvstop+128,
yycrank+45,	yysvec+42,	yyvstop+131,
yycrank+109,	yysvec+42,	yyvstop+134,
yycrank+29,	yysvec+42,	yyvstop+137,
yycrank+110,	yysvec+42,	yyvstop+140,
yycrank+14,	yysvec+42,	yyvstop+143,
yycrank+119,	yysvec+42,	yyvstop+146,
yycrank+6,	yysvec+42,	yyvstop+149,
yycrank+175,	yysvec+42,	yyvstop+152,
yycrank+121,	yysvec+42,	yyvstop+155,
yycrank+189,	yysvec+42,	yyvstop+158,
yycrank+53,	yysvec+42,	yyvstop+161,
yycrank+8,	yysvec+42,	yyvstop+164,
yycrank+12,	yysvec+42,	yyvstop+167,
yycrank+0,	0,		yyvstop+170,
yycrank+111,	0,		yyvstop+173,
yycrank+0,	0,		yyvstop+176,
yycrank+0,	0,		yyvstop+179,
yycrank+0,	0,		yyvstop+182,
yycrank+0,	0,		yyvstop+184,
yycrank+0,	0,		yyvstop+187,
yycrank+0,	0,		yyvstop+190,
yycrank+0,	0,		yyvstop+193,
yycrank+0,	0,		yyvstop+196,
yycrank+5,	0,		yyvstop+199,
yycrank+0,	0,		yyvstop+202,
yycrank+0,	0,		yyvstop+204,
yycrank+8,	0,		yyvstop+206,
yycrank+0,	0,		yyvstop+208,
yycrank+0,	0,		yyvstop+210,
yycrank+0,	0,		yyvstop+212,
yycrank+0,	0,		yyvstop+214,
yycrank+5,	0,		0,	
yycrank+11,	0,		0,	
yycrank+0,	0,		yyvstop+216,
yycrank+0,	0,		yyvstop+218,
yycrank+0,	0,		yyvstop+220,
yycrank+0,	0,		yyvstop+222,
yycrank+-7,	yysvec+25,	0,	
yycrank+0,	0,		yyvstop+224,
yycrank+0,	0,		yyvstop+226,
yycrank+0,	0,		yyvstop+228,
yycrank+0,	0,		yyvstop+230,
yycrank+0,	0,		yyvstop+232,
yycrank+0,	0,		yyvstop+234,
yycrank+7,	0,		0,	
yycrank+456,	0,		yyvstop+236,
yycrank+0,	0,		yyvstop+238,
yycrank+-513,	0,		yyvstop+240,
yycrank+0,	0,		yyvstop+242,
yycrank+479,	0,		yyvstop+244,
yycrank+0,	yysvec+35,	yyvstop+246,
yycrank+11,	0,		0,	
yycrank+368,	0,		0,	
yycrank+0,	0,		yyvstop+248,
yycrank+520,	0,		0,	
yycrank+28,	0,		0,	
yycrank+0,	0,		yyvstop+250,
yycrank+543,	0,		0,	
yycrank+20,	0,		yyvstop+252,
yycrank+0,	0,		yyvstop+254,
yycrank+0,	0,		yyvstop+256,
yycrank+0,	0,		yyvstop+258,
yycrank+0,	0,		yyvstop+260,
yycrank+35,	0,		yyvstop+262,
yycrank+0,	yysvec+42,	yyvstop+264,
yycrank+0,	0,		yyvstop+266,
yycrank+122,	yysvec+42,	yyvstop+268,
yycrank+33,	yysvec+42,	yyvstop+270,
yycrank+21,	yysvec+42,	yyvstop+272,
yycrank+0,	yysvec+42,	yyvstop+274,
yycrank+55,	yysvec+42,	yyvstop+277,
yycrank+47,	yysvec+42,	yyvstop+279,
yycrank+60,	yysvec+42,	yyvstop+281,
yycrank+82,	yysvec+42,	yyvstop+283,
yycrank+82,	yysvec+42,	yyvstop+285,
yycrank+121,	yysvec+42,	yyvstop+287,
yycrank+96,	yysvec+42,	yyvstop+289,
yycrank+84,	yysvec+42,	yyvstop+291,
yycrank+86,	yysvec+42,	yyvstop+293,
yycrank+158,	yysvec+42,	yyvstop+295,
yycrank+90,	yysvec+42,	yyvstop+297,
yycrank+99,	yysvec+42,	yyvstop+300,
yycrank+108,	yysvec+42,	yyvstop+302,
yycrank+127,	yysvec+42,	yyvstop+304,
yycrank+127,	yysvec+42,	yyvstop+306,
yycrank+139,	yysvec+42,	yyvstop+308,
yycrank+150,	yysvec+42,	yyvstop+310,
yycrank+144,	yysvec+42,	yyvstop+312,
yycrank+142,	yysvec+42,	yyvstop+314,
yycrank+150,	yysvec+42,	yyvstop+316,
yycrank+145,	yysvec+42,	yyvstop+318,
yycrank+0,	yysvec+42,	yyvstop+320,
yycrank+152,	yysvec+42,	yyvstop+323,
yycrank+0,	yysvec+42,	yyvstop+325,
yycrank+159,	yysvec+42,	yyvstop+328,
yycrank+163,	yysvec+42,	yyvstop+330,
yycrank+196,	yysvec+42,	yyvstop+332,
yycrank+168,	yysvec+42,	yyvstop+334,
yycrank+184,	yysvec+42,	yyvstop+336,
yycrank+180,	yysvec+42,	yyvstop+338,
yycrank+171,	yysvec+42,	yyvstop+340,
yycrank+191,	yysvec+42,	yyvstop+342,
yycrank+181,	yysvec+42,	yyvstop+344,
yycrank+0,	yysvec+42,	yyvstop+346,
yycrank+202,	yysvec+42,	yyvstop+349,
yycrank+194,	yysvec+42,	yyvstop+352,
yycrank+213,	yysvec+42,	yyvstop+354,
yycrank+204,	yysvec+42,	yyvstop+356,
yycrank+204,	yysvec+42,	yyvstop+358,
yycrank+221,	yysvec+42,	yyvstop+360,
yycrank+225,	yysvec+42,	yyvstop+362,
yycrank+219,	yysvec+42,	yyvstop+364,
yycrank+225,	yysvec+42,	yyvstop+366,
yycrank+210,	yysvec+42,	yyvstop+368,
yycrank+235,	yysvec+42,	yyvstop+370,
yycrank+241,	yysvec+42,	yyvstop+372,
yycrank+0,	0,		yyvstop+374,
yycrank+0,	0,		yyvstop+376,
yycrank+0,	0,		yyvstop+378,
yycrank+0,	0,		yyvstop+380,
yycrank+240,	0,		0,	
yycrank+229,	0,		0,	
yycrank+0,	0,		yyvstop+382,
yycrank+580,	0,		0,	
yycrank+279,	yysvec+100,	yyvstop+384,
yycrank+603,	0,		0,	
yycrank+0,	yysvec+102,	yyvstop+387,
yycrank+489,	0,		0,	
yycrank+0,	yysvec+178,	yyvstop+389,
yycrank+0,	yysvec+105,	yyvstop+391,
yycrank+0,	yysvec+106,	yyvstop+393,
yycrank+0,	yysvec+108,	yyvstop+395,
yycrank+0,	0,		yyvstop+397,
yycrank+0,	0,		yyvstop+399,
yycrank+235,	yysvec+42,	yyvstop+401,
yycrank+258,	yysvec+42,	yyvstop+403,
yycrank+0,	yysvec+42,	yyvstop+405,
yycrank+0,	yysvec+42,	yyvstop+408,
yycrank+0,	yysvec+42,	yyvstop+411,
yycrank+265,	yysvec+42,	yyvstop+414,
yycrank+249,	yysvec+42,	yyvstop+416,
yycrank+270,	yysvec+42,	yyvstop+418,
yycrank+273,	yysvec+42,	yyvstop+420,
yycrank+274,	yysvec+42,	yyvstop+422,
yycrank+278,	yysvec+42,	yyvstop+424,
yycrank+287,	yysvec+42,	yyvstop+426,
yycrank+284,	yysvec+42,	yyvstop+428,
yycrank+0,	yysvec+42,	yyvstop+430,
yycrank+285,	yysvec+42,	yyvstop+433,
yycrank+288,	yysvec+42,	yyvstop+436,
yycrank+293,	yysvec+42,	yyvstop+438,
yycrank+308,	yysvec+42,	yyvstop+440,
yycrank+311,	yysvec+42,	yyvstop+442,
yycrank+331,	yysvec+42,	yyvstop+444,
yycrank+313,	yysvec+42,	yyvstop+446,
yycrank+300,	yysvec+42,	yyvstop+448,
yycrank+0,	yysvec+42,	yyvstop+450,
yycrank+310,	yysvec+42,	yyvstop+453,
yycrank+330,	yysvec+42,	yyvstop+455,
yycrank+330,	yysvec+42,	yyvstop+457,
yycrank+329,	yysvec+42,	yyvstop+459,
yycrank+0,	yysvec+42,	yyvstop+461,
yycrank+334,	yysvec+42,	yyvstop+464,
yycrank+0,	yysvec+42,	yyvstop+466,
yycrank+355,	yysvec+42,	yyvstop+469,
yycrank+357,	yysvec+42,	yyvstop+471,
yycrank+0,	yysvec+42,	yyvstop+473,
yycrank+0,	yysvec+42,	yyvstop+476,
yycrank+0,	yysvec+42,	yyvstop+479,
yycrank+364,	yysvec+42,	yyvstop+482,
yycrank+372,	yysvec+42,	yyvstop+484,
yycrank+0,	yysvec+42,	yyvstop+486,
yycrank+366,	yysvec+42,	yyvstop+489,
yycrank+404,	yysvec+42,	yyvstop+491,
yycrank+399,	yysvec+42,	yyvstop+493,
yycrank+413,	yysvec+42,	yyvstop+495,
yycrank+417,	yysvec+42,	yyvstop+497,
yycrank+0,	yysvec+42,	yyvstop+499,
yycrank+405,	yysvec+42,	yyvstop+502,
yycrank+404,	yysvec+42,	yyvstop+504,
yycrank+411,	yysvec+42,	yyvstop+506,
yycrank+423,	yysvec+42,	yyvstop+508,
yycrank+437,	yysvec+42,	yyvstop+510,
yycrank+441,	yysvec+42,	yyvstop+512,
yycrank+436,	yysvec+42,	yyvstop+514,
yycrank+443,	0,		0,	
yycrank+437,	0,		0,	
yycrank+613,	0,		0,	
yycrank+0,	yysvec+238,	yyvstop+516,
yycrank+628,	0,		0,	
yycrank+638,	0,		0,	
yycrank+0,	yysvec+241,	yyvstop+518,
yycrank+445,	yysvec+42,	yyvstop+520,
yycrank+440,	yysvec+42,	yyvstop+522,
yycrank+0,	yysvec+42,	yyvstop+524,
yycrank+457,	yysvec+42,	yyvstop+527,
yycrank+452,	yysvec+42,	yyvstop+529,
yycrank+453,	yysvec+42,	yyvstop+531,
yycrank+452,	yysvec+42,	yyvstop+533,
yycrank+0,	yysvec+42,	yyvstop+535,
yycrank+454,	yysvec+42,	yyvstop+538,
yycrank+0,	yysvec+42,	yyvstop+540,
yycrank+449,	yysvec+42,	yyvstop+543,
yycrank+461,	yysvec+42,	yyvstop+545,
yycrank+467,	yysvec+42,	yyvstop+547,
yycrank+462,	yysvec+42,	yyvstop+549,
yycrank+0,	yysvec+42,	yyvstop+551,
yycrank+465,	yysvec+42,	yyvstop+554,
yycrank+0,	yysvec+42,	yyvstop+556,
yycrank+467,	yysvec+42,	yyvstop+559,
yycrank+480,	yysvec+42,	yyvstop+561,
yycrank+488,	yysvec+42,	yyvstop+563,
yycrank+502,	yysvec+42,	yyvstop+565,
yycrank+503,	yysvec+42,	yyvstop+567,
yycrank+489,	yysvec+42,	yyvstop+569,
yycrank+0,	yysvec+42,	yyvstop+571,
yycrank+0,	yysvec+42,	yyvstop+574,
yycrank+0,	yysvec+42,	yyvstop+577,
yycrank+498,	yysvec+42,	yyvstop+580,
yycrank+503,	yysvec+42,	yyvstop+582,
yycrank+511,	yysvec+42,	yyvstop+584,
yycrank+514,	yysvec+42,	yyvstop+586,
yycrank+500,	yysvec+42,	yyvstop+588,
yycrank+516,	yysvec+42,	yyvstop+590,
yycrank+512,	yysvec+42,	yyvstop+592,
yycrank+511,	yysvec+42,	yyvstop+594,
yycrank+537,	yysvec+42,	yyvstop+596,
yycrank+0,	yysvec+42,	yyvstop+598,
yycrank+540,	yysvec+42,	yyvstop+601,
yycrank+0,	yysvec+42,	yyvstop+603,
yycrank+0,	yysvec+42,	yyvstop+606,
yycrank+0,	yysvec+42,	yyvstop+609,
yycrank+546,	yysvec+42,	yyvstop+612,
yycrank+0,	yysvec+42,	yyvstop+614,
yycrank+532,	0,		0,	
yycrank+549,	0,		0,	
yycrank+648,	0,		0,	
yycrank+0,	yysvec+287,	yyvstop+617,
yycrank+0,	yysvec+42,	yyvstop+620,
yycrank+0,	yysvec+42,	yyvstop+623,
yycrank+571,	yysvec+42,	yyvstop+626,
yycrank+0,	yysvec+42,	yyvstop+628,
yycrank+0,	yysvec+42,	yyvstop+631,
yycrank+574,	yysvec+42,	yyvstop+634,
yycrank+574,	yysvec+42,	yyvstop+636,
yycrank+0,	yysvec+42,	yyvstop+638,
yycrank+596,	yysvec+42,	yyvstop+641,
yycrank+606,	yysvec+42,	yyvstop+643,
yycrank+600,	yysvec+42,	yyvstop+645,
yycrank+604,	yysvec+42,	yyvstop+647,
yycrank+0,	yysvec+42,	yyvstop+649,
yycrank+0,	yysvec+42,	yyvstop+652,
yycrank+600,	yysvec+42,	yyvstop+655,
yycrank+0,	yysvec+42,	yyvstop+657,
yycrank+597,	yysvec+42,	yyvstop+660,
yycrank+0,	yysvec+42,	yyvstop+662,
yycrank+611,	yysvec+42,	yyvstop+665,
yycrank+0,	yysvec+42,	yyvstop+667,
yycrank+612,	yysvec+42,	yyvstop+670,
yycrank+0,	yysvec+42,	yyvstop+672,
yycrank+613,	yysvec+42,	yyvstop+675,
yycrank+598,	yysvec+42,	yyvstop+677,
yycrank+606,	yysvec+42,	yyvstop+679,
yycrank+0,	yysvec+42,	yyvstop+681,
yycrank+607,	yysvec+42,	yyvstop+684,
yycrank+614,	yysvec+42,	yyvstop+686,
yycrank+0,	yysvec+42,	yyvstop+688,
yycrank+619,	0,		0,	
yycrank+611,	0,		0,	
yycrank+611,	yysvec+42,	yyvstop+691,
yycrank+625,	yysvec+42,	yyvstop+693,
yycrank+615,	yysvec+42,	yyvstop+695,
yycrank+607,	yysvec+42,	yyvstop+697,
yycrank+0,	yysvec+42,	yyvstop+699,
yycrank+609,	yysvec+42,	yyvstop+702,
yycrank+615,	yysvec+42,	yyvstop+704,
yycrank+0,	yysvec+42,	yyvstop+706,
yycrank+0,	yysvec+42,	yyvstop+709,
yycrank+611,	yysvec+42,	yyvstop+712,
yycrank+0,	yysvec+42,	yyvstop+715,
yycrank+0,	yysvec+42,	yyvstop+718,
yycrank+612,	yysvec+42,	yyvstop+721,
yycrank+0,	yysvec+42,	yyvstop+723,
yycrank+634,	yysvec+42,	yyvstop+726,
yycrank+0,	yysvec+42,	yyvstop+729,
yycrank+629,	0,		0,	
yycrank+626,	0,		0,	
yycrank+0,	yysvec+42,	yyvstop+732,
yycrank+618,	yysvec+42,	yyvstop+735,
yycrank+0,	yysvec+42,	yyvstop+737,
yycrank+632,	yysvec+42,	yyvstop+740,
yycrank+0,	yysvec+42,	yyvstop+742,
yycrank+624,	yysvec+42,	yyvstop+745,
yycrank+614,	yysvec+42,	yyvstop+747,
yycrank+631,	yysvec+42,	yyvstop+749,
yycrank+626,	yysvec+42,	yyvstop+751,
yycrank+729,	0,		0,	
yycrank+639,	0,		0,	
yycrank+0,	yysvec+42,	yyvstop+753,
yycrank+0,	yysvec+42,	yyvstop+756,
yycrank+0,	yysvec+42,	yyvstop+759,
yycrank+628,	yysvec+42,	yyvstop+762,
yycrank+630,	yysvec+42,	yyvstop+764,
yycrank+629,	yysvec+42,	yyvstop+766,
yycrank+0,	0,		yyvstop+768,
yycrank+0,	0,		yyvstop+770,
yycrank+0,	0,		yyvstop+772,
yycrank+643,	yysvec+42,	yyvstop+774,
yycrank+635,	yysvec+42,	yyvstop+776,
yycrank+646,	yysvec+42,	yyvstop+778,
yycrank+0,	yysvec+42,	yyvstop+780,
yycrank+0,	yysvec+42,	yyvstop+783,
yycrank+646,	yysvec+42,	yyvstop+786,
yycrank+634,	yysvec+42,	yyvstop+788,
yycrank+0,	yysvec+42,	yyvstop+790,
0,	0,	0};
struct yywork *yytop = yycrank+789;
struct yysvf *yybgin = yysvec+1;
char yymatch[] ={
00  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,011 ,012 ,01  ,01  ,015 ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
011 ,01  ,01  ,01  ,01  ,01  ,01  ,047 ,
01  ,01  ,01  ,'+' ,01  ,'+' ,01  ,01  ,
'0' ,'0' ,'2' ,'2' ,'2' ,'2' ,'2' ,'2' ,
'2' ,'2' ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,'A' ,'A' ,'A' ,'A' ,'E' ,'A' ,'G' ,
'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,
'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,
'G' ,'G' ,'G' ,01  ,01  ,01  ,01  ,'G' ,
01  ,'A' ,'A' ,'A' ,'A' ,'E' ,'A' ,'G' ,
'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,
'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,
'G' ,'G' ,'G' ,01  ,01  ,01  ,01  ,01  ,
0};
char yyextra[] ={
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
/*	ncform	4.1	83/08/11	*/

int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
char yytext[YYLMAX];
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
yylook(){
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	int yych;
	struct yywork *yyr;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank){		/* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
			*yylastch++ = yych = input();
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (int)yyt > (int)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((int)yyt < (int)yycrank) {		/* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0  /* && feof(yyin) */)
			{
			yysptr=yysbuf;
			return(0);
			}
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
yyback(p, m)
	int *p;
{
if (p==0) return(0);
while (*p)
	{
	if (*p++ == m)
		return(1);
	}
return(0);
}
	/* the following are only used in the lex library */
yyinput(){
	return(input());
	}
yyoutput(c)
  int c; {
	output(c);
	}
yyunput(c)
   int c; {
	unput(c);
	}
