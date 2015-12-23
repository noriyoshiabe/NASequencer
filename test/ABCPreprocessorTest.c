#include "ABCPreprocessor.h"

int main(int argc, char **argv)
{
    char buffer[1024];

    FILE *fp = fopen(argv[1], "r");

    printf("---- source ----\n");

    while (fgets(buffer, 1204, fp)) {
        fputs(buffer, stdout);
    }

    rewind(fp);

    ABCPreprocessor *preprocessor = ABCPreprocessorCreate();
    FILE *result = ABCPreprocessorProcess(preprocessor, fp);
    ABCPreprocessorDestroy(preprocessor);

    printf("\n");
    printf("---- preproccessed ----\n");

    while (fgets(buffer, 1204, result)) {
        fputs(buffer, stdout);
    }

    return 0;
}
