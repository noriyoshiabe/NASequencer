#include <cstdio>
#include <csignal>
#include <thread>

#include "namidi.h"
#include "console_writer.h"

std::mutex mtx;
std::condition_variable cv;

void signal_handler(int signo)
{
    cv.notify_all();
}

int main(int argc, char *argv[])
{
    if (2 != argc) {
        printf("Usage: ./namidi <filepath>\n\n");
        return -1;
    }

    if (SIG_ERR == signal(SIGINT, signal_handler)) {
        printf("can not catch SIGINT\n");
        exit(1);
    }

    NAMidi *namidi = new NAMidi();
    ConsoleWriter *writer = new ConsoleWriter();

    namidi->setSequenceVisitor(writer);
    namidi->setFile(argv[1]);
    namidi->start();

    std::unique_lock<std::mutex> lk(mtx);
    cv.wait(lk);

    namidi->finish();

    delete writer;
    delete namidi;

    return 0;
}
