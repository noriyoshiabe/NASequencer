#include "NAIO.h"
#include "NALog.h"

int main(int argc, char **argv)
{
    FILE *fp = NAIOCreateMemoryStream(8);
    fprintf(fp, "TESTTESTTEST\n");
    fprintf(fp, "testtesttest\n");
    fprintf(fp, "TESTTESTTEST\n");

    rewind(fp);

    int c;
    while (EOF != (c = fgetc(fp))) {
        putchar(c);
    }

    char *test = "test";

    __Dump__C(test[0]);
    __Dump__S(test);

    __Trace__
    __Dump__I(1);
    __Dump__P(fp);

    __Dump__C(test[0]);
    __Dump__S(test);

    __Trace__
    __Dump__I(1);
    __Dump__P(fp);


    fclose(fp);

    return 0;
}
