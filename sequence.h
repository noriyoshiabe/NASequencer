#pragma once
#include <vector>

class MidiMessage {
};

class ShortMessage : public MidiMessage {
public:
    uint8_t bytes[3];
};

class MetaMessage : public MidiMessage {
public:
    const uint8_t status = 0xFF;
    uint8_t type;
    size_t dataLength;
    uint8_t *data;
};

class MidiEvent {
public:
    uint32_t tick;
    MidiMessage *message;
};

class Track {
public:
    std::vector<MidiEvent> events;
};

class Sequence {
public:
    uint16_t resolution;
    std::vector<Track> tracks;
};
