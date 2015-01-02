#pragma once

#include <vector>
#include <string>
#include <set>
#include <thread>

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>

class FSWatcherListener {
public:
    virtual void onFileChanged(std::vector<std::string> changedFiles) = 0;
};

class FSWatcher {
private:
    FSWatcherListener *listener;
    std::set<std::string> filepaths;

    std::thread *thread;
    FSEventStreamRef stream;
    CFRunLoopRef runloop;

public:
    void registerFilepath(const char *filepath);
    void start();
    void run();
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
