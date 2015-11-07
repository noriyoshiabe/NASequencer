#pragma once

typedef struct _SequenceBuilder {
    void (*destroy)(void *self);
    void *(*build)(void *self);
} SequenceBuilder;
