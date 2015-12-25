#include "ABCPreprocessor.h"

int main(int argc, char **argv)
{
    char buffer[1024];

    FILE *fp = fopen(argv[1], "r");

    printf("---- source ----\n");

    while (fgets(buffer, 1024, fp)) {
        fputs(buffer, stdout);
    }

    rewind(fp);

    ABCPreprocessor *preprocessor = ABCPreprocessorCreate();
    ABCPreprocessorProcess(preprocessor, fp, argv[1]);
    FILE *result = ABCPreprocessorGetStream(preprocessor, argv[1]);
   
    printf("\n");
    printf("---- preproccessed ----\n");

    while (fgets(buffer, 1024, result)) {
        fputs(buffer, stdout);
    }

    ABCPreprocessorDestroy(preprocessor);

    return 0;
}
