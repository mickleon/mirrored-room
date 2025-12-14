#pragma once

#include <string>

#include "raylib.h"

#include "FileDialog.h"

class Button {
public:
    Rectangle rect;
    const char *text;

    bool draw();
    bool draw(bool isActive);
};

class MyUI {
private:
    Font font;
    int fontSize = 20;

    Font initFont(const char *fontPath, int fontSize);
    float rightPanelWidth = 300;

    std::string currentHint = "";
    float hintTimer = 0;
    float hintDuration = 3.0f;
    bool hintActive = false;

    Vector2 screen = Vector2{1024, 700};
    Rectangle canvas =
        Rectangle{0, 40, screen.x - rightPanelWidth, screen.y - 40};
    Rectangle panel = Rectangle{canvas.width, 0, rightPanelWidth, screen.y};

    Vector2 hintPosition;
    Rectangle hintBar;

    Button importButton = {Rectangle{5, 5, 30, 30}, "#01#"};
    Button exportButton = {Rectangle{40, 5, 30, 30}, "#02#"};
    Button normalButton = {Rectangle{90, 5, 30, 30}, "#21#"};
    Button addLineButton = {Rectangle{125, 5, 30, 30}, "#23#"};
    Button addRoundButton = {Rectangle{160, 5, 30, 30}, "#23#"};
    Button clearButton = {Rectangle{195, 5, 30, 30}, "#211#"};

public:
    enum UIMode {
        UI_NORMAL,
        UI_ADD_LINE,
        UI_ADD_ROUND,
        UI_IMPORT,
        UI_EXPORT,
        UI_CLEAR
    };

    MyUI(const char *fontPath);

    FileDialog fileDialog;

    UIMode getMode() { return mode; }

    void setMode(UIMode newMode);

    void updateSize();
    void updateHint();

    Rectangle getCanvas() { return canvas; }

    void showHint(const char *message);
    void drawPanel();
    void handleButtons(bool isClosed);

private:
    MyUI::UIMode mode = UI_NORMAL;
};
