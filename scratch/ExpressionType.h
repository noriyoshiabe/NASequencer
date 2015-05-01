#pragma once

typedef enum {
    ExpressionTypeNote,
    ExpressionTypeQuantize,
    ExpressionTypeOctaveShift,
    ExpressionTypeKey,
    ExpressionTypeTime,
    ExpressionTypeTempo,
    ExpressionTypeMarker,
    ExpressionTypeChannel,
    ExpressionTypeVelocity,
    ExpressionTypeGatetime,
    ExpressionTypeOctave,
    ExpressionTypeRest,
    ExpressionTypeTie,
    ExpressionTypeLength,
    ExpressionTypeOffset,
    ExpressionTypeLocation,
    ExpressionTypePlus,
    ExpressionTypeMinus,
    ExpressionTypeRepeat,
    ExpressionTypeBlock,
    ExpressionTypePatternDefine,
    ExpressionTypePatternExpand,
} ExpressionType;

static inline const char *ExpressionType2String(ExpressionType type)
{
#define CASE(type) case type: return #type
    switch (type) {
    CASE(ExpressionTypeNote);
    CASE(ExpressionTypeQuantize);
    CASE(ExpressionTypeOctaveShift);
    CASE(ExpressionTypeKey);
    CASE(ExpressionTypeTime);
    CASE(ExpressionTypeTempo);
    CASE(ExpressionTypeMarker);
    CASE(ExpressionTypeChannel);
    CASE(ExpressionTypeVelocity);
    CASE(ExpressionTypeGatetime);
    CASE(ExpressionTypeOctave);
    CASE(ExpressionTypeRest);
    CASE(ExpressionTypeTie);
    CASE(ExpressionTypeLength);
    CASE(ExpressionTypeOffset);
    CASE(ExpressionTypeLocation);
    CASE(ExpressionTypePlus);
    CASE(ExpressionTypeMinus);
    CASE(ExpressionTypeRepeat);
    CASE(ExpressionTypeBlock);
    CASE(ExpressionTypePatternDefine);
    CASE(ExpressionTypePatternExpand);
    }
    return "Unknown expression type";
#undef CASE
}
