#pragma once

typedef struct _FSWatcher FSWatcher;

typedef struct _FSWatcherCallbacks {
    void (*onFileChanged)(void *receiver, const char *changedFile);
    void (*onError)(void *receiver, int error, const char *message);
} FSWatcherCallbacks;

extern FSWatcher *FSWatcherCreate(FSWatcherCallbacks *callbacks, void *receiver);
extern void FSWatcherDestroy(FSWatcher *self);
extern void FSWatcherRegisterFilepath(FSWatcher *self, const char *filepath);
extern void FSWatcherStart(FSWatcher *self);
extern void FSWatcherFinish(FSWatcher *self);
