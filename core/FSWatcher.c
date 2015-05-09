#include "FSWatcher.h"

#include <CoreServices/CoreServices.h>
#include <pthread.h>
#include <stdarg.h>
#include <limits.h>
#include <libgen.h>
#include <sys/stat.h>

struct _FSWatcher {
    FSWatcherCallbacks *callbacks;
    void *receiver;
    pthread_t thread;
    CFMutableSetRef files;
    CFMutableSetRef dirPaths;
    FSEventStreamRef stream;
    CFRunLoopRef runloop;
    struct tm lastModified;;
};

FSWatcher *FSWatcherCreate(FSWatcherCallbacks *callbacks, void *receiver)
{
    FSWatcher *self = calloc(1, sizeof(FSWatcher));
    self->callbacks = callbacks;
    self->receiver = receiver;

    self->files = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);
    self->dirPaths = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);

    return self;
}

void FSWatcherDestroy(FSWatcher *self)
{
    self->receiver = NULL;

    CFRelease(self->files);
    CFRelease(self->dirPaths);

    CFRunLoopStop(self->runloop);
}

static void onFileChanged(FSWatcher *self, const char *changedFile)
{
    if (self->receiver) {
        self->callbacks->onFileChanged(self->receiver, changedFile);
    }
}

static void onError(FSWatcher *self)
{
    if (self->receiver) {
        self->callbacks->onError(self->receiver, errno, strerror(errno));
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

static void onFSChanged(FSWatcher *self)
{
    CFIndex count = CFSetGetCount(self->files);
    CFTypeRef *values = (CFTypeRef *)malloc(count * sizeof(CFTypeRef));
    CFSetGetValues(self->files, (const void **)values);

    for (int i = 0; i < count; ++i) {
        CFIndex length = CFStringGetLength(values[i]) + 1;
        char *cstring = alloca(length);
        CFStringGetCString(values[i], cstring, length, kCFStringEncodingUTF8);

        struct tm* clock = getModifiedTime(cstring);
        if (!clock) {
            onError(self);
        }
        else {
            if (memcmp(clock, &self->lastModified, sizeof(struct tm))) {
                memcpy(&self->lastModified, clock, sizeof(struct tm));
                onFileChanged(self, cstring);
                break;
            }
        }
    }

    free(values);
}

static void fsCallback(ConstFSEventStreamRef streamRef, void *_self, size_t numEvents,
        void *eventPaths, const FSEventStreamEventFlags eventFlags[], const FSEventStreamEventId eventIds[])
{
    FSWatcher *self = _self;

    for (int i = 0; i < numEvents; ++i) {
        if (eventFlags[i] & (kFSEventStreamEventFlagItemCreated | kFSEventStreamEventFlagItemRemoved
                    | kFSEventStreamEventFlagItemRenamed | kFSEventStreamEventFlagItemModified)) {
            onFSChanged(self);
            return;
        }
    }
}

void FSWatcherRegisterFilepath(FSWatcher *self, const char *filepath)
{
    struct tm* clock;
    char buf[PATH_MAX + 1];
    char *actualpath = realpath(filepath, buf);

    if (!(clock = getModifiedTime(actualpath))) {
        onError(self);
    }
    else {
        memcpy(&self->lastModified, clock, sizeof(struct tm));
        CFStringRef path = CFStringCreateWithCString(NULL, actualpath, kCFStringEncodingUTF8);
        CFStringRef dirPath = CFStringCreateWithCString(NULL, dirname(actualpath), kCFStringEncodingUTF8);
        CFSetAddValue(self->files, path);
        CFSetAddValue(self->dirPaths, dirPath);
        CFRelease(path);
        CFRelease(dirPath);
    }
}

static void *run(void *_self)
{
    FSWatcher *self = _self;

    CFIndex count = CFSetGetCount(self->dirPaths);
    CFTypeRef *values = (CFTypeRef *)malloc(count * sizeof(CFTypeRef));
    CFSetGetValues(self->dirPaths, (const void **)values);
    CFArrayRef _paths = CFArrayCreate(NULL, values, count, &kCFTypeArrayCallBacks);
    free(values);

    FSEventStreamContext context = {0, self, NULL, NULL, NULL};
 
    self->stream = FSEventStreamCreate(kCFAllocatorDefault, fsCallback, &context,
            _paths, kFSEventStreamEventIdSinceNow, 0, kFSEventStreamCreateFlagNone);

    CFRelease(_paths);

    self->runloop = CFRunLoopGetCurrent();
    FSEventStreamScheduleWithRunLoop(self->stream, self->runloop, kCFRunLoopDefaultMode);
    FSEventStreamStart(self->stream);

    CFRunLoopRun();

    FSEventStreamStop(self->stream);
    FSEventStreamInvalidate(self->stream);
    FSEventStreamRelease(self->stream);

    free(self);

    return NULL;
}

void FSWatcherStart(FSWatcher *self)
{
    pthread_create(&self->thread, NULL, run, self);
}
