%{
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
%}
/*  c.l 	*/

D	[0-9]
E	[Ee][-+]?{D}+
H       [0-9AaBbCcDdEeFf]
B       [01]

%e 2000
%a 25000
%o 5000
%p 5000
%START C NC STR CCODE CC
a	[Aa]
b	[Bb]
c	[Cc]
d	[Dd]
e	[Ee]
f	[Ff]
g	[Gg]
h	[Hh]
i	[Ii]
j	[Jj]
k	[Kk]
l	[Ll]
m	[Mm]
n	[Nn]
o	[Oo]
p	[Pp]
q	[Qq]
r	[Rr]
s	[Ss]
t	[Tt]
u	[Uu]
v	[Vv]
w	[Ww]
x	[Xx]
y	[Yy]
z	[Zz]
aa      \06
ae      \04
oe      \24
AE      \16
CF      \17

LF      \12
CR      \15
FF      \14

%%
				   {BEGIN NC;}
<NC>"/*"	 		   { comment = 1; BEGIN C;}
<C>.				   ;
<C>\n				   ;
<C>"*/"		 		   { comment = 0; BEGIN NC;}

<NC>"\\"                           { BEGIN CC; }
<CC>.                              ;
<CC>\n                             { BEGIN NC; }


<NC>"%["		 	   { BEGIN CCODE;}
<CCODE>.			   { emitCode(yytext);  }
<CCODE>\n			   { emitCode(yytext);  }
<CCODE>"%]"			   { emitCode("\n"); BEGIN NC; }

<NC>\"		 		   { strcpy(this_string, ""); BEGIN STR;}
<STR>\"\"			   { strcat(this_string, "\"\""); }
<STR>\"			   	   { BEGIN NC; 
					/*strcat(this_string, "\""); */
					return QUOTEDSTRING; 
    				   }
<STR>{aa}                          { strcat(this_string, "\\006"); }
<STR>{ae}                          { strcat(this_string, "\\004"); }
<STR>{oe}                          { strcat(this_string, "\\024"); }
<STR>{AE}                          { strcat(this_string, "\\016"); }
<STR>{CF}                          { strcat(this_string, "\\017"); }

<STR>.				   { strcat(this_string, yytext);  }

<NC>filter			return FILTER;
<NC>moduletype			return MODULETYPE;
<NC>module			return MODULE;
<NC>screen			return SCREEN;
<NC>alarm			return ALARM;
<NC>at				return AT;
<NC>main			return MAIN;
<NC>event                       return EVENT;
<NC>subr                        return SUBR;
<NC>return                      return RETURN;
<NC>"#"include[ \t\n]*\" { 

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
<NC>"#"include[ \t\n]*"<" { 
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
<NC>"#"systemid			return SYSTEMID;
<NC>char			return CHAR;
<NC>int				return INT;
<NC>float			return FLOAT;
<NC>const			return CONST;
<NC>if				return IF;
<NC>then			return THEN;
<NC>else			return ELSE;
<NC>while			return WHILE;
<NC>break                       return BREAK;
<NC>continue                    return CONTINUE;
<NC>switch                      return SWITCH;
<NC>case                        return CASE;
<NC>default                     return DEFAULT;

<NC>extern			return EXTERN;
<NC>remote                      return REMOTE;
<NC>alias                       return ALIAS;

<NC>do				return DO;
<NC>when			return WHEN;
<NC>calendar			return CALENDAR;
<NC>right			return RIGHT;
<NC>down			return DOWN;
<NC>help			return HELP;

<NC>screen_order		return SCREEN_ORDER;
<NC>range			return RANGE;
<NC>resolution			return RESOLUTION;
<NC>base                        return BASE;
<NC>is				return IS;
<NC>channel			return CHANNEL;
<NC>every			return EVERY;
<NC>duration			return DURATION;
<NC>between			return BETWEEN;
<NC>on				return ON;
<NC>off				return OFF;
<NC>decade			return DECADE;
<NC>year			return YEAR;
<NC>month			return MON;
<NC>day				return DAY;
<NC>hour			return HOUR;
<NC>min				return MIN;
<NC>sec				return SEC;
<NC>not				return NOT;
<NC>"!"				return NOT;
<NC>or				return OR;
<NC>"||"			return OR;
<NC>and				return AND;
<NC>"&&"			return AND;
<NC>"|"				return BIT_OR;
<NC>"^"				return BIT_XOR;
<NC>"~"				return CMPL;

<NC>true			 { cval = 1; return TRUE; }
<NC>false			 { cval = 0; return FALSE; }

<NC>red				return RED;
<NC>orange		 	return ORANGE;
<NC>black		 	return BLACK;

<NC>jan|feb|mar			{ cval = (yytext[0] == 'j') ? 1 :
					 (yytext[0] == 'f') ? 2 : 3;
				  return MONTH; }
<NC>apr|maj|may|jun		{ cval = (yytext[0] == 'a') ? 4 :
					 (yytext[0] == 'm') ? 5 : 6;
				  return MONTH; }
<NC>jul|aug|sep			{ cval = (yytext[0] == 'j') ? 7 :
					 (yytext[0] == 'a') ? 8 : 9;
				  return MONTH; }
<NC>okt|oct|nov|dec		{ cval = (yytext[0] == 'o') ? 10 :
					 (yytext[0] == 'n') ? 11 : 12;
				return MONTH; }
<NC>"=="			return EQUAL;
<NC>","				return COMMA;
<NC>";"				return SEMICOLON;
<NC>":"				return COLON;
<NC>"("				return LEFTPAR;
<NC>")"				return RIGHTPAR;
<NC>"["				return LEFTBRACKET;
<NC>"]"				return RIGHTBRACKET;
<NC>"{"				return LEFTBRACE;
<NC>"}"				return RIGHTBRACE;
<NC>"="				return ASSIGN;
<NC>"*="			return ASSIGN_MUL;
<NC>"/="			return ASSIGN_DIV;
<NC>"%="			return ASSIGN_MOD;
<NC>"+="			return ASSIGN_ADD;
<NC>"-="			return ASSIGN_SUB;
<NC>">>="			return ASSIGN_SR;
<NC>"<<="			return ASSIGN_SL;
<NC>"&="			return ASSIGN_AND;
<NC>"^="			return ASSIGN_XOR;
<NC>"|="			return ASSIGN_OR;

<NC>"<>"			return NOTEQUAL;
<NC>"!="			return NOTEQUAL;
<NC>"?"				return QUESTION_MARK;
<NC>"<"				return LT;
<NC>">"				return GT;
<NC>"<="			return LTE;
<NC>">="			return GTE;
<NC>">>"			return SR;
<NC>"<<"			return SL;
<NC>"++"			return PLUSPLUS;
<NC>"+"				return PLUS;
<NC>"--"			return MINUSMINUS;
<NC>"-"				return MINUS;
<NC>"*"				return MUL;
<NC>"/"				return DIV;
<NC>"%"				return MOD;

<NC>"&"                         return AMPERSAND;
<NC>"..."                       return DOTDOTDOT;

<NC>"//"[^\012]*	;
<NC>{FF}		;

<NC>"'".*"'"            {
                          if (yytext[1] == '\\')
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

<NC>"0x"{H}+  |
<NC>"0X"{H}+            {
                          sscanf(&yytext[2], "%x", &cval);
                          return INTEGER_CONSTANT;
                        }
<NC>"0b"{B}+  |
<NC>"0B"{B}+            {
                          int llop = 1;
                          cval = 0;
                          while (yytext[++llop])
                            cval = (cval << 1) | (yytext[llop] == '1');
                          return INTEGER_CONSTANT;
                        }

<NC>{D}+		{ 
#ifdef OSK
			   cval = atoi(yytext); 
#else
			   long atol();
			   cval = atol(yytext);
#endif
			   return INTEGER_CONSTANT; 
			}

<NC>{D}+"f"  |
<NC>{D}+"F"		{ 
#ifdef OSK
				cval = atoi(yytext); 
#else
				long atol();
				cval = atol(yytext);
#endif
			  cfval = (double) cval; return FLOATING_CONSTANT; 
			}

<NC>{D}+"."{D}*({E})?	|
<NC>{D}*"."{D}+({E})?	|
<NC>{D}+{E}		{
				cfval = (double) atof(yytext);
				return FLOATING_CONSTANT;
			}

<NC>[A-Za-z_][A-Za-z0-9_]*	      /* identifier */

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

<NC>[ \t\n]*                      ;
<NC>.		 ; return (LEXICALERROR);

