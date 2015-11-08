#pragma once

#include "Expression.h"

#include <stdbool.h>

extern void *NAMidiExprStatementList(const char *filepath, void *yylloc);
extern void *NAMidiExprTitle(const char *filepath, void *yylloc, const char *title);
extern void *NAMidiExprResolution(const char *filepath, void *yylloc, int reslution);
extern void *NAMidiExprTempo(const char *filepath, void *yylloc, float tempo);
extern void *NAMidiExprTimeSign(const char *filepath, void *yylloc, int numerator, int denominator);
extern void *NAMidiExprMarker(const char *filepath, void *yylloc, const char *marker);
extern void *NAMidiExprChannel(const char *filepath, void *yylloc, int channel);
extern void *NAMidiExprVoice(const char *filepath, void *yylloc, int msb, int lsb, int programNo);
extern void *NAMidiExprSynth(const char *filepath, void *yylloc, const char *identifier);
extern void *NAMidiExprVolume(const char *filepath, void *yylloc, int value);
extern void *NAMidiExprPan(const char *filepath, void *yylloc, int value);
extern void *NAMidiExprChorus(const char *filepath, void *yylloc, int value);
extern void *NAMidiExprReverb(const char *filepath, void *yylloc, int value);
extern void *NAMidiExprTranspose(const char *filepath, void *yylloc, int value);
extern void *NAMidiExprKeySign(const char *filepath, void *yylloc, const char *keySign);
extern void *NAMidiExprRest(const char *filepath, void *yylloc, int step);
extern void *NAMidiExprNote(const char *filepath, void *yylloc, const char *noteString, int step, int gatetime, int velocity);
extern void *NAMidiExprPattern(const char *filepath, void *yylloc, const char *identifier, Expression *statementList);
extern void *NAMidiExprPatternExpand(const char *filepath, void *yylloc, const char *identifier, NAArray *idList);
extern void *NAMidiExprContext(const char *filepath, void *yylloc, NAArray *idList, Expression *statementList);

extern bool NAMidiExprIsStatementList(Expression *self);
