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
    CFMutableDictionaryRef files;
    CFMutableSetRef dirPaths;
    CFRunLoopRef runloop;
};

FSWatcher *FSWatcherCreate(FSWatcherCallbacks *callbacks, void *receiver)
{
    FSWatcher *self = calloc(1, sizeof(FSWatcher));
    self->callbacks = callbacks;
    self->receiver = receiver;

    self->files = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, NULL);
    self->dirPaths = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);

    return self;
}

void FSWatcherDestroy(FSWatcher *self)
{
    CFRunLoopStop(self->runloop);
    pthread_join(self->thread, NULL);

    CFIndex count = CFDictionaryGetCount(self->files);
    struct tm *lastModifiedTimes[count];
    CFDictionaryGetKeysAndValues(self->files, NULL, (const void **)lastModifiedTimes);
    for (int i = 0; i < count; ++i) {
        free(lastModifiedTimes[i]);
    }

    CFRelease(self->files);
    CFRelease(self->dirPaths);

    free(self);
}

static void onFileChanged(FSWatcher *self, const char *changedFile)
{
    self->callbacks->onFileChanged(self->receiver, changedFile);
}

static void onError(FSWatcher *self)
{
    self->callbacks->onError(self->receiver, errno, strerror(errno));
}

static struct tm *getModifiedTime(const char *filepath)
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
    CFIndex count = CFDictionaryGetCount(self->files);
    CFStringRef filepaths[count];
    struct tm *lastModifiedTimes[count];
    CFDictionaryGetKeysAndValues(self->files, (const void **)filepaths, (const void **)lastModifiedTimes);

    for (int i = 0; i < count; ++i) {
        CFIndex length = CFStringGetLength(filepaths[i]) + 1;
        char cstring[length];
        CFStringGetCString(filepaths[i], cstring, length, kCFStringEncodingUTF8);

        struct tm *clock = getModifiedTime(cstring);
        if (!clock) {
            onError(self);
        }
        else {
            struct tm *lastModifiedTime = (struct tm *)CFDictionaryGetValue(self->files, filepaths[i]);

            if (memcmp(clock, lastModifiedTime, offsetof(struct tm, tm_isdst))) {
                memcpy(lastModifiedTime, clock, sizeof(struct tm));
                onFileChanged(self, cstring);
                break;
            }
        }
    }
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
        struct tm* _clock = malloc(sizeof(struct tm));
        memcpy(_clock, clock, sizeof(struct tm));

        CFStringRef path = CFStringCreateWithCString(NULL, actualpath, kCFStringEncodingUTF8);
        CFStringRef dirPath = CFStringCreateWithCString(NULL, dirname(actualpath), kCFStringEncodingUTF8);
        CFDictionarySetValue(self->files, path, _clock);
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
 
    FSEventStreamRef stream = FSEventStreamCreate(kCFAllocatorDefault, fsCallback, &context,
            _paths, kFSEventStreamEventIdSinceNow, 0, kFSEventStreamCreateFlagNone);

    CFRelease(_paths);

    self->runloop = CFRunLoopGetCurrent();
    FSEventStreamScheduleWithRunLoop(stream, self->runloop, kCFRunLoopDefaultMode);
    FSEventStreamStart(stream);

    CFRunLoopRun();

    FSEventStreamStop(stream);
    FSEventStreamInvalidate(stream);
    FSEventStreamRelease(stream);

    return NULL;
}

void FSWatcherStart(FSWatcher *self)
{
    pthread_create(&self->thread, NULL, run, self);
}
