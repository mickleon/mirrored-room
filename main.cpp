#include "raylib.h"
#include <cstdio>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#undef RAYGUI_IMPLEMENTATION

#include "MyUI.h"
#include "Room.h"

int main() {
    MyUI ui = MyUI("assets/fonts/AdwaitaSans-Regular.ttf");
    Room room;

    while (!WindowShouldClose()) {
        ui.updateSize();

        if (ui.fileDialog.isFileSelected()) {
            printf("%s\n", ui.fileDialog.filePath().c_str());
        }

        BeginDrawing();

        ClearBackground(LIGHTGRAY);
        DrawRectangleRec(ui.getCanvas(), RAYWHITE);
        ui.updateHint();
        ui.handleButtons();

        // Область для рисования
        BeginScissorMode(
            ui.getCanvas().x, ui.getCanvas().y, ui.getCanvas().width,
            ui.getCanvas().height
        );
        if (ui.mode == AddLine && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            CheckCollisionPointRec(GetMousePosition(), ui.getCanvas())) {
            try {
                room.addPoint(GetMousePosition());
            } catch (Room::PointsAreTooClose) {
                ui.showHint("Точки не могут находиться слишком близко");
            }
        }
        room.draw();
        EndScissorMode();

        ui.drawPanel();

        if (ui.fileDialog.isActive()) {
            GuiLock();
        }
        GuiUnlock();
        ui.fileDialog.update();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
