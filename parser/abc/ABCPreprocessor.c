#include "ABCPreprocessor.h"
#include "NAUtil.h"
#include "NAMap.h"
#include "NACString.h"

#include <stdlib.h>
#include <stdbool.h>
#include <regex.h>
#include <alloca.h>

typedef struct Macro {
    char *target;
    char *replacement;
    regex_t reg;
    bool transposing;
} Macro;

typedef struct RedefinableSymbol {
    char *symbol;
    char *replacement;
} RedefinableSymbol;

static Macro *MacroCreate(char *target, char *replacement);
static void MacroDestroy(Macro *self);

static RedefinableSymbol *RedefinableSymbolCreate(char *symbol, char *replacement);
static void RedefinableSymbolDestroy(RedefinableSymbol *self);

struct _ABCPreprocessor {
    NAMap *staticMacros;
    NAMap *transposingMacros;
    NAMap *redefinableSymbols;

    regex_t inlineRegex;
    FILE *stream;
    FILE *tmpStream;
};

static void ABCPreprocessorPreprocessTuneBodyInternal(ABCPreprocessor *self, const char *string);
static void ABCPreprocessorExpandStaticMacro(ABCPreprocessor *self, const char *string, FILE *stream);
static void ABCPreprocessorExpandTransposingMacro(ABCPreprocessor *self, const char *string, FILE *stream);
static void ABCPreprocessorExpandRedefinableSymbol(ABCPreprocessor *self, const char *string, FILE *stream);

static void putTransposedNote(char pitch, char letter, FILE *stream);

ABCPreprocessor *ABCPreprocessorCreate()
{
    ABCPreprocessor *self = calloc(1, sizeof(ABCPreprocessor));

    self->staticMacros = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);
    self->transposingMacros = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);
    self->redefinableSymbols = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);

    self->stream = NAUtilCreateMemoryStream(1024);
    self->tmpStream = NAUtilCreateMemoryStream(1024);

    regcomp(&self->inlineRegex, "\\[[+[:alpha:]]:[^\\]]*\\]", REG_EXTENDED);

    const struct {
        const char *symbol;
        const char *replacement;
    } predefined[] = {
        {"~", "!roll!"},
        {"H", "!fermata!"},
        {"L", "!accent!"},
        {"M", "!lowermordent!"},
        {"O", "!coda!"},
        {"P", "!uppermordent!"},
        {"S", "!segno!"},
        {"T", "!trill!"},
        {"u", "!upbow!"},
        {"v", "!downbow!"},
    };

    for (int i = 0; i < sizeof(predefined) / sizeof(predefined[0]); ++i) {
        RedefinableSymbol *rdSymbol = RedefinableSymbolCreate(strdup(predefined[i].symbol), strdup(predefined[i].replacement));
        NAMapPut(self->redefinableSymbols, rdSymbol->symbol, rdSymbol);
    }

    return self;
}

void ABCPreprocessorDestroy(ABCPreprocessor *self)
{
    NAMapTraverseValue(self->staticMacros, MacroDestroy);
    NAMapDestroy(self->staticMacros);

    NAMapTraverseValue(self->transposingMacros, MacroDestroy);
    NAMapDestroy(self->transposingMacros);

    NAMapTraverseValue(self->redefinableSymbols, MacroDestroy);
    NAMapDestroy(self->redefinableSymbols);

    fclose(self->stream);
    fclose(self->tmpStream);

    free(self);
}

void ABCPreprocessorSetMacro(ABCPreprocessor *self, char *target, char *replacement)
{
    Macro *prev;
    if ((prev = NAMapRemove(self->staticMacros, target))) {
        MacroDestroy(prev);
    }
    if ((prev = NAMapRemove(self->transposingMacros, target))) {
        MacroDestroy(prev);
    }
    
    Macro *macro = MacroCreate(target, replacement);

    if (macro->transposing) {
        NAMapPut(self->transposingMacros, macro->target, macro);
    }
    else {
        NAMapPut(self->staticMacros, macro->target, macro);
    }    
}

void ABCPreprocessorSetRedefinableSymbol(ABCPreprocessor *self, char *symbol, char *replacement)
{
    RedefinableSymbol *prev;
    if ((prev = NAMapRemove(self->redefinableSymbols, symbol))) {
        RedefinableSymbolDestroy(prev);
    }

    if (0 == strcmp(replacement, "!none!") || 0 == strcmp(replacement, "!nil!")) {
        return;
    }
    
    RedefinableSymbol *rdSymbol = RedefinableSymbolCreate(symbol, replacement);
    NAMapPut(self->redefinableSymbols, rdSymbol->symbol, rdSymbol);
}

char *ABCPreprocessorPreprocessTuneBody(ABCPreprocessor *self, const char *tuneBody)
{
    fflush(self->stream);

    ABCPreprocessorPreprocessTuneBodyInternal(self, tuneBody);

    int length = ftell(self->stream);
    char *result = malloc(length + 1);

    fseek(self->stream, 0, SEEK_SET);
    fread(result, 1, length, self->stream);

    result[length] = '\0';

    return result;
}

static void ABCPreprocessorPreprocessTuneBodyInternal(ABCPreprocessor *self, const char *string)
{
    regmatch_t match[1];

    if (REG_NOMATCH == regexec(&self->inlineRegex, string, 1, match, 0)) {
        char buffer[1024];

        fflush(self->tmpStream);
        ABCPreprocessorExpandStaticMacro(self, string, self->tmpStream);
        fseek(self->tmpStream, 0, SEEK_SET);
        fread(buffer, 1, 1024, self->tmpStream);

        fflush(self->tmpStream);
        ABCPreprocessorExpandTransposingMacro(self, buffer, self->tmpStream);
        fseek(self->tmpStream, 0, SEEK_SET);
        fread(buffer, 1, 1024, self->tmpStream);

        fflush(self->tmpStream);
        ABCPreprocessorExpandRedefinableSymbol(self, buffer, self->tmpStream);
        fseek(self->tmpStream, 0, SEEK_SET);
        fread(buffer, 1, 1024, self->tmpStream);

        fputs(buffer, self->stream);
    }
    else {
        int length;
        char *buffer;

        length = match[0].rm_so;
        buffer = alloca(length + 1);
        memcpy(buffer, string, length);
        buffer[length] = '\0';
        ABCPreprocessorPreprocessTuneBodyInternal(self, buffer);

        fwrite(string + match[0].rm_so, 1, match[0].rm_eo - match[0].rm_so, self->stream);

        length = strlen(string) - match[0].rm_eo;
        buffer = alloca(length + 1);
        memcpy(buffer, string + match[0].rm_eo, length);
        buffer[length] = '\0';
        ABCPreprocessorPreprocessTuneBodyInternal(self, buffer);
    }
}

static void ABCPreprocessorExpandStaticMacro(ABCPreprocessor *self, const char *string, FILE *stream)
{
    NAIterator *iterator;
START:
    iterator = NAMapGetIterator(self->staticMacros);
    while (iterator->hasNext(iterator)) {
        NAMapEntry *entry = iterator->next(iterator);
        Macro *macro = entry->value;

        regmatch_t match[1];
        if (REG_NOMATCH != regexec(&macro->reg, string, 1, match, 0)) {
            fwrite(string, 1, match[0].rm_so, stream);
            fputs(macro->replacement, stream);
            string += match[0].rm_eo;
            goto START;
        }
    }

    fputs(string, stream);
}

static void ABCPreprocessorExpandTransposingMacro(ABCPreprocessor *self, const char *string, FILE *stream)
{
    NAIterator *iterator;
START:
    iterator = NAMapGetIterator(self->transposingMacros);
    while (iterator->hasNext(iterator)) {
        NAMapEntry *entry = iterator->next(iterator);
        Macro *macro = entry->value;

        regmatch_t match[2];
        if (REG_NOMATCH != regexec(&macro->reg, string, 2, match, 0)) {
            fwrite(string, 1, match[0].rm_so, stream);
            char pitch = string[match[1].rm_so];
            for (char *pc = macro->replacement; *pc; ++pc) {
                if ('h' <= *pc && *pc <= 'z') {
                    putTransposedNote(pitch, *pc, stream);
                }
                else {
                    fputc(*pc, stream);
                }
            }

            string += match[0].rm_eo;
            goto START;
        }
    }
    
    fputs(string, stream);
}

static void putTransposedNote(char pitch, char letter, FILE *stream)
{
#define isInsideRange(c, from, to) (from <= c && c <= to)

    const char *notes[] = {
        "C,", "D,", "E,", "F,", "G,", "A,", "B,",
        "C", "D", "E", "F", "G", "A", "B",
        "c", "d", "e", "f", "g", "a", "b",
        "c'", "d'", "e'", "f'", "g'", "a'", "b'",
    };

    int transpose = letter - 'n';
    int index = isInsideRange(pitch, 'A', 'B') ? pitch - 'A' + 12 :
                isInsideRange(pitch, 'C', 'G') ? pitch - 'C' + 7 :
                isInsideRange(pitch, 'a', 'b') ? pitch - 'a' + 19 :
                isInsideRange(pitch, 'c', 'g') ? pitch - 'c' + 14 :
                -1;

    if (-1 == index) {
        return;
    }

    fputs(notes[index + transpose], stream);
}

static void ABCPreprocessorExpandRedefinableSymbol(ABCPreprocessor *self, const char *string, FILE *stream)
{
    int length = strlen(string);
    for (int i = 0; i < length; ++i) {
        char symbol[2] = {string[i], '\0'};
        RedefinableSymbol *rdSymbol = NAMapGet(self->redefinableSymbols, symbol);
        if (rdSymbol) {
            fputs(rdSymbol->replacement, stream);
        }
        else {
            fputc(string[i], stream);
        }
    }
}


static Macro *MacroCreate(char *target, char *replacement)
{
    Macro *self = calloc(1, sizeof(Macro));
    self->target = target;
    self->replacement = NACStringTrimWhiteSpace(replacement);

    char *pattern = NACStringDuplicate(target);
    char *n = strchr(pattern, 'n');
    if (n) {
        char *_pattern = pattern;
        pattern = alloca(strlen(_pattern) + 10 + 1);
        *n = '\0';
        sprintf(pattern, "%s([A-Ga-g])%s", _pattern, n + 1);
        self->transposing = true;
    }
    regcomp(&self->reg, pattern, REG_EXTENDED);

    return self;
}

static void MacroDestroy(Macro *self)
{
    free(self->target);
    free(self->replacement);
    free(self);
}

static RedefinableSymbol *RedefinableSymbolCreate(char *symbol, char *replacement)
{
    RedefinableSymbol *self = calloc(1, sizeof(RedefinableSymbol));
    self->symbol = symbol;
    self->replacement = replacement;
    return self;
}

static void RedefinableSymbolDestroy(RedefinableSymbol *self)
{
    free(self->symbol);
    free(self->replacement);
    free(self);
}
