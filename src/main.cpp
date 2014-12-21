#include <iostream>
#include "tokenizer.h"

int main(int argc, char **argv)
{
    std::list<char *> *tokens;

    try {
        tokens = Tokenizer::read("./test.namidi");
    } catch (Tokenizer::Exception &e) {
        std::cout << e.what() << std::endl;
        return -1;
    }

    for (std::list<char *>::const_iterator iterator = tokens->begin(),
            end = tokens->end(); iterator != end; ++iterator) {
        std::cout << *iterator << std::endl;
    }

    Tokenizer::destroyTokens(tokens);

    return 0;
}
