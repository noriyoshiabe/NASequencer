#pragma once

#include "parser.h"

#include <cstring>

class ConsoleWriter : public SequenceVisitor {
private:
    uint8_t numerator = 4;
    uint8_t denominator = 4;
    uint32_t resolution = 480;
    int trackCount = 0;

    char buffer[128];
    char *location(uint32_t tick) {
        int r = resolution * 4 / denominator;
        int m_l = r * numerator;
        int m = tick / m_l + 1;
        int b = (tick % m_l) / r + 1;
        int t = tick % r;
        sprintf(buffer, "%03d:%02d:%03d", m, b, t);
        return buffer;
    }

public:
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
};
