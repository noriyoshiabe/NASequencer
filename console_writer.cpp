#include "console_writer.h"

#include "cstdio"

void ConsoleWriter::write(ParseContext *context)
{
    for (std::string *error : context->errors) {
        printf("\n");
        printf("errors ---------------------------------------------------------------------------\n");
        printf("%s\n", error->c_str());
    }
    for (std::string *warning : context->warnings) {
        printf("\n");
        printf("warnings -------------------------------------------------------------------------\n");
        printf("%s\n", warning->c_str());
    }

    context->sequence->accept(this);
    for (Track *track : context->sequence->tracks) {
        track->accept(this);
        for (MidiEvent *event : track->events) {
            event->accept(this);
        }
    }
}

void ConsoleWriter::visit(Sequence *elem)
{
    printf("\n");
    printf("[Sequence] resolution: %d  track count: %lu\n", elem->resolution, elem->tracks.size());
    printf("==================================================================================\n");

    resolution = elem->resolution;
}

void ConsoleWriter::visit(Track *elem)
{
    printf("\n");
    printf("[Track] no: %d\n", ++trackCount);
    printf("----------------------------------------------------------------------------------\n");
    printf("\n");
    printf("tick       | event\n");
}

void ConsoleWriter::visit(NameEvent *elem)
{
    printf("%s | [Name] %s\n", location(elem->tick), elem->text);
}

void ConsoleWriter::visit(TempoEvent *elem)
{
    printf("%s | [Tempo] %.2f\n", location(elem->tick), elem->tempo);
}

void ConsoleWriter::visit(TimeSignatureEvent *elem)
{
    printf("%s | [Time signature] %d/%d\n", location(elem->tick), elem->numerator, elem->denominator);
    numerator = elem->numerator;
    denominator = elem->denominator;
}

void ConsoleWriter::visit(BankSelectEvent *elem)
{
    printf("%s | [Bank select] channel:%d msb:%d %d:lsb\n", location(elem->tick), elem->channel, elem->msb, elem->lsb);
}

void ConsoleWriter::visit(ProgramChangeEvent *elem)
{
    printf("%s | [Program change] channel:%d program no:%d\n", location(elem->tick), elem->channel, elem->programNo);
}

void ConsoleWriter::visit(NoteEvent *elem)
{
    printf("%s | [Note] channel:%d note no:%d velocity:%d gatetime:%d\n", location(elem->tick), elem->channel, elem->noteNo, elem->velocity, elem->gatetime);
}

void ConsoleWriter::visit(MarkerEvent *elem)
{
    printf("%s | [Marker] %s\n", location(elem->tick), elem->text);
}

void ConsoleWriter::visit(TrackEndEvent *elem)
{
    printf("%s | [Track end]\n", location(elem->tick));
}
