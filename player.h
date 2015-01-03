#pragma once

#include "sequence.h"
#include "midi_client.h"

#include <list>
#include <thread>

class Player : public SequenceVisitor {
private:
    struct TempoInfo {
        uint32_t tick;
        float tempo;
        uint32_t time;

        TempoInfo(uint32_t tick, float tempo, uint32_t time) {
            this->tick = tick;
            this->tempo = tempo;
            this->time = time;
        }
    };

    struct MidiMessage {
        uint8_t bytes[3];
        size_t length;
        uint32_t msec;
        uint32_t tick;
    };

    MidiClient client;
    std::list<MidiMessage *> messages;

    std::thread *thread;

    uint32_t resolution;
    std::list<TempoInfo> tempoTable;

    bool playing;

    void run();
    uint32_t tick2msec(uint32_t tick);

public:
    void stop();

    void visit(ParseContext *elem);
    void visit(Sequence *elem);
    void visit(Track *elem);
    void visit(NameEvent *elem);
    void visit(TempoEvent *elem);
    void visit(TimeSignatureEvent *elem);
    void visit(BankSelectEvent *elem);
    void visit(ProgramChangeEvent *elem);
    void visit(NoteEvent *elem);
    void visit(MarkerEvent *elem);
    void visit(TrackEndEvent *elem);

    Player() {
        client.initialize();
    };

    ~Player() {
        client.close();

        if (thread) {
            thread->join();
            delete thread;
        }
    }
};
