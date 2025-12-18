#include "raylib.h"
#include <cstdio>
#include <exception>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#undef RAYGUI_IMPLEMENTATION

#include "MyUI.h"
#include "Ray.h"
#include "Room.h"

const int Room::minimalDistance = 20;
const int Room::maximumPoints = 9;
const int Room::minimumPoints = 4;
const int Room::maximumRayDepth = 10;

int main() {
    MyUI ui =
        MyUI("assets/fonts/AdwaitaSans-Regular.ttf", "assets/iconset.rgi");
    Room *room = new Room();

    while (!WindowShouldClose()) {
        ui.updateSize();

        // Открытие/создание файла
        if (ui.fileDialog.isFileSelected()) {
            try {
                switch (ui.getMode()) {
                // Создание файла
                case MyUI::UI_EXPORT: {
                    ui.saveFile(room);
                    break;
                }
                // Открытие файла
                case MyUI::UI_IMPORT: {
                    room = ui.openFIle(room);
                    break;
                }
                default: break;
                }
            } catch (std::exception &e) {
                ui.showHint(e.what());
            }
            ui.setMode(MyUI::UI_NORMAL);
        }

        // Очистка экрана
        if (ui.getMode() == MyUI::UI_CLEAR) {
            room->clear();
            ui.setMode(MyUI::UI_NORMAL);
        }

        BeginDrawing();

        // Фон
        ClearBackground(LIGHTGRAY);
        DrawRectangleRec(ui.getCanvas(), RAYWHITE);

        ui.updateHint();

        // Диалог выбора файла
        if (ui.fileDialog.isActive()) {
            GuiLock();
        }

        ui.handleButtons(room->isClosed());

        // Область для рисования
        BeginScissorMode(
            ui.getCanvas().x, ui.getCanvas().y, ui.getCanvas().width,
            ui.getCanvas().height
        );

        // Рисование линий
        if (ui.getMode() == MyUI::UI_ADD_LINE) {
            if (room->isClosed()) {
                ui.showHint("Комната замкнута");
                ui.setMode(MyUI::UI_NORMAL);
            } else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
                       CheckCollisionPointRec(
                           GetMousePosition(), ui.getCanvas()
                       )) {
                try {
                    WallLine *wall = room->addWallLine(GetMousePosition());
                    if (wall) {
                        ui.showPanel(wall, nullptr);
                    }
                } catch (std::exception &e) {
                    ui.showHint(e.what());
                }
            }
        }

        // Рисование дуг
        if (ui.getMode() == MyUI::UI_ADD_ROUND) {
            if (room->isClosed()) {
                ui.showHint("Комната замкнута");
                ui.setMode(MyUI::UI_NORMAL);
            } else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
                       CheckCollisionPointRec(
                           GetMousePosition(), ui.getCanvas()
                       )) {
                try {
                    WallRound *wall = room->addWallRound(GetMousePosition());
                    if (wall) {
                        ui.showPanel(wall, nullptr);
                    }
                } catch (std::exception &e) {
                    ui.showHint(e.what());
                }
            }
        }

        // Добавление цели
        if (ui.getMode() == MyUI::UI_ADD_AIM) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
                CheckCollisionPointRec(GetMousePosition(), ui.getCanvas())) {
                room->addAim(GetMousePosition());
            }
        }

        // Добавление луча
        if (ui.getMode() == MyUI::UI_ADD_RAY) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
                CheckCollisionPointRec(GetMousePosition(), ui.getCanvas())) {
                try {
                    room->addRay(GetMousePosition());
                } catch (const std::exception &e) {
                    ui.showHint(e.what());
                }

                ui.showPanel(nullptr, room->rayStart);
            }
        }

        room->draw();
        EndScissorMode();

        // Правая панель
        if (ui.getMode() == MyUI::UI_NORMAL ||
            ui.getMode() == MyUI::UI_EDIT_ROUND ||
            ui.getMode() == MyUI::UI_EDIT_RAY) {
            if (CheckCollisionPointRec(GetMousePosition(), ui.getCanvas())) {
                Wall *closest = room->closestWall(GetMousePosition());
                RayStart *ray = room->closestRay(GetMousePosition());
                if (ray) {
                    SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                } else if (closest) {
                    SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                } else {
                    SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                }
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (ray) {
                        ui.setMode(MyUI::UI_EDIT_RAY);
                        ui.showPanel(nullptr, ray);
                    } else if (closest) {
                        ui.setMode(MyUI::UI_EDIT_ROUND);
                        ui.showPanel(closest, nullptr);
                    } else {
                        ui.setMode(MyUI::UI_NORMAL);
                        ui.showPanel(nullptr, nullptr);
                    }
                }
            }
        }
        ui.drawPanel();

        GuiUnlock();
        ui.fileDialog.update();

        EndDrawing();
    }

    CloseWindow();
    delete room;
    return 0;
}
