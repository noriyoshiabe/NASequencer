#pragma once

#include "Statement.h"

typedef struct _Resolution {
    Statement stmt;
    int resolution;
} Resolution;

typedef struct _Title {
    Statement stmt;
    char *title;
} Title;

typedef struct _Tempo {
    Statement stmt;
    float tempo;
} Tempo;

typedef struct _Time {
    Statement stmt;
    int numerator;
    int denominator;
} Time;

typedef struct _Key {
    Statement stmt;
    char *keyString;
} Key;

typedef struct _Marker {
    Statement stmt;
    char *text;
} Marker;

typedef struct _Channel {
    Statement stmt;
    int number;
} Channel;

typedef struct _Voice {
    Statement stmt;
    int msb;
    int lsb;
    int programNo;
} Voice;

typedef struct _Synth {
    Statement stmt;
    char *name;
} Synth;

typedef struct _Volume {
    Statement stmt;
    int value;
} Volume;

typedef struct _Pan {
    Statement stmt;
    int value;
} Pan;

typedef struct _Chorus {
    Statement stmt;
    int value;
} Chorus;

typedef struct _Reverb {
    Statement stmt;
    int value;
} Reverb;

typedef struct _Transpose {
    Statement stmt;
    int value;
} Transpose;

typedef struct _Rest {
    Statement stmt;
    int step;
} Rest;

typedef struct _Note {
    Statement stmt;
    char *noteString;
    NodeList *list;
} Note;

typedef struct _Include {
    Statement stmt;
    char *filepath;
} Include;

typedef struct _Pattern {
    Statement stmt;
    char *identifier;
    NodeList *list;
} Pattern;

typedef struct _Define {
    Statement stmt;
    char *identifier;
    NodeList *list;
} Define;

typedef struct _Context {
    Statement stmt;
    NodeList *ctxIdList;
    NodeList *list;
} Context;

typedef struct _Identifier {
    Statement stmt;
    char *idString;
} Identifier;

typedef struct _NoteParam {
    Statement stmt;
    int value;
} NoteParam;
