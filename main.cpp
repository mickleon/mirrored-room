#include "nlohmann/json.hpp"
#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "MyUI.h"
#include "Room.h"

const Color Wall::color = BROWN;
const float Wall::thick = 4;

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(1024, 700, "Зеркaльная комната");
    SetWindowMinSize(400, 300);
    SetTargetFPS(60);
    SetExitKey(-1);

    MyUI ui = MyUI("assets/fonts/AdwaitaSans-Regular.ttf");
    GuiSetFont(ui.font);
    GuiSetStyle(DEFAULT, TEXT_SIZE, ui.fontSize);

    Room room;

    while (!WindowShouldClose()) {
        ui.updateSize();
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Область для рисования
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            CheckCollisionPointRec(GetMousePosition(), ui.canvas)) {
            try {
                room.addPoint(GetMousePosition());
            } catch (Room::PointsAreTooClose) {
                ui.drawHint("Точки не могут находиться слишком близко");
            }
        }
        room.draw();
        EndScissorMode();

        ui.drawPanels();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
