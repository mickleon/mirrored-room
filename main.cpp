#include "nlohmann/json.hpp"
#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "MyUI.h"
#include "Room.h"

int main() {
    MyUI ui = MyUI("assets/fonts/AdwaitaSans-Regular.ttf");
    Room room;

    while (!WindowShouldClose()) {
        ui.updateSize();
        ui.fileDialog.update();

        if (!ui.fileDialog.file().empty()) {
            std::string filePath = ui.fileDialog.file();

            if (ui.mode == Import) {
                printf("Открыть файл: %s\n", filePath.c_str());
                // room.load(filePath);
            } else if (ui.mode == Export) {
                printf("Сохранить в файл: %s\n", filePath.c_str());
                // room.save(filePath);
            }

            ui.fileDialog.clearSelection();
            ui.mode = Normal;
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
        ui.fileDialog.draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
