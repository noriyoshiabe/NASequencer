#include <stdio.h>
#include <signal.h>

#include "CLI.h"

static CLI *_cli;

static void signalHandler(int signal)
{
    CLISigInt(_cli);
}

int main(int argc, char **argv)
{
    signal(SIGINT, signalHandler);
    _cli = CLICreate(1 < argc ? argv[1] : NULL);
    CLIRun(_cli);
    CLIDestroy(_cli);
    return 0;
}
