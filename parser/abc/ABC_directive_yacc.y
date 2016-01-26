%{
 
#include "ABC_directive_yacc.h"
#include "ABC_directive_lex.h"

#include "ABCParser.h"
#include "ABCAST.h"

#include <string.h>

extern int ABC_directive_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, int line, int columnOffset, void **node, const char *message);

#define location(yylloc) &((FileLocation){(char *)filepath, line, yylloc.first_column + columnOffset})
#define node(type, yylloc) ABCAST##type##Create(location(yylloc))
#define list() NAArrayCreate(4, NULL)
#define listAppend(list, node) NAArrayAppend(list, node)

%}

%name-prefix = "ABC_directive_"
%output = "ABC_directive_yacc.c"
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
    char *s;
    void *node;
    void *list;
}

%token <i>INTEGER
%token <s>VOICE_ID
%token MIDI VOICE INSTRUMENT BANK MUTE CHORDPROG
%token PROPAGATE_ACCIDENTALS NOT PITCH OCTAVE

%type <node> midi propagate_accidental
%type <node> midi_param
%type <list> midi_param_list

%type <node> statement

%destructor { free($$); } VOICE_ID

%%

input
    : /* empty */
        {
            *node = NULL;
        }
    | statement
        {
            *node = $1;
        }
    ;

statement
    : midi
    | propagate_accidental
    | error '\n'
        {
            $$ = NULL;
        }
    ;

midi
    : MIDI VOICE
        {
            ASTMidi *n = node(Midi, @$);
            n->node.children = list();
            $$ = n;
        }
    | MIDI VOICE midi_param_list
        {
            ASTMidi *n = node(Midi, @$);
            n->node.children = $3;
            $$ = n;
        }
    | MIDI CHORDPROG INTEGER
        {
            $$ = NULL; // Unsupported
        }
    ;

midi_param_list
    : midi_param
        {
            $$ = list();
            if ($1) {
                listAppend($$, $1);
            }
        }
    | midi_param_list midi_param
        {
            $$ = $1;
            if ($2) {
                listAppend($$, $2);
            }
        }
    ;

midi_param
    : VOICE_ID
        {
            ASTMidiParam *n = node(MidiParam, @$);
            n->type = MidiVoiceId;
            n->string = $1;
            $$ = n;
        }
    | INSTRUMENT '=' INTEGER
        {
            ASTMidiParam *n = node(MidiParam, @$);
            n->type = MidiInstrument;
            n->intValue = $3;
            $$ = n;
        }
    | BANK '=' INTEGER
        {
            ASTMidiParam *n = node(MidiParam, @$);
            n->type = MidiBank;
            n->intValue = $3;
            $$ = n;
        }
    | MUTE
        {
            ASTMidiParam *n = node(MidiParam, @$);
            n->type = MidiMute;
            $$ = n;
        }
    | error '\n'
        {
            $$ = NULL;
        }
    ;

propagate_accidental
    : PROPAGATE_ACCIDENTALS NOT
        {
            ASTPropagateAccidental *n = node(PropagateAccidental, @$);
            n->type = PropagateAccidentalNot;
            $$ = n;
        }
    | PROPAGATE_ACCIDENTALS OCTAVE
        {
            ASTPropagateAccidental *n = node(PropagateAccidental, @$);
            n->type = PropagateAccidentalOctave;
            $$ = n;
        }
    | PROPAGATE_ACCIDENTALS PITCH
        {
            ASTPropagateAccidental *n = node(PropagateAccidental, @$);
            n->type = PropagateAccidentalPitch;
            $$ = n;
        }
    ;

%%

int ABC_directive_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, int line, int columnOffset, void **node, const char *message)
{
    ABCParserSyntaxError(ABC_directive_get_extra(scanner), location((*yylloc)), ABC_directive_get_text(scanner));
    return 0;
}
