#include "parser.h"
#include "console_writer.h"
#include <stdio.h>
#include <thread>
#include <iostream>

#include <signal.h>

/*
std::mutex mtx;
std::condition_variable cv;

void worker()
{
    for (;;) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        printf("notify cv\n");
        //std::cout << "thread id: " << std::this_thread::get_id() << std::endl;
        cv.notify_all();
    }
}

*/

#include "fswatcher.h"

bool isExit = false;

void signal_handler(int signo)
{
    printf("SIGINT recv\n");
    isExit = true;
}

class NAMidiClient : public FSWatcherListener {
public:
    void onFileChanged(std::vector<std::string> changedFiles) {
    };
};

int main(int argc, char **argv)
{
    //th.join();
    

    //ParseContext *context = Parser::parse("./test.namidi");
    //ConsoleWriter writer;
    //writer.write(context);

    //delete context;

    if (SIG_ERR == signal(SIGINT, signal_handler)) {
        printf("Can't catch SIGINT\n");
        exit(1);
    }
    
    //std::thread th(worker);

    FSWatcher watcher(NULL);
    watcher.registerFilepath("./test.namidi");
    watcher.start();

    while (!isExit) {
        printf(".");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        fflush(stdout);
        //std::unique_lock<std::mutex> lk(mtx);
        //fflush(stdout);

        //printf("berore wait\n");
        //std::unique_lock<std::mutex> lk(mtx);
        //cv.wait(lk);
        //printf("after wait\n");
    }

    //printf("\n");

    watcher.finish();

    return 0;
}
