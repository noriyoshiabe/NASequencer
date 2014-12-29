#include <iostream>
#include <queue>
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

    std::queue<char *> tokenQ(std::deque<char *>(tokens->begin(), tokens->end()));

    while (!tokenQ.empty()) {
        std::cout << tokenQ.front() << std::endl;
        tokenQ.pop();
    }

    Tokenizer::destroyTokens(tokens);

    return 0;
}
