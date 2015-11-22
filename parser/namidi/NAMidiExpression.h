#pragma once

#include "Expression.h"
#include "NAMidiParser.h"
#include "NAMap.h"

#include <stdbool.h>

extern void *NAMidiExprStatementList(NAMidiParser *parser, ParseLocation *location);
extern void *NAMidiExprTitle(NAMidiParser *parser, ParseLocation *location, char *title);
extern void *NAMidiExprResolution(NAMidiParser *parser, ParseLocation *location, int resolution);
extern void *NAMidiExprTempo(NAMidiParser *parser, ParseLocation *location, float tempo);
extern void *NAMidiExprTimeSign(NAMidiParser *parser, ParseLocation *location, int numerator, int denominator);
extern void *NAMidiExprMarker(NAMidiParser *parser, ParseLocation *location, char *marker);
extern void *NAMidiExprChannel(NAMidiParser *parser, ParseLocation *location, int channel);
extern void *NAMidiExprVoice(NAMidiParser *parser, ParseLocation *location, int msb, int lsb, int programNo);
extern void *NAMidiExprSynth(NAMidiParser *parser, ParseLocation *location, char *identifier);
extern void *NAMidiExprVolume(NAMidiParser *parser, ParseLocation *location, int value);
extern void *NAMidiExprPan(NAMidiParser *parser, ParseLocation *location, int value);
extern void *NAMidiExprChorus(NAMidiParser *parser, ParseLocation *location, int value);
extern void *NAMidiExprReverb(NAMidiParser *parser, ParseLocation *location, int value);
extern void *NAMidiExprTranspose(NAMidiParser *parser, ParseLocation *location, int value);
extern void *NAMidiExprKeySign(NAMidiParser *parser, ParseLocation *location, char *keyString);
extern void *NAMidiExprRest(NAMidiParser *parser, ParseLocation *location, int step);
extern void *NAMidiExprNote(NAMidiParser *parser, ParseLocation *location, char *noteString, int step, int gatetime, int velocity);
extern void *NAMidiExprPattern(NAMidiParser *parser, ParseLocation *location, char *identifier, Expression *statementList);
extern void *NAMidiExprPatternExpand(NAMidiParser *parser, ParseLocation *location, char *identifier, NAArray *idList);
extern void *NAMidiExprContext(NAMidiParser *parser, ParseLocation *location, NAArray *idList, Expression *statementList);

extern bool NAMidiExprIsStatementList(Expression *self);
extern bool NAMidiExprIsPattern(Expression *self);
extern NAMap *NAMidiExprStatementListGetPatternMap(void *self);
extern char *NAMidiExprPatternGetIdentifier(void *_self);
