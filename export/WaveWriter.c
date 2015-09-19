#include "WaveWriter.h"
#include "NAByteBuffer.h"

#include <stdio.h>
#include <stdlib.h>

struct _WaveWriter {
    FILE *fp;
    NAByteBuffer *buffer;
};

WaveWriter *WaveWriterCreate()
{
    WaveWriter *self = calloc(1, sizeof(WaveWriter));
    self->buffer = NAByteBufferCreate(1024 * 1024);
    return self;
}

void WaveWriterDestroy(WaveWriter *self)
{
    if (self->fp) {
        fclose(self->fp);
    }

    NAByteBufferDestroy(self->buffer);
    free(self);
}

bool WaveWriterOpenFile(WaveWriter *self, const char *filepath)
{
    self->fp = fopen(filepath, "wb");
    return !!self->fp;
}

void WaveWriterAppendData(WaveWriter *self, int32_t *data, int count)
{
    NAByteBufferWriteData(self->buffer, data, count * sizeof(int32_t));
}

bool WaveWriterSerialize(WaveWriter *self, double sampleRate)
{
#define Word2LittleEndianBytes(word) word & 0xFF, word >> 8 & 0xFF
#define DWord2LittleEndianBytes(dword) dword & 0xFF, dword >> 8 & 0xFF, dword >> 16 & 0xFF, dword >> 24 & 0xFF
    bool ret = false;

    int length = NAByteBufferDataLength(self->buffer);

    int sizeExceptRiffHeader = 44 + length - 8;

    uint8_t fileHeader[44] = {
        'R', 'I', 'F', 'F',
        DWord2LittleEndianBytes(sizeExceptRiffHeader),
        'W', 'A', 'V', 'E',
        'f', 'm', 't', ' ',
        DWord2LittleEndianBytes(16), // length of fmt chunk
        Word2LittleEndianBytes(1), // format id
        Word2LittleEndianBytes(2), // channel count
        DWord2LittleEndianBytes(44100), // sample rate
        DWord2LittleEndianBytes(176400), // data speed of 44.1kHz 16bit stereo (44100 * 2 * 2)
        Word2LittleEndianBytes(4), // block size
        Word2LittleEndianBytes(16), // bit per sample
        'd', 'a', 't', 'a',
        DWord2LittleEndianBytes(length)
    };

    fwrite(fileHeader, 1, sizeof(fileHeader), self->fp);
    if (ferror(self->fp)) {
        goto ERROR;
    }

    void *data;
    NAByteBufferReadData(self->buffer, &data, length);
    fwrite(data, 1, length, self->fp);
    if (ferror(self->fp)) {
        goto ERROR;
    }

    ret = true;

ERROR:
    fclose(self->fp);
    self->fp = NULL;

    return ret;
}
