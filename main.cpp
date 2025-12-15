#include <exception>
#include <filesystem>
#include <fstream>

#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#undef RAYGUI_IMPLEMENTATION

#include "MyUI.h"
#include "Room.h"

using std::runtime_error;
namespace fs = std::filesystem;

int main() {
    MyUI ui = MyUI("assets/fonts/AdwaitaSans-Regular.ttf");
    Room *room = new Room();

    while (!WindowShouldClose()) {
        ui.updateSize();

        // Открытие/закрытие файла
        if (ui.fileDialog.isFileSelected()) {
            try {
                switch (ui.getMode()) {
                case MyUI::UI_EXPORT: {
                    fs::path filePath = ui.fileDialog.filePath();

                    if (filePath.extension().empty()) {
                        filePath.replace_extension(".json");
                    }

                    if (!fs::exists(filePath.parent_path())) {
                        throw runtime_error(
                            "Некоррректное имя файла: " +
                            filePath.filename().string()
                        );
                    }

                    std::ofstream file(filePath);

                    if (!file.is_open()) {
                        throw runtime_error(
                            "Не удалось открыть файл для записи: " +
                            filePath.filename().string()
                        );
                    }

                    file << room->to_json().dump(2) << '\n';

                    if (file.fail()) {
                        throw runtime_error(
                            "Нет прав на запись в файл: " +
                            filePath.filename().string()
                        );
                    }

                    file.close();

                    ui.showHint(TextFormat(
                        "Файл %s успешно экспортирован",
                        ui.fileDialog.filePath().filename().c_str()
                    ));
                    break;
                }
                case MyUI::UI_IMPORT: {
                    fs::path filePath = ui.fileDialog.filePath();
                    if (!fs::exists(filePath)) {
                        throw std::runtime_error(
                            "Файл не найден: " + filePath.filename().string()
                        );
                    }

                    if (!fs::is_regular_file(filePath)) {
                        throw runtime_error(
                            "Указанный путь не является файлом: " +
                            filePath.filename().string()
                        );
                    }

                    std::ifstream file(filePath);

                    if (!file.is_open()) {
                        throw runtime_error(
                            "Не удалось открыть файл для чтения: " +
                            filePath.filename().string()
                        );
                    }

                    json j;

                    try {
                        file >> j;
                    } catch (json::exception &e) {
                        throw runtime_error(
                            "Ошибка формата файла: " +
                            filePath.filename().string()
                        );
                    }

                    if (file.bad()) {
                        throw runtime_error(
                            "Ошибка чтения файла: " +
                            filePath.filename().string()
                        );
                    }

                    if (!file.eof() && file.fail()) {
                        throw runtime_error(
                            "Ошибка формата файла: " +
                            filePath.filename().string()
                        );
                    }

                    file.close();

                    Room *newRoom;

                    try {
                        newRoom = new Room(j);
                    } catch (const Room::RoomException &e) {
                        throw runtime_error(
                            "Некорректные данные в файле: " +
                            filePath.filename().string()
                        );
                    } catch (...) {
                        throw runtime_error(
                            "Ошибка формата файла: " +
                            filePath.filename().string()
                        );
                    }

                    delete room;

                    room = newRoom;

                    ui.showHint(TextFormat(
                        "Комната успешно загружена из файла %s",
                        ui.fileDialog.filePath().filename().c_str()
                    ));
                    break;
                }
                default: break;
                }
            } catch (std::exception &e) {
                ui.showHint(e.what());
            }
            ui.setMode(MyUI::UI_NORMAL);
        }

        if (ui.getMode() == MyUI::UI_CLEAR) {
            room->clear();
            ui.setMode(MyUI::UI_NORMAL);
        }

        BeginDrawing();

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
                    room->addWallLine(GetMousePosition());
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
                    room->addWallRound(GetMousePosition());
                } catch (std::exception &e) {
                    ui.showHint(e.what());
                }
            }
        }

        room->draw();
        EndScissorMode();

        // Правая панель
        ui.drawPanel();

        GuiUnlock();
        ui.fileDialog.update();

        EndDrawing();
    }

    CloseWindow();
    delete room;
    return 0;
}
