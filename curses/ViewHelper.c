#include "ViewHelper.h"
#include "Attribute.h"

#include <string.h>

void ViewHelperDisplayLevel(View *view, int yOffset, Level level)
{
    Size size = ViewGetFrame(view).size;
    int yL = yOffset;
    int yR = yOffset + 1;

    int levelMax = (size.width - 3);
    int peak = levelMax / 10;
    int levelL = (int)(((MAX(-500, level.L) + 500) / 500.0) * levelMax);
    int levelR = (int)(((MAX(-500, level.R) + 500) / 500.0) * levelMax);

    char bar[levelMax + 1];

    ViewSetAttr(view, AttributeDefault);
    ViewPrintf(view, 0, yL, "L: ");

    ViewSetAttr(view, Attribute(ColorLevelNone, false, true));
    memset(bar, '|', levelMax);
    bar[levelMax] = '\0';
    ViewPrintf(view, 3, yL, bar);

    ViewSetAttr(view, Attribute(ColorLevelLow, false, true));
    int levelGreenL = MIN(levelL, levelMax - peak);
    memset(bar, '|', levelGreenL);
    bar[levelGreenL] = '\0';
    ViewPrintf(view, 3, yL, bar);
    if (levelGreenL < levelL) {
        ViewSetAttr(view, Attribute(ColorLevelHigh, false, true));
        bar[levelL - levelGreenL] = '\0';
        ViewPrintf(view, size.width - peak, yL, bar);
    }

    ViewSetAttr(view, ColorDefault);
    ViewPrintf(view, 0, yR, "R: ");

    ViewSetAttr(view, Attribute(ColorLevelNone, false, true));
    memset(bar, '|', levelMax);
    bar[levelMax] = '\0';
    ViewPrintf(view, 3, yR, bar);

    ViewSetAttr(view, Attribute(ColorLevelLow, false, true));
    int levelGreenR = MIN(levelR, levelMax - peak);
    memset(bar, '|', levelGreenR);
    bar[levelGreenR] = '\0';
    ViewPrintf(view, 3, yR, bar);
    if (levelGreenR < levelR) {
        ViewSetAttr(view, Attribute(ColorLevelHigh, false, true));
        bar[levelR - levelGreenR] = '\0';
        ViewPrintf(view, size.width - peak, yR, bar);
    }
}
