#pragma once

#include "Expression.h"
#include "NAMidiParser.h"
#include "NAMap.h"

#include <stdbool.h>

extern void *NAMidiExprStatementList(NAMidiParser *parser, const char *filepath, void *yylloc);
extern void *NAMidiExprTitle(NAMidiParser *parser, const char *filepath, void *yylloc, char *title);
extern void *NAMidiExprResolution(NAMidiParser *parser, const char *filepath, void *yylloc, int reslution);
extern void *NAMidiExprTempo(NAMidiParser *parser, const char *filepath, void *yylloc, float tempo);
extern void *NAMidiExprTimeSign(NAMidiParser *parser, const char *filepath, void *yylloc, int numerator, int denominator);
extern void *NAMidiExprMarker(NAMidiParser *parser, const char *filepath, void *yylloc, char *marker);
extern void *NAMidiExprChannel(NAMidiParser *parser, const char *filepath, void *yylloc, int channel);
extern void *NAMidiExprVoice(NAMidiParser *parser, const char *filepath, void *yylloc, int msb, int lsb, int programNo);
extern void *NAMidiExprSynth(NAMidiParser *parser, const char *filepath, void *yylloc, char *identifier);
extern void *NAMidiExprVolume(NAMidiParser *parser, const char *filepath, void *yylloc, int value);
extern void *NAMidiExprPan(NAMidiParser *parser, const char *filepath, void *yylloc, int value);
extern void *NAMidiExprChorus(NAMidiParser *parser, const char *filepath, void *yylloc, int value);
extern void *NAMidiExprReverb(NAMidiParser *parser, const char *filepath, void *yylloc, int value);
extern void *NAMidiExprTranspose(NAMidiParser *parser, const char *filepath, void *yylloc, int value);
extern void *NAMidiExprKeySign(NAMidiParser *parser, const char *filepath, void *yylloc, char *keyString);
extern void *NAMidiExprRest(NAMidiParser *parser, const char *filepath, void *yylloc, int step);
extern void *NAMidiExprNote(NAMidiParser *parser, const char *filepath, void *yylloc, char *noteString, int step, int gatetime, int velocity);
extern void *NAMidiExprPattern(NAMidiParser *parser, const char *filepath, void *yylloc, char *identifier, Expression *statementList);
extern void *NAMidiExprPatternExpand(NAMidiParser *parser, const char *filepath, void *yylloc, char *identifier, NAArray *idList);
extern void *NAMidiExprContext(NAMidiParser *parser, const char *filepath, void *yylloc, NAArray *idList, Expression *statementList);

extern bool NAMidiExprIsStatementList(Expression *self);
extern bool NAMidiExprIsPattern(Expression *self);
extern NAMap *NAMidiExprStatementListGetPatternMap(void *self);
extern char *NAMidiExprPatternGetIdentifier(void *_self);
extern Expression *NAMidiExprStatementListMarge(Expression *self, Expression *statementList);
