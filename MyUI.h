#pragma once

#include "raylib.h"

class MyUI {
public:
    Font font;
    int fontSize;

    Vector2 screen;

    Font initFont(const char *fontPath, int fontSize);

    float rightPanelWidth;
    Rectangle topPanel;
    Rectangle bottomPanel;

    Rectangle canvas;
    Vector2 hintPosition;

    MyUI(const char *fontPath);

    void updateSize();

    void drawHint(const char *message);

    void drawPanels();
};
