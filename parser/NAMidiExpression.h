#pragma once

#include "Expression.h"

#include <stdbool.h>

extern Expression *NAMidiExprStatementList(const char *filepath, void *yylloc);
extern Expression *NAMidiExprTitle(const char *filepath, void *yylloc, const char *title);
extern Expression *NAMidiExprResolution(const char *filepath, void *yylloc, int reslution);
extern Expression *NAMidiExprTempo(const char *filepath, void *yylloc, float tempo);
extern Expression *NAMidiExprTimeSign(const char *filepath, void *yylloc, int numerator, int denominator);
extern Expression *NAMidiExprMarker(const char *filepath, void *yylloc, const char *marker);
extern Expression *NAMidiExprChannel(const char *filepath, void *yylloc, int channel);
extern Expression *NAMidiExprVoice(const char *filepath, void *yylloc, int msb, int lsb, int programNo);
extern Expression *NAMidiExprSynth(const char *filepath, void *yylloc, const char *identifier);
extern Expression *NAMidiExprVolume(const char *filepath, void *yylloc, int value);
extern Expression *NAMidiExprPan(const char *filepath, void *yylloc, int value);
extern Expression *NAMidiExprChorus(const char *filepath, void *yylloc, int value);
extern Expression *NAMidiExprReverb(const char *filepath, void *yylloc, int value);
extern Expression *NAMidiExprTranspose(const char *filepath, void *yylloc, int value);
extern Expression *NAMidiExprKeySign(const char *filepath, void *yylloc, const char *keySign);
extern Expression *NAMidiExprRest(const char *filepath, void *yylloc, int step);
extern Expression *NAMidiExprNote(const char *filepath, void *yylloc, const char *noteString, int step, int gatetime, int velocity);
extern Expression *NAMidiExprPattern(const char *filepath, void *yylloc, const char *identifier, Expression *statementList);
extern Expression *NAMidiExprPatternExpand(const char *filepath, void *yylloc, const char *identifier, NAArray *idList);
extern Expression *NAMidiExprContext(const char *filepath, void *yylloc, NAArray *idList, Expression *statementList);

extern bool NAMidiExprIsStatementList(Expression *self);
extern void NAMidiExprStatementListAppend(Expression *self, Expression *statementList);
