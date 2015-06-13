#include "Preset.h"

#define isValidRange(idx, length) (0 <= idx && idx < length)
#define isValidRange2(idx1, idx2, length) (0 <= idx1 && idx1 < idx2 && idx2 < length)
#define isRomSample(sampleType) (0 != (sampleType & 0x8000))

static bool ParsePreset(SoundFont *sf, int presetIdx, Preset **result);
static void PresetDestroy(Preset *self);
static void ZoneDestroy(Zone *self);
static void PresetZoneDestroy(Zone *self);
static void InstrumentZoneDestroy(Zone *self);
static bool ZoneParseGenerator(Zone *self, SoundFont *sf, SFGenList *generators, int generatorCount);
static bool ZoneParseModulator(Zone *self, SoundFont *sf, SFModList *modulators, int modulatorCount);
static bool ParseInstrument(SoundFont *sf, int instIdx, Instrument **result);
static void InstrumentDestroy(Instrument *self);
static bool ParseSampleInfo(SoundFont *sf, int shdrIdx, SampleInfo **result);
static void InstrumentDestroy(Instrument *self);

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
    if (!isValidRange2(phdr->wPresetBagNdx, phdrNext->wPresetBagNdx, sf->pbagLength)) {
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

        Zone *zone = calloc(1, sizeof(Zone));
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
            
            if (i = 0) {
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


static void PresetDestroy(Preset *self)
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

static void ZoneDestroy(Zone *self)
{
    free(zone);
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
    if (self->sampleInfo) {
        SampleInfoDestroy(self->sampleInfo);
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
            if (!ParseSampleInfo(sf, gen->genAmount.wAmount, &self->sampleInfo)) {
                return false;
            }
            break;
        default:
            // TODO other articulations
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

static bool ParseInstrument(SoundFont *sf, int instIdx, Instrument **result);
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
    if (!isValidRange2(inst->wInstBagNdx, instNext->wInstBagNdx, sf->ibagLength)) {
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

        if (!ZoneParseGenerator(zone, sf, igen, generatorCount)) {
            PresetZoneDestroy(zone);
            goto ERROR;
        }

        if (!ZoneParseModulator(zone, sf, imod, modulatorCount)) {
            PresetZoneDestroy(zone);
            goto ERROR;
        }

        if (!zone->sampleInfo) {
            // 7.7 The IBAG Sub-chunk
            // If a zone other than the first zone lacks a sampleID generator as its last generator, that zone should be ignored.
            // A global zone with no modulators and no generators should also be ignored.

            if (i = 0) {
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

    if (!instrument->globalZone && 1 > instrument->zonesCount) {
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
