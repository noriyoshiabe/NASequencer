#include "util.h"
#include "tokenizer.h"
#include "parser.h"

std::unordered_map<std::string, ExpressionFactory> Parser::factoryTable;

Parser::Parser(const char *source)
{
    if (factoryTable.empty()) {
        factoryTable[Util::normalizeCase("<EOL>")] = EndOfLineExpression::create;
        factoryTable[Util::normalizeCase(";")] = EndOfStatementExpression::create;
        factoryTable[Util::normalizeCase("delta_time")] = DeltaTimeExpression::create;
        factoryTable[Util::normalizeCase("set")] = SetExpression::create;
        factoryTable[Util::normalizeCase("unset")] = UnsetExpression::create;
        factoryTable[Util::normalizeCase("track")] = TrackExpression::create;
        factoryTable[Util::normalizeCase("time_signature")] = TimeSignatureExpression::create;
        factoryTable[Util::normalizeCase("tempo")] = TempoExpression::create;
        factoryTable[Util::normalizeCase("bank_select")] = BankSelectExpression::create;
        factoryTable[Util::normalizeCase("program_change")] = ProgramChangeExpression::create;
        factoryTable[Util::normalizeCase("marker")] = MarkerExpression::create;
        factoryTable[Util::normalizeCase("note")] = NoteExpression::create;
        factoryTable[Util::normalizeCase("track_end")] = TrackEndExpression::create;
    }

    this->source = source;
}

void Parser::parse(Context *context)
{
    Tokenizer *tokenizer = new Tokenizer(source);
    std::deque<char *> *tokenQ = tokenizer->read()->createTokenQ();

    while (!tokenQ->empty()) {
        ExpressionFactory factory = factoryTable[Util::normalizeCase(tokenQ->front())];
        tokenQ->pop_front();

        if (factory) {
            expressions.push_back(factory(tokenQ));
        }
    }

    for (int i = 0, size = expressions.size(); i < size; ++i) {
        Log("%s", expressions[i]->toString().c_str());
    }

    delete tokenQ;
    delete tokenizer;
}
