#pragma once

#include "Node.h"

typedef struct _Root {
    Node node;
} Root;

typedef struct _Resolution {
    Node node;
    int resolution;
} Resolution;

typedef struct _Title {
    Node node;
    char *title;
} Title;

typedef struct _Tempo {
    Node node;
    float tempo;
} Tempo;

typedef struct _Time {
    Node node;
    int numerator;
    int denominator;
} Time;

typedef struct _Key {
    Node node;
    char *keyString;
} Key;

typedef struct _Marker {
    Node node;
    char *text;
} Marker;

typedef struct _Channel {
    Node node;
    int number;
} Channel;

typedef struct _Voice {
    Node node;
    int msb;
    int lsb;
    int programNo;
} Voice;

typedef struct _Synth {
    Node node;
    char *name;
} Synth;

typedef struct _Volume {
    Node node;
    int value;
} Volume;

typedef struct _Pan {
    Node node;
    int value;
} Pan;

typedef struct _Chorus {
    Node node;
    int value;
} Chorus;

typedef struct _Reverb {
    Node node;
    int value;
} Reverb;

typedef struct _Transpose {
    Node node;
    int value;
} Transpose;

typedef struct _Rest {
    Node node;
    int step;
} Rest;

typedef struct _Note {
    Node node;
    char *noteString;
} Note;

typedef struct _Include {
    Node node;
    char *filepath;
} Include;

typedef struct _Pattern {
    Node node;
    char *identifier;
} Pattern;

typedef struct _Define {
    Node node;
    char *identifier;
} Define;

typedef struct _Context {
    Node node;
    NAArray *ctxIdList;
} Context;

typedef struct _Identifier {
    Node node;
    char *idString;
} Identifier;

typedef struct _NoteParam {
    Node node;
    int value;
} NoteParam;
