%{
 
#include "NAMidi_yacc.h"
#include "NAMidi_lex.h"

#include "NAMidiParser.h"
#include "NAMidiAST.h"

#include "NACString.h"

extern int NAMidi_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, void **node, const char *message);

extern Node *NAMidiParserParseIncludeFile(void *self, FileLocation *location, const char *includeFile, ASTInclude *includeNode);
extern void NAMidiParserSyntaxError(void *self, FileLocation *location, const char *token);
extern void NAMidiParserUnExpectedEOF(void *self, FileLocation *location);

#define node(type, yylloc) NAMidiAST##type##Create(&((FileLocation){(char *)filepath, yylloc.first_line, yylloc.first_column}))
#define list() NAArrayCreate(4, NULL)
#define listAppend(list, node) NAArrayAppend(list, node)

%}

%name-prefix = "NAMidi_"
%output = "NAMidi_yacc.c"
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
    void *node;
    void *list;
}

%token <i>INTEGER
%token <f>FLOAT
%token <s>STRING

%token RESOLUTION TITLE COPYRIGHT TEMPO TIME KEY MARKER DEFINE EXPAND CONTEXT WITH
       END CHANNEL VELOCITY GATETIME STEP VOICE SYNTH VOLUME PAN CHORUS
       REVERB TRANSPOSE DEFAULT INCLUDE
%token END_OF_FILE 0

%token <s>NOTE KEY_SIGN IDENTIFIER

%type <node> resolution title copyright tempo time key marker channel velocity gatetime voice
             synth volume pan chorus reverb transpose step note include expand

%type <node> define context

%type <node> statement note_param identifier
%type <list> statement_list identifier_list note_param_list

%destructor { free($$); } STRING
%destructor { free($$); } NOTE KEY_SIGN IDENTIFIER

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
    : resolution
    | title
    | copyright
    | tempo
    | time
    | key
    | marker
    | channel
    | velocity
    | gatetime
    | voice
    | synth
    | volume
    | pan
    | chorus
    | reverb
    | transpose
    | step
    | note
    | include
    | expand
    | define
    | context
    | error
        {
            yyclearin;
            $$ = NULL;
        }
    | ';'
        {
            $$ = NULL;
        }
    | '\n'
        {
            $$ = NULL;
        }
    ;

resolution
    : RESOLUTION INTEGER
        {
            ASTResolution *n = node(Resolution, @$);
            n->resolution = $2;
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

copyright
    : COPYRIGHT STRING
        {
            ASTCopyright *n = node(Copyright, @$);
            n->text = $2;
            $$ = n;
        }
    ;

tempo
    : TEMPO FLOAT
        {
            ASTTempo *n = node(Tempo, @$);
            n->tempo = $2;
            $$ = n;
        }
    ;
      
time
    : TIME INTEGER '/' INTEGER
        {
            ASTTime *n = node(Time, @$);
            n->numerator = $2;
            n->denominator = $4;
            $$ = n;
        }
    ;
      
key
    : KEY KEY_SIGN
        {
            ASTKey *n = node(Key, @$);
            n->keyString = $2;
            $$ = n;
        }
    ;

marker
    : MARKER STRING
        {
            ASTMarker *n = node(Marker, @$);
            n->text = $2;
            $$ = n;
        }
    ;

channel
    : CHANNEL INTEGER
        {
            ASTChannel *n = node(Channel, @$);
            n->number = $2;
            $$ = n;
        }
    ;

velocity
    : VELOCITY INTEGER
        {
            ASTVelocity *n = node(Velocity, @$);
            n->value = $2;
            $$ = n;
        }
    ;

gatetime
    : GATETIME INTEGER
        {
            ASTGatetime *n = node(Gatetime, @$);
            n->absolute = true;
            n->value = $2;
            $$ = n;
        }
    | GATETIME STEP
        {
            ASTGatetime *n = node(Gatetime, @$);
            $$ = n;
        }
    | GATETIME STEP '-' INTEGER
        {
            ASTGatetime *n = node(Gatetime, @$);
            n->value = -$4;
            $$ = n;
        }
    | GATETIME STEP '+' INTEGER
        {
            ASTGatetime *n = node(Gatetime, @$);
            n->value = $4;
            $$ = n;
        }
    ;

voice
    : VOICE INTEGER INTEGER INTEGER
        {
            ASTVoice *n = node(Voice, @$);
            n->msb = $2;
            n->lsb = $3;
            n->programNo = $4;
            $$ = n;
        }
    ;

synth
    : SYNTH STRING
        {
            ASTSynth *n = node(Synth, @$);
            n->name = $2;
            $$ = n;
        }
    ;

volume
    : VOLUME INTEGER
        {
            ASTVolume *n = node(Volume, @$);
            n->value = $2;
            $$ = n;
        }
    ;

pan
    : PAN INTEGER
        {
            ASTPan *n = node(Pan, @$);
            n->value = $2;
            $$ = n;
        }
    | PAN '+' INTEGER
        {
            ASTPan *n = node(Pan, @$);
            n->value = $3;
            $$ = n;
        }
    | PAN '-' INTEGER
        {
            ASTPan *n = node(Pan, @$);
            n->value = -$3;
            $$ = n;
        }
    ;

chorus
    : CHORUS INTEGER
        {
            ASTChorus *n = node(Chorus, @$);
            n->value = $2;
            $$ = n;
        }
    ;

reverb
    : REVERB INTEGER
        {
            ASTReverb *n = node(Reverb, @$);
            n->value = $2;
            $$ = n;
        }
    ;

transpose
    : TRANSPOSE INTEGER
        {
            ASTTranspose *n = node(Transpose, @$);
            n->value = $2;
            $$ = n;
        }
    | TRANSPOSE '+' INTEGER
        {
            ASTTranspose *n = node(Transpose, @$);
            n->value = $3;
            $$ = n;
        }
    | TRANSPOSE '-' INTEGER
        {
            ASTTranspose *n = node(Transpose, @$);
            n->value = -$3;
            $$ = n;
        }
    ;

step
    : INTEGER ':'
        {
            ASTStep *n = node(Step, @$);
            n->step = $1;
            $$ = n;
        }
    ;

note
    : NOTE
        {
            ASTNote *n = node(Note, @$);
            n->noteString = $1;
            n->node.children = list();
            $$ = n;
        }
    | NOTE note_param_list
        {
            ASTNote *n = node(Note, @$);
            n->noteString = $1;
            n->node.children = $2;
            $$ = n;
        }
    ;

include
    : INCLUDE STRING
        {
            ASTInclude *n = node(Include, @$);
            n->filepath = $2;
            FileLocation location = {(char *)filepath, @$.first_line, @$.first_column};
            n->root = NAMidiParserParseIncludeFile(NAMidi_get_extra(scanner), &location, $2, n);
            $$ = n;
        }
    ;

expand
    : EXPAND IDENTIFIER
        {
            ASTPattern *n = node(Pattern, @$);
            n->identifier = NACStringToUpperCase($2);
            n->node.children = list();
            $$ = n;
        }
    | EXPAND IDENTIFIER WITH identifier_list
        {
            ASTPattern *n = node(Pattern, @$);
            n->identifier = NACStringToUpperCase($2);
            n->node.children = $4;
            $$ = n;
        }
    ;

define
    : DEFINE IDENTIFIER statement_list end
        {
            ASTDefine *n = node(Define, @$);
            n->identifier = NACStringToUpperCase($2);
            n->node.children = $3;
            $$ = n;
        }
    ;

context
    : CONTEXT identifier_list statement_list end
        {
            ASTContext *n = node(Context, @$);
            n->ctxIdList = $2;
            n->node.children = $3;
            $$ = n;
        }
    ;

end
    : END
    | END_OF_FILE
        {
            FileLocation location = {(char *)filepath, @$.first_line, @$.first_column};
            NAMidiParserUnExpectedEOF(NAMidi_get_extra(scanner), &location);
        }
    ;

identifier_list
    : identifier
        {
            $$ = list();
            listAppend($$, $1);
        }
    | identifier_list ',' identifier
        {
            $$ = $1;
            listAppend($$, $3);
        }
    ;

identifier
    : IDENTIFIER
        {
            ASTIdentifier *n = node(Identifier, @$);
            n->idString = NACStringToUpperCase($1);
            $$ = n;
        }
    | DEFAULT
        {
            ASTIdentifier *n = node(Identifier, @$);
            n->idString = strdup("DEFAULT");
            $$ = n;
        }
    | INTEGER
        {
            ASTIdentifier *n = node(Identifier, @$);
            n->idString = strdup(NACStringFromInteger($1));
            $$ = n;
        }
    ;

note_param_list
    : note_param
        {
            $$ = list();
            if ($1) {
                listAppend($$, $1);
            }
        }
    | note_param_list note_param
        {
            $$ = $1;
            if ($2) {
                listAppend($$, $2);
            }
        }
    ;

note_param
    : '-'
        {
            ASTNoteParam *n = node(NoteParam, @$);
            n->value = -1;
            $$ = n;
        }
    | INTEGER
        {
            ASTNoteParam *n = node(NoteParam, @$);
            n->value = $1;
            $$ = n;
        }
    ;

%%

int NAMidi_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, void **node, const char *message)
{
    FileLocation location = {(char *)filepath, yylloc->first_line, yylloc->first_column};
    NAMidiParserSyntaxError(NAMidi_get_extra(scanner), &location, NAMidi_get_text(scanner));
    return 0;
}
