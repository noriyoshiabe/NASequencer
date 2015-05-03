#pragma once

typedef enum {
    ExpressionTypeInteger,
    ExpressionTypeFloat,
    ExpressionTypeString,
    ExpressionTypeNote,
    ExpressionTypeQuantize,
    ExpressionTypeDot,
    ExpressionTypeOctaveShift,
    ExpressionTypeKey,
    ExpressionTypeTime,
    ExpressionTypeTimeSign,
    ExpressionTypeTempo,
    ExpressionTypeMarker,
    ExpressionTypeChannel,
    ExpressionTypeVelocity,
    ExpressionTypeGatetime,
    ExpressionTypeGatetimeAuto,
    ExpressionTypeOctave,
    ExpressionTypeRest,
    ExpressionTypeTie,
    ExpressionTypeLocation,
    ExpressionTypeMeasure,
    ExpressionTypePlus,
    ExpressionTypeMinus,
    ExpressionTypeRepeat,
    ExpressionTypeParallel,
    ExpressionTypeBlock,
    ExpressionTypePatternDefine,
    ExpressionTypePatternExpand,
    ExpressionTypeOffset,
    ExpressionTypeLength,
    ExpressionTypeEOF,

    ExpressionTypeSize
} ExpressionType;

static inline const char *ExpressionType2String(ExpressionType type)
{
#define CASE(type) case type: return &(#type[14])
    switch (type) {
    CASE(ExpressionTypeInteger);
    CASE(ExpressionTypeFloat);
    CASE(ExpressionTypeString);
    CASE(ExpressionTypeNote);
    CASE(ExpressionTypeQuantize);
    CASE(ExpressionTypeDot);
    CASE(ExpressionTypeOctaveShift);
    CASE(ExpressionTypeKey);
    CASE(ExpressionTypeTime);
    CASE(ExpressionTypeTimeSign);
    CASE(ExpressionTypeTempo);
    CASE(ExpressionTypeMarker);
    CASE(ExpressionTypeChannel);
    CASE(ExpressionTypeVelocity);
    CASE(ExpressionTypeGatetime);
    CASE(ExpressionTypeGatetimeAuto);
    CASE(ExpressionTypeOctave);
    CASE(ExpressionTypeRest);
    CASE(ExpressionTypeTie);
    CASE(ExpressionTypeLocation);
    CASE(ExpressionTypeMeasure);
    CASE(ExpressionTypePlus);
    CASE(ExpressionTypeMinus);
    CASE(ExpressionTypeRepeat);
    CASE(ExpressionTypeParallel);
    CASE(ExpressionTypeBlock);
    CASE(ExpressionTypePatternDefine);
    CASE(ExpressionTypePatternExpand);
    CASE(ExpressionTypeOffset);
    CASE(ExpressionTypeLength);
    CASE(ExpressionTypeEOF);

    default:
       break;
    }
    return "Unknown expression type";
#undef CASE
}
