#pragma once

#include <ncurses.h>

typedef enum {
    ColorDefault,
    ColorMute,
    ColorSolo,
    ColorLevelLow,
    ColorLevelHigh,
} Color;

typedef enum {
    StateNormal,
    StateFocused,
    StateSelected,
} State;

static inline void AttributeInitializeColorPair()
{
    init_pair(ColorMute, COLOR_BLUE, COLOR_YELLOW);
    init_pair(ColorSolo, COLOR_BLUE, COLOR_GREEN);
    init_pair(ColorLevelLow, COLOR_GREEN, -1);
    init_pair(ColorLevelHigh, COLOR_RED, -1);
}

static inline int Attribute(Color color, bool focused, bool selected)
{
    int attrs = A_NORMAL;

    attrs |= focused ? A_UNDERLINE : 0;
    attrs |= focused && selected ? A_BOLD: 0;
    
    return attrs | COLOR_PAIR(color);
}

#define AttributeDefault A_NORMAL
