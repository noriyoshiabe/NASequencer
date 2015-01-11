#pragma once

#include "parser.h"
#include "sequence.h"
#include "fswatcher.h"
#include "player.h"

#include <functional>
#include <deque>
#include <thread>

class NAMidi : public FSWatcherListener {
private:
    bool isExit;
    std::deque<std::function<void()>> loopQ;
    std::thread *thread;
    std::mutex mtx;
    std::condition_variable cv;

    ParseContext *context;
    SequenceVisitor *visitor;
    FSWatcher *watcher;
    Player *player;
    std::string currentFile;

    void postRunLoop(std::function<void()> func);
    void run();
    void readFile();

public:
    void onFileChanged(FSWatcher *fswatcher, std::string &changedFile);
    void onError(FSWatcher *fswatcher, int error, char *message);
    void setSequenceVisitor(SequenceVisitor *visitor);
    void setFile(const char *filepath);
    void start();
    void finish();

    NAMidi();
    ~NAMidi();
};
