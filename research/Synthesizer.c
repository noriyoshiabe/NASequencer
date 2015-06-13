#include "Synthesizer.h"
#include "MidiSource.h"
#include "SoundFont.h"
#include "Preset.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _Voice {
    int64_t startedAt;
    uint8_t noteNo;
} Voice;

struct _Synthesizer {
    MidiSource srcVtbl;
    char *filepath;
    SoundFont *sf;
    Preset **presets;
    int presetCount;
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

static void GeneratorInitialize(Generator *self)
{
    self->keyRange.low = 0;
    self->keyRange.high = 127;
    self->velocityRange.low = 0;
    self->velocityRange.high = 127;
}

static Instrument *InstrumentCreate()
{
    Instrument *self = calloc(1, sizeof(Instrument));
    GeneratorInitialize(&self->generator);
    return self;
}

static void InstrumentDestroy(Instrument *self)
{
    if (self->sample.L != self->sample.R) {
        if (self->sample.R) {
            free(self->sample.R);
        }
    }

    if (self->sample.L) {
        free(self->sample.L);
    }

    free(self);
}

static Preset *PresetCreate()
{
    Preset *self = calloc(1, sizeof(Preset));
    GeneratorInitialize(&self->generator);
    return self;
}

static void PresetDestroy(Preset *self)
{
    for (int i = 0; i < self->instrumentsCount; ++i) {
        InstrumentDestroy(self->instruments[i]);
    }
    free(self->instruments);

    free(self);
}

static bool SampleFromSampleHeader(SFSampleHeader *shdr, Sample **sample)
{
    if (*sample) {
        return false;
    }

    Sample *self = calloc(1, sizeof(Sample));
    self->name = shdr->achSampleName;
    self->start = shdr->dwStart;
    self->startLoop = shdr->dwStartloop;
    self->endLoop = shdr->dwEndloop;
    self->end = shdr->dwEnd;
    self->sampleRate = shdr->dwSampleRate;
    self->originalPitch = shdr->byOriginalPitch;
    self->pitchCorrection = shdr->chPitchCorrection;

    *sample = self;
    return true;
}

static void SampleDestroy(Sample *self)
{
    free(self);
}

static bool InstrumentAdjustSampleOffset(Instrument *self, Sample *sample)
{
    // TODO add Generator's sample offset
    return true;
}

static bool InstrumentParseSample(Instrument *self, SoundFont *sf, int shdrIdx)
{
    if (!isValidRange(shdrIdx, sf->shdrLength)) {
        return false;
    }

    SFSampleHeader *shdr = &sf->shdr[shdrIdx];
    if (isRomSample(shdr->sfSampleType)) {
        return false;
    }

    switch (shdr->sfSampleType) {
    case SFSampleLinkType_monoSample:
        if (!SampleFromSampleHeader(shdr, &self->sample.L)) {
            return false;
        }
        if (!InstrumentAdjustSampleOffset(self, self->sample.L)) {
            return false;
        }
        if (self->sample.R) {
            return false;
        }
        self->sample.R = self->sample.L;
        break;
    case SFSampleLinkType_rightSample:
        if (!SampleFromSampleHeader(shdr, &self->sample.R)) {
            return false;
        }
        if (!InstrumentAdjustSampleOffset(self, self->sample.R)) {
            return false;
        }
        if (!self->sample.L) {
            return InstrumentParseSample(self, sf, shdr->wSampleLink);
        }
        break;
    case SFSampleLinkType_leftSample:
        if (!SampleFromSampleHeader(shdr, &self->sample.L)) {
            return false;
        }
        if (!InstrumentAdjustSampleOffset(self, self->sample.L)) {
            return false;
        }
        if (!self->sample.R) {
            return InstrumentParseSample(self, sf, shdr->wSampleLink);
        }
        break;
    }

    return true;
}

static bool InstrumentParseGenerator(Instrument *self, SoundFont *sf, SFInstBag *ibag, SFInstBag *ibagNext)
{
    if (!isValidRange2(ibag->wInstGenNdx, ibagNext->wInstGenNdx, sf->igenLength)) {
        return false;
    }

    for (int i = ibag->wInstGenNdx; i < ibagNext->wInstGenNdx; ++i) {
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

            // 8.1.2 Generator Enumerators Defined
            // The sampleID enumerator is the terminal generator for IGEN zones.
            return true;

        case SFGeneratorType_sampleModes:
            switch (igen->genAmount.wAmount) {
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

static void PresetAddInstrument(Preset *self, Instrument *instrument)
{
    self->instruments = realloc(self->instruments, sizeof(Instrument *) * self->instrumentsCount + 1);
    self->instruments[self->instrumentsCount] = instrument;
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
# if 0 // TODO global zone?
    if (!isValidRange2(pbag->wGenNdx, pbagNext->wGenNdx, sf->pgenLength)) {
        return false;
    }
#endif

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

            // 8.1.2 Generator Enumerators Defined
            // The instrument enumerator is the terminal generator for PGEN zones.
            return true;

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

        for (SFPresetBag *ite = pbag; ite != pbagNext; ++ite) {
            if (!PresetParseGenerator(preset, self->sf, ite, ite + 1)) {
                goto SKIP;
            }
        }
    
        SynthesizerAddPreset(self, preset);
        continue;

SKIP:
        PresetDestroy(preset);
    }

    return true;
}

static void SynthesizerDumpPresets(Synthesizer *self)
{
#define iprintf(indent, ...) do { for (int __i = 0; __i < indent; ++__i) putc(' ', stdout); printf(__VA_ARGS__); } while (0)

    for (int i = 0; i < self->presetsCount; ++i) {
        Preset *preset = self->presets[i];
        printf("[Preset]\n");
        printf("----------------------\n");
        printf("name: %s\n", preset->name);
        printf("midiPresetNo: %d\n", preset->midiPresetNo);
        printf("bankNo: %d\n", preset->bankNo);
        printf("generator.keyRange.low: %d\n", preset->generator.keyRange.low);
        printf("generator.keyRange.high: %d\n", preset->generator.keyRange.high);
        printf("generator.velocityRange.low: %d\n", preset->generator.velocityRange.low);
        printf("generator.velocityRange.high: %d\n", preset->generator.velocityRange.high);

        for (int j = 0; j < preset->instrumentsCount; ++j) {
            Instrument *instrument = preset->instruments[j];
            iprintf(2, "[Instrument]\n");
            iprintf(2, "----------------------\n");
            iprintf(2, "name: %s\n", instrument->name);
            iprintf(2, "generator.keyRange.low: %d\n", instrument->generator.keyRange.low);
            iprintf(2, "generator.keyRange.high: %d\n", instrument->generator.keyRange.high);
            iprintf(2, "generator.velocityRange.low: %d\n", instrument->generator.velocityRange.low);
            iprintf(2, "generator.velocityRange.high: %d\n", instrument->generator.velocityRange.high);
            iprintf(2, "sampleMode.loop: %s\n", instrument->sampleMode.loop ? "true" : "false");
            iprintf(2, "sampleMode.depression: %s\n", instrument->sampleMode.depression ? "true" : "false");

#if 0
            bool mono = instrument->sample.L == instrument->sample.R;
            if (mono) {
                iprintf(4, "[Sample MONO]\n");
            }
            else {
                iprintf(4, "[Sample L]\n");
            }

            iprintf(4, "----------------------\n");
            iprintf(4, "name: %s\n", instrument->sample.L->name);
            iprintf(4, "start: %d\n", instrument->sample.L->start);
            iprintf(4, "startLoop: %d\n", instrument->sample.L->startLoop);
            iprintf(4, "endLoop: %d\n", instrument->sample.L->endLoop);
            iprintf(4, "end: %d\n", instrument->sample.L->end);
            iprintf(4, "sampleRate: %d\n", instrument->sample.L->sampleRate);
            iprintf(4, "originalPitch: %d\n", instrument->sample.L->originalPitch);
            iprintf(4, "pitchCorrection: %d\n", instrument->sample.L->pitchCorrection);

            if (!mono) {
                iprintf(6, "[Sample R]\n");
                iprintf(6, "----------------------\n");
                iprintf(6, "name: %s\n", instrument->sample.R->name);
                iprintf(6, "start: %d\n", instrument->sample.R->start);
                iprintf(6, "startLoop: %d\n", instrument->sample.R->startLoop);
                iprintf(6, "endLoop: %d\n", instrument->sample.R->endLoop);
                iprintf(6, "end: %d\n", instrument->sample.R->end);
                iprintf(6, "sampleRate: %d\n", instrument->sample.R->sampleRate);
                iprintf(6, "originalPitch: %d\n", instrument->sample.R->originalPitch);
                iprintf(6, "pitchCorrection: %d\n", instrument->sample.R->pitchCorrection);
            }
#endif
        }
    }

    printf("\n");

#undef iprintf
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
    SynthesizerDumpPresets(self);

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
