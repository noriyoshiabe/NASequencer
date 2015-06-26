#include "Preset.h"

#include <stdio.h>
#include <stdlib.h>

#define isValidRange(idx, length) (0 <= idx && idx < length)
#define isValidRangeWithAsc(idx1, idx2, length) (0 <= idx1 && idx1 < idx2 && idx2 < length)
#define isRomSample(sampleType) (0 != (sampleType & 0x8000))

static bool ParsePreset(SoundFont *sf, int presetIdx, Preset **result);
static Zone *ZoneCreate();
static void ZoneDestroy(Zone *self);
static bool ZoneParseGenerator(Zone *self, SoundFont *sf, SFGenList *generators, int generatorCount);
static bool ZoneParseModulator(Zone *self, SoundFont *sf, SFModList *modulators, int modulatorCount);
static bool ParseInstrument(SoundFont *sf, int instIdx, Instrument **result);
static void InstrumentDestroy(Instrument *self);
static bool ParseSample(SoundFont *sf, int shdrIdx, Sample **result);
static void SampleDestroy(Sample *self);

bool ParsePresets(SoundFont *sf, Preset ***results, int *resultsCount)
{
    Preset **presets = NULL;
    int presetCount = 0;

    for (int i = 0; i < sf->phdrLength - 1; ++i) {
        Preset *preset = NULL;
        if (!ParsePreset(sf, i, &preset)) {
            goto ERROR;
        }

        if (preset) {
            presets = realloc(presets, sizeof(Preset *) * presetCount + 1);
            presets[presetCount] = preset;
            ++presetCount;
        }
    }

    *results = presets;
    *resultsCount = presetCount;

    return true;

ERROR:
    for (int i = 0; i < presetCount; ++i) {
        PresetDestroy(presets[i]);
    }

    if (presets) {
        free(presets);
    }

    return false;
}

static bool ParsePreset(SoundFont *sf, int presetIdx, Preset **result)
{
    SFPresetHeader *phdr = &sf->phdr[presetIdx];
    SFPresetHeader *phdrNext = &sf->phdr[presetIdx + 1];

    // 7.2 The PHDR Sub-chunk
    // If the preset bag indices are non-monotonic or
    // if the terminal preset’s wPresetBagNdx does not match the PBAG sub- chunk size,
    // the file is structurally defective and should be rejected at load time.
    if (!isValidRangeWithAsc(phdr->wPresetBagNdx, phdrNext->wPresetBagNdx, sf->pbagLength)) {
        return false;
    }

    Preset *preset = calloc(1, sizeof(Preset));
    preset->name = phdr->achPresetName;
    preset->midiPresetNo = phdr->wPreset;
    preset->bankNo = phdr->wBank;

    SFPresetBag *pbag = &sf->pbag[phdr->wPresetBagNdx];
    int zoneCount = phdrNext->wPresetBagNdx - phdr->wPresetBagNdx;

    for (int i = 0; i < zoneCount; ++i) {
        SFPresetBag *current = &pbag[i];
        SFPresetBag *next = &pbag[i + 1];
        SFGenList *pgen = &sf->pgen[current->wGenNdx];
        SFModList *pmod = &sf->pmod[current->wModNdx];
        int generatorCount = next->wGenNdx - current->wGenNdx;
        int modulatorCount = next->wModNdx - current->wModNdx;

        // 7.3 The PBAG Sub-chunk
        // If the generator or modulator indices are non-monotonic
        // or do not match the size of the respective PGEN or PMOD sub-chunks,
        // the file is structurally defective and should be rejected at load time.
        if (0 > generatorCount || 0 > modulatorCount) {
            goto ERROR;
        }

        Zone *zone = ZoneCreate();
        if (!ZoneParseGenerator(zone, sf, pgen, generatorCount)) {
            ZoneDestroy(zone);
            goto ERROR;
        }

        if (!ZoneParseModulator(zone, sf, pmod, modulatorCount)) {
            ZoneDestroy(zone);
            goto ERROR;
        }

        if (!zone->instrument) {
            // 7.3 The PBAG Sub-chunk
            // If a zone other than the first zone lacks an Instrument generator as its last generator, that zone should be ignored.
            // A global zone with no modulators and no generators should also be ignored.
            
            if (0 == i) {
                if (0 < generatorCount || 0 < modulatorCount) {
                    preset->globalZone = zone;
                }
                else {
                    ZoneDestroy(zone);
                }
            }
            else {
                ZoneDestroy(zone);
            }
        }
        else {
            preset->zones = realloc(preset->zones, sizeof(Zone *) * preset->zoneCount + 1);
            preset->zones[preset->zoneCount] = zone;
            ++preset->zoneCount;
        }
    }

    if (!preset->globalZone && 1 > preset->zoneCount) {
        // 7.2 The PHDR Sub-chunk
        // All presets except the terminal preset must have at least one zone;
        // any preset with no zones should be ignored.
        PresetDestroy(preset);
    }
    else {
        *result = preset;
    }

    return true;

ERROR:
    PresetDestroy(preset);
    return false;
}

void PresetDestroy(Preset *self)
{
    if (self->globalZone) {
        ZoneDestroy(self->globalZone);
    }

    for (int i = 0; i < self->zoneCount; ++i) {
        ZoneDestroy(self->zones[i]);
    }

    free(self->zones);
    free(self);
}

static bool ParseInstrument(SoundFont *sf, int instIdx, Instrument **result)
{
    // 8.1.2 Generator Enumerators Defined - 41 instrument
    // The value should never exceed two less than the size of the instrument list. 
    if (!isValidRange(instIdx + 1, sf->instLength)) {
        return false;
    }

    SFInst *inst = &sf->inst[instIdx];
    SFInst *instNext = &sf->inst[instIdx + 1];

    // 7.6 The INST Sub-chunk
    // If the instrument bag indices are non-monotonic or
    // if the terminal instrument’s wInstBagNdx does not match the IBAG sub-chunk size,
    // the file is structurally defective and should be rejected at load time.
    if (!isValidRangeWithAsc(inst->wInstBagNdx, instNext->wInstBagNdx, sf->ibagLength)) {
        return false;
    }

    Instrument *instrument = calloc(1, sizeof(Instrument));
    instrument->name = inst->achInstName;

    SFInstBag *ibag = &sf->ibag[inst->wInstBagNdx];
    int zoneCount = instNext->wInstBagNdx - inst->wInstBagNdx;

    for (int i = 0; i < zoneCount; ++i) {
        SFInstBag *current = &ibag[i];
        SFInstBag *next = &ibag[i + 1];
        SFGenList *igen = &sf->igen[current->wInstGenNdx];
        SFModList *imod = &sf->imod[current->wInstModNdx];
        int generatorCount = next->wInstGenNdx - current->wInstGenNdx;
        int modulatorCount = next->wInstModNdx - current->wInstModNdx;

        // 7.7 The IBAG Sub-chunk
        // If the generator or modulator indices are non-monotonic
        // or do not match the size of the respective IGEN or IMOD sub-chunks,
        // the file is structurally defective and should be rejected at load time.
        if (0 > generatorCount || 0 > modulatorCount) {
            goto ERROR;
        }

        Zone *zone = ZoneCreate();
        if (!ZoneParseGenerator(zone, sf, igen, generatorCount)) {
            ZoneDestroy(zone);
            goto ERROR;
        }

        if (!ZoneParseModulator(zone, sf, imod, modulatorCount)) {
            ZoneDestroy(zone);
            goto ERROR;
        }

        if (!zone->sample) {
            // 7.7 The IBAG Sub-chunk
            // If a zone other than the first zone lacks a sampleID generator as its last generator, that zone should be ignored.
            // A global zone with no modulators and no generators should also be ignored.

            if (0 == i) {
                if (0 < generatorCount || 0 < modulatorCount) {
                    instrument->globalZone = zone;
                }
                else {
                    ZoneDestroy(zone);
                }
            }
            else {
                ZoneDestroy(zone);
            }
        }
        else {
            instrument->zones = realloc(instrument->zones, sizeof(Zone *) * instrument->zoneCount + 1);
            instrument->zones[instrument->zoneCount] = zone;
            ++instrument->zoneCount;
        }
    }

    if (!instrument->globalZone && 1 > instrument->zoneCount) {
        // 7.6 The INST Sub-chunk
        // All instruments except the terminal instrument must have at least one zone;
        // any preset with no zones should be ignored.
        InstrumentDestroy(instrument);
    }
    else {
        *result = instrument;
    }

    return true;

ERROR:
    InstrumentDestroy(instrument);
    return false;
}

static void InstrumentDestroy(Instrument *self)
{
    if (self->globalZone) {
        ZoneDestroy(self->globalZone);
    }

    for (int i = 0; i < self->zoneCount; ++i) {
        ZoneDestroy(self->zones[i]);
    }

    free(self->zones);
    free(self);
}

static bool ParseSample(SoundFont *sf, int shdrIdx, Sample **result)
{
    // 8.1.2 Generator Enumerators Defined - 53 sampleID
    // The value should never exceed two less than the size of the sample list.
    // 7.10 The SHDR Sub-chunk
    // The terminal sample record is never referenced, ... That's why -1
    if (!isValidRange(shdrIdx, sf->shdrLength - 1)) {
        return false;
    }

    SFSampleHeader *shdr = &sf->shdr[shdrIdx];

    // Ignore ROM sample.
    if (isRomSample(shdr->sfSampleType)) {
        return true;
    }

    Sample *sample = calloc(1, sizeof(Sample));
    sample->name = shdr->achSampleName;
    sample->start = shdr->dwStart;
    sample->startLoop = shdr->dwStartloop;
    sample->endLoop = shdr->dwEndloop;
    sample->end = shdr->dwEnd;
    sample->sampleRate = shdr->dwSampleRate;
    sample->originalPitch = shdr->byOriginalPitch;
    sample->pitchCorrection = shdr->chPitchCorrection;

    // Both fludesynth and timidity++ does not use wSampleLink, also sfSampleType for left or right.
    // I cannot understand this sentence excerpted 7.10 The SHDR Sub-chunk.
    //
    // Both samples should be played entirely syncrhonously,
    // with their pitch controlled by the right sample’s generators.
    //
    // The word 'right' means appropriate?
    // Even if 'right' means right sample, `their pitch controlled
    // by the right sample’s generators.` is much more complicated.
    
    sample->sampleType = shdr->sfSampleType; // This is only for dump

    *result = sample;
    return true;
}

static void SampleDestroy(Sample *self)
{
    free(self);
}

static Zone *ZoneCreate()
{
    return calloc(1, sizeof(Zone));
}

static void ZoneDestroy(Zone *self)
{
    if (self->instrument) {
        InstrumentDestroy(self->instrument);
    }

    if (self->sample) {
        SampleDestroy(self->sample);
    }

    free(self);
}

static bool ZoneParseGenerator(Zone *self, SoundFont *sf, SFGenList *generators, int generatorCount)
{
    for (int i = 0; i < generatorCount; ++i) {
        SFGenList *gen = &generators[i];

        switch (gen->sfGenOper) {
        case SFGeneratorType_instrument:
            if (!ParseInstrument(sf, gen->genAmount.wAmount, &self->instrument)) {
                return false;
            }
            break;

        case SFGeneratorType_sampleID:
            if (!ParseSample(sf, gen->genAmount.wAmount, &self->sample)) {
                return false;
            }
            break;
        }

        self->gen.array[gen->sfGenOper] = gen->genAmount.shAmount;
        self->gen.flags |= (1ull << gen->sfGenOper);
    }

    return true;
}

static bool ZoneParseModulator(Zone *self, SoundFont *sf, SFModList *modulators, int modulatorCount)
{
    // TODO
    return true;
}

bool ZoneHasGenerator(Zone *zone, SFGeneratorType generatorType)
{
    return zone->gen.flags & (1ull << generatorType);
}

int16_t ZoneGeneratorShortValue(Zone *zone, SFGeneratorType generatorType)
{
    return zone->gen.array[generatorType];
}

bool ZoneIsInsideRange(Zone *zone, uint8_t key, uint8_t velocity)
{
    return (!ZoneHasGenerator(zone, SFGeneratorType_keyRange)
            || (zone->gen.keyRange.low <= key && key <= zone->gen.keyRange.high))
        && (!ZoneHasGenerator(zone, SFGeneratorType_velRange)
                || (zone->gen.velRange.low <= velocity && velocity <= zone->gen.velRange.high));
}

int GeneratorDefaultValue(SFGeneratorType generatorType)
{
    const int16_t None = -1;
    const Range _DefRange = {0, 127};
    const int16_t DefRange = *((int16_t *)&_DefRange);

    const int16_t defaults[SFGeneratorType_endOper] = {
        0,        // SFGeneratorType_startAddrsOffset
        0,        // SFGeneratorType_endAddrsOffset
        0,        // SFGeneratorType_startloopAddrsOffset
        0,        // SFGeneratorType_endloopAddrsOffset
        0,        // SFGeneratorType_startAddrsCoarseOffset
        0,        // SFGeneratorType_modLfoToPitch
        0,        // SFGeneratorType_vibLfoToPitch
        0,        // SFGeneratorType_modEnvToPitch
        13500,    // SFGeneratorType_initialFilterFc
        0,        // SFGeneratorType_initialFilterQ
        0,        // SFGeneratorType_modLfoToFilterFc
        0,        // SFGeneratorType_modEnvToFilterFc
        0,        // SFGeneratorType_endAddrsCoarseOffset
        0,        // SFGeneratorType_modLfoToVolume
        None,     // SFGeneratorType_unused1
        0,        // SFGeneratorType_chorusEffectsSend
        0,        // SFGeneratorType_reverbEffectsSend
        0,        // SFGeneratorType_pan
        None,     // SFGeneratorType_unused2
        None,     // SFGeneratorType_unused3
        None,     // SFGeneratorType_unused4
        -12000,   // SFGeneratorType_delayModLFO
        0,        // SFGeneratorType_freqModLFO
        -12000,   // SFGeneratorType_delayVibLFO
        0,        // SFGeneratorType_freqVibLFO
        -12000,   // SFGeneratorType_delayModEnv
        -12000,   // SFGeneratorType_attackModEnv
        -12000,   // SFGeneratorType_holdModEnv
        -12000,   // SFGeneratorType_decayModEnv
        0,        // SFGeneratorType_sustainModEnv
        -12000,   // SFGeneratorType_releaseModEnv
        0,        // SFGeneratorType_keynumToModEnvHold
        0,        // SFGeneratorType_keynumToModEnvDecay
        -12000,   // SFGeneratorType_delayVolEnv
        -12000,   // SFGeneratorType_attackVolEnv
        -12000,   // SFGeneratorType_holdVolEnv
        -12000,   // SFGeneratorType_decayVolEnv
        0,        // SFGeneratorType_sustainVolEnv
        -12000,   // SFGeneratorType_releaseVolEnv
        0,        // SFGeneratorType_keynumToVolEnvHold
        0,        // SFGeneratorType_keynumToVolEnvDecay
        None,     // SFGeneratorType_instrument
        None,     // SFGeneratorType_reserved1
        DefRange, // SFGeneratorType_keyRange
        DefRange, // SFGeneratorType_velRange
        0,        // SFGeneratorType_startloopAddrsCoarseOffset
        None,     // SFGeneratorType_keynum
        None,     // SFGeneratorType_velocity
        0,        // SFGeneratorType_initialAttenuation
        None,     // SFGeneratorType_reserved2
        0,        // SFGeneratorType_endloopAddrsCoarseOffset
        0,        // SFGeneratorType_coarseTune
        0,        // SFGeneratorType_fineTune
        None,     // SFGeneratorType_sampleID
        0,        // SFGeneratorType_sampleModes
        None,     // SFGeneratorType_reserved3
        100,      // SFGeneratorType_scaleTuning
        0,        // SFGeneratorType_exclusiveClass
        None,     // SFGeneratorType_overridingRootKey
        None,     // SFGeneratorType_unused5
    };

    return defaults[generatorType];
}

bool GeneratorIsInstrumentOnly(SFGeneratorType generatorType)
{
    switch (generatorType) {
    case SFGeneratorType_startAddrsOffset:
    case SFGeneratorType_endAddrsOffset:
    case SFGeneratorType_startloopAddrsOffset:
    case SFGeneratorType_endloopAddrsOffset:
    case SFGeneratorType_startAddrsCoarseOffset:
    case SFGeneratorType_endAddrsCoarseOffset:
    case SFGeneratorType_startloopAddrsCoarseOffset:
    case SFGeneratorType_keynum:
    case SFGeneratorType_velocity:
    case SFGeneratorType_endloopAddrsCoarseOffset:
    case SFGeneratorType_sampleModes:
    case SFGeneratorType_exclusiveClass:
    case SFGeneratorType_overridingRootKey:
        return true;
    default:
        return false;
    }
}

#if 1
#define printf printf("\r"),printf
#endif

void PresetDump(Preset *preset)
{
    printf("[Preset] %s\n", preset->name);
    printf("------------------------------------------------------------------------------\n");
    printf("name: %s\n", preset->name);
    printf("midiPresetNo: %d\n", preset->midiPresetNo);
    printf("bankNo: %d\n", preset->bankNo);
    printf("globalZone:\n");
    if (preset->globalZone) {
        ZoneDump(preset->globalZone);
    }
    printf("zones: zoneCount=%d\n", preset->zoneCount);
    for (int i = 0; i < preset->zoneCount; ++i) {
        ZoneDump(preset->zones[i]);
        if (i + 1 < preset->zoneCount) {
            printf("  -----------------------------\n");
        }
    }
    printf("\n");
}

void InstrumentDump(Instrument *instrument)
{
    printf("[Instrument] %s\n", instrument->name);
    printf("------------------------------------------------------------------------------\n");
    printf("globalZone:\n");
    if (instrument->globalZone) {
        ZoneDump(instrument->globalZone);
    }
    printf("zones: zoneCount=%d\n", instrument->zoneCount);
    for (int i = 0; i < instrument->zoneCount; ++i) {
        ZoneDump(instrument->zones[i]);
        if (i + 1 < instrument->zoneCount) {
            printf("  -----------------------------\n");
        }
    }
    printf("\n");
}

void ZoneDump(Zone *zone)
{
#define DumpShortGen(zone, type) \
    if (ZoneHasGenerator(zone, SFGeneratorType_##type)) \
      printf("  " #type ": %d\n", zone->gen.type)

    DumpShortGen(zone, startAddrsOffset);
    DumpShortGen(zone, endAddrsOffset);
    DumpShortGen(zone, startloopAddrsOffset);
    DumpShortGen(zone, endloopAddrsOffset);
    DumpShortGen(zone, startAddrsCoarseOffset);
    DumpShortGen(zone, modLfoToPitch);
    DumpShortGen(zone, vibLfoToPitch);
    DumpShortGen(zone, modEnvToPitch);
    DumpShortGen(zone, initialFilterFc);
    DumpShortGen(zone, initialFilterQ);
    DumpShortGen(zone, modLfoToFilterFc);
    DumpShortGen(zone, modEnvToFilterFc);
    DumpShortGen(zone, endAddrsCoarseOffset);
    DumpShortGen(zone, modLfoToVolume);
    DumpShortGen(zone, unused1);
    DumpShortGen(zone, chorusEffectsSend);
    DumpShortGen(zone, reverbEffectsSend);
    DumpShortGen(zone, pan);
    DumpShortGen(zone, unused2);
    DumpShortGen(zone, unused3);
    DumpShortGen(zone, unused4);
    DumpShortGen(zone, delayModLFO);
    DumpShortGen(zone, freqModLFO);
    DumpShortGen(zone, delayVibLFO);
    DumpShortGen(zone, freqVibLFO);
    DumpShortGen(zone, delayModEnv);
    DumpShortGen(zone, attackModEnv);
    DumpShortGen(zone, holdModEnv);
    DumpShortGen(zone, decayModEnv);
    DumpShortGen(zone, sustainModEnv);
    DumpShortGen(zone, releaseModEnv);
    DumpShortGen(zone, keynumToModEnvHold);
    DumpShortGen(zone, keynumToModEnvDecay);
    DumpShortGen(zone, delayVolEnv);
    DumpShortGen(zone, attackVolEnv);
    DumpShortGen(zone, holdVolEnv);
    DumpShortGen(zone, decayVolEnv);
    DumpShortGen(zone, sustainVolEnv);
    DumpShortGen(zone, releaseVolEnv);
    DumpShortGen(zone, keynumToVolEnvHold);
    DumpShortGen(zone, keynumToVolEnvDecay);

    if (zone->instrument)
        printf("  instrument: %d - %s\n", zone->gen.instrument, zone->instrument->name);

    DumpShortGen(zone, reserved1);

    if (ZoneHasGenerator(zone, SFGeneratorType_keyRange))
        printf("  keyRange: %d-%d\n", zone->gen.keyRange.low, zone->gen.keyRange.high);
    if (ZoneHasGenerator(zone, SFGeneratorType_velRange))
        printf("  velRange: %d-%d\n", zone->gen.velRange.low, zone->gen.velRange.high);

    DumpShortGen(zone, startloopAddrsCoarseOffset);
    DumpShortGen(zone, keynum);
    DumpShortGen(zone, velocity);
    DumpShortGen(zone, initialAttenuation);
    DumpShortGen(zone, reserved2);
    DumpShortGen(zone, endloopAddrsCoarseOffset);
    DumpShortGen(zone, coarseTune);
    DumpShortGen(zone, fineTune);

    if (zone->sample)
        printf("  sample: %d - %s\n", zone->gen.sampleID, zone->sample->name);

    if (ZoneHasGenerator(zone, SFGeneratorType_sampleModes))
        printf("  sampleModes: %d - %s\n", zone->gen.sampleModes,
                zone->gen.sampleModes == 1 ? "loops continuously" :
                zone->gen.sampleModes == 3 ? "loops for the duration of key depression then proceeds to play the remainder of the sample." :
                "no loop");

    DumpShortGen(zone, reserved3);
    DumpShortGen(zone, scaleTuning);
    DumpShortGen(zone, exclusiveClass);
    DumpShortGen(zone, overridingRootKey);
    DumpShortGen(zone, unused5);


#undef DumpShortGen
}

void SampleDump(Sample *sample)
{
    printf("[Sample] %s\n", sample->name);
    printf("------------------------------------------------------------------------------\n");
    printf("start: %d\n", sample->start);
    printf("startLoop: %d\n", sample->startLoop);
    printf("endLoop: %d\n", sample->endLoop);
    printf("end: %d\n", sample->end);
    printf("sampleRate: %d\n", sample->sampleRate);
    printf("originalPitch: %d\n", sample->originalPitch);
    printf("pitchCorrection: %d\n", sample->pitchCorrection);
    printf("sampleType: %d - %s\n", sample->sampleType,
            sample->sampleType == 1 ? "MONO" :
            sample->sampleType == 2 ? "R" :
            sample->sampleType == 4 ? "L" :
            "Unsuported");
    printf("\n");
}
