#include "ABCParser.h"
#include "ABC_yacc.h"
#include "ABC_lex.h"

#include "NAArray.h"
#include "NAUtil.h"

#include <ctype.h>

#define isValidRange(v, from, to) (from <= v && v <= to)

extern int ABC_parse(yyscan_t scanner);

static bool ABCParserParseFile(void *self, const char *filepath);
static void ABCParserDestroy(void *self);

static bool ABCParserParseFileInternal(ABCParser *self, const char *filepath, int line, int column);

struct _ABCParser {
    Parser interface;
    ParseResult *result;
    char *currentFile;

    // TODO context
    int tick;
    int id;
};

Parser *ABCParserCreate(ParseResult *result)
{
    ABCParser *self = calloc(1, sizeof(ABCParser));
    self->interface.parseFile = ABCParserParseFile;
    self->interface.destroy = ABCParserDestroy;
    self->result = result;
    return (Parser *)self;
}

static bool ABCParserParseFile(void *self, const char *filepath)
{
    return ABCParserParseFileInternal(self, filepath, 0, 0);
}

static bool ABCParserParseFileInternal(ABCParser *self, const char *filepath, int line, int column)
{
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        ABCParserError(self, line, column, ParseErrorKindFileNotFound);
        return false;
    }

    self->result->sequence = SequenceCreate();

    self->currentFile = strdup(filepath); // TODO include
    NAArrayAppend(self->result->filepaths, self->currentFile);

    yyscan_t scanner;
    ABC_lex_init_extra(self, &scanner);
    YY_BUFFER_STATE state = ABC__create_buffer(fp, YY_BUF_SIZE, scanner);
    ABC__switch_to_buffer(state, scanner);

    bool success = 0 == ABC_parse(scanner);

    // TODO
    self->result->sequence->title = strdup("TEST");
    TimeTableSetLength(self->result->sequence->timeTable, self->tick);

    ABC__delete_buffer(state, scanner);
    ABC_lex_destroy(scanner);
    fclose(fp);

    return success;
}

static void ABCParserDestroy(void *_self)
{
    ABCParser *self = _self;
    free(self);
}

bool ABCParserProcess(ABCParser *self, int line, int column, ABCExpressionType type, va_list argList)
{
    bool success = true;

    switch (type) {
    case ABCExpressionTypeNote:
        {
            const BaseNote noteTable[] = {
                BaseNote_A, BaseNote_B, BaseNote_C,
                BaseNote_D, BaseNote_E, BaseNote_F, BaseNote_G
            };

            char *pc = va_arg(argList, char *);

            BaseNote baseNote = noteTable[tolower(*pc) - 97];
            Accidental accidental = AccidentalNone;
            int octave = isupper(*pc) ? 2 : 3;

            char *c;
            while (*(c = ++pc)) {
                switch (*c) {
                case ',':
                    // TODO Illegal octave down
                    --octave;
                    break;
                case '\'':
                    // TODO Illegal octave up
                    ++octave;
                    break;
                case '^':
                    accidental = AccidentalSharp;
                    // TODO Double Sharp
                    break;
                case '_':
                    accidental = AccidentalFlat;
                    // TODO Double Flat
                    break;
                case '=':
                    accidental = AccidentalNatural;
                    break;
                }
            }

            int step = 480; // TODO length
            int gatetime = 480; // TODO length
            int velocity = 127; // TODO pianissimo, forte, etc..

            KeySign keySign = KeySignCMajor; // TODO from context

            int noteNo = NoteTableGetNoteNo(keySign, baseNote, accidental, octave); 
            if (!isValidRange(noteNo, 0, 127)) {
                ABCParserError(self, line, column, ParseErrorKindInvalidNoteRange);
                success = false;
                break;
            }

            NoteEvent *event = MidiEventAlloc(MidiEventTypeNote, ++self->id, self->tick, sizeof(NoteEvent) - sizeof(MidiEvent));
            event->noteNo = noteNo;
            event->channel = 1; // TODO from context
            event->gatetime = gatetime;
            event->velocity = velocity;

            NAArrayAppend(self->result->sequence->events, event);

            self->tick += step;
        }

        break;
    default:
        break;
    }

    return success;
}



void ABCParserError(ABCParser *self, int line, int column, ParseErrorKind errorKind)
{
    self->result->error.kind = errorKind;
    self->result->error.location.line = line;
    self->result->error.location.column = column;
    self->result->error.location.filepath = self->currentFile;
}
