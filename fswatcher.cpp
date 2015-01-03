#include "fswatcher.h"

#include <cstdlib>
#include <cstring>
#include <cerrno>

#include <libgen.h>
#include <unistd.h>
#include <sys/stat.h>

static void executer(FSWatcher *self)
{
    self->run();
}

static void fsCallback(ConstFSEventStreamRef streamRef, void *self, size_t numEvents,
        void *eventPaths, const FSEventStreamEventFlags eventFlags[], const FSEventStreamEventId eventIds[])
{
    char **paths = (char **)eventPaths;
    for (int i = 0; i < numEvents; ++i) {
        if (eventFlags[i] & (kFSEventStreamEventFlagItemCreated | kFSEventStreamEventFlagItemRemoved
                    | kFSEventStreamEventFlagItemRenamed | kFSEventStreamEventFlagItemModified)) {
            ((FSWatcher *)self)->onFSChanged();
            return;
        }
    }
}

static struct tm*getModifiedTime(const char *filepath)
{
    struct stat attrib;

    if (0 != stat(filepath, &attrib)) {
        return NULL;
    }
    else {
        return gmtime(&(attrib.st_mtime));
    }
}

void FSWatcher::registerFilepath(const char *filepath)
{
    struct tm* clock;
    char buf[PATH_MAX + 1];
    char *actualpath = realpath(filepath, buf);

    if (!(clock = getModifiedTime(actualpath))) {
        listener->onError(this, errno, strerror(errno));
    }
    else {
        memcpy(&lastModified, clock, sizeof(struct tm));
        files.emplace(std::string(actualpath));
        dirpaths.emplace(std::string(dirname(actualpath)));
    }
}

void FSWatcher::start()
{
    thread = new std::thread(executer, this);
}

void FSWatcher::run()
{
    CFMutableArrayRef _paths = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);

    for (std::string path : dirpaths) {
        CFStringRef _string = CFStringCreateWithCString(kCFAllocatorDefault, path.c_str(), kCFStringEncodingUTF8);
        CFArrayAppendValue(_paths, _string);
        CFRelease(_string);
    }

    FSEventStreamContext context = {0, this, NULL, NULL, NULL};
 
    stream = FSEventStreamCreate(kCFAllocatorDefault, &fsCallback, &context,
            _paths, kFSEventStreamEventIdSinceNow, 0, kFSEventStreamCreateFlagNone);

    CFRelease(_paths);

    runloop = CFRunLoopGetCurrent();
    FSEventStreamScheduleWithRunLoop(stream, runloop, kCFRunLoopDefaultMode);
    FSEventStreamStart(stream);

    CFRunLoopRun();
}

void FSWatcher::onFSChanged()
{
    struct tm* clock;

    for (std::string file : files) {
        if (!(clock = getModifiedTime(file.c_str()))) {
            listener->onError(this, errno, strerror(errno));
        }
        else {
            if (memcmp(clock, &lastModified, sizeof(struct tm))) {
                memcpy(&lastModified, clock, sizeof(struct tm));
                listener->onFileChanged(this, file);
                break;
            }
        }
    }
}

void FSWatcher::finish()
{
    FSEventStreamStop(stream);
    FSEventStreamInvalidate(stream);
    FSEventStreamRelease(stream);

    CFRunLoopStop(runloop);
}
