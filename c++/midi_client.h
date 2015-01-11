#pragma once

#include <CoreMIDI/CoreMIDI.h>

class MidiClient {
private:
    MIDIClientRef clientRef;
    MIDIPortRef outPortRef;
    MIDIEndpointRef destPointRef;
    MIDIPacketList *packetListPtr;

public:
    bool initialize();
    bool send(uint8_t *bytes, size_t length);
    bool close();
};
