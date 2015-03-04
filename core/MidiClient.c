#include "MidiClient.h"

#define PACKET_LIST_SIZE 1024

struct _MidiClient {
    NAType __;
    MIDIClientRef clientRef;
    MIDIPortRef outPortRef;
    MIDIEndpointRef destPointRef;
    MIDIPacketList *packetListPtr;
};

static void *__MidiClientInit(void *_self, ...)
{
    MidiClient *self = _self;
    self->packetListPtr = (MIDIPacketList *)malloc(PACKET_LIST_SIZE);
    return self;
}

static void __MidiClientDestroy(void *_self)
{
    MidiClient *self = _self;
    free(self->packetListPtr);
}

bool MidiClientOpen(MidiClient *self)
{
    self->destPointRef = MIDIGetDestination(0);
    if (!self->destPointRef) {
        printf("midi output destination is not available.\n");
        return false;
    }

    CFStringRef strRef;
    MIDIObjectGetStringProperty(self->destPointRef, kMIDIPropertyDisplayName, &strRef);
    if (strRef) {
        char str[64];
        CFStringGetCString(strRef, str, sizeof(str), kCFStringEncodingUTF8);
        CFRelease(strRef);
#if 0
        printf("connected to %s\n", str);
#endif
    }

    MIDIClientCreate(CFSTR("namidi:MidiClient"), NULL, NULL, &self->clientRef);
    MIDIOutputPortCreate(self->clientRef, CFSTR("namidi:MidiClient:outPort"), &self->outPortRef);
    
    return true;
}

void MidiClientSend(MidiClient *self, uint8_t *bytes, size_t length)
{
    MIDIPacket *packet = MIDIPacketListInit(self->packetListPtr);
    MIDIPacketListAdd(self->packetListPtr, PACKET_LIST_SIZE, packet, 0, length, bytes);
    MIDISend(self->outPortRef, self->destPointRef, self->packetListPtr);
#if 0
    char buf[256] = {0};
    for (int i = 0; i < length; ++i) {
        char s[4];
        sprintf(s, " %02X", message[i]);
        strcat(buf, s);
    }
    printf("message: %s\n", buf);
#endif
}

void MidiClientClose(MidiClient *self)
{
    MIDIPortDispose(self->outPortRef);
	MIDIClientDispose(self->clientRef);
}

NADeclareVtbl(MidiClient, NAType, __MidiClientInit, __MidiClientDestroy, NULL, NULL, NULL, NULL, NULL);
NADeclareClass(MidiClient, NAType);
