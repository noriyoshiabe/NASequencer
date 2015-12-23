#include "ABCPreprocessor.h"
#include "NAUtil.h"
#include "NAMap.h"
#include "NACString.h"

#include <stdlib.h>
#include <stdbool.h>
#include <regex.h>
#include <alloca.h>

static const char *PATTERN = "^m:[[:space:]]*([~[:alnum:]]+)[[:space:]]*=(.*)$";

typedef struct Macro {
    char *target;
    char *replacement;
    regex_t reg;
    bool transposing;
} Macro;

static Macro *MacroCreate(char *target, char *replacement);
static void MacroDestroy(Macro *self);

struct _ABCPreprocessor {
    NAMap *staticMacros;
    NAMap *transposingMacros;
    regex_t regex;
};

static bool ABCPreprocessorParseMacro(ABCPreprocessor *self, char *line);
static void ABCPreprocessorExpandMacro(ABCPreprocessor *self, char *line, FILE *result);

static void putTransposedNote(char pitch, char letter, FILE *result);

ABCPreprocessor *ABCPreprocessorCreate()
{
    ABCPreprocessor *self = calloc(1, sizeof(ABCPreprocessor));
    self->staticMacros = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);
    self->transposingMacros = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);
    regcomp(&self->regex, PATTERN, REG_EXTENDED);
    return self;
}

void ABCPreprocessorDestroy(ABCPreprocessor *self)
{
    NAMapTraverseValue(self->staticMacros, MacroDestroy);
    NAMapDestroy(self->staticMacros);

    NAMapTraverseValue(self->transposingMacros, MacroDestroy);
    NAMapDestroy(self->transposingMacros);

    free(self);
}

FILE *ABCPreprocessorProcess(ABCPreprocessor *self, FILE *fp)
{
    FILE *result = NAUtilCreateMemoryStream(1024);

    char line[1024];
    while (fgets(line, 1024, fp)) {
        if (ABCPreprocessorParseMacro(self, line)) {
            fputs(line, result);
        }
        else {
            ABCPreprocessorExpandMacro(self, line, result);
        }
    }

    rewind(result);
    return result;
}

static bool ABCPreprocessorParseMacro(ABCPreprocessor *self, char *line)
{
    int length;
    regmatch_t match[3];

    if (REG_NOMATCH == regexec(&self->regex, line, 3, match, 0)) {
        return false;
    }

    length = match[1].rm_eo - match[1].rm_so;
    char *target = malloc(length + 1);
    memcpy(target, line + match[1].rm_so, length);
    target[length] = '\0';

    length = match[2].rm_eo - match[2].rm_so;
    char *replacement = malloc(length + 1);
    memcpy(replacement, line + match[2].rm_so, length);
    replacement[length] = '\0';

    Macro *macro = MacroCreate(target, replacement);
    if (macro->transposing) {
        Macro *prev = NAMapRemove(self->transposingMacros, macro->target);
        if (prev) {
            MacroDestroy(prev);
        }
        NAMapPut(self->transposingMacros, macro->target, macro);
    }
    else {
        Macro *prev = NAMapRemove(self->staticMacros, macro->target);
        if (prev) {
            MacroDestroy(prev);
        }
        NAMapPut(self->staticMacros, macro->target, macro);
    }

    return true;
}

static void ABCPreprocessorExpandMacro(ABCPreprocessor *self, char *line, FILE *result)
{
    NAIterator *iterator;

START:
    iterator = NAMapGetIterator(self->staticMacros);
    while (iterator->hasNext(iterator)) {
        NAMapEntry *entry = iterator->next(iterator);
        Macro *macro = entry->value;

        regmatch_t match[1];
        if (REG_NOMATCH != regexec(&macro->reg, line, 1, match, 0)) {
            fwrite(line, 1, match[0].rm_so, result);
            fputs(macro->replacement, result);
            line += match[0].rm_eo;
            goto START;
        }
    }

    iterator = NAMapGetIterator(self->transposingMacros);
    while (iterator->hasNext(iterator)) {
        NAMapEntry *entry = iterator->next(iterator);
        Macro *macro = entry->value;

        regmatch_t match[2];
        if (REG_NOMATCH != regexec(&macro->reg, line, 2, match, 0)) {
            fwrite(line, 1, match[0].rm_so, result);
            char pitch = line[match[1].rm_so];
            for (char *pc = macro->replacement; *pc; ++pc) {
                if ('h' <= *pc && *pc <= 'z') {
                    putTransposedNote(pitch, *pc, result);
                }
                else {
                    fputc(*pc, result);
                }
            }

            line += match[0].rm_eo;
            goto START;
        }
    }
    
    fputs(line, result);
}

static void putTransposedNote(char pitch, char letter, FILE *result)
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

    fputs(notes[index + transpose], result);
}


static char *removeComment(char *replacement)
{
    char *percent = strchr(replacement, '%');
    if (percent) {
        *percent = '\0';
    }
    return replacement;
}

static Macro *MacroCreate(char *target, char *replacement)
{
    Macro *self = calloc(1, sizeof(Macro));
    self->target = target;
    self->replacement = NACStringTrimWhiteSpace(removeComment(replacement));

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
