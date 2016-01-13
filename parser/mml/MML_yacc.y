%{
 
#include "MML_yacc.h"
#include "MML_lex.h"

#include "MMLParser.h"
#include "MMLAST.h"

#include "NACString.h"

extern int MML_error(YYLTYPE *yylloc, yyscan_t scanner, void **node, const char *message);

#define node(type, yylloc) MMLAST##type##Create(&((FileLocation){MMLParserGetCurrentFilepath(MML_get_extra(scanner)), yylloc.first_line, yylloc.first_column}))
#define list() NAArrayCreate(4, NULL)
#define listAppend(list, node) NAArrayAppend(list, node)

%}

%name-prefix = "MML_"
%output = "MML_yacc.c"
%defines

%pure-parser
%lex-param   { yyscan_t scanner }
%parse-param { yyscan_t scanner }
%parse-param { void **node }
%locations

%union {
    int i;
    float f;
    char *s;
    void *node;
    void *list;
}

%token <i>INTEGER
%token <f>FLOAT
%token <s>STRING

%token END_OF_FILE 0
%token D_TIMEBASE D_TITLE D_COPYRIGHT D_MARKER  D_VELOCITY D_OCTAVE  REVERSE   
%token CHANNEL SYNTHESIZER BANK_SELECT PROGRAM_CHANGE VOLUME CHORUS REVERB EXPRESSION PAN DETUNE TEMPO
%token OCTAVE TRANSPOSE LENGTH VELOCITY TUPLET_START GATETIME REPEAT_START REPEAT_END REPEAT_BREAK
%token <s> NOTE REST TUPLET_END

%type <node> d_timebase d_title d_copyright d_marker d_velocity d_octave channel synthesizer bank_select program_change
%type <node> volume chorus reverb expression pan detune tempo note rest octave
%type <node> transpose tie length gatetime velocity tuplet track_change repeat repeat_break chord

%type <i> signed_integer

%type <node> statement
%type <list> statement_list

%destructor { free($$); } STRING
%destructor { free($$); } NOTE
%destructor { free($$); } REST
%destructor { free($$); } TUPLET_END

%%

input
    : /* empty */
        {
            ASTRoot *n = node(Root, @$);
            n->node.children = list();
            *node = n;
        }
    | statement_list
        {
            ASTRoot *n = node(Root, @$);
            n->node.children = $1;
            *node = n;
        }
    ;

statement_list
    : statement
        {
            $$ = list();
            if ($1) {
                listAppend($$, $1);
            }
        }
    | statement_list statement
        {
            $$ = $1;
            if ($2) {
                listAppend($$, $2);
            }
        }
    ;

statement
    : d_timebase
    | d_title
    | d_copyright
    | d_marker
    | d_velocity
    | d_octave
    | channel
    | synthesizer
    | bank_select
    | program_change
    | volume
    | chorus
    | reverb
    | expression
    | pan
    | detune
    | tempo
    | note
    | rest
    | octave
    | transpose
    | tie
    | length
    | gatetime
    | velocity
    | tuplet
    | track_change
    | repeat
    | repeat_break
    | chord
    | error
        {
            yyclearin;
            $$ = NULL;
        }
    ;

d_timebase
    : D_TIMEBASE INTEGER
        {
            $$ = NULL;
        }
    ;

d_title
    : D_TITLE STRING
        {
            $$ = NULL;
        }
    ;

d_copyright
    : D_COPYRIGHT STRING
        {
            $$ = NULL;
        }
    ;

d_marker
    : D_MARKER STRING
        {
            $$ = NULL;
        }
    ;

d_velocity
    : D_VELOCITY REVERSE
        {
            $$ = NULL;
        }
    ;

d_octave
    : D_OCTAVE REVERSE
        {
            $$ = NULL;
        }
    ;

channel
    : CHANNEL INTEGER
        {
            $$ = NULL;
        }
    ;

synthesizer
    : SYNTHESIZER STRING
        {
            $$ = NULL;
        }
    ;

bank_select
    : BANK_SELECT INTEGER ',' INTEGER
        {
            $$ = NULL;
        }
    ;

program_change
    : PROGRAM_CHANGE INTEGER
        {
            $$ = NULL;
        }
    ;

volume
    : VOLUME INTEGER
        {
            $$ = NULL;
        }
    ;

chorus
    : CHORUS INTEGER
        {
            $$ = NULL;
        }
    ;

reverb
    : REVERB INTEGER
        {
            $$ = NULL;
        }
    ;

expression
    : EXPRESSION INTEGER
        {
            $$ = NULL;
        }
    ;

pan
    : PAN signed_integer
        {
            $$ = NULL;
        }
    ;

detune
    : DETUNE signed_integer
        {
            $$ = NULL;
        }
    ;

tempo
    : TEMPO FLOAT
        {
            $$ = NULL;
        }
    ;

note
    : NOTE
        {
            $$ = NULL;
        }
    ;

rest
    : REST
        {
            $$ = NULL;
        }
    ;

octave
    : OCTAVE signed_integer
        {
            $$ = NULL;
        }
    | '<'
        {
            $$ = NULL;
        }
    | '>'
        {
            $$ = NULL;
        }
    ;

transpose
    : TRANSPOSE signed_integer
        {
            $$ = NULL;
        }
    ;

tie
    : '&'
        {
            $$ = NULL;
        }
    ;

length
    : LENGTH INTEGER
        {
            $$ = NULL;
        }
    ;

gatetime
    : GATETIME INTEGER
        {
            $$ = NULL;
        }
    ;

velocity
    : VELOCITY INTEGER
        {
            $$ = NULL;
        }
    | '('
        {
            $$ = NULL;
        }
    | ')'
        {
            $$ = NULL;
        }
    ;

tuplet
    : TUPLET_START statement_list TUPLET_END
        {
            $$ = NULL;
        }
    ;

track_change
    : ';'
        {
            $$ = NULL;
        }
    ;

repeat
    : REPEAT_START statement_list REPEAT_END
        {
            $$ = NULL;
        }
    | REPEAT_START INTEGER statement_list REPEAT_END
        {
            $$ = NULL;
        }
    ;

repeat_break
    : REPEAT_BREAK
        {
            $$ = NULL;
        }
    ;

chord
    : '[' statement_list ']'
        {
            $$ = NULL;
        }
    ;

signed_integer
    : INTEGER { $$ = $1; }
    | '+' INTEGER { $$ = $2; }
    | '-' INTEGER { $$ = -$2; }
    ;

%%

int MML_error(YYLTYPE *yylloc, yyscan_t scanner, void **node, const char *message)
{
    FileLocation location = {MMLParserGetCurrentFilepath(MML_get_extra(scanner)), yylloc->first_line, yylloc->first_column};
    MMLParserSyntaxError(MML_get_extra(scanner), &location, MML_get_text(scanner));
    return 0;
}
