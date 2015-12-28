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

    FILE *stream;
};

static bool ABCPreprocessorParseInclude(ABCPreprocessor *self, char *line);
static bool ABCPreprocessorParseMacro(ABCPreprocessor *self, char *line);
static bool ABCPreprocessorParseRedefinableSymbol(ABCPreprocessor *self, char *line);
static bool ABCPreprocessorParseInstruction(ABCPreprocessor *self, char *line);
static void ABCPreprocessorExpandMacro(ABCPreprocessor *self, char *line, FILE *stream);

static void putTransposedNote(char pitch, char letter, FILE *stream);

ABCPreprocessor *ABCPreprocessorCreate()
{
    ABCPreprocessor *self = calloc(1, sizeof(ABCPreprocessor));

    self->staticMacros = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);
    self->transposingMacros = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);
    self->redefinableSymbols = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);
    self->stream = NAUtilCreateMemoryStream(1024);

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
    return NULL;
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
