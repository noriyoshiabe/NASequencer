#include "tokenizer.h"

std::list<char *> *Tokenizer::read(const char *filename)
{
    Tokenizer *tokenizer = new Tokenizer();

    FILE *fp;
    fp = fopen(filename, "r");

    tokenizer->read(fp);
    fclose(fp);

    std::list<char *> *ret = tokenizer->getTokens();
    delete tokenizer;
    
    return ret;
}

void Tokenizer::destroyTokens(std::list<char *> *tokens)
{
    for (std::list<char *>::const_iterator iterator = tokens->begin(),
            end = tokens->end(); iterator != end; ++iterator) {
        free(*iterator);
    }
    delete tokens;
}

void Tokenizer::read(FILE *fp)
{
    while (EOF != (c = fgetc(fp))) {
        switch (state) {
        case NONE:
        case OPERAND:
            switch (state) {
            case NONE:
                if (isOperand(c)) {
                    pushBuffer(c);
                    stateChange(OPERAND);
                }
                break;

            case OPERAND:
                if (isOperand(c)) {
                    pushBuffer(c);
                }
                else {
                    pushToken();
                    stateChange(NONE);
                }
                break;

            default:
                break;
            }

            if (!isLineCommentStart(c)
                    && !isLineCommentEnd(c)
                    && !isBlockCommentStart(c)
                    && !isBlockCommentEnd(c)) {
                if (isOperator(c)) {
                    pushToken(c);
                }
            }

            if (isLineCommentStart(c)) {
                popToken();
                stateChange(LINE_COMMENT);
            } 
            else if (isBlockCommentStart(c)) {
                popToken();
                stateChange(BLOCK_COMMENT);
            }

            {
                char *last = tokens->back();
                if (isLineBreak(c) && (NULL == last || ';' != last[0])) {
                    pushToken(';');
                }
            }

            if ('\'' == c) {
                stateChange(SINGLE_QUOTE_LITERAL);
            }
            else if ('"' == c) {
                stateChange(DOUBLE_QUOTE_LITERAL);
            }

            break;

        case SINGLE_QUOTE_LITERAL:
            if ('\'' == c && last != '\\') {
                pushToken();
                state = NONE;
                index = 0;
            }
            else {
                pushBuffer(c);
            }
            break;

        case DOUBLE_QUOTE_LITERAL:
            if ('"' == c && last != '\\') {
                pushToken();
                stateChange(NONE);
            }
            else {
                pushBuffer(c);
            }
            break;

        case LINE_COMMENT:
            if (isLineCommentEnd(c)) {
                stateChange(NONE);
            }
            break;

        case BLOCK_COMMENT:
            if (isBlockCommentEnd(c)) {
                stateChange(NONE);
            }
            break;

        default:
            break;
        }

        last = c;
    }
}
