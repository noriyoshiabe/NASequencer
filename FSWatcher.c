#include "FSWatcher.h"

#include <pthread.h>
#include <stdarg.h>
#include <limits.h>
#include <libgen.h>
#include <sys/stat.h>
#include <NACFHelper.h>

NADeclareAbstractClass(FSWatcherListener);

struct _FSWatcher {
    void *listener;
    pthread_t thread;
    CFMutableSetRef files;
    CFMutableSetRef dirPaths;
    FSEventStreamRef stream;
    CFRunLoopRef runloop;
    struct tm lastModified;;
};

static void *__FSWatcherInit(void *_self, ...)
{
    FSWatcher *self = _self;
    va_list ap;
    va_start(ap, _self);
    self->listener = va_arg(ap, void *);
    va_end(ap);

    self->files = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);
    self->dirPaths = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);

    return self;
}

static void __FSWatcherDestroy(void *_self)
{
    FSWatcher *self = _self;

    pthread_join(self->thread, NULL);

    CFRelease(self->files);
    CFRelease(self->dirPaths);
}

NADeclareVtbl(FSWatcher, NAType,
        __FSWatcherInit,
        __FSWatcherDestroy,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        );
NADeclareClass(FSWatcher, NAType);

static void onFileChanged(FSWatcher *self, CFStringRef changedFile)
{
    NAVtbl(self->listener, FSWatcherListener)->onFileChanged(self->listener, self, changedFile);
}

static void onError(FSWatcher *self)
{
    CFStringRef message = CFStringCreateWithCString(NULL, strerror(errno), kCFStringEncodingUTF8);
    NAVtbl(self->listener, FSWatcherListener)->onError(self->listener, self, errno, message);
    CFRelease(message);
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
    CFSetGetValues(self->files, (const void **)&values);

    for (int i = 0; i < count; ++i) {
        struct tm* clock = getModifiedTime(NACFString2CString(values[i]));
        if (!clock) {
            onError(self);
        }
        else {
            if (memcmp(clock, &self->lastModified, sizeof(struct tm))) {
                memcpy(&self->lastModified, clock, sizeof(struct tm));
                onFileChanged(self, values[i]);
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

    char **paths = (char **)eventPaths;
    for (int i = 0; i < numEvents; ++i) {
        if (eventFlags[i] & (kFSEventStreamEventFlagItemCreated | kFSEventStreamEventFlagItemRemoved
                    | kFSEventStreamEventFlagItemRenamed | kFSEventStreamEventFlagItemModified)) {
            onFSChanged(self);
            return;
        }
    }
}

void FSWatcherRegisterFilepath(FSWatcher *self, CFStringRef filepath)
{
    struct tm* clock;
    char buf[PATH_MAX + 1];
    char *actualpath = realpath(NACFString2CString(filepath), buf);

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
    CFSetGetValues(self->files, (const void **)&values);
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

    return NULL;
}

void FSWatcherStart(FSWatcher *self)
{
    pthread_create(&self->thread, NULL, run, self);
}

void FSWatcherFinish(FSWatcher *self)
{
    FSEventStreamStop(self->stream);
    FSEventStreamInvalidate(self->stream);
    FSEventStreamRelease(self->stream);

    CFRunLoopStop(self->runloop);
}
