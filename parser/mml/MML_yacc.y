%{
 
#include "MML_yacc.h"
#include "MML_lex.h"

#include "MMLParser.h"
#include "MMLAST.h"

#include "NACString.h"

extern int MML_error(YYLTYPE *yylloc, yyscan_t scanner, void **node, const char *message);

#define location(yylloc) &((FileLocation){MMLParserGetCurrentFilepath(MML_get_extra(scanner)), yylloc.first_line, yylloc.first_column})
#define node(type, yylloc) MMLAST##type##Create(location(yylloc))
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
%token OCTAVE TRANSPOSE R_TRANSPOSE LENGTH GATETIME A_GATETIME VELOCITY A_VELOCITY TUPLET_START
%token REPEAT_START REPEAT_END REPEAT_BREAK
%token <s> NOTE REST TUPLET_END VELOCITY_SHIFT

%type <node> d_timebase d_title d_copyright d_marker d_velocity d_octave channel synthesizer bank_select program_change
%type <node> volume chorus reverb expression pan detune tempo note rest octave
%type <node> transpose tie length gatetime velocity tuplet track_change repeat repeat_break chord

%type <i> signed_integer
%type <s> tuplet_end

%type <node> statement
%type <list> statement_list

%destructor { free($$); } STRING
%destructor { free($$); } NOTE
%destructor { free($$); } REST
%destructor { free($$); } TUPLET_END
%destructor { free($$); } VELOCITY_SHIFT

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
            ASTTimebase *n = node(Timebase, @$);
            n->timebase = $2;
            $$ = n;
        }
    ;

d_title
    : D_TITLE STRING
        {
            ASTTitle *n = node(Title, @$);
            n->title = $2;
            $$ = n;
        }
    ;

d_copyright
    : D_COPYRIGHT STRING
        {
            ASTCopyright *n = node(Copyright, @$);
            n->text = $2;
            $$ = n;
        }
    ;

d_marker
    : D_MARKER STRING
        {
            ASTMarker *n = node(Marker, @$);
            n->text = $2;
            $$ = n;
        }
    ;

d_velocity
    : D_VELOCITY REVERSE
        {
            ASTVelocityReverse *n = node(VelocityReverse, @$);
            $$ = n;
        }
    ;

d_octave
    : D_OCTAVE REVERSE
        {
            ASTOctaveReverse *n = node(OctaveReverse, @$);
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

synthesizer
    : SYNTHESIZER STRING
        {
            ASTSynth *n = node(Synth, @$);
            n->name = $2;
            $$ = n;
        }
    ;

bank_select
    : BANK_SELECT INTEGER ',' INTEGER
        {
            ASTBankSelect *n = node(BankSelect, @$);
            n->msb = $2;
            n->lsb = $4;
            $$ = n;
        }
    ;

program_change
    : PROGRAM_CHANGE INTEGER
        {
            ASTProgramChange *n = node(ProgramChange, @$);
            n->programNo = $2;
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

pan
    : PAN signed_integer
        {
            ASTPan *n = node(Pan, @$);
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

note
    : NOTE
        {
            ASTNote *n = node(Note, @$);
            n->noteString = $1;
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

octave
    : OCTAVE signed_integer
        {
            ASTOctave *n = node(Octave, @$);
            n->value = $2;
            $$ = n;
        }
    | '<'
        {
            ASTOctave *n = node(Octave, @$);
            n->direction = '<';
            $$ = n;
        }
    | '>'
        {
            ASTOctave *n = node(Octave, @$);
            n->direction = '>';
            $$ = n;
        }
    ;

transpose
    : TRANSPOSE signed_integer
        {
            ASTTransepose *n = node(Transepose, @$);
            n->value = $2;
            $$ = n;
        }
    | R_TRANSPOSE signed_integer
        {
            ASTTransepose *n = node(Transepose, @$);
            n->relative = true;
            n->value = $2;
            $$ = n;
        }
    ;

tie
    : '&'
        {
            ASTTie *n = node(Tie, @$);
            $$ = n;
        }
    ;

length
    : LENGTH INTEGER
        {
            ASTLength *n = node(Length, @$);
            n->length = $2;
            $$ = n;
        }
    ;

gatetime
    : GATETIME INTEGER
        {
            ASTGatetime *n = node(Gatetime, @$);
            n->value = $2;
            $$ = n;
        }
    | A_GATETIME INTEGER
        {
            ASTGatetime *n = node(Gatetime, @$);
            n->absolute = true;
            n->value = $2;
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
    | A_VELOCITY INTEGER
        {
            ASTVelocity *n = node(Velocity, @$);
            n->absolute = true;
            n->value = $2;
            $$ = n;
        }
    | VELOCITY_SHIFT
        {
            ASTVelocity *n = node(Velocity, @$);
            n->direction = $1[0];
            n->value = $1[1] ? atoi($1 + 1) : 1;
            $$ = n;
            free($1);
        }
    ;

tuplet
    : TUPLET_START statement_list tuplet_end
        {
            ASTTuplet *n = node(Tuplet, @$);
            n->lengthString = $3;
            n->node.children = $2;
            $$ = n;
        }
    ;

tuplet_end
    : TUPLET_END
        {
            $$ = $1;
        }
    | END_OF_FILE
        {
            MMLParserUnExpectedEOF(MML_get_extra(scanner), location(@$));
            $$ = strdup("");
        }
    ;

track_change
    : ';'
        {
            ASTTrackChange *n = node(TrackChange, @$);
            $$ = n;
        }
    ;

repeat
    : REPEAT_START statement_list repeat_end
        {
            ASTRepeat *n = node(Repeat, @$);
            n->times = 2;
            n->node.children = $2;
            $$ = n;
        }
    | REPEAT_START INTEGER statement_list repeat_end
        {
            ASTRepeat *n = node(Repeat, @$);
            n->times = $2;
            n->node.children = $3;
            $$ = n;
        }
    ;

repeat_break
    : REPEAT_BREAK
        {
            ASTRepeatBreak *n = node(RepeatBreak, @$);
            $$ = n;
        }
    ;

repeat_end
    : REPEAT_END
    | END_OF_FILE
        {
            MMLParserUnExpectedEOF(MML_get_extra(scanner), location(@$));
        }
    ;

chord
    : '[' statement_list chord_end
        {
            ASTChord *n = node(Chord, @$);
            n->node.children = $2;
            $$ = n;
        }
    ;

chord_end
    : ']'
    | END_OF_FILE
        {
            MMLParserUnExpectedEOF(MML_get_extra(scanner), location(@$));
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
