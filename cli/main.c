#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <getopt.h>

#include "CLI.h"
#include "NAUtil.h"

static void showHelp();
static bool isAudioFileType(const char *filepath);

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

    if (output) {
        if (!input) {
            fprintf(stderr, "-o, --output option is specifid with no input source file\n");
            return EXIT_FAILURE;
        }

        if (isAudioFileType(output) && !soundSources[0]) {
            fprintf(stderr, "WAV and MP3 output require valid sound source with -s, --sound-font option.\n");
            return EXIT_FAILURE;
        }
    }

    _cli = CLICreate(input, soundSources);

    bool success;
    if (output) {
        success = CLIExport(_cli, output);
    }
    else {
        signal(SIGINT, signalHandler);
        success = CLIRunShell(_cli);
    }

    CLIDestroy(_cli);
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

static void showHelp()
{
    printf(
          "Usage: namidi [options] [file]\n"
          "Options:\n"
          " -o, --outout <file>      Write output to SMF, WAV or AAC.\n"
          "                          WAV and AAC output require valid synthesizer with -s, --sound-font option.\n"
          " -s, --sound-font <file>  Specify sound font file for synthesizer.\n"
          " -h, --help               This help text.\n"
          );
}

static bool isAudioFileType(const char *filepath)
{
    const char *exts[] = {
        "wav",
        "wave",
        "m4a",
        "aac",
    };

    const char *ext = NAUtilGetFileExtenssion(filepath);
    for (int i = 0; i < sizeof(exts) / sizeof(exts[0]); ++i) {
        if (0 == strcmp(exts[i], ext)) {
            return true;
        }
    }

    return false;
}
