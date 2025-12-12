#pragma once

#include <string>

#include "raylib.h"

#include "FileDialog.h"

class Button {
public:
    Rectangle rect;
    const char *text;

    bool draw();
};

enum UIMode { Normal, AddLine, AddRound, Import, Export };

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

public:
    MyUI(const char *fontPath);

    FileDialog fileDialog;

    UIMode mode = Normal;

    void updateSize();
    void updateHint();

    Rectangle getCanvas() { return canvas; }

    void showHint(const char *message);
    void drawPanel();
    void handleButtons();
};
