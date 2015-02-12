/*
 * main.c file
 */
 
#include "ParseContext.h"
#include "ConsoleWriter.h"
#include "MidiClient.h"

int main(int argc, char **argv)
{
    CFStringRef string = CFStringCreateWithCString(NULL, argv[1], kCFStringEncodingUTF8);
    ParseContext *parseContext = ParseContextParse(string);
    CFRelease(string);

    ConsoleWriter *writer = NATypeNew(ConsoleWriter);

    ParseContextViewRender(writer, parseContext);

    MidiClient *cl = NATypeNew(MidiClient);
    MidiClientOpen(cl);
    uint8_t bytes[] = {0x90, 41, 0x00};
    MidiClientSend(cl, bytes, sizeof(bytes));
    MidiClientClose(cl);
    NARelease(cl);

    NARelease(parseContext);
    NARelease(writer);

    return 0;
}
