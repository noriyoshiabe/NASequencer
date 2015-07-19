#include "Voice.h"
#include "Define.h"
#include "Modulator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

static void VoiceModulatorInitialize(Voice *self);
static void VoiceUpdateSampleIncrement(Voice *self);
static uint32_t VoiceSampleStart(Voice *self);
static uint32_t VoiceSampleEnd(Voice *self);
static uint32_t VoiceSampleStartLoop(Voice *self);
static uint32_t VoiceSampleEndLoop(Voice *self);
static int16_t VoiceGeneratorValue(Voice *self, SFGeneratorType generatorType);
static int16_t VoiceModulatorValue(Voice *self, SFGeneratorType generatorType);
static int16_t VoiceNRPNValue(Voice *self, SFGeneratorType generatorType);

extern void VoiceInitialize(Voice *self, Channel *channel, uint8_t noteNo, uint8_t velocity,
        Zone *presetGlobalZone, Zone *presetZone, Zone *instrumentGlobalZone, Zone *instrumentZone,
        SoundFont *sf, double sampleRate)
{
    self->channel = channel;
    self->preset = channel->preset;

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
    self->sustain = false;

    self->sampleModes = VoiceGeneratorValue(self, SFGeneratorType_sampleModes);
    self->exclusiveClass = VoiceGeneratorValue(self, SFGeneratorType_exclusiveClass);

    EnvelopeInit(&self->modEnv);
    EnvelopeInit(&self->volEnv);

    LFOInit(&self->modLfo);
    LFOInit(&self->vibLfo);

    VoiceModulatorInitialize(self);
    VoiceUpdateRuntimeParams(self);
}

static void VoiceModulatorInitialize(Voice *self)
{
    int count = 0;

    self->modCount = 0;

    self->mod[self->modCount++] = &MIDINoteOnVelocityToInitialAttenuation;
    self->mod[self->modCount++] = &MIDINoteOnVelocityToFilterCutoff;
    self->mod[self->modCount++] = &MIDIChannelPressureToVibratoLFOPitchDepth;
    self->mod[self->modCount++] = &MIDIContinuousController1ToVibratoLFOPitchDepth;
    self->mod[self->modCount++] = &MIDIContinuousController7ToInitialAttenuation;
    self->mod[self->modCount++] = &MIDIContinuousController10ToPanPosition;
    self->mod[self->modCount++] = &MIDIContinuousController11ToInitialAttenuation;
    self->mod[self->modCount++] = &MIDIContinuousController91ToReverbEffectsSend;
    self->mod[self->modCount++] = &MIDIContinuousController93ToChorusEffectsSend;
    self->mod[self->modCount++] = &MIDIPitchWheelToInitialPitchControlledByMIDIPitchWheelSensitivity;

    // 9.5.1 Controller Model Theory of Operation

    // A modulator, contained within a global instrument zone,
    // that is identical to a default modulator supersedes or replaces the default modulator.
    count = self->instrumentGlobalZone ? self->instrumentGlobalZone->modCount : 0;
    for (int i = 0; i < count; ++i) {
        if (MAX_MODULATOR <= self->modCount) {
            break;
        }

        ModulatorAddOverwrite((const Modulator **)self->mod, &self->modCount, self->instrumentGlobalZone->mod[i]);
    }

    // A modulator, that is contained in a local instrument zone,
    // which is identical to a default modulator or to a modulator in a global instrument zone supersedes or replaces that modulator.
    count = self->instrumentZone ? self->instrumentZone->modCount : 0;
    for (int i = 0; i < count; ++i) {
        if (MAX_MODULATOR <= self->modCount) {
            break;
        }

        ModulatorAddOverwrite((const Modulator **)self->mod, &self->modCount, self->instrumentZone->mod[i]);
    }

    // A modulator, contained within a global preset zone,
    // that is identical to a default modulator or to a modulator in an instrument
    // adds to that modulator.
    //
    // A modulator, contained within a local preset zone,
    // that is identical to a modulator in a global preset zone supersedes or replaces
    // that modulator in the global preset zone.
    // That modulator then has its effects added to the destination summing node of
    // all zones in the given instrument.
    
    int modCount = 0;
    Modulator *modList[MAX_MODULATOR];

    count = self->presetGlobalZone ? self->presetGlobalZone->modCount : 0;
    for (int i = 0; i < count; ++i) {
        if (MAX_MODULATOR <= modCount) {
            break;
        }
        modList[modCount++] = self->presetGlobalZone->mod[i];
    }

    count = self->presetZone ? self->presetZone->modCount : 0;
    for (int i = 0; i < count; ++i) {
        if (MAX_MODULATOR <= modCount) {
            break;
        }

        ModulatorAddOverwrite((const Modulator **)modList, &modCount, self->presetZone->mod[i]);
    }

    for (int i = 0; i < modCount; ++i) {
        if (MAX_MODULATOR <= self->modCount) {
            break;
        }

        self->mod[self->modCount++] = modList[i];
    }
}

void VoiceUpdateRuntimeParams(Voice* self)
{
    self->sampleStartLoop = VoiceSampleStartLoop(self);
    self->sampleEndLoop = VoiceSampleEndLoop(self);
    self->sampleEnd = VoiceSampleEnd(self);

    self->pan = VoiceGeneratorValue(self, SFGeneratorType_pan);
    self->initialAttenuation = VoiceGeneratorValue(self, SFGeneratorType_initialAttenuation);

    self->modLfoToPitch = VoiceGeneratorValue(self, SFGeneratorType_modLfoToPitch);
    self->vibLfoToPitch = VoiceGeneratorValue(self, SFGeneratorType_vibLfoToPitch);
    self->modEnvToPitch = VoiceGeneratorValue(self, SFGeneratorType_modEnvToPitch);

    self->initialFilterFc = VoiceGeneratorValue(self, SFGeneratorType_initialFilterFc);
    self->initialFilterQ = VoiceGeneratorValue(self, SFGeneratorType_initialFilterQ);
    self->modLfoToFilterFc = VoiceGeneratorValue(self, SFGeneratorType_modLfoToFilterFc);
    self->modEnvToFilterFc = VoiceGeneratorValue(self, SFGeneratorType_modEnvToFilterFc);

    self->modLfoToVolume = VoiceGeneratorValue(self, SFGeneratorType_modLfoToVolume);

    self->chorusEffectsSend = VoiceGeneratorValue(self, SFGeneratorType_chorusEffectsSend);
    self->reverbEffectsSend = VoiceGeneratorValue(self, SFGeneratorType_reverbEffectsSend);

    EnvelopeUpdateRuntimeParams(&self->modEnv,
            EnvelopeTypeModulation,
            VoiceGeneratorValue(self, SFGeneratorType_delayModEnv),
            VoiceGeneratorValue(self, SFGeneratorType_attackModEnv),
            VoiceGeneratorValue(self, SFGeneratorType_holdModEnv),
            VoiceGeneratorValue(self, SFGeneratorType_decayModEnv),
            VoiceGeneratorValue(self, SFGeneratorType_sustainModEnv),
            VoiceGeneratorValue(self, SFGeneratorType_releaseModEnv),
            VoiceGeneratorValue(self, SFGeneratorType_keynumToModEnvHold),
            VoiceGeneratorValue(self, SFGeneratorType_keynumToModEnvDecay),
            self->keyForSample);

    EnvelopeUpdateRuntimeParams(&self->volEnv,
            EnvelopeTypeVolume,
            VoiceGeneratorValue(self, SFGeneratorType_delayVolEnv),
            VoiceGeneratorValue(self, SFGeneratorType_attackVolEnv),
            VoiceGeneratorValue(self, SFGeneratorType_holdVolEnv),
            VoiceGeneratorValue(self, SFGeneratorType_decayVolEnv),
            VoiceGeneratorValue(self, SFGeneratorType_sustainVolEnv),
            VoiceGeneratorValue(self, SFGeneratorType_releaseVolEnv),
            VoiceGeneratorValue(self, SFGeneratorType_keynumToVolEnvHold),
            VoiceGeneratorValue(self, SFGeneratorType_keynumToVolEnvDecay),
            self->keyForSample);

    LFOUpdateRuntimeParams(&self->modLfo,
            VoiceGeneratorValue(self, SFGeneratorType_delayModLFO),
            VoiceGeneratorValue(self, SFGeneratorType_freqModLFO),
            self->sampleRate);

    LFOUpdateRuntimeParams(&self->vibLfo,
            VoiceGeneratorValue(self, SFGeneratorType_delayVibLFO),
            VoiceGeneratorValue(self, SFGeneratorType_freqVibLFO),
            self->sampleRate);

    VoiceUpdateSampleIncrement(self);
}

static void VoiceUpdateSampleIncrement(Voice *self)
{
    int16_t v;
    Sample *sample = self->instrumentZone->sample;

    self->keyForSample = 0 <= (v = VoiceGeneratorValue(self, SFGeneratorType_keynum)) ? v : self->key;
    double originalPitch = 0 <= (v = VoiceGeneratorValue(self, SFGeneratorType_overridingRootKey)) ? v : sample->originalPitch;

    double scaleTuning = VoiceGeneratorValue(self, SFGeneratorType_scaleTuning);
    double cent = (self->keyForSample - originalPitch) * scaleTuning;
    cent += self->instrumentZone->sample->pitchCorrection;
    cent += VoiceGeneratorValue(self, SFGeneratorType_coarseTune) * 100.0;
    cent += VoiceGeneratorValue(self, SFGeneratorType_fineTune);
    cent += VoiceGeneratorValue(self, SFGeneratorType_initialPitch);
    cent += self->channel->masterCoarseTune * 100.0;
    cent += self->channel->masterFineTune;

    self->sampleIncrement = (double)sample->sampleRate / self->sampleRate;
    self->sampleIncrement *= Cent2FreqRatio(cent);
}

static inline double VoiceCurrentTime(Voice *self)
{
    return (double)self->tick / self->sampleRate;
}

void VoiceUpdate(Voice *self)
{
    double currentTime = VoiceCurrentTime(self);

    EnvelopeUpdate(&self->modEnv, currentTime);
    EnvelopeUpdate(&self->volEnv, currentTime);

    LFOUpdate(&self->modLfo, currentTime);
    LFOUpdate(&self->vibLfo, currentTime);

    ++self->tick;
}

AudioSample VoiceComputeSample(Voice *self)
{
    int index = floor(self->sampleIndex);
    double over = self->sampleIndex - (double)index;
    int32_t indexSample = (self->sf->smpl[index] << 8) + (self->sf->sm24 ? self->sf->sm24[index] : 0);
    int32_t nextSample = (self->sf->smpl[index + 1] << 8) + (self->sf->sm24 ? self->sf->sm24[index + 1] : 0);

    double normalized = ((double)indexSample * (1.0 - over) + (double)nextSample * over) / (double)0x7FFFFF;

    int16_t pan = Clip(self->pan, -500, 500);
    double coef = M_PI / 2.0 / 1000.0;
    double left = sin(coef * (-pan + 500));
    double right = sin(coef * (pan + 500));

    AudioSample sample;
    sample.L = normalized * left;
    sample.R = normalized * right;

    double frequency_cent = self->initialFilterFc;
    frequency_cent += self->modLfoToFilterFc * LFOValue(&self->modLfo);
    frequency_cent += self->modEnvToFilterFc * EnvelopeValue(&self->modEnv);
    frequency_cent = Clip(frequency_cent, 1500.0, 13500.0);

    double q_cB = Clip(self->initialFilterQ, 0, 960);

    LowPassFilterCalcLPFCoefficient(&self->LPF, self->sampleRate, frequency_cent, q_cB);
    sample = LowPassFilterApply(&self->LPF, sample);
  
    // Excerpt from fluid_synth
    //   `Alternate attenuation scale used by EMU10K1 cards when setting the attenuation at the preset or instrument level within the SoundFont bank.`
    // And TiMidity++ comments it out!! that implementation for initialAttenuation.
    // This is not doucmented on SF2.4 specification but probably many major soundfont is tunend with EMU10K1's specs.
    const double AlternateAttenuationScale = 0.4;

    double attenuation = self->initialAttenuation * AlternateAttenuationScale;
    double volume = cBAttn2Value(attenuation);
    volume *= cB2Value(self->modLfoToVolume * LFOValue(&self->modLfo));
    volume *= EnvelopeValue(&self->volEnv);
    volume = Clip(volume, 0.0, 1.0);

    sample.L *= volume;
    sample.R *= volume;

    return sample;
}

void VoiceIncrementSample(Voice *self)
{
    double sampleIncrement = self->sampleIncrement;

    sampleIncrement *= Cent2FreqRatio((double)self->modEnvToPitch * EnvelopeValue(&self->modEnv));
    sampleIncrement *= Cent2FreqRatio((double)self->vibLfoToPitch * LFOValue(&self->vibLfo));
    sampleIncrement *= Cent2FreqRatio((double)self->modLfoToPitch * LFOValue(&self->modLfo));

    self->sampleIndex += sampleIncrement;

    if (self->sampleModes & 0x01) {
        if (self->sampleModes & 0x02 && EnvelopeIsReleased(&self->volEnv)) {
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

double VoiceChorusEffectsSend(Voice *self)
{
    return (double)self->chorusEffectsSend * 0.001;
}

double VoiceReverbEffectsSend(Voice *self)
{
    return (double)self->reverbEffectsSend * 0.001;
}

void VoiceRelease(Voice *self)
{
    double currentTime = VoiceCurrentTime(self);
    EnvelopeRelease(&self->modEnv, currentTime);
    EnvelopeRelease(&self->volEnv, currentTime);
}

void VoiceTerminate(Voice *self)
{
    double currentTime = VoiceCurrentTime(self);
    EnvelopeFinish(&self->modEnv, currentTime);
    EnvelopeFinish(&self->volEnv, currentTime);
    self->exclusiveClass = 0;
}

bool VoiceIsReleased(Voice *self)
{
    return EnvelopeIsFinished(&self->volEnv) || self->sampleEnd < self->sampleIndex;
}

static uint32_t VoiceSampleStart(Voice *self)
{
    uint32_t ret = self->instrumentZone->sample->start;
    ret += VoiceGeneratorValue(self, SFGeneratorType_startAddrsOffset);
    ret += VoiceGeneratorValue(self, SFGeneratorType_startAddrsCoarseOffset) << 15; // * 32768
    return ret;
}

static uint32_t VoiceSampleEnd(Voice *self)
{
    uint32_t ret = self->instrumentZone->sample->end;
    ret += VoiceGeneratorValue(self, SFGeneratorType_endAddrsOffset);
    ret += VoiceGeneratorValue(self, SFGeneratorType_endAddrsCoarseOffset) << 15; // * 32768
    return ret;
}

static uint32_t VoiceSampleStartLoop(Voice *self)
{
    uint32_t ret = self->instrumentZone->sample->startLoop;
    ret += VoiceGeneratorValue(self, SFGeneratorType_startloopAddrsOffset);
    ret += VoiceGeneratorValue(self, SFGeneratorType_startloopAddrsCoarseOffset) << 15; // * 32768
    return ret;
}

static uint32_t VoiceSampleEndLoop(Voice *self)
{
    uint32_t ret = self->instrumentZone->sample->endLoop;
    ret += VoiceGeneratorValue(self, SFGeneratorType_endloopAddrsOffset);
    ret += VoiceGeneratorValue(self, SFGeneratorType_endloopAddrsCoarseOffset) << 15; // * 32768
    return ret;
}

static int16_t VoiceGeneratorValue(Voice *self, SFGeneratorType generatorType)
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

    return value
        + VoiceModulatorValue(self, generatorType)
        + VoiceNRPNValue(self, generatorType);
}

static int16_t VoiceModulatorValue(Voice *self, SFGeneratorType generatorType)
{
    int16_t result = 0;

    for (int i = 0; i < self->modCount; ++i) {
        if (self->mod[i]->sfModDestOper == generatorType) {
            result += ModulatorGetValue(self->mod[i], self->channel, self);
        }
    }

    return result;
}

static int16_t VoiceNRPNValue(Voice *self, SFGeneratorType generatorType)
{
    return self->channel->nrpnValues[generatorType];
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
    printf("channel: %d\n", voice->channel->number);
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
    //printf("  startAddrsOffset: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_startAddrsOffset));
    //printf("  endAddrsOffset: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_endAddrsOffset));
    //printf("  startloopAddrsOffset: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_startloopAddrsOffset));
    //printf("  endloopAddrsOffset: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_endloopAddrsOffset));
    //printf("  startAddrsCoarseOffset: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_startAddrsCoarseOffset));
    printf("  modLfoToPitch: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_modLfoToPitch));
    printf("  vibLfoToPitch: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_vibLfoToPitch));
    printf("  modEnvToPitch: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_modEnvToPitch));
    printf("  initialFilterFc: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_initialFilterFc));
    printf("  initialFilterQ: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_initialFilterQ));
    printf("  modLfoToFilterFc: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_modLfoToFilterFc));
    printf("  modEnvToFilterFc: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_modEnvToFilterFc));
    //printf("  endAddrsCoarseOffset: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_endAddrsCoarseOffset));
    printf("  modLfoToVolume: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_modLfoToVolume));
    //printf("  unused1: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_unused1));
    printf("  chorusEffectsSend: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_chorusEffectsSend));
    printf("  reverbEffectsSend: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_reverbEffectsSend));
    printf("  pan: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_pan));
    //printf("  unused2: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_unused2));
    //printf("  unused3: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_unused3));
    //printf("  unused4: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_unused4));
    printf("  delayModLFO: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_delayModLFO));
    printf("  freqModLFO: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_freqModLFO));
    printf("  delayVibLFO: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_delayVibLFO));
    printf("  freqVibLFO: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_freqVibLFO));
    printf("  delayModEnv: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_delayModEnv));
    printf("  attackModEnv: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_attackModEnv));
    printf("  holdModEnv: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_holdModEnv));
    printf("  decayModEnv: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_decayModEnv));
    printf("  sustainModEnv: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_sustainModEnv));
    printf("  releaseModEnv: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_releaseModEnv));
    printf("  keynumToModEnvHold: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_keynumToModEnvHold));
    printf("  keynumToModEnvDecay: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_keynumToModEnvDecay));
    printf("  delayVolEnv: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_delayVolEnv));
    printf("  attackVolEnv: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_attackVolEnv));
    printf("  holdVolEnv: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_holdVolEnv));
    printf("  decayVolEnv: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_decayVolEnv));
    printf("  sustainVolEnv: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_sustainVolEnv));
    printf("  releaseVolEnv: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_releaseVolEnv));
    printf("  keynumToVolEnvHold: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_keynumToVolEnvHold));
    printf("  keynumToVolEnvDecay: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_keynumToVolEnvDecay));
    //printf("  instrument: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_instrument));
    //printf("  reserved1: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_reserved1));
    //printf("  keyRange: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_keyRange));
    //printf("  velRange: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_velRange));
    //printf("  startloopAddrsCoarseOffset: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_startloopAddrsCoarseOffset));
    printf("  keynum: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_keynum));
    printf("  velocity: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_velocity));
    printf("  initialAttenuation: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_initialAttenuation));
    //printf("  reserved2: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_reserved2));
    //printf("  endloopAddrsCoarseOffset: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_endloopAddrsCoarseOffset));
    printf("  coarseTune: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_coarseTune));
    printf("  fineTune: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_fineTune));
    //printf("  sampleID: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_sampleID));
    printf("  sampleModes: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_sampleModes));
    //printf("  reserved3: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_reserved3));
    printf("  scaleTuning: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_scaleTuning));
    printf("  exclusiveClass: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_exclusiveClass));
    printf("  overridingRootKey: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_overridingRootKey));
    //printf("  unused5: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_unused5));
    //printf("  endOper: %d\n", VoiceGeneratorValue(voice, SFGeneratorType_endOper));

    printf("\n");
}
