#include "raylib.h"
#include <cstdio>
#include <exception>

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

        // Открытие/закрытие файла
        if (ui.fileDialog.isFileSelected()) {
            try {
                switch (ui.getMode()) {
                case UI_IMPORT: {
                    room.load(ui.fileDialog.filePath());
                    break;
                }
                case UI_EXPORT: {
                    room.save(ui.fileDialog.filePath());
                    break;
                }
                default: break;
                }
            } catch (std::exception &e) {
                ui.showHint(e.what());
            }
        }

        if (ui.getMode() == UI_CLEAR) {
            room.clear();
            ui.setMode(UI_NORMAL);
        }

        BeginDrawing();

        ClearBackground(LIGHTGRAY);
        DrawRectangleRec(ui.getCanvas(), RAYWHITE);
        ui.updateHint();

        // Диалог выбора файла
        if (ui.fileDialog.isActive()) {
            GuiLock();
        }

        ui.handleButtons(room.isClosed());

        // Область для рисования
        BeginScissorMode(
            ui.getCanvas().x, ui.getCanvas().y, ui.getCanvas().width,
            ui.getCanvas().height
        );

        // Рисование линий
        if (ui.getMode() == UI_ADD_LINE) {
            if (room.isClosed()) {
                ui.showHint("Комната замкнута");
                ui.setMode(UI_NORMAL);
            } else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
                       CheckCollisionPointRec(
                           GetMousePosition(), ui.getCanvas()
                       )) {
                try {
                    room.addWallLine(GetMousePosition());
                } catch (std::exception &e) {
                    ui.showHint(e.what());
                }
            }
        }

        // Рисование дуг
        if (ui.getMode() == UI_ADD_ROUND) {
            if (room.isClosed()) {
                ui.showHint("Комната замкнута");
                ui.setMode(UI_NORMAL);
            } else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
                       CheckCollisionPointRec(
                           GetMousePosition(), ui.getCanvas()
                       )) {
                try {
                    room.addWallRound(GetMousePosition());
                } catch (std::exception &e) {
                    ui.showHint(e.what());
                }
            }
        }

        room.draw();
        EndScissorMode();

        // Правая панель
        ui.drawPanel();

        GuiUnlock();
        if (ui.fileDialog.update()) {
            ui.setMode(UI_NORMAL);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
