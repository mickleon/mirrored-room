#include "nlohmann/json.hpp"
#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "MyUI.h"
#include "Room.h"

const Color Wall::color = BROWN;
const float Wall::thick = 4;

int main() {
    MyUI ui = MyUI("assets/fonts/AdwaitaSans-Regular.ttf");
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
                ui.showHint("Точки не могут находиться слишком близко");
            }
        }
        room.draw();
        EndScissorMode();

        ui.updateHint();
        ui.drawPanels();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
