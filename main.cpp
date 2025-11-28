#include "raygui.h"
#include "raylib.h"

#include "Room.h"

// Задаем параметры отрисовки
const Color Wall::color = BROWN;
const float Wall::thick = 4;

// Инициализация шрифта с кириллицей
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

int main() {
    const int windowWidth = 1000;
    const int windowHeight = 700;
    const int fontSize = 20; // In px
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(windowWidth, windowHeight, "Зеркальная комната");
    SetTargetFPS(60);
    Font font = InitFont("assets/fonts/AdwaitaSans-Regular.ttf", fontSize);

    Room room;

    Point *p1 = room.addPoint(0.0f, 0.0f);
    Point *p2 = room.addPoint(100.0f, 100.0f);
    Point *p3 = room.addPoint(200.0f, 400.0f);

    room.addWallLine(p1, p2);
    room.addWallRound(p2, p3);

    while (!WindowShouldClose()) {
        BeginDrawing();

        p2->setCoord(GetMousePosition());

        room.draw();

        ClearBackground(RAYWHITE);

        EndDrawing();
    }

    UnloadFont(font);
    CloseWindow();
    return 0;
}
