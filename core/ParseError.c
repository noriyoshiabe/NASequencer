#include "ParseError.h"

static void __ParseErrorDestroy(void *_self)
{
    ParseError *self = _self;

    if (self->filepath) {
        CFRelease(self->filepath);
    }

    if (self->message) {
        CFRelease(self->message);
    }
}

static void *__ParseErrorDescription(const void *_self)
{
    const ParseError *self = _self;
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<ParseError: kind=%d - %s filepath=%@ message=%@ location=%d:%d - %d:%d>"),
            self->kind, ParseError2String(self->kind), self->filepath, self->message,
            self->location.firstLine, self->location.firstColumn,
            self->location.lastLine, self->location.lastColumn);
}

NADeclareVtbl(ParseError, NAType,
        NULL,
        __ParseErrorDestroy,
        NULL,
        NULL,
        NULL,
        NULL,
        __ParseErrorDescription,
        );
NADeclareClass(ParseError, NAType);
