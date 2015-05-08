#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct _SMFWriter SMFWriter;

extern SMFWriter *SMFWriterCreate(const char *filepath);
extern void SMFWriterDestroy(SMFWriter *self);

extern void SMFWriterSetResolution(SMFWriter *self, uint16_t resolution);
extern void SMFWriterAppendTime(SMFWriter *self, int32_t tick, uint8_t numerator, uint8_t denominator);
extern void SMFWriterAppendTempo(SMFWriter *self, int32_t tick, float tempo);
extern void SMFWriterAppendMarker(SMFWriter *self, int32_t tick, const char *text);
extern void SMFWriterAppendNoteOn(SMFWriter *self, int32_t tick, uint8_t channel, uint8_t noteNo, uint8_t velocity);
extern void SMFWriterAppendNoteOff(SMFWriter *self, int32_t tick, uint8_t channel, uint8_t noteNo, uint8_t velocity);
extern void SMFWriterAppendSound(SMFWriter *self, int32_t tick, uint8_t channel, uint8_t msb, uint8_t lsb, uint8_t programNo);

extern bool SMFWriterSerialize(SMFWriter *self);
