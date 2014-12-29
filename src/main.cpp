#include <iostream>
#include "tokenizer.h"

int main(int argc, char **argv)
{
    Tokenizer *tokenizer = new Tokenizer();
    std::deque<char *> *tokenQ = tokenizer->read("./test.namidi")->createTokenQ();

    while (!tokenQ->empty()) {
        std::cout << tokenQ->front() << std::endl;
        tokenQ->pop_front();
    }

    delete tokenQ;
    delete tokenizer;

    return 0;
}
