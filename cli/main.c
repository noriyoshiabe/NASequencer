#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <getopt.h>

#include "CLI.h"

static void showError(CLIError error);
static void showHelp();

static struct option _options[] = {
    { "output", required_argument, NULL, 'o'},
    { "sound-font", required_argument, NULL, 's'},
    { "help", no_argument, NULL, 'h'},

    {NULL, 0, NULL, 0}
};

static CLI *_cli;

static void signalHandler(int signal)
{
    CLISigInt(_cli);
}

int main(int argc, char **argv)
{
    int opt;
    
    const char *soundSources[8] = {NULL};
    int index = 0;

    const char *output = NULL;
    const char *input = NULL;

    while (-1 != (opt = getopt_long(argc, argv, "o:s:h", _options, NULL))) {
        switch (opt) {
        case 'o':
            output = optarg;
            break;
        case 's':
            soundSources[index++] = optarg;
            break;
        case 'h':
            showHelp();
            return EXIT_SUCCESS;
        case '?':
            return EXIT_FAILURE;
        }
    }

    if (optind < argc) {
        input = argv[optind];
    }

    _cli = CLICreate(input, soundSources);

    CLIError error;
    if (output) {
        error = CLIExport(_cli, output);
    }
    else {
        signal(SIGINT, signalHandler);
        error = CLIRunShell(_cli);
    }

    CLIDestroy(_cli);

    if (CLIErrorNoError != error) {
        showError(error);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static void showError(CLIError error)
{
    switch (error) {
    case CLIErrorNoError:
        break;
    case CLIErrorExportWithNoInputFile:
        fprintf(stderr, "-o, --output option is specifid with no input source file\n");
        break;
    case CLIErrorExportWithUnsupportedFileType:
        fprintf(stderr, "Unsupported output file type.\n");
        break;
    case CLIErrorExportWithParseFailed:
        fprintf(stderr, "Parse failed.\n");
        break;
    case CLIErrorExportWithNoSoundSource:
        fprintf(stderr, "WAV and MP3 output require valid sound source with -s, --sound-font option.\n");
        break;
    case CLIErrorExportWithSoundSourceLoadFailed:
        fprintf(stderr, "Load sound source failed.\n");
        break;
    case CLIErrorExportWithCannotWriteToOutputFile:
        fprintf(stderr, "Cannot write to output file.\n");
        break;
    }
}

static void showHelp()
{
    printf(
          "Usage: namidi [options] [file]\n"
          "Options:\n"
          " -o, --outout <file>      Write output to SMF, WAV or MP3.\n"
          "                          WAV and MP3 output require valid synthesizer with -s, --sound-font option.\n"
          " -s, --sound-font <file>  Specify sound font file for synthesizer.\n"
          " -h, --help               This help text.\n"
          );
}
