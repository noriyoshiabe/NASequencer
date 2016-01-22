%{
 
#include "ABC_tune_body_yacc.h"
#include "ABC_tune_body_lex.h"

#include "ABCParser.h"
#include "ABCAST.h"

#include "NALog.h"

#include <string.h>

extern int ABC_tune_body_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, int line, int columnOffset, void **node, const char *message);

#define location(yylloc) &((FileLocation){(char *)filepath, line, yylloc.first_column + columnOffset})
#define node(type, yylloc) ABCAST##type##Create(location(yylloc))
#define list() NAArrayCreate(4, NULL)
#define listAppend(list, node) NAArrayAppend(list, node)

#define isInline() (0 < columnOffset)

%}

%name-prefix = "ABC_tune_body_"
%output = "ABC_tune_body_yacc.c"
%defines

%pure-parser
%lex-param   { yyscan_t scanner }
%parse-param { yyscan_t scanner }
%parse-param { const char *filepath }
%parse-param { int line }
%parse-param { int columnOffset }
%parse-param { void **node }
%locations

%union {
    char c;
    char *s;
    void *node;
    void *list;
}

%token <s>INLINE_FIELD ANNOTATION DECORATION NOTE REST REPEAT_BAR TUPLET CHORD_END BROKEN_RHYTHM
%token ACCIACCATURA
%token EXTRA_SPACE RESERVED

%type <node> inline_field line_break annotation decoration note broken_rhythm rest repeat_bar
%type <node> tie slur dot grace_note tuplet chord extra_space overlay reserved

%type <node> statement      grace_note_statement      chord_statement
%type <list> statement_list grace_note_statement_list chord_statement_list

%destructor { free($$); } INLINE_FIELD ANNOTATION DECORATION NOTE REST REPEAT_BAR TUPLET CHORD_END BROKEN_RHYTHM

%%

input
    : /* empty */
        {
            ASTTuneBody *n = node(TuneBody, ((YYLTYPE){0, 1}));
            n->node.children = list();
            *node = n;
        }
    | statement_list
        {
            ASTTuneBody *n = node(TuneBody, ((YYLTYPE){0, 1}));
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
    : inline_field
    | line_break
    | annotation
    | decoration
    | note
    | broken_rhythm
    | rest
    | repeat_bar
    | tie
    | slur
    | dot
    | grace_note
    | tuplet
    | chord
    | extra_space
    | overlay
    | reserved
    | error
        {
            yyerrok;
            yyclearin;
            $$ = NULL;
        }
    ;

inline_field
    : INLINE_FIELD
        {
            __LogTrace__("---- INLINE_FIELD [%s] %d - %d\n", $1, line, @$.first_column);
            $$ = ABCParserParseInformation(ABC_tune_body_get_extra(scanner), filepath, line, @$.first_column, $1);
            free($1);
        }
    ;

line_break
    : '\n'
        {
            $$ = isInline() ? NULL : node(LineBreak, @$);
        }
    ;

annotation
    : ANNOTATION
        {
            ASTAnnotation *n = node(Annotation, @$);
            n->text = $1;
            $$ = n;
        }
    ;

decoration
    : DECORATION
        {
            ASTDecoration *n = node(Decoration, @$);
            n->symbol = $1;
            $$ = n;
        }
    ;

note
    : NOTE
        {
            ASTNote *n = node(Note, @$);
            n->noteString = $1;
            $$ = n;
        }
    ;

broken_rhythm
    : BROKEN_RHYTHM
        {
            ASTBrokenRhythm *n = node(BrokenRhythm, @$);
            n->direction = $1[0];
            n->count = strlen($1);
            $$ = n;
        }
    ;

rest
    : REST
        {
            ASTRest *n = node(Rest, @$);
            n->restString = $1;
            $$ = n;
        }
    ;

repeat_bar
    : REPEAT_BAR
        {
            ASTRepeatBar *n = node(RepeatBar, @$);
            n->symbols = $1;
            $$ = n;
        }
    ;

tie
    : '-'
        {
            ASTTie *n = node(Tie, @$);
            $$ = n;
        }
    ;

slur
    : '('
        {
            ASTSlur *n = node(Slur, @$);
            n->direction = '(';
            $$ = n;
        }
    | ')'
        {
            ASTSlur *n = node(Slur, @$);
            n->direction = ')';
            $$ = n;
        }
    ;

dot
    : '.'
        {
            ASTDot *n = node(Dot, @$);
            $$ = n;
        }

grace_note
    : '{' grace_note_statement_list grace_note_end
        {
            ASTGraceNote *n = node(GraceNote, @$);
            n->node.children = $2;
            $$ = n;
        }
    | ACCIACCATURA grace_note_statement_list grace_note_end
        {
            ASTGraceNote *n = node(GraceNote, @$);
            n->acciaccatura = true;
            n->node.children = $2;
            $$ = n;
        }
    ;

grace_note_end
    : '}'
    | line_break
        {
            ABCParserUnExpectedEOL(ABC_tune_body_get_extra(scanner), location(@$));
        }
    ;

grace_note_statement_list
    : grace_note_statement
        {
            $$ = list();
            if ($1) {
                listAppend($$, $1);
            }
        }
    | grace_note_statement_list grace_note_statement
        {
            $$ = $1;
            if ($2) {
                listAppend($$, $2);
            }
        }
    ;

grace_note_statement
    : decoration
    | note
    | broken_rhythm
    | tie
    | slur
    | dot
    | error
        {
            yyerrok;
            yyclearin;
            $$ = NULL;
        }
    ;

tuplet
    : TUPLET
        {
            ASTTuplet *n = node(Tuplet, @$);
            n->tupletString = $1;
            $$ = n;
        }
    ;

chord
    : '[' chord_statement_list CHORD_END
        {
            ASTChord *n = node(Chord, @$);
            n->lengthString = $3;
            n->node.children = $2;
            $$ = n;
        }
    ;

chord_statement_list
    : chord_statement
        {
            $$ = list();
            if ($1) {
                listAppend($$, $1);
            }
        }
    | chord_statement_list chord_statement
        {
            $$ = $1;
            if ($2) {
                listAppend($$, $2);
            }
        }
    ;

chord_statement
    : note
    | error
        {
            yyerrok;
            yyclearin;
            $$ = NULL;
        }
    ;

extra_space
    : EXTRA_SPACE
        {
            $$ = NULL; // Ignore
        }
    ;

overlay
    : '&'
        {
            ASTOverlay *n = node(Overlay, @$);
            $$ = n;
        }
    ;

reserved
    : RESERVED
        {
            $$ = NULL; // Ignore
        }
    ;

%%

int ABC_tune_body_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, int line, int columnOffset, void **node, const char *message)
{
    ABCParserSyntaxError(ABC_tune_body_get_extra(scanner), location((*yylloc)), ABC_tune_body_get_text(scanner));
    return 0;
}
