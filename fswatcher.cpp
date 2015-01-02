#include "fswatcher.h"

static void executer(FSWatcher *self)
{
    self->run();
}

static void fsCallback(
    ConstFSEventStreamRef streamRef,
    void *clientCallBackInfo,
    size_t numEvents,
    void *eventPaths,
    const FSEventStreamEventFlags eventFlags[],
    const FSEventStreamEventId eventIds[])
{
    int i;
    char **paths = (char **)eventPaths;

    printf("Callback called\n");
    for (i=0; i<numEvents; i++) {
        /* フラグはunsigned long、IDはuint64_t */
        printf("Change %llu in %s, flags %u\n", eventIds[i], paths[i], eventFlags[i]);
    }
}

void FSWatcher::registerFilepath(const char *filepath)
{
    filepaths.insert(std::string(filepath));
}

void FSWatcher::start()
{
    thread = new std::thread(executer, this);
}

void FSWatcher::run()
{
    CFMutableArrayRef _paths = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);

    for (std::string path : filepaths) {
        CFStringRef _string = CFStringCreateWithCString(kCFAllocatorDefault, path.c_str(), kCFStringEncodingUTF8);
        CFArrayAppendValue(_paths, _string);
        CFRelease(_string);
    }

    FSEventStreamContext context = {0, this, NULL, NULL, NULL};
 
    stream = FSEventStreamCreate(kCFAllocatorDefault,
        &fsCallback,
        &context,
        _paths,
        kFSEventStreamEventIdSinceNow,
        1.0,
        kFSEventStreamCreateFlagNone
    );

    CFRelease(_paths);

    runloop = CFRunLoopGetCurrent();
    FSEventStreamScheduleWithRunLoop(stream, runloop, kCFRunLoopDefaultMode);
    FSEventStreamStart(stream);
    CFRunLoopRun();
}

void FSWatcher::finish()
{
    FSEventStreamStop(stream);
    FSEventStreamInvalidate(stream);
    FSEventStreamRelease(stream);

    CFRunLoopStop(runloop);
}
