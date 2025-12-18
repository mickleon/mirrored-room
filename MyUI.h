#pragma once

#include <string>

#include "raylib.h"

#include "FileDialog.h"
#include "Room.h"

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
    Wall *wall = nullptr;
    RayStart *rayStart = nullptr;

    Vector2 hintPosition;
    Rectangle hintBar;

    Button importButton = {Rectangle{5, 5, 30, 30}, "#05#"};
    Button exportButton = {Rectangle{40, 5, 30, 30}, "#02#"};
    Button normalButton = {Rectangle{90, 5, 30, 30}, "#21#"};
    Button addLineButton = {Rectangle{125, 5, 30, 30}, "#23#"};
    Button addRoundButton = {Rectangle{160, 5, 30, 30}, "#22#"};
    Button addRayButton = {Rectangle{210, 5, 30, 30}, "#145#"};
    Button clearButton = {Rectangle{260, 5, 30, 30}, "#24#"};

public:
    enum UIMode {
        UI_NORMAL,
        UI_ADD_LINE,
        UI_ADD_ROUND,
        UI_ADD_RAY,
        UI_EDIT_LINE,
        UI_EDIT_ROUND,
        UI_EDIT_RAY,
        UI_IMPORT,
        UI_EXPORT,
        UI_CLEAR
    };

    MyUI(const char *fontPath, const char *iconsPath);

    FileDialog fileDialog;

    UIMode getMode() { return mode; }

    void setMode(UIMode newMode);

    void updateSize();
    void updateHint();

    Rectangle getCanvas() { return canvas; }

    void saveFile(Room *room);
    Room *openFIle(Room *room);

    void showHint(const char *message);
    void showPanel(Wall *wall, RayStart *rayStart);
    void drawPanel();
    void handleButtons(bool isClosed);

private:
    MyUI::UIMode mode = UI_NORMAL;
};
