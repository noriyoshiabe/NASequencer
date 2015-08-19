#pragma once

typedef enum {
    StatementResolution,
    StatementTitle,
    StatementTempo,
    StatementTimeSign,
    StatementMeasure,
    StatementMarker,
    StatementPattern,
    StatementPatternDefine,
    StatementEnd,
    StatementChannel,
    StatementVoice,
    StatementVolume,
    StatementPan,
    StatementChrous,
    StatementReverb,
    StatementPhrase,
    StatementPhraseDefine,
    StatementTranspose,
    StatementKey,
    StatementNote,
    StatementRest,
} Statement;

typedef struct _ParseLocation {
    int line;
    int column;
} ParseLocation;

typedef void (*ParserCallback)(void *context, ParseLocation *location, Statement statement, ...);
typedef void (*ParserErrorCallback)(void *context, ParseLocation *location, const char *message);

static inline const char *Statement2String(Statement statement)
{
#define CASE(statement) case statement: return &(#statement[9])
    switch (statement) {
    CASE(StatementResolution);
    CASE(StatementTitle);
    CASE(StatementTempo);
    CASE(StatementTimeSign);
    CASE(StatementMeasure);
    CASE(StatementMarker);
    CASE(StatementPattern);
    CASE(StatementPatternDefine);
    CASE(StatementEnd);
    CASE(StatementChannel);
    CASE(StatementVoice);
    CASE(StatementVolume);
    CASE(StatementPan);
    CASE(StatementChrous);
    CASE(StatementReverb);
    CASE(StatementPhrase);
    CASE(StatementPhraseDefine);
    CASE(StatementTranspose);
    CASE(StatementKey);
    CASE(StatementNote);
    CASE(StatementRest);

    default:
       break;
    }

    return "Unknown statement";
#undef CASE
}
