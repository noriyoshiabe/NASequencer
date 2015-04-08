#include "NAMidiParser.h"

#include "Parser.h"
#include "Lexer.h"

#define TABLE_SIZE (TOKEN_END - TOKEN_BEGIN)
#define IDX(type) (type - TOKEN_BEGIN - 1)

static ExpressionParseFunction dispatchTable[TABLE_SIZE] = {NULL};

extern void registerExpressionParser(int type, ExpressionParser parseFunction)
{
    dispatchTable[IDX(type)] = pfn;
}

int yyparse(void *scanner, Expression **expression);

int yyerror(YYLTYPE *yylloc, void *scanner, Expression **expression, const char *message)
{
    ParserError *error = yyget_extra(scanner);
    error->location = *((ParseLocation *)yylloc);
    error->message = message;
    return 0;
}

static bool parseDSL(const char *filepath, Expression **expression, ParseError **error)
{
    bool ret = false;
    void *scanner;
    *expression = NULL;

    *error = ParseErrorCreate();
    (*error)->filepath = filepath;

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        error->kind = PARSE_ERROR_FILE_NOT_FOUND;
        return ret;
    }

    if (yylex_init_extra(*error, &scanner)) {
        (*error)->kind = PARSE_ERROR_INIT_ERROR;
        goto ERROR_1;
    }

    YY_BUFFER_STATE state = yy_create_buffer(fp, YY_BUF_SIZE, scanner);
    yy_switch_to_buffer(state, scanner);

    if (yyparse(scanner, expression)) {
        (*error)->kind = PARSE_ERROR_SYNTAX_ERROR;
        if (*expression) {
            deleteExpression(*expression);
            *expression = NULL;
        }
        goto ERROR_2;
    }

    free(*error);
    *error = NULL;
    ret = true;

ERROR_2:
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);
ERROR_1:
    fclose(fp);

    return ret;
}


ParseContext *ParseContextCreate(const char *filepath, Sequence *sequence)
{
    ParseContext *ret = calloc(1, sizeof(ParseContext));
    ret->filepath = filepath;
    ret->sequence = sequence;
    ret->channel = 1;
    ret->octave = 4;
    ret->velocity = 100;
    return ret;
}

ParseContext *ParseContextCreateLocal(const ParseContext *from)
{
    ParseContext *local = ParseContextCreate(from->filepath, from->sequence);

    local->channel = from->channel;
    local->velocity = from->velocity;
    local->gatetime = from->gatetime;
    local->octave = from->octave;

    local->resolution = SequenceGetResolution(from->sequence);
    local->timeSign = SequenceGetTimeSignByTick(from->sequence, from->tick);
    local->tempo = SequenceGetTempoByTick(from->sequence, from->tick);

    return local;
}

void ParseContextDestroy(ParseContext *context)
{
    free(context);
}

ParseError *ParseErrorCreate()
{
    return calloc(1, sizeof(ParseError));
}

ParseError *ParseErrorCreateWithError(ParseErrorKind kind, const char *filepath, const char *message, ParseLocation location)
{
    ParseError *ret = ParseErrorCreate();
    ret->kind = kind;
    ret->filepath = filepath;
    ret->message = message;
    ret->location = location;
    return ret;
}

void ParseErrorDestroy(ParseError *error)
{
    free(error);
}


static bool parseExpression(Expression *expression, ParseContext *context, void *value, ParseError **error)
{
#if 0
    printf("Attempt to parse expression [%s]\n", tokenType2String(expression->tokenType));
#endif

    ExpressionParseFunction function = dispatchTable[IDX(expression->tokenType)];
    if (!function) {
        printf("Dispatch function is not found. tokenType=%s", tokenType2String(expression->tokenType));
        abort();
    }
    return function(expression, context, value, error);
}


static Sequence *parseAST(Expression *expression, const char *filepath, ParseError **error)
{
    Sequence *ret = SequenceCreate();
    ParseContext *context = ParseContextCreate(filepath, ret);

    for (Expression *expr = expression; expr; expr = expr->right) {
        if (!parseExpression(expr, context, NULL, error)) {
            SequenceDestroy(ret);
            ret = NULL;
            goto ERROR;
        }
    }

ERROR:
    ParseContextDestroy(context);

    return ret;
}

Sequence *NAMidiParserParse(NAMIDIParser *self, const char *filepath, ParseError **error)
{
    Expression *expression;
    *error = NULL;

    if (!parseDSL(filepath, &expression, error)) {
        return NULL;
    }

#if 0
    dumpExpression(expression);
#endif

    Sequence *ret = parseAST(self, expression, filepath, error);
    deleteExpression(expression);
    return ret;
}
