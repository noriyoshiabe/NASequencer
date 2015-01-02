#include "parser.h"
#include "console_writer.h"
#include <cstdio>
#include <thread>
#include <iostream>

#include <csignal>

#include <libgen.h>

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

class NAMidi : public FSWatcherListener {
public:
    void onFileChanged(std::vector<std::string> &changedFiles) {
        for (std::string filepath : changedFiles) {
            printf("--- %s\n", filepath.c_str());
        }
    };

    void onError(int error, char *message) {
        printf("--- error %d - %s\n", error, message);
    }
};

#include <ctime>
int main(int argc, char *argv[])
{
    if (2 != argc) {
        printf("Usage: ./namidi <filepath>\n\n");
        return -1;
    }
    //th.join();
    //

    //ParseContext *context = Parser::parse("./test.namidi");
    //ConsoleWriter writer;
    //writer.write(context);
    //delete context;

    if (SIG_ERR == signal(SIGINT, signal_handler)) {
        printf("Can't catch SIGINT\n");
        exit(1);
    }
    
    //std::thread th(worker);

    NAMidi namidi;
    FSWatcher watcher(&namidi);
    watcher.registerFilepath(argv[1]);
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
