%{
 
#include "NAMidi_yacc.h"
#include "NAMidi_lex.h"

#include "NAMidiParser.h"
#include "NAMidiAST.h"

#include "NACString.h"

extern int NAMidi_error(YYLTYPE *yylloc, yyscan_t scanner, void **node, const char *message);

#define location(yylloc) &((FileLocation){NAMidiParserGetCurrentFilepath(NAMidi_get_extra(scanner)), yylloc.first_line, yylloc.first_column})
#define node(type, yylloc) NAMidiAST##type##Create(location(yylloc))
#define list() NAArrayCreate(4, NULL)
#define listAppend(list, node) NAArrayAppend(list, node)

%}

%name-prefix = "NAMidi_"
%output = "NAMidi_yacc.c"
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

%token RESOLUTION TITLE COPYRIGHT TEMPO TIME KEY PERCUSSION MARKER PATTERN EXPAND
       END CHANNEL VELOCITY GATETIME STEP BANK PROGRAM SYNTH VOLUME PAN CHORUS
       REVERB EXPRESSION PITCH DETUNE SENSE SUSTAIN TRANSPOSE NOTE_PARAM_V NOTE_PARAM_GT ON OFF
%token END_OF_FILE 0

%token <s>NOTE KEY_SIGN IDENTIFIER

%type <node> resolution title copyright tempo time key percussion marker channel velocity
             gatetime bank program synth volume pan chorus reverb expression pitch
             detune pitch_sense sustain transpose step note expand

%type <node> pattern
%type <i>    signed_integer

%type <node> statement note_param
%type <list> statement_list note_param_list

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
    | percussion
    | marker
    | channel
    | velocity
    | gatetime
    | bank
    | program
    | synth
    | volume
    | pan
    | chorus
    | reverb
    | expression
    | pitch
    | detune
    | pitch_sense
    | sustain
    | transpose
    | step
    | note
    | pattern
    | expand
    | error
        {
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
    | TEMPO INTEGER
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

percussion
    : PERCUSSION ON
        {
            ASTPercussion *n = node(Percussion, @$);
            n->on = true;
            $$ = n;
        }
    | PERCUSSION OFF
        {
            ASTPercussion *n = node(Percussion, @$);
            n->on = false;
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
    | GATETIME STEP signed_integer
        {
            ASTGatetime *n = node(Gatetime, @$);
            n->value = $3;
            $$ = n;
        }
    ;

bank
    : BANK INTEGER
        {
            ASTBank *n = node(Bank, @$);
            n->bankNo = $2;
            $$ = n;
        }
    ;

program
    : PROGRAM INTEGER
        {
            ASTProgram *n = node(Program, @$);
            n->programNo = $2;
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
    : PAN signed_integer
        {
            ASTPan *n = node(Pan, @$);
            n->value = $2;
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

expression
    : EXPRESSION INTEGER
        {
            ASTExpression *n = node(Expression, @$);
            n->value = $2;
            $$ = n;
        }
    ;

pitch
    : PITCH signed_integer
        {
            ASTPitch *n = node(Pitch, @$);
            n->value = $2;
            $$ = n;
        }
    ;

detune
    : DETUNE signed_integer
        {
            ASTDetune *n = node(Detune, @$);
            n->value = $2;
            $$ = n;
        }
    ;

pitch_sense
    : PITCH SENSE INTEGER
        {
            ASTPitchSense *n = node(PitchSense, @$);
            n->value = $3;
            $$ = n;
        }
    ;

sustain
    : SUSTAIN ON
        {
            ASTSustain *n = node(Sustain, @$);
            n->value = 127;
            $$ = n;
        }
    | SUSTAIN OFF
        {
            ASTSustain *n = node(Sustain, @$);
            n->value = 0;
            $$ = n;
        }
    ;

transpose
    : TRANSPOSE signed_integer
        {
            ASTTranspose *n = node(Transpose, @$);
            n->value = $2;
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

pattern
    : PATTERN IDENTIFIER statement_list end
        {
            ASTPattern *n = node(Pattern, @$);
            n->identifier = NACStringToUpperCase($2);
            n->node.children = $3;
            $$ = n;
        }
    ;

end
    : END
    | END_OF_FILE
        {
            NAMidiParserUnExpectedEOF(NAMidi_get_extra(scanner), location(@$));
        }
    ;

expand
    : EXPAND IDENTIFIER
        {
            ASTExpand *n = node(Expand, @$);
            n->identifier = NACStringToUpperCase($2);
            n->node.children = list();
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
    : NOTE_PARAM_V '=' INTEGER
        {
            ASTNoteParam *n = node(NoteParam, @$);
            n->type = NoteParamTypeVelocity;
            n->value = $3;
            $$ = n;
        }
    | NOTE_PARAM_GT '=' INTEGER
        {
            ASTNoteParam *n = node(NoteParam, @$);
            n->type = NoteParamTypeGatetime;
            n->value = $3;
            $$ = n;
        }
    ;

signed_integer
    : INTEGER { $$ = $1; }
    | '+' INTEGER { $$ = $2; }
    | '-' INTEGER { $$ = -$2; }
    ;

%%

int NAMidi_error(YYLTYPE *yylloc, yyscan_t scanner, void **node, const char *message)
{
    NAMidiParserSyntaxError(NAMidi_get_extra(scanner), location((*yylloc)), NAMidi_get_text(scanner));
    return 0;
}
