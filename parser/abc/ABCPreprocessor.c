#include "ABCPreprocessor.h"
#include "NAUtil.h"
#include "NAMap.h"
#include "NACString.h"
#include "NAStack.h"

#include <stdlib.h>
#include <stdbool.h>
#include <regex.h>
#include <alloca.h>
#include <libgen.h>

static const char *INCLUDE = "^I:[[:space:]]*abc-include[[:space:]]+([0-9a-zA-Z_\\/\\-]+\\.abh|'[0-9a-zA-Z_\\/ \\-]+\\.abh'|\"[0-9a-zA-Z_\\/ \\-]+\\.abh\")[[:space:]]*$";
static const char *MACRO = "^m:[[:space:]]*([~[:alnum:]]+)[[:space:]]*=(.*)$";

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
    regex_t includeRegex;
    regex_t macroRegex;
    NAMap *streamMap;
    char *currentFile;
    NAStack *fileStack;
};

static bool ABCPreprocessorParseInclude(ABCPreprocessor *self, char *line);
static bool ABCPreprocessorParseMacro(ABCPreprocessor *self, char *line);
static void ABCPreprocessorExpandMacro(ABCPreprocessor *self, char *line, FILE *stream);

static void putTransposedNote(char pitch, char letter, FILE *stream);

ABCPreprocessor *ABCPreprocessorCreate()
{
    ABCPreprocessor *self = calloc(1, sizeof(ABCPreprocessor));
    self->staticMacros = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);
    self->transposingMacros = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);
    regcomp(&self->includeRegex, INCLUDE, REG_EXTENDED);
    regcomp(&self->macroRegex, MACRO, REG_EXTENDED);
    self->streamMap = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);
    self->fileStack = NAStackCreate(1);
    return self;
}

void ABCPreprocessorDestroy(ABCPreprocessor *self)
{
    NAMapTraverseValue(self->staticMacros, MacroDestroy);
    NAMapDestroy(self->staticMacros);

    NAMapTraverseValue(self->transposingMacros, MacroDestroy);
    NAMapDestroy(self->transposingMacros);

    NAIterator *iterator = NAMapGetIterator(self->streamMap);
    while (iterator->hasNext(iterator)) {
        NAMapEntry *entry = iterator->next(iterator);
        char *filepath = entry->key;
        FILE *fp = entry->value;
        free(filepath);
        fclose(fp);
    }
    NAMapDestroy(self->streamMap);

    NAStackDestroy(self->fileStack);

    free(self);
}

void ABCPreprocessorProcess(ABCPreprocessor *self, FILE *input, const char *filepath)
{
    if (NAMapContainsKey(self->streamMap, (char *)filepath)) {
        return;
    }

    FILE *stream = NAUtilCreateMemoryStream(1024);
    NAMapPut(self->streamMap, strdup(filepath), stream);

    if (self->currentFile) {
        NAStackPush(self->fileStack, self->currentFile);
    }
    self->currentFile = (char *)filepath;

    char line[1024];
    while (fgets(line, 1024, input)) {
        if (ABCPreprocessorParseInclude(self, line)) {
            fputs(line, stream);
        } else if (ABCPreprocessorParseMacro(self, line)) {
            fputs(line, stream);
        }
        else {
            ABCPreprocessorExpandMacro(self, line, stream);
        }
    }

    self->currentFile = NAStackPop(self->fileStack);
}

FILE *ABCPreprocessorGetStream(ABCPreprocessor *self, const char *filepath)
{
    FILE *ret = NAMapGet(self->streamMap, (char *)filepath);
    if (ret) {
        rewind(ret);
    }
    return ret;
}

static bool ABCPreprocessorParseInclude(ABCPreprocessor *self, char *line)
{
    regmatch_t match[2];

    if (REG_NOMATCH == regexec(&self->includeRegex, line, 2, match, 0)) {
        return false;
    }

    int length = match[1].rm_eo - match[1].rm_so;
    char *filename = alloca(length + 1);
    memcpy(filename, line + match[1].rm_so, length);
    if ('\'' == filename[0] || '"' == filename[0]) {
        memmove(filename, filename + 1, length - 2);
        filename[length - 2] = '\0';
    }
    else {
        filename[length] = '\0';
    }

    char *directory = dirname(self->currentFile);
    char *fullpath = NAUtilBuildPathWithDirectory(directory, filename);

    FILE *fp = fopen(fullpath, "r");
    if (fp) {
        ABCPreprocessorProcess(self, fp, fullpath);
    }

    free(fullpath);
    return true;
}

static bool ABCPreprocessorParseMacro(ABCPreprocessor *self, char *line)
{
    int length;
    regmatch_t match[3];

    if (REG_NOMATCH == regexec(&self->macroRegex, line, 3, match, 0)) {
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

static void ABCPreprocessorExpandMacro(ABCPreprocessor *self, char *line, FILE *stream)
{
    NAIterator *iterator;

START:
    iterator = NAMapGetIterator(self->staticMacros);
    while (iterator->hasNext(iterator)) {
        NAMapEntry *entry = iterator->next(iterator);
        Macro *macro = entry->value;

        regmatch_t match[1];
        if (REG_NOMATCH != regexec(&macro->reg, line, 1, match, 0)) {
            fwrite(line, 1, match[0].rm_so, stream);
            fputs(macro->replacement, stream);
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
            fwrite(line, 1, match[0].rm_so, stream);
            char pitch = line[match[1].rm_so];
            for (char *pc = macro->replacement; *pc; ++pc) {
                if ('h' <= *pc && *pc <= 'z') {
                    putTransposedNote(pitch, *pc, stream);
                }
                else {
                    fputc(*pc, stream);
                }
            }

            line += match[0].rm_eo;
            goto START;
        }
    }
    
    fputs(line, stream);
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
