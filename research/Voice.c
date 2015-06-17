#include "Voice.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int16_t VoiceGeneratorShortValue(Voice *self, SFGeneratorType generatorType);

uint32_t VoiceSampleStart(Voice *self)
{
    uint32_t ret = self->instrumentZone->sample->start;
    ret += VoiceGeneratorShortValue(self, SFGeneratorType_startAddrsOffset);
    ret += VoiceGeneratorShortValue(self, SFGeneratorType_startAddrsCoarseOffset) << 15; // * 32768
    return ret;
}

uint32_t VoiceSampleEnd(Voice *self)
{
    uint32_t ret = self->instrumentZone->sample->end;
    ret += VoiceGeneratorShortValue(self, SFGeneratorType_endAddrsOffset);
    ret += VoiceGeneratorShortValue(self, SFGeneratorType_endAddrsCoarseOffset) << 15; // * 32768
    return ret;
}

uint32_t VoiceSampleStartLoop(Voice *self)
{
    uint32_t ret = self->instrumentZone->sample->startLoop;
    ret += VoiceGeneratorShortValue(self, SFGeneratorType_startloopAddrsOffset);
    ret += VoiceGeneratorShortValue(self, SFGeneratorType_startloopAddrsCoarseOffset) << 15; // * 32768
    return ret;
}

uint32_t VoiceSampleEndLoop(Voice *self)
{
    uint32_t ret = self->instrumentZone->sample->endLoop;
    ret += VoiceGeneratorShortValue(self, SFGeneratorType_endloopAddrsOffset);
    ret += VoiceGeneratorShortValue(self, SFGeneratorType_endloopAddrsCoarseOffset) << 15; // * 32768
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
        self->instrumentZone && ZoneHasGenerator(self->instrumentZone, generatorType) ? self->instrumentZone->gen.array[generatorType] :
        self->instrumentGlobalZone && ZoneHasGenerator(self->instrumentGlobalZone, generatorType) ? self->instrumentGlobalZone->gen.array[generatorType] :
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
            self->presetZone && ZoneHasGenerator(self->presetZone, generatorType) ? self->presetZone->gen.array[generatorType] :
            self->presetGlobalZone && ZoneHasGenerator(self->presetGlobalZone, generatorType) ? self->presetGlobalZone->gen.array[generatorType] :
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

VoicePool *VoicePoolCreate()
{
    VoicePool *self = calloc(1, sizeof(VoicePool));

    self->buffer = calloc(MAX_POLYPHONY, sizeof(Voice));
    self->freeList = &self->buffer[0];

    Voice *prev = NULL;

    for (int i = 0; i < MAX_POLYPHONY; ++i) {
        if (prev) {
            prev->next = &self->buffer[i];
        }

        self->buffer[i].prev = prev;
        prev = &self->buffer[i];
    }

    self->buffer[MAX_POLYPHONY - 1].next = NULL;

    return self;
}

void VoicePoolDestroy(VoicePool *self)
{
    free(self->buffer);
    free(self);
}

Voice* VoicePoolAllocVoice(VoicePool *self)
{
    if (!self->freeList) {
        return NULL;
    }

    Voice *ret = self->freeList;
    self->freeList = ret->next;

    memset(ret, 0, sizeof(Voice));
    return ret;
}

void VoicePoolDeallocVoice(VoicePool *self, Voice *voice)
{
    voice->prev = NULL;
    voice->next = self->freeList;
    self->freeList = voice;
}

void VoiceDump(Voice *voice)
{
    printf("[Voice]\n");
    printf("-----------------------------------------\n");
    printf("channel: %d\n", voice->channel);
    printf("key: %d\n", voice->key);
    printf("velocity: %d\n", voice->velocity);
    printf("startTick: %d\n", voice->startTick);

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

    printf("generatorValue:\n");
    printf("  start: %d\n", VoiceSampleStart(voice));
    printf("  end: %d\n", VoiceSampleEnd(voice));
    printf("  startLoop: %d\n", VoiceSampleStartLoop(voice));
    printf("  endLoop: %d\n", VoiceSampleEndLoop(voice));
    printf("  attackVolEnv: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_attackVolEnv));

    printf("\n");
}
