#include <stdarg.h>

typedef struct _DebugWindow DebugWindow;

extern DebugWindow *DebugWindowCreate(int width, int height);
extern void DebugWindowAppendLog(DebugWindow *self, const char *fmt, va_list arglist);
extern void DebugWindowDestroy(DebugWindow *self);
