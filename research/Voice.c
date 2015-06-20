#include "Voice.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <sys/param.h>

#define Timecent2Sec(tc) (pow(2.0, (double)tc / 1200.0))
#define ConvexPositiveUnipolar(x) (sqrt(1.0 - pow((double)x - 1.0, 2.0)))
#define Clip(v, min, max) (MIN(v, MAX(v, min)))

static void VoiceUpdateCachedParams(Voice *self);
static void VoiceUpdateVolEnv(Voice *self, float nextTime);
static uint32_t VoiceSampleStart(Voice *self);
static uint32_t VoiceSampleEnd(Voice *self);
static uint32_t VoiceSampleStartLoop(Voice *self);
static uint32_t VoiceSampleEndLoop(Voice *self);
static int16_t VoiceGeneratorShortValue(Voice *self, SFGeneratorType generatorType);

static void VoiceUpdateVolEnvDelayPhase(Voice *self, float nextTime);
static void VoiceUpdateVolEnvAttackPhase(Voice *self, float nextTime);
static void VoiceUpdateVolEnvHoldPhase(Voice *self, float nextTime);
static void VoiceUpdateVolEnvDecayPhase(Voice *self, float nextTime);
static void VoiceUpdateVolEnvSustainPhase(Voice *self, float nextTime);
static void VoiceUpdateVolEnvReleasePhase(Voice *self, float nextTime);

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

    self->keyForSample = 0 <= (v = VoiceGeneratorShortValue(self, SFGeneratorType_keynum)) ? v : self->key;
    float originalPitch = 0 <= (v = VoiceGeneratorShortValue(self, SFGeneratorType_overridingRootKey)) ? v : sample->originalPitch;

    self->sampleIncrement = (float)sample->sampleRate / self->sampleRate;
    self->sampleIncrement *= pow(2.0, (self->keyForSample - originalPitch) / 12.0);

    // TODO
    // coarseTune
    // fineTune
    // scaleTuning
    

    self->sampleModes = VoiceGeneratorShortValue(self, SFGeneratorType_sampleModes);

    VoiceUpdateCachedParams(self);
}

static void VoiceUpdateCachedParams(Voice *self)
{
    self->cache.delayVolEnv = VoiceGeneratorShortValue(self, SFGeneratorType_delayVolEnv);
    self->cache.attackVolEnv = VoiceGeneratorShortValue(self, SFGeneratorType_attackVolEnv);
    self->cache.holdVolEnv = VoiceGeneratorShortValue(self, SFGeneratorType_holdVolEnv);
    self->cache.decayVolEnv = VoiceGeneratorShortValue(self, SFGeneratorType_decayVolEnv);
    self->cache.sustainVolEnv = VoiceGeneratorShortValue(self, SFGeneratorType_sustainVolEnv);
    self->cache.releaseVolEnv = VoiceGeneratorShortValue(self, SFGeneratorType_releaseVolEnv);
    self->cache.keynumToVolEnvHold = VoiceGeneratorShortValue(self, SFGeneratorType_keynumToVolEnvHold);
    self->cache.keynumToVolEnvDecay = VoiceGeneratorShortValue(self, SFGeneratorType_keynumToVolEnvDecay);

    self->cache.sampleStartLoop = VoiceSampleStartLoop(self);
    self->cache.sampleEndLoop = VoiceSampleEndLoop(self);
    self->cache.sampleEnd = VoiceSampleEnd(self);

    self->cache.pan = VoiceGeneratorShortValue(self, SFGeneratorType_pan);
}

void VoiceUpdate(Voice *self)
{
    float nextTime = (float)++self->tick / self->sampleRate;
    VoiceUpdateVolEnv(self, nextTime);
    self->time = nextTime;
}

static void VoiceUpdateVolEnv(Voice *self, float nextTime)
{
    void (*phases[])(Voice *, float) = {
        VoiceUpdateVolEnvDelayPhase,
        VoiceUpdateVolEnvAttackPhase,
        VoiceUpdateVolEnvHoldPhase,
        VoiceUpdateVolEnvDecayPhase,
        VoiceUpdateVolEnvSustainPhase,
        VoiceUpdateVolEnvReleasePhase,
    };
    
    phases[self->phase](self, nextTime);
}

static inline void VoiceUpdateVolEnvPhaseNext(Voice *self, float nextTime, float endTime)
{
    if (nextTime >= endTime) {
        ++self->phase;
        self->startPhaseTime = nextTime;
    }
}

static void VoiceUpdateVolEnvDelayPhase(Voice *self, float nextTime)
{
    // 8.1.1 Kinds of Generator Enumerators
    // 33 delayVolEnv

    // The most negative number (-32768) conventionally indicates no delay.
    if (-32768 >= self->cache.delayVolEnv) {
        VoiceUpdateVolEnvPhaseNext(self, nextTime, 0);
        return;
    }

    float timecent = Clip(self->cache.delayVolEnv, -12000.0f, 5000.0f);
    float endTime = Timecent2Sec(timecent) + self->startPhaseTime;

    self->volEnv = 0.0f;

    VoiceUpdateVolEnvPhaseNext(self, nextTime, endTime);
}

static void VoiceUpdateVolEnvAttackPhase(Voice *self, float nextTime)
{
    // 8.1.1 Kinds of Generator Enumerators
    // 34 attackVolEnv

    // The most negative number (-32768) conventionally indicates instantaneous attack.
    if (-32768 >= self->cache.attackVolEnv) {
        VoiceUpdateVolEnvPhaseNext(self, nextTime, 0);
        return;
    }

    float timecent = self->cache.attackVolEnv;
    timecent = Clip(timecent, -12000.0f, 8000.0f);
    float duration = Timecent2Sec(timecent);
    float endTime = duration + self->startPhaseTime;

    // 9.1.7 Envelope Generators
    // The envelope then rises in a convex curve to a value of one during the attack phase.
    self->volEnv = ConvexPositiveUnipolar((self->time - self->startPhaseTime) / duration);

    VoiceUpdateVolEnvPhaseNext(self, nextTime, endTime);
}

static void VoiceUpdateVolEnvHoldPhase(Voice *self, float nextTime)
{
    // 8.1.1 Kinds of Generator Enumerators
    // 35 holdVolEnv

    // The most negative number (-32768) conventionally indicates no hold phase.
    if (-32768 >= self->cache.holdVolEnv) {
        VoiceUpdateVolEnvPhaseNext(self, nextTime, 0);
        return;
    }

    float timecent = self->cache.holdVolEnv;

    // 39 keynumToVolEnvHold
    // This is the degree, in timecents per KeyNumber units,
    // to which the hold time of the Volume Envelope is decreased by increasing MIDI key number.
    // The hold time at key number 60 is always unchanged.
    // The unit scaling is such that a value of 100 provides a hold time which tracks the keyboard;
    // that is, an upward octave causes the hold time to halve.
    timecent *= pow(2.0, self->cache.keynumToVolEnvHold - (60.0f - self->keyForSample) / 1200.0f);

    timecent = Clip(timecent, -12000.0f, 5000.0f);

    float duration = Timecent2Sec(timecent);
    float endTime = duration + self->startPhaseTime;

    self->volEnv = 1.0f;

    VoiceUpdateVolEnvPhaseNext(self, nextTime, endTime);
}

static void VoiceUpdateVolEnvDecayPhase(Voice *self, float nextTime)
{
    // 8.1.1 Kinds of Generator Enumerators
    // 36 decayVolEnv

    float timecent = self->cache.decayVolEnv;

    // 40 keynumToVolEnvDecay
    // This is the degree, in timecents per KeyNumber units,
    // to which the hold time of the Volume Envelope is decreased by increasing MIDI key number.
    // The hold time at key number 60 is always unchanged.
    // The unit scaling is such that a value of 100 provides a hold time that tracks the keyboard;
    // that is, an upward octave causes the hold time to halve. 
    timecent *= pow(2.0, self->cache.keynumToVolEnvDecay - (60.0f - self->keyForSample) / 1200.0f);

    timecent = Clip(timecent, -12000.0f, 8000.0f);

    float duration = Timecent2Sec(timecent);
    float endTime = duration + self->startPhaseTime;

    float sustainLevel = 1.0f - 0.001f * Clip(self->cache.sustainVolEnv, 0, 1440);

    // 9.1.7 Envelope Generators
    // When the hold phase ends, the envelope enters a decay phase during which its value decreases linearly to a sustain level.
    float f = (self->time - self->startPhaseTime) / duration;
    self->volEnv = 0.0 < duration ? MAX(0.0f, 1.0f * (1.0f - f) + f * sustainLevel) : sustainLevel;

    VoiceUpdateVolEnvPhaseNext(self, nextTime, endTime);
}

static void VoiceUpdateVolEnvSustainPhase(Voice *self, float nextTime)
{
    // 9.1.7 Envelope Generators
    // When the sustain level is reached, the envelope enters sustain phase,
    // during which the envelope stays at the sustain level. 
    float sustainLevel = 1.0f - 0.001f * Clip(self->cache.sustainVolEnv, 0, 1440);
    self->volEnv = sustainLevel;

    // Whenever a key-off occurs, the envelope immediately enters a release phase
}

static void VoiceUpdateVolEnvReleasePhase(Voice *self, float nextTime)
{
    float timecent = self->cache.releaseVolEnv;
    timecent = Clip(timecent, -12000.0f, 8000.0f);
    float duration = Timecent2Sec(timecent);
    float endTime = duration + self->startPhaseTime;

    // 9.1.7 Envelope Generators
    // Whenever a key-off occurs, the envelope immediately enters a release phase
    // during which the value linearly ramps from the current value to zero.
    // When zero is reached, the envelope value remains at zero.
    self->volEnv = 0.0 < duration ? MAX(0.0f, ((endTime - self->time) * self->releasedVolEnv) / duration) : 0.0f;
}

extern AudioSample VoiceComputeSample(Voice *self)
{
    int index = floor(self->sampleIndex);
    float over = self->sampleIndex - (float)index;
    int32_t indexSample = (self->sf->smpl[index] << 8) + (self->sf->sm24 ? self->sf->sm24[index] : 0);
    int32_t nextSample = (self->sf->smpl[index + 1] << 8) + (self->sf->sm24 ? self->sf->sm24[index + 1] : 0);

    float normalized = ((float)indexSample * (1.0f - over) + (float)nextSample * over) / (float)(SHRT_MAX << 8);

    // TODO velocity

    float pan = self->cache.pan / 500.0f;
    float left = 1.0f - self->cache.pan;
    float right = 1.0f + self->cache.pan;

    left = Clip(left, 0.0f, 1.0f);
    right = Clip(right, 0.0f, 1.0f);

    AudioSample ret;
    ret.L = normalized * self->volEnv * left;
    ret.R = normalized * self->volEnv * right;
    
    return ret;
}

void VoiceIncrementSample(Voice *self)
{
    self->sampleIndex += self->sampleIncrement;

    if (self->sampleModes & 0x01) {
        if (self->sampleModes & 0x02 && self->phase == VolEnvPhaseRelease) {
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
    self->phase = VolEnvPhaseRelease;
    self->startPhaseTime = self->time;
    self->releasedVolEnv = self->volEnv;
}

bool VoiceIsReleased(Voice *self)
{
    return (self->phase == VolEnvPhaseRelease && 0.0f >= self->volEnv)
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
