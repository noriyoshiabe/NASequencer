#include "CoreMidiClient.h"
#include <CoreMIDI/CoreMIDI.h>

#define PACKET_LIST_SIZE 1024

typedef struct _CoreMidiClient {
    MidiClient vtbl;

    MIDIClientRef clientRef;
    MIDIPortRef outPortRef;
    MIDIEndpointRef destPointRef;
} CoreMidiClient;

static void _CoreMidiClientOpen(CoreMidiClient *self)
{
    self->destPointRef = MIDIGetDestination(0);
    if (!self->destPointRef) {
#if 0
        printf("midi output destination is not available.\n");
#endif
        return;
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
}

static void _CoreMidiClientSend(void *_self, uint8_t *bytes, size_t length)
{
    CoreMidiClient *self = _self;
    MIDIPacketList packetList[PACKET_LIST_SIZE];

    MIDIPacket *packet = MIDIPacketListInit(packetList);
    MIDIPacketListAdd(packetList, PACKET_LIST_SIZE, packet, 0, length, bytes);
    MIDISend(self->outPortRef, self->destPointRef, packetList);
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

static void _CoreMidiClientDestroy(void *self)
{
}

static bool _CoreMidiClientIsAvailable(void *_self)
{
    CoreMidiClient *self = _self;
    return !!self->destPointRef;
}

static bool _CoreMidiClientHasProperty(void *self, const char *name)
{
    return false;
}

static void _CoreMidiClientSetProperty(void *self, const char *name, const void *value)
{
}

static void _CoreMidiClientGetProperty(void *self, const char *name, void *value)
{
}

static CoreMidiClient *_sharedInstance = NULL;
static void _CoreMidiClientAtExit(void)
{
    if (_sharedInstance) {
        if (_sharedInstance->outPortRef) {
            MIDIPortDispose(_sharedInstance->outPortRef);
        }
        if (_sharedInstance->clientRef) {
            MIDIClientDispose(_sharedInstance->clientRef);
        }
    }
}

MidiClient *CoreMidiClientSharedInstance()
{
    if (!_sharedInstance) {
        _sharedInstance = calloc(1, sizeof(CoreMidiClient));

        _sharedInstance->vtbl.send = _CoreMidiClientSend;
        _sharedInstance->vtbl.destroy = _CoreMidiClientDestroy;
        _sharedInstance->vtbl.isAvailable = _CoreMidiClientIsAvailable;
        _sharedInstance->vtbl.hasProperty = _CoreMidiClientHasProperty;
        _sharedInstance->vtbl.setProperty = _CoreMidiClientSetProperty;
        _sharedInstance->vtbl.getProperty = _CoreMidiClientGetProperty;

        _CoreMidiClientOpen(_sharedInstance);
        atexit(_CoreMidiClientAtExit);
    }

    return (MidiClient *)_sharedInstance;
}
