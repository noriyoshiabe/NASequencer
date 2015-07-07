#include "Voice.h"
#include "Define.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

static void VoiceUpdateSampleIncrement(Voice *self);
static uint32_t VoiceSampleStart(Voice *self);
static uint32_t VoiceSampleEnd(Voice *self);
static uint32_t VoiceSampleStartLoop(Voice *self);
static uint32_t VoiceSampleEndLoop(Voice *self);
static int16_t VoiceGeneratorShortValue(Voice *self, SFGeneratorType generatorType);

extern void VoiceInitialize(Voice *self, uint8_t channel, uint8_t noteNo, uint8_t velocity,
        Zone *presetGlobalZone, Zone *presetZone, Zone *instrumentGlobalZone, Zone *instrumentZone,
        SoundFont *sf, double sampleRate)
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
    self->sampleIndex = VoiceSampleStart(self);

    self->sampleModes = VoiceGeneratorShortValue(self, SFGeneratorType_sampleModes);
    self->exclusiveClass = VoiceGeneratorShortValue(self, SFGeneratorType_exclusiveClass);

    self->sampleStartLoop = VoiceSampleStartLoop(self);
    self->sampleEndLoop = VoiceSampleEndLoop(self);
    self->sampleEnd = VoiceSampleEnd(self);

    self->pan = VoiceGeneratorShortValue(self, SFGeneratorType_pan);
    self->initialAttenuation = VoiceGeneratorShortValue(self, SFGeneratorType_pan);

    VoiceUpdateSampleIncrement(self);

    ADSREnvelopeInit(&self->volEnv,
            ADSREnvelopeTypeVolume,
            VoiceGeneratorShortValue(self, SFGeneratorType_delayVolEnv),
            VoiceGeneratorShortValue(self, SFGeneratorType_attackVolEnv),
            VoiceGeneratorShortValue(self, SFGeneratorType_holdVolEnv),
            VoiceGeneratorShortValue(self, SFGeneratorType_decayVolEnv),
            VoiceGeneratorShortValue(self, SFGeneratorType_sustainVolEnv),
            VoiceGeneratorShortValue(self, SFGeneratorType_releaseVolEnv),
            VoiceGeneratorShortValue(self, SFGeneratorType_keynumToVolEnvHold),
            VoiceGeneratorShortValue(self, SFGeneratorType_keynumToVolEnvDecay),
            self->keyForSample);

    int16_t initialFilterFc = VoiceGeneratorShortValue(self, SFGeneratorType_initialFilterFc);
    int16_t initialFilterQ = VoiceGeneratorShortValue(self, SFGeneratorType_initialFilterQ);
    initialFilterFc = Clip(initialFilterFc, 1500, 13500);
    initialFilterQ = Clip(initialFilterQ, 0, 960);
    IIRFilterCalcLPFCoefficient(&self->LPF, sampleRate, initialFilterFc, initialFilterQ);
}

static void VoiceUpdateSampleIncrement(Voice *self)
{
    int16_t v;
    Sample *sample = self->instrumentZone->sample;

    self->keyForSample = 0 <= (v = VoiceGeneratorShortValue(self, SFGeneratorType_keynum)) ? v : self->key;
    double originalPitch = 0 <= (v = VoiceGeneratorShortValue(self, SFGeneratorType_overridingRootKey)) ? v : sample->originalPitch;

    double scaleTuning = VoiceGeneratorShortValue(self, SFGeneratorType_scaleTuning);
    double cent = (self->keyForSample - originalPitch) * scaleTuning;
    cent += self->instrumentZone->sample->pitchCorrection;
    cent += VoiceGeneratorShortValue(self, SFGeneratorType_coarseTune) * 100.0;
    cent += VoiceGeneratorShortValue(self, SFGeneratorType_fineTune);

    self->sampleIncrement = (double)sample->sampleRate / self->sampleRate;
    self->sampleIncrement *= pow(2.0, cent / 1200.0);
}

static inline double VoiceCurrentTime(Voice *self)
{
    return (double)self->tick / self->sampleRate;
}

void VoiceUpdate(Voice *self)
{
    ADSREnvelopeUpdate(&self->volEnv, VoiceCurrentTime(self));
    ++self->tick;
}

AudioSample VoiceComputeSample(Voice *self)
{
    int index = floor(self->sampleIndex);
    double over = self->sampleIndex - (double)index;
    int32_t indexSample = (self->sf->smpl[index] << 8) + (self->sf->sm24 ? self->sf->sm24[index] : 0);
    int32_t nextSample = (self->sf->smpl[index + 1] << 8) + (self->sf->sm24 ? self->sf->sm24[index + 1] : 0);

    double normalized = ((double)indexSample * (1.0 - over) + (double)nextSample * over) / (double)(SHRT_MAX << 8);

    // TODO velocity

    double pan = self->pan / 500.0;
    double left = 1.0 - self->pan;
    double right = 1.0 + self->pan;

    left = Clip(left, 0.0, 1.0);
    right = Clip(right, 0.0, 1.0);

    double attenuation = cBAttn2Value(self->initialAttenuation);
    double volEnvValue = ADSREnvelopeValue(&self->volEnv);

    AudioSample sample;
    sample.L = normalized * attenuation * volEnvValue * left;
    sample.R = normalized * attenuation * volEnvValue * right;
    
    return IIRFilterApply(&self->LPF, sample);
}

void VoiceIncrementSample(Voice *self)
{
    self->sampleIndex += self->sampleIncrement;

    if (self->sampleModes & 0x01) {
        if (self->sampleModes & 0x02 && ADSREnvelopeIsReleased(&self->volEnv)) {
            ;
        }
        else {
            if (self->sampleEndLoop < self->sampleIndex) {
                double over = self->sampleIndex - self->sampleEndLoop;
                self->sampleIndex = self->sampleStartLoop + over;
            }
        }
    }
}

void VoiceRelease(Voice *self)
{
    ADSREnvelopeRelease(&self->volEnv, VoiceCurrentTime(self));
}

void VoiceTerminate(Voice *self)
{
    ADSREnvelopeFinish(&self->volEnv, VoiceCurrentTime(self));
    self->exclusiveClass = 0;
}

bool VoiceIsReleased(Voice *self)
{
    return ADSREnvelopeIsFinished(&self->volEnv) || self->sampleEnd < self->sampleIndex;
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

#if 1
#define printf printf("\r"),printf
#endif

void VoiceDump(Voice *voice)
{
    printf("[Voice]\n");
    printf("-----------------------------------------\n");
    printf("channel: %d\n", voice->channel);
    printf("key: %d\n", voice->key);
    printf("velocity: %d\n", voice->velocity);
    printf("tick: %d\n", voice->tick);
    printf("sampleIncrement: %f\n", voice->sampleIncrement);
    printf("sampleIndex: %f\n", voice->sampleIndex);

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
    printf("  ---------------------------------------\n");
    //printf("  startAddrsOffset: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_startAddrsOffset));
    //printf("  endAddrsOffset: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_endAddrsOffset));
    //printf("  startloopAddrsOffset: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_startloopAddrsOffset));
    //printf("  endloopAddrsOffset: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_endloopAddrsOffset));
    //printf("  startAddrsCoarseOffset: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_startAddrsCoarseOffset));
    printf("  modLfoToPitch: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_modLfoToPitch));
    printf("  vibLfoToPitch: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_vibLfoToPitch));
    printf("  modEnvToPitch: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_modEnvToPitch));
    printf("  initialFilterFc: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_initialFilterFc));
    printf("  initialFilterQ: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_initialFilterQ));
    printf("  modLfoToFilterFc: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_modLfoToFilterFc));
    printf("  modEnvToFilterFc: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_modEnvToFilterFc));
    //printf("  endAddrsCoarseOffset: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_endAddrsCoarseOffset));
    printf("  modLfoToVolume: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_modLfoToVolume));
    //printf("  unused1: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_unused1));
    printf("  chorusEffectsSend: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_chorusEffectsSend));
    printf("  reverbEffectsSend: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_reverbEffectsSend));
    printf("  pan: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_pan));
    //printf("  unused2: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_unused2));
    //printf("  unused3: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_unused3));
    //printf("  unused4: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_unused4));
    printf("  delayModLFO: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_delayModLFO));
    printf("  freqModLFO: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_freqModLFO));
    printf("  delayVibLFO: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_delayVibLFO));
    printf("  freqVibLFO: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_freqVibLFO));
    printf("  delayModEnv: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_delayModEnv));
    printf("  attackModEnv: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_attackModEnv));
    printf("  holdModEnv: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_holdModEnv));
    printf("  decayModEnv: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_decayModEnv));
    printf("  sustainModEnv: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_sustainModEnv));
    printf("  releaseModEnv: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_releaseModEnv));
    printf("  keynumToModEnvHold: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_keynumToModEnvHold));
    printf("  keynumToModEnvDecay: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_keynumToModEnvDecay));
    printf("  delayVolEnv: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_delayVolEnv));
    printf("  attackVolEnv: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_attackVolEnv));
    printf("  holdVolEnv: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_holdVolEnv));
    printf("  decayVolEnv: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_decayVolEnv));
    printf("  sustainVolEnv: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_sustainVolEnv));
    printf("  releaseVolEnv: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_releaseVolEnv));
    printf("  keynumToVolEnvHold: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_keynumToVolEnvHold));
    printf("  keynumToVolEnvDecay: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_keynumToVolEnvDecay));
    //printf("  instrument: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_instrument));
    //printf("  reserved1: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_reserved1));
    //printf("  keyRange: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_keyRange));
    //printf("  velRange: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_velRange));
    //printf("  startloopAddrsCoarseOffset: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_startloopAddrsCoarseOffset));
    printf("  keynum: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_keynum));
    printf("  velocity: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_velocity));
    printf("  initialAttenuation: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_initialAttenuation));
    //printf("  reserved2: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_reserved2));
    //printf("  endloopAddrsCoarseOffset: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_endloopAddrsCoarseOffset));
    printf("  coarseTune: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_coarseTune));
    printf("  fineTune: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_fineTune));
    //printf("  sampleID: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_sampleID));
    printf("  sampleModes: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_sampleModes));
    //printf("  reserved3: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_reserved3));
    printf("  scaleTuning: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_scaleTuning));
    printf("  exclusiveClass: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_exclusiveClass));
    printf("  overridingRootKey: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_overridingRootKey));
    //printf("  unused5: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_unused5));
    //printf("  endOper: %d\n", VoiceGeneratorShortValue(voice, SFGeneratorType_endOper));

    printf("\n");
}
