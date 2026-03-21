#pragma once

typedef struct _Point {
    int x;
    int y;
} Point;

typedef struct _Size {
    int width;
    int height;
} Size;

typedef struct _Rect {
    Point point;
    Size size;
} Rect;
