#include "ParseContext.h"
#include "DSLParser.h"
#include "ASTParser.h"

ParseContext *ParseContextParse(const char *filepath)
{
    ParseContext *ret = NATypeNew(ParseContext, filepath);
    Expression *expression;

    if (!DSLParserParseFile(filepath, &expression, &ret->error)) {
        return ret;
    }

#if 0
    dumpExpression(expression);
#endif

    ASTParserParseExpression(expression, filepath, &ret->sequence, &ret->patterns, &ret->error);

    deleteExpression(expression);

    return ret;
}

static void *__ParseContextInit(void *_self, ...)
{
    ParseContext *self = _self;
    va_list ap;
    va_start(ap, _self);
    self->filepath = va_arg(ap, const char *);
    va_end(ap);

    return self;
}

static void __ParseContextDestroy(void *_self)
{
    ParseContext *self = _self;
    if (self->sequence) {
        NARelease(self->sequence);
    }
    if (self->patterns) {
        CFRelease(self->patterns);
    }
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
