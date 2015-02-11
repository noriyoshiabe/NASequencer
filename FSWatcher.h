#pragma once

#include <NAType.h>
#include <CoreServices/CoreServices.h>

typedef struct _FSWatcher FSWatcher;
NAExportClass(FSWatcher);

extern void FSWatcherRegisterFilepath(FSWatcher *self, const char *filepath);
extern void FSWatcherStart(FSWatcher *self);
extern void FSWatcherFinish(FSWatcher *self);

typedef struct _FSWatcherListenerVtbl {
    void (*onFileChanged)(void *self, FSWatcher *fswatcher, CFStringRef changedFile);
    void (*onError)(void *self, FSWatcher *fswatcher, int error, CFStringRef message);
} FSWatcherListenerVtbl;

NAExportClass(FSWatcherListener);
