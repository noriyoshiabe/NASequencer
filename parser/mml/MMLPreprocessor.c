#include "MMLPreprocessor.h"

typedef struct _MMLPreprocessor {
} MMLPreprocessor;

MMLPreprocessor *MMLPreprocessorCreate(ParseContext *context);
void MMLPreprocessorDestroy(MMLPreprocessor *self);
void MMLPreprocessorAppendMacro(MMLPreprocessor *self, char *difinition, int line, int column);
char *MMLPreprocessorExpandMacro(MMLPreprocessor *self, char *string, int line, int column);
