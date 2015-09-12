#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <getopt.h>

#include "CLI.h"
#include "Exporter.h"
#include "NAUtil.h"

typedef enum {
    OutputFileTypeSMF,
    OutputFileTypeMP3,
    OutputFileTypeUnknown,
} OutputFileType;

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
    const char *soundSource = NULL;
    const char *output = NULL;
    const char *input = NULL;

    while (-1 != (opt = getopt_long(argc, argv, "o:s:h", _options, NULL))) {
        switch (opt) {
        case 'o':
            output = optarg;
            break;
        case 's':
            soundSource = optarg;
            break;
        case 'h':
            showHelp();
            return 0;
        case '?':
            return -1;
        }
    }

    if (optind < argc) {
        input = argv[optind];
    }

    if (output) {
        if (!input) {
            fprintf(stderr, "-o, --output option is specifid with no input source file\n");
            return -1;
        }

        const char *ext = NAUtilGetFileExtenssion(output);
        const struct {
            const char *ext;
            OutputFileType type;
        } table[] = {
            {"mid", OutputFileTypeSMF},
            {"midi", OutputFileTypeSMF},
            {"smf", OutputFileTypeSMF},
            {"mp3", OutputFileTypeMP3},
        };

        OutputFileType type = OutputFileTypeUnknown;

        for (int i = 0; i < sizeof(table) / sizeof(table[0]); ++i) {
            if (0 == strcmp(table[i].ext, ext)) {
                type = table[i].type;
                break;
            }
        }

        switch (type) {
        case OutputFileTypeSMF:
            {
                Exporter *exporter = ExporterCreate(input, NULL);
                bool success = ExporterWriteToSMF(exporter, output);
                ExporterDestroy(exporter);

                if (!success) {
                    fprintf(stderr, "Export failed.\n");
                    return -1;
                }
            }
            break;
        case OutputFileTypeMP3:
            if (!soundSource) {
                fprintf(stderr, "MP3 output requires valid synthesizer with -s, --sound-font option.\n");
                return -1;
            }
            
            printf("TODO: MP3 output is not implemented yet. (-- )v\n");
            break;
        case OutputFileTypeUnknown:
            fprintf(stderr, "Unsupported output file type .%s\n", ext);
            return -1;
        }
    }
    else {
        signal(SIGINT, signalHandler);
        _cli = CLICreate(input, soundSource);
        CLIRun(_cli);
        CLIDestroy(_cli);
    }

    return 0;
}

static void showHelp()
{
    printf(
          "Usage: namidi [options] [file]\n"
          "Options:\n"
          " -o, --outout <file>      Write output to SMF or MP3.\n"
          "                          MP3 output requires valid synthesizer with -s, --sound-font option.\n"
          " -s, --sound-font <file>  Specify sound font file for synthesizer.\n"
          " -h, --help               This help text.\n"
          );
}
