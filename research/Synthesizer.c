#include "Synthesizer.h"
#include "MidiSource.h"
#include "SoundFont.h"
#include "Preset.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_POLYPHONY 64

typedef struct _Voice {
    uint8_t channel;
    uint8_t key;
    uint8_t velocity;

    int64_t startedAt;

    Zone *presetGlobalZone;
    Zone *presetZone;
    Zone *instrumentGlobalZone;
    Zone *instrumentZone;

    SoundFont *sf;

    struct _Voice *next;
    struct _Voice *prev;
} Voice;

struct _Synthesizer {
    MidiSource srcVtbl;

    char *filepath;
    SoundFont *sf;

    Preset **presets;
    int presetCount;

    Voice *voicePool;
    Voice *voiceList;
    Voice *voiceFreeList;
};

static Preset *SynthesizerFindPreset(Synthesizer *self, uint16_t midiPresetNo, uint16_t bankNo);
static int PresetComparator(const void *preset1, const void *preset2);
static int SynthesizerNoteOn(Synthesizer *self, uint8_t channel, uint8_t noteNo, uint8_t velocity);

static void SynthesizerInitializeVoiceFreeList(Synthesizer *self);
static Voice *SynthesizerVoiceAlloc(Synthesizer *self);
static void SynthesizerVoiceDealloc(Synthesizer *self, Voice *voice);
static void VoiceDump(Voice *voice);
static void SynthesizerVoiceListDump(Synthesizer *self);

static uint32_t VoiceSampleStart(Voice *self);
static int16_t VoiceGeneratorShortValue(Voice *self, SFGeneratorType generatorType);

static void send(void *self, uint8_t *bytes, size_t length)
{
}

static bool isAvailable(void *self)
{
    return true;
}

static bool hasProperty(void *self, const char *name)
{
    return false;
}

static void setProperty(void *self, const char *name, const void *value)
{
}

static void getProperty(void *self, const char *name, void *value)
{
}

Synthesizer *SynthesizerCreate(const char *filepath)
{
    Synthesizer *self = calloc(1, sizeof(Synthesizer));

    self->srcVtbl.send = send;
    self->srcVtbl.isAvailable = isAvailable;
    self->srcVtbl.hasProperty = hasProperty;
    self->srcVtbl.setProperty = setProperty;
    self->srcVtbl.getProperty = getProperty;

    self->filepath = strdup(filepath);
    self->sf = SoundFontRead(filepath, NULL);

    SynthesizerInitializeVoiceFreeList(self);

    ParsePresets(self->sf, &self->presets, &self->presetCount);
    qsort(self->presets, self->presetCount, sizeof(Preset *), PresetComparator);

#if 0
    for (int i = 0; i < self->presetCount; ++i) {
        PresetDump(self->presets[i]);
    }
#endif

#if 0
    Preset *preset = SynthesizerFindPreset(self, 25, 128);
    //Preset *preset = self->presets[0];
    PresetDump(preset);

    for (int i = 0; i < preset->zoneCount; ++i) {
        InstrumentDump(preset->zones[i]->instrument);
    }

    for (int i = 0; i < preset->zoneCount; ++i) {
        for (int j = 0; j < preset->zones[i]->instrument->zoneCount; ++j) {
            SampleDump(preset->zones[i]->instrument->zones[j]->sample);
        }
    }
#endif

#if 1
    SynthesizerNoteOn(self, 0, 64, 127);
    SynthesizerVoiceListDump(self);
#endif

    return self;
}

void SynthesizerDestroy(Synthesizer *self)
{
    for (int i = 0; i < self->presetCount; ++i) {
        PresetDestroy(self->presets[i]);
    }

    if (self->presets) {
        free(self->presets);
    }

    free(self->voicePool);

    SoundFontDestroy(self->sf);

    free(self->filepath);
    free(self);
}

static Preset *SynthesizerFindPreset(Synthesizer *self, uint16_t midiPresetNo, uint16_t bankNo)
{
    Preset preset = { .midiPresetNo = midiPresetNo, .bankNo = bankNo };
    Preset *key = &preset;
    Preset **result = bsearch(&key, self->presets, self->presetCount, sizeof(Preset *), PresetComparator);
    return result ? *result : NULL;
}

static int PresetComparator(const void *_preset1, const void *_preset2)
{
    const Preset **preset1 = (const Preset **)_preset1;
    const Preset **preset2 = (const Preset **)_preset2;

    return (((*preset1)->bankNo << 16) | (*preset1)->midiPresetNo) - (((*preset2)->bankNo << 16) | (*preset2)->midiPresetNo);
}

static int SynthesizerNoteOn(Synthesizer *self, uint8_t channel, uint8_t noteNo, uint8_t velocity)
{
    int voicedCount = 0;

    Preset *preset = self->presets[0]; // TODO from channel map
    
    for (int i = 0; i < preset->zoneCount; ++i) {
        Zone *presetZone = preset->zones[i];

        if (!ZoneIsInsideRange(presetZone, noteNo, velocity)) {
            continue;
        }

        Instrument *instrument = presetZone->instrument;
        for (int j = 0; j < instrument->zoneCount; ++j) {
            Zone *instrumentZone = instrument->zones[j];

            if (!ZoneIsInsideRange(instrumentZone, noteNo, velocity)) {
                continue;
            }

            Voice *voice = SynthesizerVoiceAlloc(self);
            if (!voice) {
                goto END;
            }

            voice->channel = channel;
            voice->key = noteNo;
            voice->velocity = velocity;

            voice->presetGlobalZone = preset->globalZone;
            voice->presetZone = presetZone;
            voice->instrumentGlobalZone = instrument->globalZone;
            voice->instrumentZone = instrumentZone;

            voice->sf = self->sf;
            
            // TODO record current time to startedAt

            ++voicedCount;
        }
    }

END:
    return voicedCount;
}

static void SynthesizerInitializeVoiceFreeList(Synthesizer *self)
{
    self->voicePool = calloc(MAX_POLYPHONY, sizeof(Voice));
    self->voiceFreeList = &self->voicePool[0];

    Voice *prev = NULL;

    for (int i = 0; i < MAX_POLYPHONY; ++i) {
        if (prev) {
            prev->next = &self->voicePool[i];
        }

        self->voicePool[i].prev = prev;
        prev = &self->voicePool[i];
    }

    self->voicePool[MAX_POLYPHONY - 1].next = NULL;
}

static Voice *SynthesizerVoiceAlloc(Synthesizer *self)
{
    if (!self->voiceFreeList) {
        return NULL;
    }

    Voice *ret = self->voiceFreeList;
    self->voiceFreeList = ret->next;

    memset(ret, 0, sizeof(Voice));

    if (self->voiceList) {
        self->voiceList->prev = ret;
    }

    ret->next = self->voiceList;
    ret->prev = NULL;
    self->voiceList = ret;

    return ret;
}

static void SynthesizerVoiceDealloc(Synthesizer *self, Voice *voice)
{
    if (voice->prev) {
        voice->prev->next = voice->next;
    }

    if (voice->next) {
        voice->next->prev = voice->prev;
    }

    voice->prev = NULL;
    voice->next = self->voiceFreeList;
    self->voiceFreeList = voice;
}

static uint32_t VoiceSampleStart(Voice *self)
{
    uint32_t ret = self->instrumentZone->sample->start;
    ret += VoiceGeneratorShortValue(self, SFGeneratorType_startAddrsOffset);
    ret += VoiceGeneratorShortValue(self, SFGeneratorType_startAddrsCoarseOffset) << 15; // * 32768
    return ret;
}

static int16_t VoiceGeneratorShortValue(Voice *self, SFGeneratorType generatorType)
{
    // 9.4 The SoundFont Generator Model
    // - A generator in a global instrument zone that is identical to a default generator
    //   supersedes or replaces the default generator.
    // - A generator in a local instrument zone that is identical to a default generator
    //   or to a generator in a global instrument zone supersedes or replaces that generator.

    int16_t value =
        ZoneHasGenerator(self->instrumentZone, generatorType) ? self->instrumentZone->gen.array[generatorType] :
        ZoneHasGenerator(self->instrumentGlobalZone, generatorType) ? self->instrumentGlobalZone->gen.array[generatorType] :
        GeneratorDefaultValue(generatorType);

    // 8.5 Precedence and Absolute and Relative values.
    // There are some generators that are not available at the Preset Level. These are:
    //   # Name
    //   0 startAddrsOffset
    //   1 endAddrsOffset
    //   2 startloopAddrsOffset
    //   3 endloopAddrsOffset
    //   4 startAddrsCoarseOffset
    //   12 endAddrsCoarseOffset
    //   45 startloopAddrsCoarseOffset
    //   46 keynum
    //   47 velocity
    //   50 endloopAddrsCoarseOffset 54 sampleModes
    //   57 exclusiveClass
    //   58 overridingRootKey
    // If these generators are encountered in the Preset Level, they should be ignored.
    
    if (!GeneratorIsInstrumentOnly(generatorType)) {

        // 9.4 The SoundFont Generator Model
        // - A generator at the preset level adds to a generator at the instrument level
        //   if both generators are identical.
        // - A generator in a global preset zone that is identical to a default generator
        //   or to a generator in an instrument adds to that generator.
        // - A generator in a local preset zone that is identical to a generator in a global preset zone
        //   supersedes or replaces that generator in the global preset zone.
        //   That generator then has its effects added to the destination-summing node of all zones in the given instrument.
        value += 
            ZoneHasGenerator(self->presetZone, generatorType) ? self->presetZone->gen.array[generatorType] :
            ZoneHasGenerator(self->presetGlobalZone, generatorType) ? self->presetGlobalZone->gen.array[generatorType] :
            0;
    }

    // What does 'not identical' mean?  excerpted 9.4 The SoundFont Generator Model
    // - A generator in a global preset zone which is not identical to a default generator
    //   and is not identical to a generator in an instrument has its effect added to the given synthesis parameter.
    // - A generator in a local preset zone which is not identical to a default generator
    //   or a generator in a global preset zone has its effects added to the destination summing node of all zones
    //   in the given instrument.

    return value;
}

static void VoiceDump(Voice *voice)
{
    printf("[Voice]\n");
    printf("-----------------------------------------\n");
    printf("channel: %d\n", voice->channel);
    printf("key: %d\n", voice->key);
    printf("velocity: %d\n", voice->velocity);
    printf("startedAt: %lld\n", voice->startedAt);

    if (voice->presetGlobalZone) {
        printf("presetGlobalZone:\n");
        ZoneDump(voice->presetGlobalZone);
    }

    printf("presetZone:\n");
    ZoneDump(voice->presetZone);

    if (voice->instrumentGlobalZone) {
        printf("instrumentGlobalZone:\n");
        ZoneDump(voice->instrumentGlobalZone);
    }

    printf("instrumentZone:\n");
    ZoneDump(voice->instrumentZone);

    printf("\n");

    SampleDump(voice->instrumentZone->sample);
    printf("start: %d\n", VoiceSampleStart(voice));

    printf("test attackVolEnv: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_attackVolEnv));

    printf("\n");
}

static void SynthesizerVoiceListDump(Synthesizer *self)
{
    for (Voice *voice = self->voiceList; NULL != voice; voice = voice->next) {
        VoiceDump(voice);
    }
}
