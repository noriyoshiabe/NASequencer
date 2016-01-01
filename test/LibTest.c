#include "NAIO.h"

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

    fclose(fp);

    return 0;
}
