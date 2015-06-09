#include "Synthesizer.h"
#include "MidiSource.h"
#include "SoundFont.h"

#include <stdlib.h>
#include <string.h>

typedef struct _Preset Preset;
typedef struct _Instrument Instrument;
typedef struct _Generator Generator;
typedef struct _Sample Sample;
typedef struct _Voice Voice;

struct _Preset {
    const char *name;
    uint16_t midiPresetNo;
    uint16_t bankNo;

    Generator generator;

    Instrument *instruments;
    int instrumentsCount;
};

struct _Instrument {
    const char *name;
    Generator generator;

    struct {
        Sample L;
        Sample R;
    } sample;

    struct {
        bool loop;
        bool depression;
    } sampleMode;

    Preset *preset;
};

struct _Generator {
    struct {
        uint8_t low;
        uint8_t high;
    } keyRange;

    struct {
        uint8_t low;
        uint8_t high;
    } velocityRange;
};

struct _Sample {
    const char *name;
    uint32_t start;
    uint32_t startLoop;
    uint32_t endLoop;
    uint32_t end;
    uint32_t sampleRate;
    uint8_t originalPitch;
    int8_t pitchCorrection;
};

struct _Voice {
    Instrument *instrument;
    int64_t startedAt;
    uint8_t noteNo;
};



struct _Synthesizer {
    MidiSource srcVtbl;
    char *filepath;
    SoundFont *sf;
    Preset *presets;
    int presetsCount;
};

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

static Instrument *InstrumentCreate()
{
    return calloc(1, sizeof(Instrument));
}

static void InstrumentDestroy(Instrument *self)
{
    free(self);
}

static Preset *PresetCreate()
{
    return calloc(1, sizeof(Preset));
}

static void PresetDestroy(Preset *self)
{
    for (int i = 0; i < self->instrumentsCount; ++i) {
        InstrumentDestroy(self->instruments[i]);
    }
    free(self->instruments);

    free(self);
}

#define isValidRange(idx, length) (0 <= idx && idx < length)
#define isValidRange2(idx1, idx2, length) (0 <= idx1 && idx1 < idx2 && idx2 < length)

static bool InstrumentParseSample(Instrument *self, SoundFont *sf, int shdrIdx)
{
    // TODO
}

static bool InstrumentParseGenerator(Instrument *self, SoundFont *sf, SFInstBag *ibag, SFInstBag *ibagNext)
{
    if (!isValidRange2(ibag->wInstGenNdx, ibagNext->wInstGenNdx, sf->igenLength)) {
        return false;
    }

    for (int i = ibag->wInstGen; i < ibagNext->wInstGenNdx; ++i) {
        SFInstGenList *igen = &sf->igen[i];
        switch (igen->sfGenOper) {
        case SFGeneratorType_keyRange:
            self->generator.keyRange.low = igen->genAmount.ranges.byLo;
            self->generator.keyRange.high = igen->genAmount.ranges.byHi;
            break;
        case SFGeneratorType_velRange:
            self->generator.velocityRange.low = igen->genAmount.ranges.byLo;
            self->generator.velocityRange.high = igen->genAmount.ranges.byHi;
            break;
        case SFGeneratorType_sampleID:
            if (!InstrumentParseSample(self, sf, igen->genAmount.wAmount)) {
                return false;
            }
            break;
        case SFGeneratorType_sampleModes:
            switch (igen->sampleMode) {
            case SFSampleModeType_Continuously:
                self->sampleMode.loop = true;
                break;
            case SFSampleModeType_KeyDepression:
                self->sampleMode.loop = true;
                self->sampleMode.depression = true;
                break;
            }
            break;
        default:
            break;
        }
    }

    return true;
}

static void PresetAddInstrument(Preset *self, Instrument *preset)
{
    self->instruments = realloc(self->instruments, sizeof(Instrument *) * self->instrumentsCount + 1);
    self->instruments[self->instrumentsCount];
    ++self->instrumentsCount;
}

static bool PresetParseInstrument(Preset *self, SoundFont *sf, int instIdx)
{
    if (!isValidRange(instIdx + 1, sf->instLength)) {
        return false;
    }

    SFInst *inst = &sf->inst[instIdx];
    SFInst *instNext = &sf->inst[instIdx + 1];
    if (!isValidRange2(inst->wInstBagNdx, instNext->wInstBagNdx, sf->ibagLength)) {
        return false;
    }

    SFInstBag *ibag = &sf->ibag[inst->wInstBagNdx];
    SFInstBag *ibagNext = &sf->ibag[instNext->wInstBagNdx];

    Instrument *instrument = InstrumentCreate();
    instrument->preset = self;
    instrument->name = inst->achInstName;

    if (!InstrumentParseGenerator(instrument, sf, ibag, ibagNext)) {
        InstrumentDestroy(instrument);
        return false;
    }
    else {
        PresetAddInstrument(self, instrument);
        return true;
    }
}

static bool PresetParseGenerator(Preset *self, SoundFont *sf, SFPresetBag *pbag, SFPresetBag *pbagNext)
{
    if (!isValidRange2(pbag->wGenNdx, pbagNext->wGenNdx, sf->pgenLength)) {
        return false;
    }

    for (int i = pbag->wGenNdx; i < pbagNext->wGenNdx; ++i) {
        SFGenList *pgen = &sf->pgen[i];
        switch (pgen->sfGenOper) {
        case SFGeneratorType_keyRange:
            self->generator.keyRange.low = pgen->genAmount.ranges.byLo;
            self->generator.keyRange.high = pgen->genAmount.ranges.byHi;
            break;
        case SFGeneratorType_velRange:
            self->generator.velocityRange.low = pgen->genAmount.ranges.byLo;
            self->generator.velocityRange.high = pgen->genAmount.ranges.byHi;
            break;
        case SFGeneratorType_instrument:
            if (!PresetParseInstrument(self, sf, pgen->genAmount.wAmount)) {
                return false;
            }
            break;
        default:
            break;
        }
    }

    return true;
}

static void SynthesizerAddPreset(Synthesizer *self, Preset *preset)
{
    self->presets = realloc(self->presets, sizeof(Preset *) * self->presetsCount + 1);
    self->presets[self->presetsCount] = preset;
    ++self->presetsCount;
}

static bool SynthesizerParsePresets(Synthesizer *self)
{
    for (int i = 0; i < self->sf->phdrLength - 1; ++i) {
        SFPresetHeader *phdr = &self->sf->phdr[i];
        SFPresetHeader *phdrNext = &self->sf->phdr[i + 1];

        if (!isValidRange2(phdr->wPresetBagNdx, phdrNext->wPresetBagNdx, self->sf->pbagLength)) {
            return false;
        }

        SFPresetBag *pbag = &self->sf->pbag[phdr->wPresetBagNdx];
        SFPresetBag *pbagNext = &self->sf->pbag[phdrNext->wPresetBagNdx];

        Preset *preset = calloc(1, sizeof(Preset));
        preset->name = phdr->achPresetName;
        preset->midiPresetNo = phdr->wPreset;
        preset->bankNo = phdr->wPreset;
        
        if (!PresetParseGenerator(preset, self->sf, pbag, pbagNext)) {
            PresetDestroy(preset);
        }
        else {
            SynthesizerAddPreset(self, preset);
        }
    }

    return true;
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

    SynthesizerParsePresets(self);

    return self;
}

void SynthesizerDestroy(Synthesizer *self)
{
    for (int i = 0; i < self->presetsCount; ++i) {
        PresetDestroy(self->presets[i]);
    }
    free(self->presets);

    SoundFontDestroy(self->sf);
    free(self->filepath);
    free(self);
}
