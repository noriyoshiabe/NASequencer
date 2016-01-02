#include "NALog.h"

const int NALogLevelSilent = 0;
const int NALogLevelDebug = 1;
const int NALogLevelTrace = 2;

#ifdef __TEST__
int NALogLevel = NALogLevelDebug;
#else
int NALogLevel = NALogLevelSilent;
#endif
FILE *NALogStream = NULL;

__attribute__((constructor))
static void initialize()
{
    NALogStream = stderr;
}
