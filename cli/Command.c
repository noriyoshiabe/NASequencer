#include "Command.h"
#include "PianoRollView.h"
#include "NAArray.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>

struct _Command {
    void (*execute)(Command *self, CLI *cli);
    NAArray *argv;
};

typedef struct _CommandTable {
    const char *cmd;
    void (*execute)(Command *, CLI *);
} CommandTable;

static CommandTable commandTable[];

static void CommandDestroy(Command *self);

static void EmptyCommandExecute(Command *self, CLI *cli)
{
}

static void UnknownCommandExecute(Command *self, CLI *cli)
{
    printf("Unknown command: %s\n", NAArrayGetValueAt(self->argv, 0));
}

static void PlayCommandExecute(Command *self, CLI *cli)
{
    Player *player = NAMidiGetPlayer(CLIGetNAMidi(cli));
    PlayerPlay(player);
}

static void StopCommandExecute(Command *self, CLI *cli)
{
    Player *player = NAMidiGetPlayer(CLIGetNAMidi(cli));
    PlayerStop(player);
}

static void RewindCommandExecute(Command *self, CLI *cli)
{
    Player *player = NAMidiGetPlayer(CLIGetNAMidi(cli));
    PlayerRewind(player);
}

static void ForwardCommandExecute(Command *self, CLI *cli)
{
    Player *player = NAMidiGetPlayer(CLIGetNAMidi(cli));
    PlayerForward(player);
}

static void BackwardCommandExecute(Command *self, CLI *cli)
{
    Player *player = NAMidiGetPlayer(CLIGetNAMidi(cli));
    PlayerBackWard(player);
}

static void SeekCommandExecute(Command *self, CLI *cli)
{
    if (2 > NAArrayCount(self->argv)) {
        fprintf(stderr, "measure number is missing.\n");
        return;
    }

    char *err;
    char *text = NAArrayGetValueAt(self->argv, 1);
    long measure = strtol(text, &err, 10);
    if ('\0' != *err) {
        fprintf(stderr, "cannot parse measure number. %s\n", text);
        return;
    }

    if (1 > measure) {
        fprintf(stderr, "invalid measure number. %ld\n", measure);
        return;
    }

    Player *player = NAMidiGetPlayer(CLIGetNAMidi(cli));
    PlayerSeek(player, measure);
}

static void ShowCommandExecute(Command *self, CLI *cli)
{
    char *err, *text;

    int from = -1;
    int length = -1;

    int count = NAArrayCount(self->argv);
    if (1 < count) {
        text = NAArrayGetValueAt(self->argv, 1);
        long number = strtol(text, &err, 10);

        if ('\0' != *err) {
            fprintf(stderr, "cannot parse measure number of from. %s\n", text);
            return;
        }

        if (1 > number) {
            fprintf(stderr, "invalid measure number. %ld\n", number);
            return;
        }

        from = number;
    }

    if (2 < count) {
        text = NAArrayGetValueAt(self->argv, 2);
        long number = strtol(text, &err, 10);

        if ('\0' != *err) {
            fprintf(stderr, "cannot parse length. %s\n", text);
            return;
        }

        if (1 > number) {
            fprintf(stderr, "invalid length. %ld\n", number);
            return;
        }

        length = number;
    }

    PianoRollView *view = CLIGetPianoRollView(cli);
    PianoRollViewSetFrom(view, from);
    PianoRollViewSetLength(view, length);
    PianoRollViewRender(view);
}

static void SynthCommandExecute(Command *self, CLI *cli)
{
    const struct {
        const char *label;
        int width;
    } table[] = {
        {"Idx", 3},
        {"Identifier", 40},
        {"Filepath", 64},
    };

    for (int i = 0; i < sizeof(table)/sizeof(table[0]); ++i) {
        printf("%-*s%s", table[i].width, table[i].label, i == sizeof(table)/sizeof(table[0]) - 1 ? "\n" : " | ");
    }

    for (int i = 0; i < sizeof(table)/sizeof(table[0]); ++i) {
        for (int j = 0; j < table[i].width; ++j) {
            printf("-");
        }

        printf("%s", i == sizeof(table)/sizeof(table[0]) - 1 ? "\n" : "-|-");
    }
    
    MidiSourceManager *manager = MidiSourceManagerSharedInstance();
    NAArray *descriptions = MidiSourceManagerGetAvailableDescriptions(manager);
    int count = NAArrayCount(descriptions);
    MidiSourceDescription **values = NAArrayGetValues(descriptions);
    for (int i = 0; i < count; ++i) {
        MidiSourceDescription *description = values[i];
        for (int j = 0; j < sizeof(table)/sizeof(table[0]); ++j) {
            char text[128];
            int width = table[j].width;
            int limit = width + 1;
            switch (j) {
            case 0:
                snprintf(text, limit, "%*d", width, i);
                break;
            case 1:
                snprintf(text, limit, "%-*s", width, description->name);
                break;
            case 2:
                snprintf(text, limit, "%-*s", width, description->filepath);
                break;
            }

            printf("%s%s", text, j == sizeof(table)/sizeof(table[0]) - 1 ? "\n" : " | ");
        }
    }
}

static void PresetCommandExecute(Command *self, CLI *cli)
{
    MidiSourceManager *manager = MidiSourceManagerSharedInstance();
    NAArray *descriptions = MidiSourceManagerGetAvailableDescriptions(manager);

    int count = NAArrayCount(self->argv);
    if (2 > count) {
        fprintf(stderr, "index of synthesizers is missing.\n");
        return;
    }

    char *err;
    char *text = NAArrayGetValueAt(self->argv, 1);
    long index = strtol(text, &err, 10);

    if ('\0' != *err) {
        fprintf(stderr, "cannot parse index of synthesizers. %s\n", text);
        return;
    }

    if (index < 0 || NAArrayCount(descriptions) <= index) {
        fprintf(stderr, "index of synthesizers is out of range. %ld\n", index);
        return;
    }

    const struct {
        const char *label;
        int width;
    } table[] = {
        {"Name", 20},
        {"MSB", 3},
        {"LSB", 3},
        {"Prg", 3},
    };

    for (int i = 0; i < sizeof(table)/sizeof(table[0]); ++i) {
        printf("%-*s%s", table[i].width, table[i].label, i == sizeof(table)/sizeof(table[0]) - 1 ? "\n" : " | ");
    }

    for (int i = 0; i < sizeof(table)/sizeof(table[0]); ++i) {
        for (int j = 0; j < table[i].width; ++j) {
            printf("-");
        }

        printf("%s", i == sizeof(table)/sizeof(table[0]) - 1 ? "\n" : "-|-");
    }

    MidiSourceDescription *description = NAArrayGetValueAt(descriptions, index);
    MidiSource *source = MidiSourceManagerAllocMidiSource(manager, description, 44100.0);
    int presetCount = source->getPresetCount(source);
    PresetInfo **presetInfos = source->getPresetInfos(source);
    for (int i = 0; i < presetCount; ++i) {
        PresetInfo *info = presetInfos[i];
        for (int j = 0; j < sizeof(table)/sizeof(table[0]); ++j) {
            char text[64];
            int width = table[j].width;
            int limit = width + 1;
            switch (j) {
            case 0:
                snprintf(text, limit, "%-*s", width, info->name);
                break;
            case 1:
                snprintf(text, limit, "%*d", width, (info->bankNo >> 7) & 0x7F);
                break;
            case 2:
                snprintf(text, limit, "%*d", width, info->bankNo & 0x7F);
                break;
            case 3:
                snprintf(text, limit, "%*d", width, info->programNo);
                break;
            }

            printf("%s%s", text, j == sizeof(table)/sizeof(table[0]) - 1 ? "\n" : " | ");
        }
    }

    MidiSourceManagerDeallocMidiSource(manager, source);
}

static void StatusCommandExecute(Command *self, CLI *cli)
{
    const struct {
        const char *label;
        int width;
    } table[] = {
        {"Ch", 2},
        {"Midi Source", 40},
        {"Preset", 20},
        {"Volume", 6},
        {"Pan", 6},
        {"Chorus", 6},
        {"Reverb", 6},
        {"Mute", 4},
        {"Solo", 4},
    };

    for (int i = 0; i < sizeof(table)/sizeof(table[0]); ++i) {
        printf("%-*s%s", table[i].width, table[i].label, i == sizeof(table)/sizeof(table[0]) - 1 ? "\n" : " | ");
    }

    for (int i = 0; i < sizeof(table)/sizeof(table[0]); ++i) {
        for (int j = 0; j < table[i].width; ++j) {
            printf("-");
        }

        printf("%s", i == sizeof(table)/sizeof(table[0]) - 1 ? "\n" : "-|-");
    }

    Mixer *mixer = NAMidiGetMixer(CLIGetNAMidi(cli));
    NAArray *array = MixerGetChannels(mixer);
    int count = NAArrayCount(array);
    MixerChannel **channels = NAArrayGetValues(array);
    for (int i = 0; i < count; ++i) {
        MixerChannel *channel = channels[i];
        for (int j = 0; j < sizeof(table)/sizeof(table[0]); ++j) {
            char text[64];
            int width = table[j].width;
            int limit = width + 1;
            switch (j) {
            case 0:
                snprintf(text, limit, "%*d", width, MixerChannelGetNumber(channel));
                break;
            case 1:
                {
                    MidiSourceDescription *description = MixerChannelGetMidiSourceDescription(channel);
                    snprintf(text, limit, "%-*s", width, description->name);
                }
                break;
            case 2:
                {
                    PresetInfo *preset = MixerChannelGetPresetInfo(channel);
                    snprintf(text, limit, "%-*s", width, preset->name);
                }
                break;
            case 3:
                snprintf(text, limit, "%*d", width, MixerChannelGetVolume(channel));
                break;
            case 4:
                snprintf(text, limit, "%*d", width, MixerChannelGetPan(channel) - 64);
                break;
            case 5:
                snprintf(text, limit, "%*d", width, MixerChannelGetChorusSend(channel));
                break;
            case 6:
                snprintf(text, limit, "%*d", width, MixerChannelGetReverbSend(channel));
                break;
            case 7:
                snprintf(text, limit, "%*s", width, MixerChannelGetMute(channel) ? "ON" : "OFF");
                break;
            case 8:
                snprintf(text, limit, "%*s", width, MixerChannelGetSolo(channel) ? "ON" : "OFF");
                break;
            }

            printf("%s%s", text, j == sizeof(table)/sizeof(table[0]) - 1 ? "\n" : " | ");
        }
    }
}

static void ExitCommandExecute(Command *self, CLI *cli)
{
    free(self);
    CLIExit(cli);
}

static Command *CommandCreate(void (*execute)(Command *, CLI *), NAArray *argv)
{
    Command *self = calloc(1, sizeof(Command));
    self->execute = execute;
    self->argv = argv;
    return self;
}

Command *CommandParse(const char *line)
{
    int length = strlen(line);
    char *str = alloca(length + 1);
    strcpy(str, line);

    NAArray *argv = NAArrayCreate(4, NADescriptionCString);

    char *saveptr, *token, *s = str;
    while ((token = strtok_r(s, " ", &saveptr))) {
        NAArrayAppend(argv, strdup(token));
        s = NULL;
    }

    void (*execute)(Command *, CLI *);

    if (0 < NAArrayCount(argv)) {
        for (int i = 0; NULL != commandTable[i].cmd; ++i) {
            if (0 == strcmp(commandTable[i].cmd, NAArrayGetValueAt(argv, 0))) {
                execute = commandTable[i].execute;
                break;
            }
        }

        if (!execute) {
            execute = UnknownCommandExecute;
        }
    }
    else {
        execute = EmptyCommandExecute;
    }

    return CommandCreate(execute, argv);
}

void CommandExecute(Command *self, CLI *cli)
{
    self->execute(self, cli);
    CommandDestroy(self);
}

static void CommandDestroy(Command *self)
{
    NAArrayTraverse(self->argv, free);
    NAArrayDestroy(self->argv);
    free(self);
}

char *CommandCompletionEntry(const char *text, int state)
{
    static int index, length;

    if (0 == state) {
        index = 0;
        length = strlen(text);
    }

    const char *cmd;
    while ((cmd = commandTable[index].cmd)) {
        ++index;

        if (0 == strncmp(text, cmd, length)) {
            return strdup(cmd);
        }
    }

    return NULL;
}

static CommandTable commandTable[] = {
    {"play", PlayCommandExecute},
    {"stop", StopCommandExecute},
    {"rewind", RewindCommandExecute},
    {"forward", ForwardCommandExecute},
    {"backward", BackwardCommandExecute},
    {"seek", SeekCommandExecute},
    {"show", ShowCommandExecute},
    {"synth", SynthCommandExecute},
    {"preset", PresetCommandExecute},
    {"status", StatusCommandExecute},
    {"exit", ExitCommandExecute},

    {NULL, NULL}
};
