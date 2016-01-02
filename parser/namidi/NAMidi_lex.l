%{
  
#include "NAMidi_yacc.h"

#include <string.h>

#define YY_USER_INIT \
    yylineno = 1; \
    yycolumn = 1;

#define YY_USER_ACTION \
    yylloc->first_line = yylineno; \
    yylloc->first_column = yycolumn; \
    if (strchr(yytext, '\n')) ++yylineno, yycolumn = 1; \
    else yycolumn += yyleng;

%}

%option prefix="NAMidi_"
%option outfile="NAMidi_lex.c"
%option header-file="NAMidi_lex.h"
%option reentrant
%option noyywrap
%option bison-bridge
%option bison-locations
%option case-insensitive
%option stack

%x COMMENT
%x ERROR

NONZERO        [1-9]
DIGIT          [[:digit:]]

INTEGER        {NONZERO}{DIGIT}*|0
FLOAT          ({NONZERO}{DIGIT}*|0)\.{DIGIT}+
STRING         \"[^\"]*\"|'[^']*'

RESOLUTION     resolution
TITLE          title
TEMPO          tempo
TIME           time
MARKER         marker
DEFINE         define
END            end
CHANNEL        channel
VOICE          voice
SYNTH          synth
VOLUME         volume
PAN            pan
CHORUS         chorus
REVERB         reverb
TRANSPOSE      transpose
KEY            key
DEFAULT        default
INCLUDE        include

KEY_SIGN       [cdefgab](#|b)?(maj|min)
NOTE           [cdefgab](#|##|b|bb|n)?(\-?[[:digit:]]{1,2})?
IDENTIFIER     [_[:alnum:]][_[:alnum:]\-]*[_[:alnum:]]

WSPACE         [[:blank:]]+
LINE_COMMENT   (\/\/|==|--).*$

%%

{INTEGER}        { yylval->i = atoi(yytext); return INTEGER; }
{FLOAT}          { yylval->f = atof(yytext); return FLOAT; }
{STRING}         { yylval->s = strdup(yytext + 1);
                   yylval->s[strlen(yylval->s) - 1] = '\0';
                   return STRING; }

{RESOLUTION}     { return RESOLUTION; }
{TITLE}          { return TITLE; }
{TEMPO}          { return TEMPO; }
{TIME}           { return TIME; }
{KEY}            { return KEY; }
{MARKER}         { return MARKER; }
{DEFINE}         { return DEFINE; }
{END}            { return END; }
{CHANNEL}        { return CHANNEL; }
{VOICE}          { return VOICE; }
{SYNTH}          { return SYNTH; }
{VOLUME}         { return VOLUME; }
{PAN}            { return PAN; }
{CHORUS}         { return CHORUS; }
{REVERB}         { return REVERB; }
{TRANSPOSE}      { return TRANSPOSE; }
{DEFAULT}        { return DEFAULT; }
{INCLUDE}        { return INCLUDE; }

{NOTE}           { yylval->s = strdup(yytext); return NOTE; }
{KEY_SIGN}       { yylval->s = strdup(yytext); return KEY_SIGN; }
{IDENTIFIER}     { yylval->s = strdup(yytext); return IDENTIFIER; }

"+"              { return '+'; }
"-"              { return '-'; }
"/"              { return '/'; }
"("              { return '('; }
")"              { return ')'; }
"{"              { return '{'; }
"}"              { return '}'; }
","              { return ','; }
";"              { return ';'; }
":"              { return ':'; }
\n               { return '\n'; }

{WSPACE}         { }
{LINE_COMMENT}   { }

"/*"             { BEGIN(COMMENT); }
<COMMENT>"*/"    { BEGIN(INITIAL); }
<COMMENT>\n      { }
<COMMENT>.       { }

<ERROR>\n        { yy_pop_state(yyscanner); return '\n'; }
<ERROR>.         { }

.                { return 1; }
<<EOF>>          { return 0; }
 
%%

void NAMidi_lex_set_error_until_eol(yyscan_t scanner)
{
    yy_push_state(ERROR, scanner);
}
