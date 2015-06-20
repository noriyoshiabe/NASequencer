#include "Voice.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <sys/param.h>

#define Timecent2Sec(tc) (pow(2.0, (double)tc / 1200.0))
#define ConvexPositiveUnipolar(x) (sqrt(1.0 - pow((double)x - 1.0, 2.0)))

static void VoiceUpdateCachedParams(Voice *self);
static void VoiceUpdateVolEnv(Voice *self, float nextTime);
static uint32_t VoiceSampleStart(Voice *self);
static uint32_t VoiceSampleEnd(Voice *self);
static uint32_t VoiceSampleStartLoop(Voice *self);
static uint32_t VoiceSampleEndLoop(Voice *self);
static int16_t VoiceGeneratorShortValue(Voice *self, SFGeneratorType generatorType);

extern void VoiceInitialize(Voice *self, uint8_t channel, uint8_t noteNo, uint8_t velocity,
        Zone *presetGlobalZone, Zone *presetZone, Zone *instrumentGlobalZone, Zone *instrumentZone,
        SoundFont *sf, float sampleRate)
{
    self->channel = channel;
    self->key = noteNo;
    self->velocity = velocity;

    self->presetGlobalZone = presetGlobalZone;
    self->presetZone = presetZone;
    self->instrumentGlobalZone = instrumentGlobalZone;
    self->instrumentZone = instrumentZone;

    self->sf = sf;
    self->sampleRate = sampleRate;

    self->tick = 0;
    self->time = 0.0f;
    self->sampleIndex = VoiceSampleStart(self);

    self->phase = VolEnvPhaseDelay;
    self->startPhaseTime = 0.0f;
    self->volEnv = 0.0f;
    self->releasedVolEnv = 0.0f;

    int16_t v;
    Sample *sample = self->instrumentZone->sample;

    int16_t key = 0 <= (v = VoiceGeneratorShortValue(self, SFGeneratorType_keynum)) ? v : self->key;
    int16_t originalPitch = 0 <= (v = VoiceGeneratorShortValue(self, SFGeneratorType_overridingRootKey)) ? v : sample->originalPitch;

    self->sampleIncrement = (float)sample->sampleRate / self->sampleRate;
    self->sampleIncrement *= pow(2.0, (float)(key - originalPitch) / 12.0);

    // TODO
    // coarseTune
    // fineTune
    // scaleTuning
    

    self->sampleModes = VoiceGeneratorShortValue(self, SFGeneratorType_sampleModes);

    VoiceUpdateCachedParams(self);
}

static void VoiceUpdateCachedParams(Voice *self)
{
    self->cache.volEnvValues[VolEnvPhaseDelay] = Timecent2Sec(VoiceGeneratorShortValue(self, SFGeneratorType_delayVolEnv));
    self->cache.volEnvValues[VolEnvPhaseAttack] = Timecent2Sec(VoiceGeneratorShortValue(self, SFGeneratorType_attackVolEnv));
    self->cache.volEnvValues[VolEnvPhaseHold] = Timecent2Sec(VoiceGeneratorShortValue(self, SFGeneratorType_holdVolEnv));
    self->cache.volEnvValues[VolEnvPhaseDecay] = Timecent2Sec(VoiceGeneratorShortValue(self, SFGeneratorType_decayVolEnv));
    self->cache.volEnvValues[VolEnvPhaseRelase] = Timecent2Sec(VoiceGeneratorShortValue(self, SFGeneratorType_releaseVolEnv));

    self->cache.volEnvValues[VolEnvPhaseSustain] = 1.0f - 0.001f * (float)VoiceGeneratorShortValue(self, SFGeneratorType_sustainVolEnv);

    self->cache.sampleStartLoop = (float)VoiceSampleStartLoop(self);
    self->cache.sampleEndLoop = (float)VoiceSampleEndLoop(self);
    self->cache.sampleEnd = (float)VoiceSampleEnd(self);
}

void VoiceUpdate(Voice *self)
{
    float nextTime = (float)++self->tick / self->sampleRate;
    VoiceUpdateVolEnv(self, nextTime);
    self->time = nextTime;
}

static void VoiceUpdateVolEnv(Voice *self, float nextTime)
{
    // TODO
    // keynumToVolEnvHold
    // keynumToVolEnvDecay

    double duration = self->cache.volEnvValues[self->phase];
    double endTime = duration + self->startPhaseTime;

    double sustainLevel = self->cache.volEnvValues[self->phase];
    double f;

    switch (self->phase) {
    case VolEnvPhaseDelay:
        self->volEnv = 0.0f;
        break;
    case VolEnvPhaseAttack:
        self->volEnv = ConvexPositiveUnipolar((self->time - self->startPhaseTime) / duration);
        break;
    case VolEnvPhaseHold:
        self->volEnv = 1.0f;
        break;
    case VolEnvPhaseDecay:
        f = (self->time - self->startPhaseTime) / duration;
        self->volEnv = 0.0 < duration
            ? MAX(0.0f, 1.0f * (1.0f - f) + f * sustainLevel)
            : sustainLevel;
        break;
    case VolEnvPhaseSustain:
        self->volEnv = sustainLevel;
        break;
    case VolEnvPhaseRelase:
        self->volEnv = 0.0 < duration
            ? MAX(0.0f, ((endTime - self->time) * self->releasedVolEnv) / duration)
            : 0.0f;
        break;
    default:
        break;
    }

    switch (self->phase) {
    case VolEnvPhaseDelay:
    case VolEnvPhaseAttack:
    case VolEnvPhaseHold:
    case VolEnvPhaseDecay:
        if (nextTime >= endTime) {
            ++self->phase;
            self->startPhaseTime = nextTime;
        }
        break;
    case VolEnvPhaseSustain:
    case VolEnvPhaseRelase:
    default:
        break;
    }
}

extern AudioSample VoiceComputeSample(Voice *self)
{
    int index = floor(self->sampleIndex);
    float over = self->sampleIndex - (float)index;
    int32_t indexSample = (self->sf->smpl[index] << 8) + (self->sf->sm24 ? self->sf->sm24[index] : 0);
    int32_t nextSample = (self->sf->smpl[index + 1] << 8) + (self->sf->sm24 ? self->sf->sm24[index + 1] : 0);

    float normalized = ((float)indexSample * (1.0f - over) + (float)nextSample * over) / (float)(SHRT_MAX << 8);

    // TODO pan
    // TODO velocity

    AudioSample ret;
    ret.L = normalized * self->volEnv;
    ret.R = normalized * self->volEnv;
    
    return ret;
}

void VoiceIncrementSample(Voice *self)
{
    self->sampleIndex += self->sampleIncrement;

    if (self->sampleModes & 0x01) {
        if (self->sampleModes & 0x02 && self->phase == VolEnvPhaseRelase) {
            ;
        }
        else {
            if (self->cache.sampleEndLoop < self->sampleIndex) {
                float over = self->sampleIndex - self->cache.sampleEndLoop;
                self->sampleIndex = self->cache.sampleStartLoop + over;
            }
        }
    }
}

void VoiceRelease(Voice *self)
{
    self->phase = VolEnvPhaseRelase;
    self->startPhaseTime = self->time;
    self->releasedVolEnv = self->volEnv;
}

bool VoiceIsReleased(Voice *self)
{
    return (self->phase == VolEnvPhaseRelase && 0.0f >= self->volEnv)
        || self->cache.sampleEnd < self->sampleIndex;
}

static uint32_t VoiceSampleStart(Voice *self)
{
    uint32_t ret = self->instrumentZone->sample->start;
    ret += VoiceGeneratorShortValue(self, SFGeneratorType_startAddrsOffset);
    ret += VoiceGeneratorShortValue(self, SFGeneratorType_startAddrsCoarseOffset) << 15; // * 32768
    return ret;
}

static uint32_t VoiceSampleEnd(Voice *self)
{
    uint32_t ret = self->instrumentZone->sample->end;
    ret += VoiceGeneratorShortValue(self, SFGeneratorType_endAddrsOffset);
    ret += VoiceGeneratorShortValue(self, SFGeneratorType_endAddrsCoarseOffset) << 15; // * 32768
    return ret;
}

static uint32_t VoiceSampleStartLoop(Voice *self)
{
    uint32_t ret = self->instrumentZone->sample->startLoop;
    ret += VoiceGeneratorShortValue(self, SFGeneratorType_startloopAddrsOffset);
    ret += VoiceGeneratorShortValue(self, SFGeneratorType_startloopAddrsCoarseOffset) << 15; // * 32768
    return ret;
}

static uint32_t VoiceSampleEndLoop(Voice *self)
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
    printf("tick: %d\n", voice->tick);
    printf("time: %f\n", voice->time);
    printf("sampleIncrement: %f\n", voice->sampleIncrement);
    printf("sampleIndex: %f\n", voice->sampleIndex);
    printf("phase: %d\n", voice->phase);
    printf("startPhaseTime: %f\n", voice->startPhaseTime);
    printf("volEnv: %f\n", voice->volEnv);
    printf("releasedVolEnv: %f\n", voice->releasedVolEnv);

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
    printf("  delayVolEnv: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_delayVolEnv));
    printf("  attackVolEnv: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_attackVolEnv));
    printf("  holdVolEnv: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_holdVolEnv));
    printf("  decayVolEnv: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_decayVolEnv));
    printf("  sustainVolEnv: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_sustainVolEnv));
    printf("  releaseVolEnv: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_releaseVolEnv));

    printf("\n");
}
