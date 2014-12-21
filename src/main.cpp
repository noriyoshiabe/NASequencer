#include <iostream>
#include "tokenizer.h"

int main(int argc, char **argv)
{
    FILE *fp;
    fp = fopen( "./test.namidi" , "r");

    Tokenizer tokenizer;
    try {
        tokenizer.read(fp);
    } catch (Tokenizer::Exception &e) {
        std::cout << e.message << std::endl;
        return -1;
    }

    std::list<char *> *tokens = tokenizer.getTokens();

    for (std::list<char *>::const_iterator iterator = tokens->begin(),
            end = tokens->end(); iterator != end; ++iterator) {
        std::cout << *iterator << std::endl;
    }

    fclose(fp);

    return 0;
}
