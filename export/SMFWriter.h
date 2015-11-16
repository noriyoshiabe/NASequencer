#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct _SMFWriter SMFWriter;

extern SMFWriter *SMFWriterCreate();
extern void SMFWriterDestroy(SMFWriter *self);

extern bool SMFWriterOpenFile(SMFWriter *self, const char *filepath);
extern void SMFWriterSetResolution(SMFWriter *self, uint16_t resolution);
extern void SMFWriterAppendTime(SMFWriter *self, int32_t tick, uint8_t numerator, uint8_t denominator);
extern void SMFWriterAppendTempo(SMFWriter *self, int32_t tick, float tempo);
extern void SMFWriterAppendKey(SMFWriter *self, int32_t tick, uint8_t sf, uint8_t mi);
extern void SMFWriterAppendTitle(SMFWriter *self, int32_t tick, const char *text);
extern void SMFWriterAppendMarker(SMFWriter *self, int32_t tick, const char *text);
extern void SMFWriterAppendNoteOn(SMFWriter *self, int32_t tick, uint8_t channel, uint8_t noteNo, uint8_t velocity);
extern void SMFWriterAppendNoteOff(SMFWriter *self, int32_t tick, uint8_t channel, uint8_t noteNo, uint8_t velocity);
extern void SMFWriterAppendControlChange(SMFWriter *self, int32_t tick, uint8_t channel, uint8_t ccNo, uint8_t value);
extern void SMFWriterAppendProgramChange(SMFWriter *self, int32_t tick, uint8_t channel, uint8_t programNo);

extern bool SMFWriterSerialize(SMFWriter *self);
