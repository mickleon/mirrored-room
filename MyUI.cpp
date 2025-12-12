#include "raygui.h"
#include "raylib.h"
#include <cstdio>

#include "MyUI.h"

Font MyUI::initFont(const char *fontPath, int fontSize) {
    int charsCount = 0;
    int* chars = LoadCodepoints(
        "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдеёжзийклмнопрстуфхцчшщъыьэюя"
        "0123456789"
        ".,!?-+()[]{}:;/\\\"'`~@#$%^&*=_|<> "
        "QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm", 
        &charsCount
    );

    Font font = LoadFontEx(fontPath, fontSize, chars, charsCount);
    UnloadCodepoints(chars);
    return font;
}

MyUI::MyUI(const char *fontPath) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(1024, 700, "Зеркaльная комната");

    fontSize = 20;
    font = initFont(fontPath, fontSize);
    screen = Vector2{1024, 700};
    rightPanelWidth = 300;

    updateSize();

    currentHint = "";
    hintTimer = 0;
    hintDuration = 3.0f;
    hintActive = false;

    SetWindowMinSize(400, 300);
    SetTargetFPS(60);
    SetExitKey(-1);

    GuiSetFont(font);
    GuiSetStyle(DEFAULT, TEXT_SIZE, fontSize);
}

void MyUI::showHint(const char *message) {
    currentHint = message;
    hintTimer = 0;
    hintActive = true;
}

void MyUI::drawPanels() {
    // Верхняя правая панель
    GuiPanel(topPanel, "Панель 1");

    // Нижняя правая панель
    GuiPanel(bottomPanel, "Панель 2");
}

void MyUI::updateSize() {
    screen.x = GetScreenWidth();
    screen.y = GetScreenHeight();

    canvas = Rectangle{0, 0, screen.x - rightPanelWidth, screen.y};

    topPanel = Rectangle{canvas.width, 0, rightPanelWidth, screen.y / 2};
    bottomPanel =
        Rectangle{canvas.width, screen.y / 2, rightPanelWidth, screen.y / 2};

    hintPosition = {5, screen.y - 25};
    hintBar = {0, screen.y - 30, canvas.width, 30};
}

void MyUI::updateHint() {
    if (hintActive) {
        hintTimer += GetFrameTime();

        if (hintTimer >= hintDuration) {
            hintActive = false;
            currentHint = "";
        }

        DrawRectangleRec(hintBar, Fade(LIGHTGRAY, 0.5f));

        DrawTextEx(
            font, currentHint.c_str(), hintPosition, fontSize, 0, DARKGRAY
        );
    }
}
