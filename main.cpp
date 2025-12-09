#include "nlohmann/json.hpp"
#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "Room.h"

Font InitFont(const char *fontPath, int fontSize) {
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

const Color Wall::color = BROWN;
const float Wall::thick = 4;

int main() {
    // Инициализация окна
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    const int screenWidth = 1024;
    const int screenHeight = 700;
    const int rightPanelWidth = 300;
    InitWindow(screenWidth, screenHeight, "Зеркaльная комната");
    SetWindowMinSize(400, 300);
    SetTargetFPS(60);
    SetExitKey(-1);

    // Настройка шрифта
    Font font = InitFont("assets/fonts/AdwaitaSans-Regular.ttf", 20);
    GuiSetFont(font);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);

    Room room;

    Point *p1 = room.addPoint(0.0f, 0.0f);
    Point *p2 = room.addPoint(100.0f, 100.0f);
    Point *p3 = room.addPoint(200.0f, 200.0f);

    room.addWallLine(p1, p2);
    room.addWallRound(p2, p3);

    while (!WindowShouldClose()) {
        int currentWidth = GetScreenWidth();
        int currentHeight = GetScreenHeight();
        int canvasWidth = currentWidth - rightPanelWidth;
        int topPanelHeight = currentHeight / 2;
        int bottomPanelHeight = currentHeight - topPanelHeight;

        BeginDrawing();

        ClearBackground(DARKGRAY);

        // Основная область
        Rectangle canvas = {0, 0, (float)canvasWidth, (float)currentHeight};
        DrawRectangleRec(canvas, RAYWHITE);
        BeginScissorMode(0, 0, canvasWidth, currentHeight);
        if (CheckCollisionPointRec(GetMousePosition(), canvas)) {
            room.movePoint(p2, GetMousePosition().x, GetMousePosition().y);
        }
        room.draw();
        EndScissorMode();

        // Верхняя правая панель
        Rectangle topPanel = {
            (float)canvasWidth, 0.0f, (float)rightPanelWidth,
            (float)topPanelHeight
        };
        GuiPanel(topPanel, "Панель 1");
        float contentX = topPanel.x + 10.0f;
        float contentY = topPanel.y + 30.0f;

        // Нижняя правая панель
        Rectangle bottomPanel = {
            (float)canvasWidth, (float)topPanelHeight, (float)rightPanelWidth,
            (float)bottomPanelHeight
        };

        GuiPanel(bottomPanel, "Панель 2");

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
