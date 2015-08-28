#pragma once

typedef struct _CLI CLI;

extern CLI *CLICreate(const char *filepath);
extern void CLIRun(CLI *self);
extern void CLISigInt(CLI *self);
extern void CLIDestroy(CLI *self);
