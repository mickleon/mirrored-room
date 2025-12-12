#include "nlohmann/json.hpp"
#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "MyUI.h"
#include "Room.h"

const Color Wall::color = BROWN;
const float Wall::thick = 4;
const int Room::minimalDistance = 20;

int main() {
    MyUI ui = MyUI("assets/fonts/AdwaitaSans-Regular.ttf");
    Room room;

    while (!WindowShouldClose()) {
        ui.updateSize();

        BeginDrawing();

        ClearBackground(LIGHTGRAY);
        DrawRectangleRec(ui.canvas, RAYWHITE);
        ui.updateHint();
        ui.handleButtons();

        // Область для рисования
        BeginScissorMode(
            ui.canvas.x, ui.canvas.y, ui.canvas.width, ui.canvas.height
        );
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            CheckCollisionPointRec(GetMousePosition(), ui.canvas)) {
            try {
                room.addPoint(GetMousePosition());
            } catch (Room::PointsAreTooClose) {
                ui.showHint("Точки не могут находиться слишком близко");
            }
        }
        room.draw();
        EndScissorMode();

        ui.drawPanel();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
