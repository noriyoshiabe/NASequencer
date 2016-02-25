#include "FSWatcher.h"
#include "NAMap.h"
#include "NAArray.h"
#include "NACInteger.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <limits.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/stat.h>

struct _FSWatcher {
    FSWatcherCallbacks *callbacks;
    void *receiver;
    pthread_t thread;
    bool exit;
    NAMap *watchingPaths;
};

FSWatcher *FSWatcherCreate(FSWatcherCallbacks *callbacks, void *receiver)
{
    FSWatcher *self = calloc(1, sizeof(FSWatcher));
    self->callbacks = callbacks;
    self->receiver = receiver;

    self->watchingPaths = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);

    return self;
}

void FSWatcherDestroy(FSWatcher *self)
{
    self->exit = true;

    if (0 < self->thread) {
        pthread_join(self->thread, NULL);
    }

    NAMapTraverseKey(self->watchingPaths, free);
    NAMapTraverseValue(self->watchingPaths, free);
    NAMapDestroy(self->watchingPaths);

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

void FSWatcherRegisterFilepath(FSWatcher *self, const char *filepath)
{
    struct tm *clock;
    char buf[PATH_MAX + 1];
    char *actualpath = realpath(filepath, buf);
    if (!actualpath || !(clock = getModifiedTime(actualpath))) {
        onError(self);        
    }
    else {
        if (!NAMapContainsKey(self->watchingPaths, actualpath)) {
            struct tm *_clock = malloc(sizeof(struct tm));
            memcpy(_clock, clock, sizeof(struct tm));
            NAMapPut(self->watchingPaths, strdup(actualpath), _clock);
        }
    }
}

static void *run(void *_self)
{
    FSWatcher *self = _self;

    NAIterator *iterator;
    struct timespec timeout;
    
    int kq = kqueue();
    if (0 > kq) {
        onError(self);        
        return NULL;
    }

    int fileNum = NAMapCount(self->watchingPaths);
    struct kevent *changes = calloc(fileNum, sizeof(struct kevent));
    struct kevent *events = calloc(fileNum, sizeof(struct kevent));
    NAArray *fileDescriptors = NAArrayCreate(4, NADescriptionCInteger);

    int index = 0;
    iterator = NAMapGetIterator(self->watchingPaths);
    while (iterator->hasNext(iterator)) {
        NAMapEntry *entry = iterator->next(iterator);
        char *filepath = entry->key;
        int fd = open(filepath, O_EVTONLY);
        if (0 > fd) {
            onError(self);        
            goto ERROR;
        }

        EV_SET(&changes[index++], fd,
                EVFILT_VNODE,
                EV_ADD | EV_CLEAR,
                NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_LINK | NOTE_RENAME | NOTE_REVOKE,
                0,
                filepath);

        NAArrayAppend(fileDescriptors, NACIntegerFromInteger(fd));
    }

    while (!self->exit) {
        timeout.tv_sec = 0;
        timeout.tv_nsec = 50 * 1000 * 1000;

        int count = kevent(kq, changes, fileNum, events, fileNum, &timeout);
        for (int i = 0; i < count; ++i) {
            if (EV_ERROR == events[i].flags) {
                onError(self);
            }
            else {
                struct tm *clock = getModifiedTime(events[i].udata);
                if (!clock) {
                    if (events[i].fflags & NOTE_RENAME) {
                        ; // notified with normal saving
                    }
                    else {
                        onError(self);
                    }
                }
                else {
                    struct tm *lastModifiedTime = NAMapGet(self->watchingPaths, events[i].udata);
                    if (memcmp(clock, lastModifiedTime, offsetof(struct tm, tm_isdst))) {
                        memcpy(lastModifiedTime, clock, sizeof(struct tm));
                        onFileChanged(self, events[i].udata);
                        break;
                    }
                }
            }
        }
    }

ERROR:
    close(kq);

    iterator = NAArrayGetIterator(fileDescriptors);
    while (iterator->hasNext(iterator)) {
        int *fd = iterator->next(iterator);
        close(*fd);
        free(fd);
    }
    NAArrayDestroy(fileDescriptors);

    free(changes);
    free(events);

    return NULL;
}

void FSWatcherStart(FSWatcher *self)
{
    pthread_create(&self->thread, NULL, run, self);
}
