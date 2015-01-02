#pragma once
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <string>

class Sequence;
class Track;
class NameEvent;
class TempoEvent;
class TimeSignatureEvent;
class BankSelectEvent;
class ProgramChangeEvent;
class NoteEvent;
class MarkerEvent;
class TrackEndEvent;

class SequenceVisitor {
public:
    virtual void visit(Sequence *elem) = 0;
    virtual void visit(Track *elem) = 0;
    virtual void visit(NameEvent *elem) = 0;
    virtual void visit(TempoEvent *elem) = 0;
    virtual void visit(TimeSignatureEvent *elem) = 0;
    virtual void visit(BankSelectEvent *elem) = 0;
    virtual void visit(ProgramChangeEvent *elem) = 0;
    virtual void visit(NoteEvent *elem) = 0;
    virtual void visit(MarkerEvent *elem) = 0;
    virtual void visit(TrackEndEvent *elem) = 0;
};

class SequenceElement {
public:
    virtual void accept(SequenceVisitor *visitor) = 0;
};

class MidiEvent : public SequenceElement {
public:
    uint32_t tick;

    MidiEvent(uint32_t tick) {
        this->tick = tick;
    }
    virtual ~MidiEvent() {}
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

class Sequence : public SequenceElement {
public:
    uint16_t resolution;
    std::vector<Track *> tracks;

    void accept(SequenceVisitor *visitor) {
        visitor->visit(this);
    };
};

class Track : public SequenceElement {
public:
    std::vector<MidiEvent *> events;

    void accept(SequenceVisitor *visitor) {
        visitor->visit(this);
    };
};

class NameEvent : public TextEvent {
public:
    NameEvent(uint32_t tick, const char *text) : TextEvent(tick, text) {}

    void accept(SequenceVisitor *visitor) {
        visitor->visit(this);
    };
};

class TempoEvent : public MidiEvent {
public:
    float tempo;

    TempoEvent(uint32_t tick, float tempo) : MidiEvent(tick) {
        this->tempo = tempo;
    }

    void accept(SequenceVisitor *visitor) {
        visitor->visit(this);
    };
};

class TimeSignatureEvent : public MidiEvent {
public:
    uint8_t numerator;
    uint8_t denominator;

    TimeSignatureEvent(uint32_t tick, uint8_t numerator, uint8_t denominator) : MidiEvent(tick) {
        this->numerator = numerator;
        this->denominator = denominator;
    }

    void accept(SequenceVisitor *visitor) {
        visitor->visit(this);
    };
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

    void accept(SequenceVisitor *visitor) {
        visitor->visit(this);
    };
};

class ProgramChangeEvent : public MidiEvent {
public:
    uint8_t channel;
    uint8_t programNo;

    ProgramChangeEvent(uint32_t tick, uint32_t channel, uint8_t programNo) : MidiEvent(tick) {
        this->channel = channel;
        this->programNo = programNo;
    }

    void accept(SequenceVisitor *visitor) {
        visitor->visit(this);
    };
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

    void accept(SequenceVisitor *visitor) {
        visitor->visit(this);
    };
};

class MarkerEvent : public TextEvent {
public:
    MarkerEvent(uint32_t tick, const char *text) : TextEvent(tick, text) {}

    void accept(SequenceVisitor *visitor) {
        visitor->visit(this);
    };
};

class TrackEndEvent : public MidiEvent {
public:
    TrackEndEvent(uint32_t tick) : MidiEvent(tick) {}

    void accept(SequenceVisitor *visitor) {
        visitor->visit(this);
    };
};
