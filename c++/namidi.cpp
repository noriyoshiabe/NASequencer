#include "namidi.h"

void NAMidi::postRunLoop(std::function<void()> func)
{
    std::unique_lock<std::mutex> lk(mtx);
    loopQ.push_back(func);
    cv.notify_all();
}

void NAMidi::onFileChanged(FSWatcher *fswatcher, std::string &changedFile)
{
    postRunLoop([=]{
        printf("File changed. %s\n", changedFile.c_str());
        readFile();
    });
};

void NAMidi::onError(FSWatcher *fswatcher, int error, char *message)
{
    postRunLoop([=]{
        printf("FSWatcher error: %d - %s\n", error, message);
    });
}

void NAMidi::setSequenceVisitor(SequenceVisitor *visitor)
{
    postRunLoop([=]{
        this->visitor = visitor;
        
        if (context) {
            context->accept(visitor);
        }
    });
}

void NAMidi::setFile(const char *filepath)
{
    postRunLoop([=]{
        if (watcher) {
            watcher->finish();
            delete watcher;
        }

        watcher = new FSWatcher(this);
        watcher->registerFilepath(filepath);
        watcher->start();

        currentFile = std::string(filepath);
        readFile();
    });
}

void NAMidi::readFile()
{
    if (context) {
        delete context;
    }

    context = Parser::parse(currentFile.c_str());

    if (visitor) {
        context->accept(visitor);
    }

    context->accept(player);
}

void NAMidi::start()
{
    thread = new std::thread(&NAMidi::run, this);
}

void NAMidi::run()
{
    while (!isExit) {
        std::unique_lock<std::mutex> lk(mtx);
        if (loopQ.empty()) {
            cv.wait(lk);
        }
        else {
            std::function<void()> func = loopQ.front();
            loopQ.pop_front();
            func();
        }
    }
}

void NAMidi::finish()
{
    player->stop();
    isExit = true;
    cv.notify_all();
}

NAMidi::NAMidi()
{
    player = new Player();
}

NAMidi::~NAMidi()
{
    delete player;

    if (thread) {
        thread->join();
        delete thread;
    }

    if (context) {
        delete context;
    }

    if (watcher) {
        watcher->finish();
        delete watcher;
    }
}
