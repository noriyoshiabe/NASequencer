#include "NAMidi.h"
#include "ConsoleWriter.h"
#include "MessageQueue.h"

#include <signal.h>

static MessageQueue *msgQ;
static Message dummy = {0, NULL};

static void signalHandler(int signo)
{
    MessageQueuePost(msgQ, &dummy);
}

int main(int argc, char **argv)
{
    if (2 != argc) {
        printf("Usage: ./namidi <filepath>\n\n");
        return -1;
    }

    if (SIG_ERR == signal(SIGINT, signalHandler)) {
        printf("can not catch SIGINT\n");
        exit(1);
    }

    msgQ = MessageQueueCreate();

    CFStringRef filepath = CFStringCreateWithCString(NULL, argv[1], kCFStringEncodingUTF8);

    NAMidi *namidi = NATypeNew(NAMidi);
    ConsoleWriter *writer = NATypeNew(ConsoleWriter);

    NAMidiAddObserver(namidi, writer);
    NAMidiSetFile(namidi, filepath);
    CFRelease(filepath);

    NAMidiParse(namidi);
    NAMidiPlay(namidi);

    MessageQueueWait(msgQ, &dummy);
    MessageQueueDestroy(msgQ);

    NARelease(namidi);
    NARelease(writer);

    return 0;
}
