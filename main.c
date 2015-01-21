/*
 * main.c file
 */
 
#include "DSLParser.h"
#include "Sequence.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int main(int argc, char **argv)
{
    DSLParser *parser = DSLParserCreate();
    if (!DSLParserParseFile(parser, argv[1])) {
        printf("ERROR=%d\n", parser->error);
    }
    else {
        DSLParserDumpExpression(parser);

        Sequence *sequence = NATypeNew(Sequence);
        sequence->resolution = 480;
        sequence->title = malloc(32);
        sprintf(sequence->title, "This is a test!");

        Track *track = NATypeNew(Track);

        NoteEvent *note1 = NATypeNew(NoteEvent, 0, 9, 46, 127, 240);
        NoteEvent *note2 = NATypeNew(NoteEvent, 240, 9, 46, 127, 240);
        NoteEvent *note3 = NATypeNew(NoteEvent, 480, 9, 46, 127, 240);
        NoteEvent *note4 = NATypeNew(NoteEvent, 720, 9, 46, 127, 240);
        NoteEvent *note5 = NATypeNew(NoteEvent, 960, 9, 46, 127, 240);

        CFArrayAppendValue(track->events, note1);
        CFArrayAppendValue(track->events, note2);
        CFArrayAppendValue(track->events, note3);
        CFArrayAppendValue(track->events, note4);
        CFArrayAppendValue(track->events, note5);

        CFArrayAppendValue(sequence->tracks, track);

        CFStringRef cfString = NADescription(sequence);
        CFShow(cfString);
        CFRelease(cfString);

        NARelease(note1);
        NARelease(note2);
        NARelease(note3);
        NARelease(note4);
        NARelease(note5);
        NARelease(track);

        NARelease(sequence);

        DSLParserDestroy(parser);
    }
 
    return 0;
}
