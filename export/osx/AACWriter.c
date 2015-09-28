#include "AACWriter.h"
#include "NAByteBuffer.h"

#include <AudioToolbox/AudioToolbox.h>
#include <stdlib.h>

#define OUTPUT_BUF_SIZE 32768

struct _AACWriter {
    double sampleRate;
    NAByteBuffer *buffer;
    uint8_t *outputBuffer;
    AudioConverterRef converter;
    AudioFileID destinationFileID;
    UInt32 outputSizePerPacket;
    UInt32 numOutputPackets;
    AudioStreamPacketDescription *outputPacketDescriptions;
};

AACWriter *AACWriterCreate(double sampleRate)
{
    AACWriter *self = calloc(1, sizeof(AACWriter));
    self->sampleRate = sampleRate;
    self->buffer = NAByteBufferCreate(1024 * 1024);
    return self;
}

void AACWriterDestroy(AACWriter *self)
{
    if (self->converter) {
        AudioConverterDispose(self->converter);
    }

    if (self->destinationFileID) {
        AudioFileClose(self->destinationFileID);
    }

    if (self->outputBuffer) {
        free(self->outputBuffer);
    }

    if (self->outputPacketDescriptions) {
        free(self->outputPacketDescriptions);
    }

    NAByteBufferDestroy(self->buffer);
    free(self);
}

bool AACWriterOpenFile(AACWriter *self, const char *filepath)
{
    AudioStreamBasicDescription srcFormat = {
        .mSampleRate = self->sampleRate,
        .mFormatID = kAudioFormatLinearPCM,
        .mChannelsPerFrame = 2,
        .mBitsPerChannel = 16,
        .mBytesPerPacket = 2 * 2,
        .mBytesPerFrame = 2 * 2,
        .mFramesPerPacket = 1,
        .mFormatFlags = kLinearPCMFormatFlagIsPacked | kLinearPCMFormatFlagIsSignedInteger,
    };

    AudioStreamBasicDescription dstFormat = {
        .mSampleRate = self->sampleRate,
        .mFormatID = kAudioFormatMPEG4AAC,
        .mChannelsPerFrame = 2,
    };

    OSStatus err;

    UInt32 size = sizeof(dstFormat);
    err = AudioFormatGetProperty(kAudioFormatProperty_FormatInfo, 0, NULL, &size, &dstFormat);
    if (noErr != err) {
        return false;
    }

    err = AudioConverterNew(&srcFormat, &dstFormat, &self->converter);
    if (noErr != err) {
        return false;
    }

    UInt32 outputBitRate = 192000;
    err = AudioConverterSetProperty(self->converter, kAudioConverterEncodeBitRate, sizeof(outputBitRate), &outputBitRate);
    if (noErr != err) {
        return false;
    }

    size = sizeof(self->outputSizePerPacket);
    err = AudioConverterGetProperty(self->converter, kAudioConverterPropertyMaximumOutputPacketSize, &size, &self->outputSizePerPacket);
    if (noErr != err) {
        return false;
    }

    self->outputBuffer = malloc(OUTPUT_BUF_SIZE);
    self->numOutputPackets = OUTPUT_BUF_SIZE / self->outputSizePerPacket;
    self->outputPacketDescriptions = calloc(self->numOutputPackets, sizeof(AudioStreamPacketDescription));

    CFStringRef _string = CFStringCreateWithCString(NULL, filepath, kCFStringEncodingUTF8);
    CFURLRef url = CFURLCreateWithFileSystemPath(NULL, _string, kCFURLPOSIXPathStyle, false);
    err = AudioFileCreateWithURL(url, kAudioFileM4AType, &dstFormat, kAudioFileFlags_EraseFile, &self->destinationFileID);
    CFRelease(_string);
    CFRelease(url);

    return noErr == err;
}

void AACWriterAppendData(AACWriter *self, int32_t *data, int count)
{
    NAByteBufferWriteData(self->buffer, data, count * sizeof(int32_t));
}

static bool AACWriterWriteCookie(AACWriter *self)
{
	UInt32 cookieSize;
	OSStatus err = AudioConverterGetPropertyInfo(self->converter, kAudioConverterCompressionMagicCookie, &cookieSize, NULL);
    if (noErr != err) {
        return false;
    }
 
    char cookie[cookieSize];

    err = AudioConverterGetProperty(self->converter, kAudioConverterCompressionMagicCookie, &cookieSize, cookie);
    if (noErr != err) {
        return false;
    }

    return noErr == AudioFileSetProperty(self->destinationFileID, kAudioFilePropertyMagicCookieData, cookieSize, cookie);
}

static OSStatus AACWriterEncoderDataProc(AudioConverterRef inAudioConverter, UInt32 *ioNumberDataPackets,
        AudioBufferList *ioData, AudioStreamPacketDescription **outDataPacketDescription, void *inUserData)
{
    AACWriter *self = inUserData;
	
	if (*ioNumberDataPackets > self->numOutputPackets) {
    //    *ioNumberDataPackets = self->numOutputPackets;
    }
    printf("-- %d\n", *ioNumberDataPackets);

    void *data;
    int outNumBytes = NAByteBufferReadData(self->buffer, &data, OUTPUT_BUF_SIZE);

	ioData->mBuffers[0].mData = data;
	ioData->mBuffers[0].mDataByteSize = outNumBytes;
	ioData->mBuffers[0].mNumberChannels = 2;

    //*outDataPacketDescription = NULL;

    return noErr;
}

static bool WritePacketTableInfo(AACWriter *self)
{
    UInt32 isWritable;
    UInt32 dataSize;
    OSStatus err = AudioFileGetPropertyInfo(self->destinationFileID, kAudioFilePropertyPacketTableInfo, &dataSize, &isWritable);
    if (noErr != err) {
        return false;
    }

    if (!isWritable) {
        return true;
    }

    AudioConverterPrimeInfo primeInfo;
    dataSize = sizeof(primeInfo);

    err = AudioConverterGetProperty(self->converter, kAudioConverterPrimeInfo, &dataSize, &primeInfo);
    if (noErr != err) {
        return false;
    }

    AudioFilePacketTableInfo pti;
    dataSize = sizeof(pti);
    err = AudioFileGetProperty(self->destinationFileID, kAudioFilePropertyPacketTableInfo, &dataSize, &pti);
    if (noErr != err) {
        return false;
    }

    UInt64 totalFrames = pti.mNumberValidFrames + pti.mPrimingFrames + pti.mRemainderFrames;

    pti.mPrimingFrames = primeInfo.leadingFrames;
    pti.mRemainderFrames = primeInfo.trailingFrames;
    pti.mNumberValidFrames = totalFrames - pti.mPrimingFrames - pti.mRemainderFrames;

    err = AudioFileSetProperty(self->destinationFileID, kAudioFilePropertyPacketTableInfo, sizeof(pti), &pti);
    if (noErr != err) {
        return false;
    }

    return true;
}

bool AACWriterSerialize(AACWriter *self)
{
    OSStatus err;

    if (!AACWriterWriteCookie(self)) {
        return false;
    }

    AudioBufferList fillBufList;
    fillBufList.mNumberBuffers = 1;
    fillBufList.mBuffers[0].mNumberChannels = 2;
    fillBufList.mBuffers[0].mDataByteSize = OUTPUT_BUF_SIZE;
    fillBufList.mBuffers[0].mData = self->outputBuffer;

    SInt64 outputFilePos = 0;
    UInt32 ioOutputDataPackets = self->numOutputPackets;

    while (0 < ioOutputDataPackets) {
        err = AudioConverterFillComplexBuffer(self->converter, AACWriterEncoderDataProc, self, &ioOutputDataPackets, &fillBufList, self->outputPacketDescriptions);
        if (noErr != err) {
            return false;
        }

        UInt32 inNumBytes = fillBufList.mBuffers[0].mDataByteSize;
        err = AudioFileWritePackets(self->destinationFileID, false, inNumBytes, self->outputPacketDescriptions, outputFilePos, &ioOutputDataPackets, self->outputBuffer);
        printf("----------- %d\n", outputFilePos);
        if (noErr != err) {
            return false;
        }

        outputFilePos += ioOutputDataPackets;
    }

    if (!WritePacketTableInfo(self)) {
        return false;
    }

    if (!AACWriterWriteCookie(self)) {
        return false;
    }

    return true;
}
