#pragma once
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <string>

class MidiEvent {
public:
    uint32_t tick;

    MidiEvent(uint32_t tick) {
        this->tick = tick;
    }
    virtual ~MidiEvent() {}

    virtual std::string toString() { return std::string("MidiEvent: ") + std::to_string(tick); };
};

class TempoEvent : public MidiEvent {
public:
    float tempo;

    TempoEvent(uint32_t tick, float tempo) : MidiEvent(tick) {
        this->tempo = tempo;
    }
};

class TimeSignatureEvent : public MidiEvent {
public:
    uint8_t numerator;
    uint8_t denominator;

    TimeSignatureEvent(uint32_t tick, uint8_t numerator, uint8_t denominator) : MidiEvent(tick) {
        this->numerator = numerator;
        this->denominator = denominator;
    }
};

class TextEvent : public MidiEvent {
public:
    char *text;
    size_t textLength;

    TextEvent(uint32_t tick, const char *text) : MidiEvent(tick) {
        textLength = strlen(text);
        this->text = (char *)malloc(textLength + 1);
        memcpy(this->text, text, textLength);
        this->text[textLength] = '\0';
    }

    virtual ~TextEvent() {
        free(text);
    }
};

class NameEvent : public TextEvent {
public:
    NameEvent(uint32_t tick, const char *text) : TextEvent(tick, text) {}
};

class TrackEndEvent : public MidiEvent {
public:
    TrackEndEvent(uint32_t tick) : MidiEvent(tick) {}
};

class BankSelectEvent : public MidiEvent {
public:
    uint8_t channel;
    uint8_t msb;
    uint8_t lsb;

    BankSelectEvent(uint32_t tick, uint32_t channel, uint8_t msb, uint8_t lsb) : MidiEvent(tick) {
        this->channel = channel;
        this->msb = msb;
        this->lsb = lsb;
    }
};

class ProgramChangeEvent : public MidiEvent {
public:
    uint8_t channel;
    uint8_t programNo;

    ProgramChangeEvent(uint32_t tick, uint32_t channel, uint8_t programNo) : MidiEvent(tick) {
        this->channel = channel;
        this->programNo = programNo;
    }
};

class NoteEvent : public MidiEvent {
public:
    uint8_t channel;
    uint8_t noteNo;
    uint8_t velocity;
    uint32_t gatetime;

    NoteEvent(uint32_t tick, uint8_t channel, uint8_t noteNo, uint8_t velocity, uint32_t gatetime) : MidiEvent(tick) {
        this->channel = channel;
        this->noteNo = noteNo;
        this->velocity = velocity;
        this->gatetime = gatetime;
    }
};

class MarkerEvent : public TextEvent {
public:
    MarkerEvent(uint32_t tick, const char *text) : TextEvent(tick, text) {}
};

class Track {
public:
    std::vector<MidiEvent *> events;
};

class Sequence {
public:
    uint16_t resolution;
    std::vector<Track *> tracks;
};
