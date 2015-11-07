#pragma once

typedef struct _SequenceBuilder SequenceBuilder;

extern SequenceBuilder *SequenceBuilderCreate();
extern void *SequenceBuilderBuild(SequenceBuilder *self);
extern void SequenceBuilderDestroy(SequenceBuilder *self);
