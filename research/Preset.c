#include "Preset.h"

#include <stdio.h>
#include <stdlib.h>

#define isValidRange(idx, length) (0 <= idx && idx < length)
#define isValidRangeWithAsc(idx1, idx2, length) (0 <= idx1 && idx1 < idx2 && idx2 < length)
#define isRomSample(sampleType) (0 != (sampleType & 0x8000))

static bool ParsePreset(SoundFont *sf, int presetIdx, Preset **result);
static Zone *ZoneCreate();
static void ZoneDestroy(Zone *self);
static void PresetZoneDestroy(Zone *self);
static void InstrumentZoneDestroy(Zone *self);
static bool ZoneParseGenerator(Zone *self, SoundFont *sf, SFGenList *generators, int generatorCount);
static bool ZoneParseModulator(Zone *self, SoundFont *sf, SFModList *modulators, int modulatorCount);
static bool ParseInstrument(SoundFont *sf, int instIdx, Instrument **result);
static void InstrumentDestroy(Instrument *self);
static bool ParseSample(SoundFont *sf, int shdrIdx, Sample **resultL, Sample **resultR);
static void ParseSampleHeader(SFSampleHeader *shdr, Sample **sample);
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
    preset->bankNo = phdr->wPreset;

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
            PresetZoneDestroy(zone);
            goto ERROR;
        }

        if (!ZoneParseModulator(zone, sf, pmod, modulatorCount)) {
            PresetZoneDestroy(zone);
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
                    PresetZoneDestroy(zone);
                }
            }
            else {
                PresetZoneDestroy(zone);
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
        PresetZoneDestroy(self->zones[i]);
    }

    free(self->zones);
    free(self);
}

static Zone *ZoneCreate()
{
    Zone *self = calloc(1, sizeof(Zone));

    self->keyRange.low = 0;
    self->keyRange.high = 127;
    self->velocityRange.low = 0;
    self->velocityRange.high = 127;

    return self;
}

static void ZoneDestroy(Zone *self)
{
    free(self);
}

static void PresetZoneDestroy(Zone *self)
{
    if (self->instrument) {
        InstrumentDestroy(self->instrument);
    }

    ZoneDestroy(self);
}

static void InstrumentZoneDestroy(Zone *self)
{
    if (self->sample.L) {
        bool stereo = self->sample.L != self->sample.R;
        SampleDestroy(self->sample.L);
        if (stereo) {
            SampleDestroy(self->sample.R);
        }
    }

    ZoneDestroy(self);
}

static bool ZoneParseGenerator(Zone *self, SoundFont *sf, SFGenList *generators, int generatorCount)
{
    for (int i = 0; i < generatorCount; ++i) {
        SFGenList *gen = &generators[i];

        switch (gen->sfGenOper) {
        case SFGeneratorType_keyRange:
            self->keyRange.low = gen->genAmount.ranges.byLo;
            self->keyRange.high = gen->genAmount.ranges.byHi;
            break;
        case SFGeneratorType_velRange:
            self->velocityRange.low = gen->genAmount.ranges.byLo;
            self->velocityRange.high = gen->genAmount.ranges.byHi;
            break;
        case SFGeneratorType_instrument:
            if (!ParseInstrument(sf, gen->genAmount.wAmount, &self->instrument)) {
                return false;
            }
            break;
        case SFGeneratorType_sampleID:
            if (!ParseSample(sf, gen->genAmount.wAmount, &self->sample.L, &self->sample.R)) {
                return false;
            }
            break;
        case SFGeneratorType_sampleModes:
            switch (gen->genAmount.wAmount) {
            case SFSampleModeType_Continuously:
                self->sample.loop = true;
                break;
            case SFSampleModeType_KeyDepression:
                self->sample.loop = true;
                self->sample.depression = true;
                break;
            }
            break;
        default:
            // TODO other articulations and generators
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
            PresetZoneDestroy(zone);
            goto ERROR;
        }

        if (!ZoneParseModulator(zone, sf, imod, modulatorCount)) {
            PresetZoneDestroy(zone);
            goto ERROR;
        }

        if (!zone->sample.L) {
            // 7.7 The IBAG Sub-chunk
            // If a zone other than the first zone lacks a sampleID generator as its last generator, that zone should be ignored.
            // A global zone with no modulators and no generators should also be ignored.

            if (0 == i) {
                if (0 < generatorCount || 0 < modulatorCount) {
                    instrument->globalZone = zone;
                }
                else {
                    InstrumentZoneDestroy(zone);
                }
            }
            else {
                InstrumentZoneDestroy(zone);
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
        InstrumentZoneDestroy(self->zones[i]);
    }

    free(self->zones);
    free(self);
}

static bool ParseSample(SoundFont *sf, int shdrIdx, Sample **resultL, Sample **resultR)
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

    Sample *L = NULL;
    Sample *R = NULL;

    switch (shdr->sfSampleType) {
    case SFSampleLinkType_monoSample:
        ParseSampleHeader(shdr, &L);
        R = L;
        break;
    case SFSampleLinkType_rightSample:
        ParseSampleHeader(shdr, &R);
        if (!isValidRange(shdr->wSampleLink, sf->shdrLength)) {
            goto ERROR;
        }
        else {
            SFSampleHeader *shdrL = &sf->shdr[shdr->wSampleLink];
            if (isRomSample(shdrL->sfSampleType)) {
                goto ERROR;
            }

            ParseSampleHeader(shdrL, &L);
        }
        break;
    case SFSampleLinkType_leftSample:
        ParseSampleHeader(shdr, &L);
        if (!isValidRange(shdr->wSampleLink, sf->shdrLength)) {
            goto ERROR;
        }
        else {
            SFSampleHeader *shdrR = &sf->shdr[shdr->wSampleLink];
            if (isRomSample(shdrR->sfSampleType)) {
                goto ERROR;
            }

            ParseSampleHeader(shdrR, &R);
        }
        break;
    case SFSampleLinkType_linkedSample:
        // 7.10 The SHDR Sub-chunk
        // The linked sample type is not currently fully defined in the SoundFont 2 specification,
        // but will ultimately support a circularly linked list of samples using wSampleLink.
        break;
    }

    *resultL = L;
    *resultR = R;

    return true;

ERROR:
    if (L) {
        SampleDestroy(L);
    }

    if (R) {
        SampleDestroy(R);
    }

    return false;
}

static void ParseSampleHeader(SFSampleHeader *shdr, Sample **result)
{
    Sample *sample = calloc(1, sizeof(Sample));
    sample->name = shdr->achSampleName;
    sample->start = shdr->dwStart;
    sample->startLoop = shdr->dwStartloop;
    sample->endLoop = shdr->dwEndloop;
    sample->end = shdr->dwEnd;
    sample->sampleRate = shdr->dwSampleRate;
    sample->originalPitch = shdr->byOriginalPitch;
    sample->pitchCorrection = shdr->chPitchCorrection;

    *result = sample;
}

static void SampleDestroy(Sample *self)
{
    free(self);
}

#define iprintf(indent, ...) do { for (int __i = 0; __i < indent; ++__i) putc(' ', stdout); printf(__VA_ARGS__); } while (0)

static void PresetZoneDump(Zone *zone);
static void ZoneDump(Zone *zone, int indent);
static void InstrumentDump(Instrument *instrument);
static void InstrumentZoneDump(Zone *zone);

void PresetDump(Preset *preset)
{
    printf("[Preset] %s\n", preset->name);
    printf("------------------------------------------------------------------------------\n");
    printf("name: %s\n", preset->name);
    printf("midiPresetNo: %d\n", preset->midiPresetNo);
    printf("bankNo: %d\n", preset->bankNo);
    printf("globalZone:\n");

    if (preset->globalZone) {
        PresetZoneDump(preset->globalZone);
    }

    printf("zones: zoneCount=%d\n", preset->zoneCount);
    for (int i = 0; i < preset->zoneCount; ++i) {
        PresetZoneDump(preset->zones[i]);
    }

    printf("\n");
}

static void PresetZoneDump(Zone *zone)
{
    ZoneDump(zone, 2);
    if (zone->instrument) {
        iprintf(2, "instrument:\n");
        InstrumentDump(zone->instrument);
    }
}

static void ZoneDump(Zone *zone, int indent)
{
    iprintf(indent, "------------------------\n");
    iprintf(indent, "keyRange: %d-%d\n", zone->keyRange.low, zone->keyRange.high);
    iprintf(indent, "velocityRange: %d-%d\n", zone->velocityRange.low, zone->velocityRange.high);
}

static void InstrumentDump(Instrument *instrument)
{
    iprintf(4, "name: %s\n", instrument->name);
    iprintf(4, "globalZone:\n");

    if (instrument->globalZone) {
        InstrumentZoneDump(instrument->globalZone);
    }

    iprintf(4, "zones: zoneCount=%d\n", instrument->zoneCount);
    for (int i = 0; i < instrument->zoneCount; ++i) {
        InstrumentZoneDump(instrument->zones[i]);
    }
}

static void InstrumentZoneDump(Zone *zone)
{
    ZoneDump(zone, 6);
    if (zone->sample.L) {
        iprintf(6, "sample:\n");
        iprintf(8, "loop: %s\n", zone->sample.loop ? "true" : "false");
        iprintf(8, "depression: %s\n", zone->sample.depression ? "true" : "false");
        iprintf(8, "stereo: %s\n", zone->sample.L != zone->sample.R ? "true" : "false");
        iprintf(8, "%s: %s\n", zone->sample.L != zone->sample.R ? "L" : "MONO", zone->sample.L->name);
        if (zone->sample.L != zone->sample.R) {
            iprintf(8, "R: %s\n", zone->sample.R->name);
        }
    }
}
