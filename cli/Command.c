#include "Command.h"
#include "PianoRollView.h"
#include "NAArray.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>
#include <sys/param.h>

struct _Command {
    void (*execute)(Command *self, CLI *cli);
    NAArray *argv;
};

typedef struct _CommandTable {
    const char *cmd;
    void (*execute)(Command *, CLI *);
    const char *synopsis;
    const char *description;
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

static void SourceCommandExecute(Command *self, CLI *cli)
{
    if (2 > NAArrayCount(self->argv)) {
        fprintf(stderr, "filepath is missing.\n");
        return;
    }

    const char *filepath = NAArrayGetValueAt(self->argv, 1);

    NAMidi *namidi = CLIGetNAMidi(cli);
    NAMidiSetWatchEnable(namidi, true);
    NAMidiParse(namidi, filepath);

    CLISetFilepath(cli, filepath);
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

static bool ParseViewArgs(NAArray *argv, int *channel, int *from, int *length)
{
    char *err, *text;

    *channel = -1;
    *from = -1;
    *length = -1;

    int count = NAArrayCount(argv);
    if (1 < count) {
        text = NAArrayGetValueAt(argv, 1);
        long number = strtol(text, &err, 10);

        if ('\0' == *err && 1 <= number && number <= 16) {
            *channel = number;
        }
    }

    if (2 < count) {
        text = NAArrayGetValueAt(argv, 2);
        long number = strtol(text, &err, 10);

        if ('\0' != *err) {
            fprintf(stderr, "cannot parse measure number of from. %s\n", text);
            return false;
        }

        if (1 > number) {
            fprintf(stderr, "invalid measure number. %ld\n", number);
            return false;
        }

        *from = number;
    }

    if (3 < count) {
        text = NAArrayGetValueAt(argv, 3);
        long number = strtol(text, &err, 10);

        if ('\0' != *err) {
            fprintf(stderr, "cannot parse length. %s\n", text);
            return false;
        }

        if (1 > number) {
            fprintf(stderr, "invalid length. %ld\n", number);
            return false;
        }

        *length = number;
    }

    return true;
}

static void ShowCommandExecute(Command *self, CLI *cli)
{
    int channel, from, length;

    if (!ParseViewArgs(self->argv, &channel, &from, &length)) {
        return;
    }

    PianoRollView *view = CLIGetPianoRollView(cli);
    PianoRollViewSetChannel(view, channel);
    PianoRollViewSetFrom(view, from);
    PianoRollViewSetLength(view, length);
    PianoRollViewRender(view);

    CLISetActiveView(cli, view);
}

static void StepCommandExecute(Command *self, CLI *cli)
{
    char *err, *text;

    int step = 120;

    int count = NAArrayCount(self->argv);
    if (1 < count) {
        text = NAArrayGetValueAt(self->argv, 1);
        long number = strtol(text, &err, 10);

        if ('\0' == *err && 0 < number) {
            step = number;
        }
    }

    PianoRollView *view = CLIGetPianoRollView(cli);
    PianoRollViewSetStep(view, step);
    PianoRollViewRender(view);

    CLISetActiveView(cli, view);
}

static void ListCommandExecute(Command *self, CLI *cli)
{
    int channel, from, length;

    if (!ParseViewArgs(self->argv, &channel, &from, &length)) {
        return;
    }

    EventListView *view = CLIGetEventListView(cli);
    EventListViewSetChannel(view, channel);
    EventListViewSetFrom(view, from);
    EventListViewSetLength(view, length);
    EventListViewRender(view);

    CLISetActiveView(cli, view);
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
    NAIterator *iterator = NAArrayGetIterator(array);
    while (iterator->hasNext(iterator)) {
        MixerChannel *channel = iterator->next(iterator);
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

static void MuteCommandExecute(Command *self, CLI *cli)
{
    char *err, *text;

    int channel = -1;

    int count = NAArrayCount(self->argv);
    if (1 < count) {
        text = NAArrayGetValueAt(self->argv, 1);
        long number = strtol(text, &err, 10);

        if ('\0' != *err) {
            fprintf(stderr, "cannot parse channel. %s\n", text);
            return;
        }

        if (number < 1 || 16 < number) {
            fprintf(stderr, "channel is out of range [1-16]. %s\n", text);
            return;
        }

        channel = number;
    }

    Mixer *mixer = NAMidiGetMixer(CLIGetNAMidi(cli));
    NAArray *array = MixerGetChannels(mixer);

    if (-1 != channel) {
        MixerChannel *mixerChannel = NAArrayGetValueAt(array, channel - 1);
        bool mute = MixerChannelGetMute(mixerChannel);
        MixerChannelSetMute(mixerChannel, !mute);
    }
    else {
        NAIterator *iterator = NAArrayGetIterator(array);
        while (iterator->hasNext(iterator)) {
            MixerChannelSetMute(iterator->next(iterator), false);
        }
    }

    StatusCommandExecute(self, cli);
}

static void SoloCommandExecute(Command *self, CLI *cli)
{
    char *err, *text;

    int channel = -1;

    int count = NAArrayCount(self->argv);
    if (1 < count) {
        text = NAArrayGetValueAt(self->argv, 1);
        long number = strtol(text, &err, 10);

        if ('\0' != *err) {
            fprintf(stderr, "cannot parse channel. %s\n", text);
            return;
        }

        if (number < 1 || 16 < number) {
            fprintf(stderr, "channel is out of range [1-16]. %s\n", text);
            return;
        }

        channel = number;
    }

    Mixer *mixer = NAMidiGetMixer(CLIGetNAMidi(cli));
    NAArray *array = MixerGetChannels(mixer);

    if (-1 != channel) {
        MixerChannel *mixerChannel = NAArrayGetValueAt(array, channel - 1);
        bool solo = MixerChannelGetSolo(mixerChannel);
        MixerChannelSetSolo(mixerChannel, !solo);
    }
    else {
        NAIterator *iterator = NAArrayGetIterator(array);
        while (iterator->hasNext(iterator)) {
            MixerChannelSetSolo(iterator->next(iterator), false);
        }
    }

    StatusCommandExecute(self, cli);
}

static void LoadCommandExecute(Command *self, CLI *cli)
{
    if (2 > NAArrayCount(self->argv)) {
        fprintf(stderr, "sound source filepath is missing.\n");
        return;
    }

    MidiSourceManager *manager = MidiSourceManagerSharedInstance();
    if (!MidiSourceManagerLoadMidiSourceDescriptionFromSoundFont(manager, NAArrayGetValueAt(self->argv, 1))) {
        fprintf(stderr, "load sound source failed.\n");
    }
}

static void UnloadCommandExecute(Command *self, CLI *cli)
{
    if (2 > NAArrayCount(self->argv)) {
        fprintf(stderr, "index of synthesizers is missing.\n");
        return;
    }

    MidiSourceManager *manager = MidiSourceManagerSharedInstance();
    NAArray *descriptions = MidiSourceManagerGetAvailableDescriptions(manager);

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

    MidiSourceManagerUnloadMidiSourceDescription(manager, NAArrayGetValueAt(descriptions, index));
}

static void ExportCommandExecute(Command *self, CLI *cli)
{
    if (2 > NAArrayCount(self->argv)) {
        fprintf(stderr, "output filepath is missing.\n");
        return;
    }

    CLIExport(cli, NAArrayGetValueAt(self->argv, 1));
}

static void HelpCommandExecute(Command *self, CLI *cli)
{
    printf("\n");

    int maxSynopsisWidth = 0;
    for (int i = 0; commandTable[i].cmd; ++i) {
        maxSynopsisWidth = MAX(maxSynopsisWidth, strlen(commandTable[i].synopsis));
    }

    for (int i = 0; commandTable[i].cmd; ++i) {
        printf("%s", commandTable[i].synopsis);

        char *description = strdup(commandTable[i].description);
        char *saveptr, *token, *s = description;
        while ((token = strtok_r(s, "\n", &saveptr))) {
            if (s) {
                printf("%*s  - %s\n", maxSynopsisWidth - (int)strlen(commandTable[i].synopsis), "", token);
            }
            else {
                printf("%*s    %s\n", maxSynopsisWidth, "", token);
            }
            s = NULL;
        }
        free(description);
    }

    printf("\n");
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

    char *c;
    while ((c = strstr(str, "\\ "))) {
        memmove(c + 1, c + 2, strlen(c + 2) + 1);
    }

    char *saveptr, *token, *s = str;
    while ((token = strtok_r(s, " ", &saveptr))) {
        for (int i = 0; token[i]; ++i) {
            if (token[i] == '\\') {
                token[i] = ' ';
            }
        }
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
    {"source", SourceCommandExecute, "source <file>", "parse DSL source and build new sequence."},
    {"play", PlayCommandExecute, "play", "play sequence."},
    {"stop", StopCommandExecute, "stop", "stop playing."},
    {"rewind", RewindCommandExecute, "rewind", "rewind playing position."},
    {"forward", ForwardCommandExecute, "forward", "forward playing position to next measure."},
    {"backward", BackwardCommandExecute, "backward", "forward playing position to previous measure or head of current measue."},
    {"seek", SeekCommandExecute, "seek <measure number>", "seek playing position to measure number."},
    {"show", ShowCommandExecute, "show [from [length]]", "show piano roll.\nstart with the measure if 'from' is specifid.\nlimit displaying measures if 'length' is specifid."},
    {"step", StepCommandExecute, "step <step per column>", "adjust length of console column for piano roll."},
    {"list", ListCommandExecute, "list [from [length]]", "show event list.\nstart with the measure if 'from' is specifid.\nlimit displaying measures if 'length' is specifid."},
    {"synth", SynthCommandExecute, "synth", "list synthesizers loaded currently."},
    {"preset", PresetCommandExecute, "preset <index>", "list presets of synthesizer specifid by index of synthesizers list."},
    {"status", StatusCommandExecute, "status", "show channel statuses."},
    {"mute", MuteCommandExecute, "mute [channel]", "enable channel's mute.\ndisable all channel's mute unless channel number is specifid."},
    {"solo", SoloCommandExecute, "solo [channel]", "enable channel's solo.\ndisable all channel's solo unless channel number is specifid."},
    {"load", LoadCommandExecute, "load <file>", "load synthesizer.\nsupported file type is currently .sf2 only."},
    {"unload", UnloadCommandExecute, "unload <index>", "unload synthesizer specifid by index of synthesizers list."},
    {"export", ExportCommandExecute, "export <file>", "export sequence.\nsupported file types are currently .smf, .mid, .midi, .wav, .wave and .m4a."},
    {"help", HelpCommandExecute, "help", "display this help."},
    {"exit", ExitCommandExecute, "exit", "exit namidi."},

    {NULL, NULL, NULL, NULL}
};
