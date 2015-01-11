#include "midi_client.h"
#include <stdio.h>

#define PACKET_LIST_SIZE 1024

bool MidiClient::initialize()
{
    OSStatus err;

    err = MIDIClientCreate(CFSTR("namidi:MidiClient"), NULL, NULL, &clientRef);
    if (err != noErr) {
        printf("MIDIClientCreate err = %d\n", err);
        return false;
    }
    
    err = MIDIOutputPortCreate(clientRef, CFSTR("namidi:MidiClient:outPort"), &outPortRef);
    if (err != noErr) {
        printf("MIDIOutputPortCreate err = %d\n", err);
        goto ERROR_1;
    }
    
    destPointRef = MIDIGetDestination(0);
    
    CFStringRef strRef;
    err = MIDIObjectGetStringProperty(destPointRef, kMIDIPropertyDisplayName, &strRef);
    if (err != noErr) {
        printf("MIDIObjectGetStringProperty err = %d\n", err);
        goto ERROR_2;
    }
    
    char str[64];
    CFStringGetCString(strRef, str, sizeof(str), kCFStringEncodingUTF8);
    printf("connected to %s\n", str);
    
    CFRelease(strRef);
    
    packetListPtr = (MIDIPacketList *)malloc(PACKET_LIST_SIZE);
    
    return true;

ERROR_2:
    MIDIPortDispose(outPortRef);

ERROR_1:
    MIDIClientDispose(clientRef);

    return false;
}

bool MidiClient::send(uint8_t *bytes, size_t length)
{
    MIDIPacket *packet = MIDIPacketListInit(packetListPtr);
    packet = MIDIPacketListAdd(packetListPtr, PACKET_LIST_SIZE, packet, 0, length, bytes);

    OSStatus err = MIDISend(outPortRef, destPointRef, packetListPtr);
    if (err != noErr) {
        printf("MIDISend err = %d\n", err);
        return false;
    }

#if 0
    char buf[256] = {0};
    for (int i = 0; i < length; ++i) {
        char s[4];
        sprintf(s, " %02X", message[i]);
        strcat(buf, s);
    }
    printf("message: %s\n", buf);
#endif

    return true;
}

bool MidiClient::close()
{
    bool ret = true;
    OSStatus err;
    
    err = MIDIPortDispose(outPortRef);
	if (err != noErr) {
        printf("MIDIPortDispose outPortRef err = %d\n", err);
        ret = false;
    }
	
	err = MIDIClientDispose(clientRef);
	if (err != noErr) {
        printf("MIDIClientDispose clientRef err = %d\n", err);
        ret = false;
    }

    return ret;
}
