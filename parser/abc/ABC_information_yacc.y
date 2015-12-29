%{
 
#include "ABC_information_yacc.h"
#include "ABC_information_lex.h"

#include "ABCParser.h"
#include "ABCAST.h"

extern int ABC_information_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, int line, int columnOffset, void **node, const char *message);
extern void ABC_information_lex_set_error(yyscan_t scanner);

#define node(type, yylloc) ABCAST##type##Create(&((FileLocation){(char *)filepath, line, yylloc.first_column + columnOffset}))
#define list() NAArrayCreate(4, NULL)
#define listAppend(list, node) NAArrayAppend(list, node)

%}

%name-prefix = "ABC_information_"
%output = "ABC_information_yacc.c"
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
    int i;
    float f;
    char *s;
    char c;
    void *node;
    void *list;
}

%token REFERENCE_NUMBER TITLE KEY INSTRUCTION METER UNIT_NOTE_LENGTH TEMPO PARTS VOICE
%token INST_INCLUDE INST_CHARSET INST_VERSION INST_CREATOR INST_LINEBREAK INST_DECORATION
%token SYMBOL_LINE CONTINUATION

%token <c> STRING_INFORMATION

%token <i> INTEGER
%token <s> STRING
%token <c> CHAR
%token <s> FILEPATH VERSION_NUMBER
%token <s> KEY_TONIC KEY_MODE KEY_ACCIDENTAL CLEF_NAME PITCH NONE PART_LABEL
%token <s> MACRO_TARGET REDEFINABLE_SYMBOL REDEFINABLE_REPLACEMENT
%token <s> VOICE_ID VOICE_NAME

%token CLEF MIDDLE TRANSPOSE OCTAVE STAFF_LINES
%token COMMON_TIME NAME SUBNAME STEM UP DOWN

%type <s> key_tonic part_expr
%type <i> signed_integer numerator

%type <node> reference_number title key meter unit_note_length tempo parts voice
%type <node> instruction macro redefinable_symbol
%type <node> string_information symbol_line continuation

%type <node> key_param      tempo_param      voice_param
%type <list> key_param_list tempo_param_list voice_param_list

%type <node> statement

%%

input
    : statement
        {
            *node = $1;
        }
    ;

statement
    : string_information
    | reference_number
    | title
    | key
    | meter
    | unit_note_length
    | tempo
    | parts
    | instruction
    | macro
    | symbol_line
    | redefinable_symbol
    | continuation
    | voice
    | error
        {
            ABC_information_lex_set_error(scanner);
            yyerrok;
            yyclearin;
            $$ = NULL;
        }
    | /* empty */
        {
            $$ = NULL;
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
            n->type = KeyClef;
            n->string = $3;
            $$ = n;
        }
    | MIDDLE '=' PITCH
        {
            ASTKeyParam *n = node(KeyParam, @$);
            n->type = KeyMiddle;
            n->string = $3;
            $$ = n;
        }
    | TRANSPOSE '=' signed_integer
        {
            ASTKeyParam *n = node(KeyParam, @$);
            n->type = KeyTranspose;
            n->intValue = $3;
            $$ = n;
        }
    | OCTAVE '=' signed_integer
        {
            ASTKeyParam *n = node(KeyParam, @$);
            n->type = KeyOctave;
            n->intValue = $3;
            $$ = n;
        }
    | STAFF_LINES '=' INTEGER
        {
            ASTKeyParam *n = node(KeyParam, @$);
            n->type = KeyStaffLines;
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
    : INSTRUCTION INST_CHARSET STRING
        {
            ASTInstCharSet *n = node(InstCharSet, @$);
            n->name = $3;
            $$ = n;
        }
    | INSTRUCTION INST_VERSION VERSION_NUMBER
        {
            ASTInstVersion *n = node(InstVersion, @$);
            n->numberString = $3;
            $$ = n;
        }
    | INSTRUCTION INST_INCLUDE FILEPATH
        {
            ASTInstInclude *n = node(InstInclude, @$);
            n->filepath = $3;
            FileLocation location = {(char *)filepath, @$.first_line, @$.first_column};
            n->root = ABCParserParseIncludeFile(ABC_information_get_extra(scanner), &location, $3, &n->fullpath);
            $$ = n;
        }
    | INSTRUCTION INST_CREATOR STRING
        {
            ASTInstCreator *n = node(InstCreator, @$);
            n->name = $3;
            $$ = n;
        }
    | INSTRUCTION INST_LINEBREAK CHAR
        {
            ASTInstLineBreak *n = node(InstLineBreak, @$);
            n->character = $3;
            $$ = n;
        }
    | INSTRUCTION INST_LINEBREAK STRING
        {
            ASTInstLineBreak *n = node(InstLineBreak, @$);

            if (0 == strcmp("<EOL>", $3)) {
                n->character = '\n';
            }
            else if (0 == strcmp("<none>", $3)) {
                n->character = -1;
            }

            free($3);
            $$ = n;
        }
    | INSTRUCTION INST_DECORATION CHAR
        {
            ASTInstDecoration *n = node(InstDecoration, @$);
            n->character = $3;
            $$ = n;
        }
    ;

macro
    : MACRO_TARGET '=' STRING
        {
            ASTMacro *n = node(Macro, @$);
            n->target = $1;
            n->replacement = $3;
            $$ = n;
        }
    ;

symbol_line
    : SYMBOL_LINE STRING
        {
            ASTSymbolLine *n = node(SymbolLine, @$);
            n->string = $2;
            $$ = n;
        }
    ;

redefinable_symbol
    : REDEFINABLE_SYMBOL '=' REDEFINABLE_REPLACEMENT
        {
            ASTRedefinableSymbol *n = node(RedefinableSymbol, @$);
            n->symbol = $1;
            n->replacement = $3;
            $$ = n;
        }
    ;

continuation
    : CONTINUATION STRING
        {
            ASTContinuation *n = node(Continuation, @$);
            n->string = $2;
            $$ = n;
        }
    ;

voice
    : VOICE VOICE_ID voice_param_list
        {
            ASTVoice *n = node(Voice, @$);
            n->identifier = $2;
            n->node.children = $3;
            $$ = n;
        }
    ;

voice_param_list
    : voice_param
        {
            $$ = list();
            if ($1) {
                listAppend($$, $1);
            }
        }
    | voice_param_list voice_param
        {
            $$ = $1;
            if ($2) {
                listAppend($$, $2);
            }
        }
    ;

voice_param
    : NAME '=' VOICE_NAME
        {
            ASTVoiceParam *n = node(VoiceParam, @$);
            n->type = VoiceName;
            n->string = $3;
            $$ = n;
        }
    | SUBNAME '=' VOICE_NAME
        {
            ASTVoiceParam *n = node(VoiceParam, @$);
            n->type = VoiceSubname;
            n->string = $3;
            $$ = n;
        }
    | STEM '=' UP
        {
            ASTVoiceParam *n = node(VoiceParam, @$);
            n->type = VoiceStemUp;
            $$ = n;
        }
    | STEM '=' DOWN
        {
            ASTVoiceParam *n = node(VoiceParam, @$);
            n->type = VoiceStemDown;
            $$ = n;
        }
    | CLEF '=' CLEF_NAME
        {
            ASTVoiceParam *n = node(VoiceParam, @$);
            n->type = VoiceClef;
            n->string = $3;
            $$ = n;
        }
    | MIDDLE '=' PITCH
        {
            ASTVoiceParam *n = node(VoiceParam, @$);
            n->type = VoiceMiddle;
            n->string = $3;
            $$ = n;
        }
    | TRANSPOSE '=' signed_integer
        {
            ASTVoiceParam *n = node(VoiceParam, @$);
            n->type = VoiceTranspose;
            n->intValue = $3;
            $$ = n;
        }
    | OCTAVE '=' signed_integer
        {
            ASTVoiceParam *n = node(VoiceParam, @$);
            n->type = VoiceOctave;
            n->intValue = $3;
            $$ = n;
        }
    | STAFF_LINES '=' INTEGER
        {
            ASTVoiceParam *n = node(VoiceParam, @$);
            n->type = VoiceStaffLines;
            n->intValue = $3;
            $$ = n;
        }
    | /* empty */
        {
            $$ = NULL;
        }
    ;

%%

int ABC_information_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, int line, int columnOffset, void **node, const char *message)
{
    FileLocation location = {(char *)filepath, line, yylloc->first_column + columnOffset};
    ABCParserSyntaxError(ABC_information_get_extra(scanner), &location, ABC_information_get_text(scanner));
    return 0;
}
