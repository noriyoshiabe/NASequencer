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
    Zone *self = calloc(1, sizeof(Zone));

    self->gen.range.key.low = 0;
    self->gen.range.key.high = 127;
    self->gen.range.velocity.low = 0;
    self->gen.range.velocity.high = 127;
    
    self->gen.substitution.keynum = -1;
    self->gen.substitution.velocity = -1;

    self->gen.sample.overridingRootKey = -1;

    return self;
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
        case SFGeneratorType_startAddrsOffset:
        case SFGeneratorType_endAddrsOffset:
        case SFGeneratorType_startloopAddrsOffset:
        case SFGeneratorType_endloopAddrsOffset:
        case SFGeneratorType_startAddrsCoarseOffset:
        case SFGeneratorType_modLfoToPitch:
        case SFGeneratorType_vibLfoToPitch:
        case SFGeneratorType_modEnvToPitch:
        case SFGeneratorType_initialFilterFc:
        case SFGeneratorType_initialFilterQ:
        case SFGeneratorType_modLfoToFilterFc:
        case SFGeneratorType_modEnvToFilterFc:
        case SFGeneratorType_endAddrsCoarseOffset:
        case SFGeneratorType_modLfoToVolume:
            // TODO
            break;

        case SFGeneratorType_unused1:
            break;

        case SFGeneratorType_chorusEffectsSend:
        case SFGeneratorType_reverbEffectsSend:
        case SFGeneratorType_pan:
            // TODO
            break;

        case SFGeneratorType_unused2:
        case SFGeneratorType_unused3:
        case SFGeneratorType_unused4:
            break;

        case SFGeneratorType_delayModLFO:
        case SFGeneratorType_freqModLFO:
        case SFGeneratorType_delayVibLFO:
        case SFGeneratorType_freqVibLFO:
        case SFGeneratorType_delayModEnv:
        case SFGeneratorType_attackModEnv:
        case SFGeneratorType_holdModEnv:
        case SFGeneratorType_decayModEnv:
        case SFGeneratorType_sustainModEnv:
        case SFGeneratorType_releaseModEnv:
        case SFGeneratorType_keynumToModEnvHold:
        case SFGeneratorType_keynumToModEnvDecay:
        case SFGeneratorType_delayVolEnv:
        case SFGeneratorType_attackVolEnv:
        case SFGeneratorType_holdVolEnv:
        case SFGeneratorType_decayVolEnv:
        case SFGeneratorType_sustainVolEnv:
        case SFGeneratorType_releaseVolEnv:
        case SFGeneratorType_keynumToVolEnvHold:
        case SFGeneratorType_keynumToVolEnvDecay:
            // TODO
            break;

        case SFGeneratorType_instrument:
            if (!ParseInstrument(sf, gen->genAmount.wAmount, &self->instrument)) {
                return false;
            }
            break;

        case SFGeneratorType_reserved1:
            break;

        case SFGeneratorType_keyRange:
            self->gen.range.key.low = gen->genAmount.ranges.byLo;
            self->gen.range.key.high = gen->genAmount.ranges.byHi;
            break;

        case SFGeneratorType_velRange:
            self->gen.range.velocity.low = gen->genAmount.ranges.byLo;
            self->gen.range.velocity.high = gen->genAmount.ranges.byHi;
            break;

        case SFGeneratorType_startloopAddrsCoarseOffset:
        case SFGeneratorType_keynum:
        case SFGeneratorType_velocity:
        case SFGeneratorType_initialAttenuation:
            // TODO
            break;

        case SFGeneratorType_reserved2:
            break;

        case SFGeneratorType_endloopAddrsCoarseOffset:
        case SFGeneratorType_coarseTune:
        case SFGeneratorType_fineTune:
            // TODO
            break;

        case SFGeneratorType_sampleID:
            if (!ParseSample(sf, gen->genAmount.wAmount, &self->sample)) {
                return false;
            }
            break;

        case SFGeneratorType_sampleModes:
            switch (gen->genAmount.wAmount) {
            case SFSampleModeType_Continuously:
                self->gen.sample.loop = true;
                break;
            case SFSampleModeType_UntilRelease:
                self->gen.sample.loop = true;
                self->gen.sample.untilRelease = true;
                break;
            }
            break;

        case SFGeneratorType_reserved3:
            break;

        case SFGeneratorType_scaleTuning:
        case SFGeneratorType_exclusiveClass:
        case SFGeneratorType_overridingRootKey:
            // TODO
            break;

        case SFGeneratorType_unused5:
        case SFGeneratorType_endOper:
            break;
        }
    }

    return true;
}

static bool ZoneParseModulator(Zone *self, SoundFont *sf, SFModList *modulators, int modulatorCount)
{
    // TODO
    return true;
}

static void ZoneDump(Zone *zone);

#define iprintf(indent, ...) do { for (int __i = 0; __i < indent; ++__i) putc(' ', stdout); printf(__VA_ARGS__); } while (0)

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
            iprintf(2, "-----------------------------\n");
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
            iprintf(2, "-----------------------------\n");
        }
    }
    printf("\n");
}

static void ZoneDump(Zone *zone)
{
    iprintf(2, "gen.range.key: %d-%d\n", zone->gen.range.key.low, zone->gen.range.key.high);
    iprintf(2, "gen.range.velocity: %d-%d\n", zone->gen.range.velocity.low, zone->gen.range.velocity.high);

    if (zone->instrument) {
        iprintf(2, "instrument: %s\n", zone->instrument->name);
    }

    if (zone->sample) {
        iprintf(2, "gen.sample.loop: %s\n", zone->gen.sample.loop ? "true" : "false");
        iprintf(2, "gen.sample.untilRelease: %s\n", zone->gen.sample.untilRelease ? "true" : "false");
        iprintf(2, "sample: %s\n", zone->sample->name);
    }
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
    printf("sampleType: %s %04X\n",
            sample->sampleType == 1 ? "MONO" :
            sample->sampleType == 2 ? "R" :
            sample->sampleType == 4 ? "L" :
            "Unsuported",
            sample->sampleType);
    printf("\n");
}
