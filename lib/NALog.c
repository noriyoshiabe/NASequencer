#include "NALog.h"

const int NALogLevelSilent = 0;
const int NALogLevelDebug = 1;
const int NALogLevelTrace = 2;

int NALogLevel = NALogLevelSilent;
FILE *NALogStream = NULL;

__attribute__((constructor))
static void initialize()
{
    NALogStream = stderr;
}
