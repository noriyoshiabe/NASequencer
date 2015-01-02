#pragma once

#include <vector>
#include <string>
#include <set>
#include <map>
#include <thread>

#include <ctime>

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>

class FSWatcher;

class FSWatcherListener {
public:
    virtual void onFileChanged(FSWatcher *fswatcher, std::vector<std::string> &changedFiles) = 0;
    virtual void onError(FSWatcher *fswatcher, int error, char *message) = 0;
};

class FSWatcher {
private:
    FSWatcherListener *listener;
    std::map<std::string, struct tm> files;
    std::set<std::string> dirpaths;

    std::thread *thread;
    FSEventStreamRef stream;
    CFRunLoopRef runloop;

public:
    void registerFilepath(const char *filepath);
    void start();
    void run();
    void onFSChanged();
    void finish();

    FSWatcher(FSWatcherListener *listener) {
        this->listener = listener;
    }

    ~FSWatcher() {
        if (thread) {
            thread->join();
            delete thread;
        }
    }
};
