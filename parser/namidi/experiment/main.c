#include "NAMidiStatement.h"
#include "NAMidi_yacc.h"
#include "NAMidi_lex.h"

#include <stdio.h>

extern int NAMidi_parse(yyscan_t scanner, const char *filepath);

int main(int argc, char **argv)
{
    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        return 1;
    }

    yyscan_t scanner;
    NAMidi_lex_init_extra(NULL, &scanner);
    YY_BUFFER_STATE state = NAMidi__create_buffer(fp, YY_BUF_SIZE, scanner);
    NAMidi__switch_to_buffer(state, scanner);

    int ret = NAMidi_parse(scanner, argv[1]);

    NAMidi__delete_buffer(state, scanner);
    NAMidi_lex_destroy(scanner);
    fclose(fp);

    return ret;
}

int NAMidi_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, const char *message)
{
    fprintf(stderr, "%s:%d:%d %s\n", filepath, yylloc->first_line, yylloc->first_column, message);
    return 0;
}
