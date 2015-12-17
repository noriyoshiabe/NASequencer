%{
 
#include "ABC_yacc.h"
#include "ABC_lex.h"

#include "ABCParser.h"
#include "ABCAST.h"

extern void ABC_lex_set_error_until_eol_if_needed(yyscan_t scanner);
extern int ABC_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, void **node, const char *message);

extern Node *ABCParserParseIncludeFile(void *self, FileLocation *location, const char *includeFile);
extern void ABCParserSyntaxError(void *self, FileLocation *location, const char *token);

#define node(type, yylloc) ABCAST##type##Create(&((FileLocation){(char *)filepath, yylloc.first_line, yylloc.first_column}))
#define list() NAArrayCreate(4, NULL)
#define listAppend(list, node) NAArrayAppend(list, node)

%}

%name-prefix = "ABC_"
%output = "ABC_yacc.c"
%defines

%pure-parser
%lex-param   { yyscan_t scanner }
%parse-param { yyscan_t scanner }
%parse-param { const char *filepath }
%parse-param { void **node }
%locations

%union {
    int i;
    float f;
    char *s;
    char c;
    void *node;
    void *list;
}

%token REFERENCE_NUMBER TITLE KEY INSTRUCTION METER UNIT_NOTE_LENGTH TEMPO PARTS
%token INCLUDE CHARSET VERSION_INSTRUCTION CREATOR LINEBREAK_INSTRUCTION DECORATION

%token <c> STRING_INFORMATION

%token <i> INTEGER
%token <s> STRING
%token <s> VERSION NOTE FILEPATH VERSION_NUMBER
%token <s> KEY_TONIC KEY_MODE KEY_ACCIDENTAL CLEF_NAME PITCH NONE PART_LABEL

%token CLEF MIDDLE TRANSPOSE OCTAVE STAFF_LINES
%token COMMON_TIME

%type <s> key_tonic part_expr
%type <i> signed_integer numerator

%type <node> version reference_number title key meter unit_note_length tempo parts note
%type <node> instruction line_break
%type <node> string_information

%type <node> key_param      tempo_param
%type <list> key_param_list tempo_param_list

%type <node> statement
%type <list> statement_list

%%

input
    : statement_list
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
    : version
    | string_information
    | reference_number
    | title
    | key
    | meter
    | unit_note_length
    | tempo
    | parts
    | instruction
    | note
    | line_break
    | error
        {
            ABC_lex_set_error_until_eol_if_needed(scanner);
            yyerrok;
            yyclearin;
            $$ = NULL;
        }
    | /* empty */
        {
            $$ = NULL;
        }
    ;

version
    : VERSION '\n'
        {
            ASTVersion *n = node(Version, @$);
            n->versionString = $1;
            $$ = n;
        }
    ;

string_information
    : STRING_INFORMATION STRING
        {
            ASTStringInformation *n = node(StringInformation, @$);
            n->field = $1;
            n->string = $2;
            $$ = n;
        }
    ;

reference_number
    : REFERENCE_NUMBER INTEGER
        {
            ASTReferenceNumber *n = node(ReferenceNumber, @$);
            n->number = $2;
            $$ = n;
        }
    ;

title
    : TITLE STRING
        {
            ASTTitle *n = node(Title, @$);
            n->title = $2;
            $$ = n;
        }
    ;

key
    : KEY key_param_list
        {
            ASTKey *n = node(Key, @$);
            n->node.children = $2;
            $$ = n;
        }
    ;

key_param_list
    : key_param
        {
            $$ = list();
            if ($1) {
                listAppend($$, $1);
            }
        }
    | key_param_list key_param
        {
            $$ = $1;
            if ($2) {
                listAppend($$, $2);
            }
        }
    ;

key_param
    : key_tonic
        {
            ASTKeyParam *n = node(KeyParam, @$);
            n->type = KeyTonic;
            n->string = $1;
            $$ = n;
        }
    | KEY_MODE
        {
            ASTKeyParam *n = node(KeyParam, @$);
            n->type = KeyMode;
            n->string = $1;
            $$ = n;
        }
    | KEY_ACCIDENTAL
        {
            ASTKeyParam *n = node(KeyParam, @$);
            n->type = KeyAccidental;
            n->string = $1;
            $$ = n;
        }
    | CLEF '=' CLEF_NAME
        {
            ASTKeyParam *n = node(KeyParam, @$);
            n->type = Clef;
            n->string = $3;
            $$ = n;
        }
    | MIDDLE '=' PITCH
        {
            ASTKeyParam *n = node(KeyParam, @$);
            n->type = Middle;
            n->string = $3;
            $$ = n;
        }
    | TRANSPOSE '=' signed_integer
        {
            ASTKeyParam *n = node(KeyParam, @$);
            n->type = Transpose;
            n->intValue = $3;
            $$ = n;
        }
    | OCTAVE '=' signed_integer
        {
            ASTKeyParam *n = node(KeyParam, @$);
            n->type = Octave;
            n->intValue = $3;
            $$ = n;
        }
    | STAFF_LINES '=' INTEGER
        {
            ASTKeyParam *n = node(KeyParam, @$);
            n->type = StaffLines;
            n->intValue = $3;
            $$ = n;
        }
    | /* empty */
        {
            $$ = NULL;
        }
    ;

key_tonic
    : PITCH
        {
            $$ = $1;
        }
    | NONE
        {
            $$ = $1;
        }
    | KEY_TONIC
        {
            $$ = $1;
        }
    ;

signed_integer
    : INTEGER
        {
            $$ = $1;
        }
    | '+' INTEGER
        {
            $$ = $2;
        }
    | '-' INTEGER
        {
            $$ = -$2;
        }
    ;

meter
    : METER numerator '/' INTEGER
        {
            ASTMeter *n = node(Meter, @$);
            n->numerator = $2;
            n->denominator = $4;
            $$ = n;
        }
    | METER '(' numerator ')' '/' INTEGER
        {
            ASTMeter *n = node(Meter, @$);
            n->numerator = $3;
            n->denominator = $6;
            $$ = n;
        }
    | METER NONE
        {
            ASTMeter *n = node(Meter, @$);
            n->free = true;
            $$ = n;

            free($2);
        }
    | METER COMMON_TIME
        {
            ASTMeter *n = node(Meter, @$);
            n->commonTime = true;
            $$ = n;
        }
    | METER COMMON_TIME '|'
        {
            ASTMeter *n = node(Meter, @$);
            n->cutTime = true;
            $$ = n;
        }
    ;

numerator
    : INTEGER
        {
           $$ = $1
        }
    | numerator '+' INTEGER
        {
           $$ = $1 + $3
        }
    | numerator '-' INTEGER
        {
           $$ = $1 - $3
        }
    ;

unit_note_length
    : UNIT_NOTE_LENGTH INTEGER '/' INTEGER
        {
            ASTUnitNoteLength *n = node(UnitNoteLength, @$);
            n->numerator = $2;
            n->denominator = $4;
            $$ = n;
        }
    | UNIT_NOTE_LENGTH INTEGER
        {
            ASTUnitNoteLength *n = node(UnitNoteLength, @$);
            n->numerator = $2;
            n->denominator = -1;
            $$ = n;
        }
    ;

tempo
    : TEMPO tempo_param_list
        {
            ASTTempo *n = node(Tempo, @$);
            n->node.children = $2;
            $$ = n;
        }
    ;

tempo_param_list
    : tempo_param
        {
            $$ = list();
            if ($1) {
                listAppend($$, $1);
            }
        }
    | tempo_param_list tempo_param
        {
            $$ = $1;
            if ($2) {
                listAppend($$, $2);
            }
        }
    ;

tempo_param
    : STRING
        {
            ASTTempoParam *n = node(TempoParam, @$);
            n->type = TextString;
            n->string = $1;
            $$ = n;
        }
    | INTEGER '/' INTEGER
        {
            ASTTempoParam *n = node(TempoParam, @$);
            n->type = BeatUnit;
            n->numerator = $1;
            n->denominator = $3;
            $$ = n;
        }
    | '=' INTEGER
        {
            ASTTempoParam *n = node(TempoParam, @$);
            n->type = BeatCount;
            n->beatCount = $2;
            $$ = n;
        }
    ;

parts
    : PARTS part_expr
        {
            ASTParts *n = node(Parts, @$);
            n->list = $2;
            $$ = n;
        }
    ;

part_expr
    : PART_LABEL
        {
            $$ = $1;
        }
    | '(' part_expr ')'
        {
            $$ = $2;
        }
    | part_expr INTEGER
        {
            $$ = malloc(strlen($1) * $2 + 1);
            $$[0] = '\0';
            for (int i = 0; i < $2; ++i) {
                strcat($$, $1);
            }
            free($1);
        }
    | part_expr part_expr
        {
            $$ = realloc($1, strlen($1) + strlen($2) + 1);
            strcat($$, $2);
            free($2);
        }
    ;

instruction
    : INSTRUCTION INCLUDE FILEPATH
        {
            ASTInclude *n = node(Include, @$);
            n->filepath = $3;
            FileLocation location = {(char *)filepath, @$.first_line, @$.first_column};
            n->root = ABCParserParseIncludeFile(ABC_get_extra(scanner), &location, $3);
            $$ = n;
        }
    | INSTRUCTION CHARSET STRING
        {
            ASTInstruction *n = node(Instruction, @$);
            n->string = $3;
            $$ = n;
        }
    | INSTRUCTION VERSION_INSTRUCTION VERSION_NUMBER
        {
            ASTVersion *n = node(Version, @$);
            n->numberString = $3;
            $$ = n;
        }
    | INSTRUCTION CREATOR STRING
        {
            ASTInstruction *n = node(Instruction, @$);
            n->string = $3;
            $$ = n;
        }
    | INSTRUCTION LINEBREAK_INSTRUCTION STRING
        {
            // TODO
            ASTInstruction *n = node(Instruction, @$);
            n->string = $3;
            $$ = n;
        }
    | INSTRUCTION DECORATION STRING
        {
            // TODO
            ASTInstruction *n = node(Instruction, @$);
            n->string = $3;
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

line_break
    : '\n'
        {
            $$ = node(LineBreak, @$);
        }
    ;

%%

int ABC_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, void **node, const char *message)
{
    FileLocation location = {(char *)filepath, yylloc->first_line, yylloc->first_column};
    ABCParserSyntaxError(ABC_get_extra(scanner), &location, ABC_get_text(scanner));
    return 0;
}
