#pragma once

#include <string>

#include "raylib.h"

class MyUI {
private:
    Font initFont(const char *fontPath, int fontSize);
    float rightPanelWidth;

    std::string currentHint;
    float hintTimer;
    float hintDuration;
    bool hintActive;

    Rectangle hintBar;

public:
    Font font;
    int fontSize;

    Vector2 screen;
    Rectangle topPanel;
    Rectangle bottomPanel;
    Rectangle canvas;
    Vector2 hintPosition;

    MyUI(const char *fontPath);
    void updateSize();
    void updateHint();
    void showHint(const char *message);
    void drawPanels();
};
