#pragma once

#include "NAMidiStatement.h"

typedef struct _NAMidiStatementVisitor {
    void (*visitResolution)(void *self, Resolution *stmt);
    void (*visitTitle)(void *self, Title *stmt);
    void (*visitTempo)(void *self, Tempo *stmt);
    void (*visitTime)(void *self, Time *stmt);
    void (*visitKey)(void *self, Key *stmt);
    void (*visitMarker)(void *self, Marker *stmt);
    void (*visitChannel)(void *self, Channel *stmt);
    void (*visitVoice)(void *self, Voice *stmt);
    void (*visitSynth)(void *self, Synth *stmt);
    void (*visitVolume)(void *self, Volume *stmt);
    void (*visitPan)(void *self, Pan *stmt);
    void (*visitChorus)(void *self, Chorus *stmt);
    void (*visitReverb)(void *self, Reverb *stmt);
    void (*visitTranspose)(void *self, Transpose *stmt);
    void (*visitRest)(void *self, Rest *stmt);
    void (*visitNote)(void *self, Note *stmt);
    void (*visitInclude)(void *self, Include *stmt);
    void (*visitPattern)(void *self, Pattern *stmt);
    void (*visitDefine)(void *self, Define *stmt);
    void (*visitContext)(void *self, Context *stmt);
    void (*visitIdentifier)(void *self, Identifier *stmt);
    void (*visitNoteParam)(void *self, NoteParam *stmt);
} NAMidiStatementVisitor;
