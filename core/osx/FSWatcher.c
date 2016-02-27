#include "FSWatcher.h"
#include "NASet.h"
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
    NASet *watchingPaths;
};

FSWatcher *FSWatcherCreate(FSWatcherCallbacks *callbacks, void *receiver)
{
    FSWatcher *self = calloc(1, sizeof(FSWatcher));
    self->callbacks = callbacks;
    self->receiver = receiver;

    self->watchingPaths = NASetCreate(NAHashCString, NADescriptionCString);

    return self;
}

void FSWatcherDestroy(FSWatcher *self)
{
    self->exit = true;

    if (0 < self->thread) {
        pthread_join(self->thread, NULL);
    }

    NASetTraverse(self->watchingPaths, free);
    NASetDestroy(self->watchingPaths);

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

void FSWatcherRegisterFilepath(FSWatcher *self, const char *filepath)
{
    char buf[PATH_MAX + 1];
    char *actualpath = realpath(filepath, buf);
    if (!actualpath) {
        onError(self);        
    }
    else {
        if (!NASetContains(self->watchingPaths, actualpath)) {
            NASetAdd(self->watchingPaths, strdup(actualpath));
        }
    }
}

char *flagstring(int flags)
{
    static char ret[512];
    char *or = "";
 
    ret[0]='\0'; // clear the string.
    if (flags & NOTE_DELETE) {strcat(ret,or);strcat(ret,"NOTE_DELETE");or="|";}
    if (flags & NOTE_WRITE) {strcat(ret,or);strcat(ret,"NOTE_WRITE");or="|";}
    if (flags & NOTE_EXTEND) {strcat(ret,or);strcat(ret,"NOTE_EXTEND");or="|";}
    if (flags & NOTE_ATTRIB) {strcat(ret,or);strcat(ret,"NOTE_ATTRIB");or="|";}
    if (flags & NOTE_LINK) {strcat(ret,or);strcat(ret,"NOTE_LINK");or="|";}
    if (flags & NOTE_RENAME) {strcat(ret,or);strcat(ret,"NOTE_RENAME");or="|";}
    if (flags & NOTE_REVOKE) {strcat(ret,or);strcat(ret,"NOTE_REVOKE");or="|";}
 
    return ret;
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

    int fileNum = NASetCount(self->watchingPaths);
    struct kevent *changes = calloc(fileNum, sizeof(struct kevent));
    struct kevent *events = calloc(fileNum, sizeof(struct kevent));
    NAArray *fileDescriptors = NAArrayCreate(4, NADescriptionCInteger);

    int index = 0;
    iterator = NASetGetIterator(self->watchingPaths);
    while (iterator->hasNext(iterator)) {
        char *filepath = iterator->next(iterator);
        int fd = open(filepath, O_EVTONLY);
        if (0 > fd) {
            onError(self);        
            goto ERROR;
        }

        EV_SET(&changes[index++], fd,
                EVFILT_VNODE,
                EV_ADD | EV_CLEAR,
                NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND | /*NOTE_ATTRIB |*/ NOTE_LINK | NOTE_RENAME | NOTE_REVOKE,
                0,
                filepath);

        NAArrayAppend(fileDescriptors, NACIntegerFromInteger(fd));
    }

    while (!self->exit) {
        timeout.tv_sec = 0;
        timeout.tv_nsec = 50 * 1000 * 1000;

        int count = kevent(kq, changes, fileNum, events, fileNum, &timeout);
        if (0 > count || EV_ERROR == events[0].flags) {
            onError(self);
        }
        else if (0 < count) {
            onFileChanged(self, events[0].udata);
            break;
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
