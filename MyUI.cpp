#include <cstdio>
#include <cstring>

#include "raygui.h"
#include "raylib.h"

#include "MyUI.h"

using std::string;

bool Button::draw() {
    return GuiButton(rect, text);
}

bool Button::draw(bool isActive) {
    int status = GuiButton(rect, text);
    if (isActive) {
        DrawRectangleRec(rect, ColorAlpha(BLUE, 0.2));
    }
    return status;
}

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

    font = initFont(fontPath, fontSize);
    fileDialog = FileDialog();

    updateSize();

    SetWindowMinSize(520, 460);
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

void MyUI::drawPanel() {
    GuiPanel(panel, "Панель 1");
}

void MyUI::setMode(UIMode newMode) {
    switch (newMode) {
    case UI_IMPORT: {
        mode = UI_IMPORT;
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        fileDialog.show(FileDialog::FILE_DIALOG_OPEN);
        break;
    }
    case UI_EXPORT: {
        mode = UI_EXPORT;
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        fileDialog.show(FileDialog::FILE_DIALOG_SAVE);
        break;
    }
    case UI_NORMAL: {
        mode = UI_NORMAL;
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        break;
    }
    case UI_ADD_LINE: {
        mode = UI_ADD_LINE;
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        break;
    }
    case UI_ADD_ROUND: {
        mode = UI_ADD_ROUND;
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        break;
    }
    case UI_CLEAR: {
        mode = UI_CLEAR;
    }
    }
}

void MyUI::handleButtons(bool isClosed) {
    if (importButton.draw()) {
        setMode(UI_IMPORT);
    }

    if (exportButton.draw()) {
        setMode(UI_EXPORT);
    }

    if (normalButton.draw(getMode() == UI_NORMAL)) {
        setMode(UI_NORMAL);
    }

    if (addLineButton.draw(getMode() == UI_ADD_LINE)) {
        if (isClosed) {
            showHint("Комната замкнута");
        } else {
            setMode(UI_ADD_LINE);
        }
    }

    if (addRoundButton.draw(getMode() == UI_ADD_ROUND)) {
        if (isClosed) {
            showHint("Комната замкнута");
        } else {
            setMode(UI_ADD_ROUND);
        }
    }

    if (clearButton.draw()) {
        setMode(UI_CLEAR);
    }
}

void MyUI::updateSize() {
    screen.x = GetScreenWidth();
    screen.y = GetScreenHeight();

    canvas.x = 0;
    canvas.y = 40;
    canvas.width = screen.x - rightPanelWidth;
    canvas.height = screen.y - 40;

    panel.x = canvas.width;
    panel.y = 0;
    panel.width = rightPanelWidth;
    panel.height = screen.y;

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
