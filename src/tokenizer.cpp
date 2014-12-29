#include "tokenizer.h"

const char *Tokenizer::EOL = "<EOL>";

Tokenizer *Tokenizer::read()
{
    if (!tokens.empty()) {
        throw Exception("Illegal state with read(). Tokens are already read.");
    }

    FILE *fp = fopen(source, "r");

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

            if ('\'' == c) {
                stateChange(SINGLE_QUOTE_LITERAL);
            }
            else if ('"' == c) {
                stateChange(DOUBLE_QUOTE_LITERAL);
            }

            if (isLineBreak(c)) {
                pushToken(EOL);
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

            if (isLineBreak(c)) {
                pushToken(EOL);
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

    fclose(fp);

    return this;
}
