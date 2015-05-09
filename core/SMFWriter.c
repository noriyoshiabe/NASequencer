#include "SMFWriter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ALLOCATE_UNIT 1024

typedef struct _Track {
    uint8_t *buffer;
    size_t length;
    size_t allocatedCount;
    int32_t lastTick;
} Track;

struct _SMFWriter {
    const char *filepath;
    uint16_t resolution;
    Track tracks[17];
    int trackNum;
};

SMFWriter *SMFWriterCreate(const char *filepath)
{
    SMFWriter *ret = calloc(1, sizeof(SMFWriter));
    ret->filepath = filepath;
    return ret;
}

void SMFWriterDestroy(SMFWriter *self)
{
    for (int i = 0; i < 17; ++i) {
        if (self->tracks[i].buffer) {
            free(self->tracks[i].buffer);
        }
    }

    free(self);
}

void SMFWriterSetResolution(SMFWriter *self, uint16_t resolution)
{
    self->resolution = resolution;
}

static void writeToTrack(SMFWriter *self, uint8_t channel, const uint8_t *bytes, size_t size)
{
    Track *track = &self->tracks[channel];
    if (!track->buffer) {
        track->buffer = malloc(ALLOCATE_UNIT * ++track->allocatedCount);
        ++self->trackNum;
    }

    if (track->allocatedCount * ALLOCATE_UNIT < track->length + size) {
        track->buffer = realloc(track->buffer, ALLOCATE_UNIT * ++track->allocatedCount);
    }

    memcpy(track->buffer + track->length, bytes, size);
    track->length += size;
}

static void flexibleLength(size_t length, uint8_t *bytes, size_t *size)
{
    if (0x1FFFFF < length) {
      bytes[0] = (0x80 | (0x000000FF & (length >> 21)));
      bytes[1] = (0x80 | (0x000000FF & (length >> 14)));
      bytes[2] = (0x80 | (0x000000FF & (length >> 7)));
      bytes[3] = ((~0x80) & (0x000000FF & length));
      *size = 4;
    }
    else if (0x3FFF < length) {
      bytes[0] = (0x80 | (0x000000FF & (length >> 14)));
      bytes[1] = (0x80 | (0x000000FF & (length >> 7)));
      bytes[2] = ((~0x80) & (0x000000FF & length));
      *size = 3;
    }
    else if (0x7F < length) {
      bytes[0] = (0x80 | (0x000000FF & (length >> 7)));
      bytes[1] = ((~0x80) & (0x000000FF & length));
      *size = 2;
    }
    else {
      bytes[0] = ((~0x80) & (0x000000FF & length));
      *size = 1;
    }
}

static void writeDeltaTime(SMFWriter *self, uint8_t channel, int32_t tick)
{
    Track *track = &self->tracks[channel];
    uint32_t delta = tick - track->lastTick;
    track->lastTick = tick;

    size_t size;
    uint8_t bytes[4];
    flexibleLength(delta, bytes, &size);

    writeToTrack(self, channel, bytes, size);
}

void SMFWriterAppendTime(SMFWriter *self, int32_t tick, uint8_t numerator, uint8_t denominator)
{
    writeDeltaTime(self, 0, tick);

    uint8_t denominatorPowerOfMinus = 0;
    while (1 < denominator) {
        denominator /= 2;
        ++denominatorPowerOfMinus;
    }

    uint8_t bytes[7] = {0xFF, 0x58, 0x04, numerator, denominatorPowerOfMinus, 0x18, 0x08};
    writeToTrack(self, 0, bytes, sizeof(bytes));
}

void SMFWriterAppendTempo(SMFWriter *self, int32_t tick, float tempo)
{
    writeDeltaTime(self, 0, tick);

    uint32_t micro = 60000000 / tempo;
    uint8_t bytes[6] = {
        0xFF, 0x51, 0x03,
        0x000000FF & (micro >> 16),
        0x000000FF & (micro >> 8),
        0x000000FF & micro
    };

    writeToTrack(self, 0, bytes, sizeof(bytes));
}

void SMFWriterAppendMarker(SMFWriter *self, int32_t tick, const char *text)
{
    writeDeltaTime(self, 0, tick);

    uint8_t bytes[2] = {0xFF, 0x06};
    writeToTrack(self, 0, bytes, sizeof(bytes));

    uint8_t lengthBytes[4];
    size_t size;
    size_t textLength = strlen(text);
    flexibleLength(textLength, lengthBytes, &size);
    writeToTrack(self, 0, lengthBytes, sizeof(lengthBytes));
    writeToTrack(self, 0, (uint8_t *)text, textLength);
}

void SMFWriterAppendNoteOn(SMFWriter *self, int32_t tick, uint8_t channel, uint8_t noteNo, uint8_t velocity)
{
    writeDeltaTime(self, channel, tick);

    uint8_t bytes[3] = {0x90 | (channel - 1), noteNo, velocity};
    writeToTrack(self, channel, bytes, sizeof(bytes));
}

void SMFWriterAppendNoteOff(SMFWriter *self, int32_t tick, uint8_t channel, uint8_t noteNo, uint8_t velocity)
{
    writeDeltaTime(self, channel, tick);

    uint8_t bytes[3] = {0x80 | (channel - 1), noteNo, 0x00};
    writeToTrack(self, channel, bytes, sizeof(bytes));
}

void SMFWriterAppendSound(SMFWriter *self, int32_t tick, uint8_t channel, uint8_t msb, uint8_t lsb, uint8_t programNo)
{
    uint8_t msbBytes[3] = {0xB0 | (channel - 1), 0x00, msb};
    uint8_t lsbBytes[3] = {0xB0 | (channel - 1), 0x20, lsb};
    uint8_t programNoBytes[2] = {0xC0 | (channel - 1), programNo};

    writeDeltaTime(self, channel, tick);
    writeToTrack(self, channel, msbBytes, sizeof(msbBytes));
    writeDeltaTime(self, channel, tick);
    writeToTrack(self, channel, lsbBytes, sizeof(lsbBytes));
    writeDeltaTime(self, channel, tick);
    writeToTrack(self, channel, programNoBytes, sizeof(programNoBytes));
}

static void appendTrackEnd(SMFWriter *self)
{
    uint8_t bytes[3] = {0xFF, 0x2F, 0x00};

    for (int i = 0; i < 17; ++i) {
        Track *track = &self->tracks[i];
        if (0 < track->length) {
            writeDeltaTime(self, i, track->lastTick);
            writeToTrack(self, i, bytes, sizeof(bytes));
        }
    }
}

bool SMFWriterSerialize(SMFWriter *self)
{
    bool ret = false;

    appendTrackEnd(self);

    FILE *file = fopen(self->filepath, "wb");
    if (!file) {
        return ret;
    }

    uint8_t fileHeader[14] = {
        'M', 'T', 'h', 'd',
        0x00, 0x00, 0x00, 0x06,
        0x00, 0x01,
        0x00, self->trackNum,
        0x00FF & (self->resolution >> 8), 0x00FF & self->resolution
    };

    fwrite(fileHeader, 1, sizeof(fileHeader), file);
    if (ferror(file)) {
        goto ERROR;
    }

    for (int i = 0; i < 17; ++i) {
        Track *track = &self->tracks[i];

        if (0 < track->length) {
            uint8_t trackHeader[8] = {
                'M', 'T', 'r', 'k',
                0x000000FF & (track->length >> 24),
                0x000000FF & (track->length >> 16),
                0x000000FF & (track->length >> 8),
                0x000000FF & track->length
            };

            fwrite(trackHeader, 1, sizeof(trackHeader), file);
            if (ferror(file)) {
                goto ERROR;
            }

            fwrite(track->buffer, 1, track->length, file);
            if (ferror(file)) {
                goto ERROR;
            }
        }
    }

    ret = true;

ERROR:
    fclose(file);
    return ret;
}
