#include "ParseContext.h"
#include "DSLParser.h"
#include "ASTParser.h"

ParseContext *ParseContextParse(const char *filepath, ParseError *error)
{
    ParseContext *ret = NULL;

    Expression *expression;
    Sequence *sequence;
    CFMutableDictionaryRef patterns;

    if (!DSLParserParseFile(filepath, &expression, error)) {
        return ret;
    }

#if 0
    dumpExpression(expression);
#endif

    if (!ASTParserParseExpression(expression, filepath, &sequence, &patterns, error)) {
        goto ERROR;
    }

    ret = NATypeNew(ParseContext, filepath, sequence, patterns);

ERROR:
    deleteExpression(expression);

    return ret;
}

static void *__ParseContextInit(void *_self, ...)
{
    ParseContext *self = _self;
    va_list ap;
    va_start(ap, _self);
    self->filepath = va_arg(ap, const char *);
    self->sequence = NARetain(va_arg(ap, Sequence *));
    self->patterns = (void *)CFRetain(va_arg(ap, CFMutableDictionaryRef));
    va_end(ap);

    NARetain(self->sequence);
    CFRetain(self->patterns);

    return self;
}

static void __ParseContextDestroy(void *_self)
{
    ParseContext *self = _self;
    NARelease(self->sequence);
    CFRelease(self->patterns);
}

static void *__ParseContextDescription(const void *_self)
{
    const ParseContext *self = _self;
    CFStringRef sequence = NADescription(self->sequence);
    void *ret = (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<ParseContext: filepath=%s sequence=%@ patterns=%@>"),
            self->filepath, sequence, self->patterns);
    CFRelease(sequence);
    return ret;
}

NADeclareVtbl(ParseContext, NAType,
        __ParseContextInit,
        __ParseContextDestroy,
        NULL,
        NULL,
        NULL,
        NULL,
        __ParseContextDescription
        );
NADeclareClass(ParseContext, NAType);
