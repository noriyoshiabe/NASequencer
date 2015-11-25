#pragma once

#include "NAMidiStatement.h"

typedef struct _NAMidiParser NAMidiParser;

extern void NAMidiParserVisitResolution(NAMidiParser *self, Resolution *stmt);
extern void NAMidiParserVisitTitle(NAMidiParser *self, Title *stmt);
extern void NAMidiParserVisitTempo(NAMidiParser *self, Tempo *stmt);
extern void NAMidiParserVisitTime(NAMidiParser *self, Time *stmt);
extern void NAMidiParserVisitKey(NAMidiParser *self, Key *stmt);
extern void NAMidiParserVisitMarker(NAMidiParser *self, Marker *stmt);
extern void NAMidiParserVisitChannel(NAMidiParser *self, Channel *stmt);
extern void NAMidiParserVisitVoice(NAMidiParser *self, Voice *stmt);
extern void NAMidiParserVisitSynth(NAMidiParser *self, Synth *stmt);
extern void NAMidiParserVisitVolume(NAMidiParser *self, Volume *stmt);
extern void NAMidiParserVisitPan(NAMidiParser *self, Pan *stmt);
extern void NAMidiParserVisitChorus(NAMidiParser *self, Chorus *stmt);
extern void NAMidiParserVisitReverb(NAMidiParser *self, Reverb *stmt);
extern void NAMidiParserVisitTranspose(NAMidiParser *self, Transpose *stmt);
extern void NAMidiParserVisitRest(NAMidiParser *self, Rest *stmt);
extern void NAMidiParserVisitNote(NAMidiParser *self, Note *stmt);
extern void NAMidiParserVisitInclude(NAMidiParser *self, Include *stmt);
extern void NAMidiParserVisitPattern(NAMidiParser *self, Pattern *stmt);
extern void NAMidiParserVisitDefine(NAMidiParser *self, Define *stmt);
extern void NAMidiParserVisitContext(NAMidiParser *self, Context *stmt);
extern void NAMidiParserVisitIdentifier(NAMidiParser *self, Identifier *stmt);
extern void NAMidiParserVisitNoteParam(NAMidiParser *self, NoteParam *stmt);
