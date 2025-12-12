#pragma once

#include <string>

#include "raylib.h"

enum Mode { Normal, AddLine, AddRound };

class Button {
public:
    Rectangle rect;
    const char *text;

    bool draw();
};

class MyUI {
private:
    Font initFont(const char *fontPath, int fontSize);
    float rightPanelWidth;

    std::string currentHint;
    float hintTimer;
    float hintDuration;
    bool hintActive;

    Rectangle hintBar;

    Button importButton;
    Button exportButton;
    Button normalButton;
    Button addLineButton;
    Button addRoundButton;

public:
    Font font;
    int fontSize;

    Vector2 screen;
    Rectangle panel;
    Rectangle canvas;
    Vector2 hintPosition;

    Mode mode;

    MyUI(const char *fontPath);
    void updateSize();
    void updateHint();

    void showHint(const char *message);
    void drawPanel();
    void handleButtons();
};
