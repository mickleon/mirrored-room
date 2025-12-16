#include <fstream>
#include <stdexcept>

#include "raygui.h"
#include "raylib.h"

#include "MyUI.h"
#include "Room.h"

using std::string, std::runtime_error;

bool Button::draw() {
    return GuiButton(rect, text);
}

bool Button::draw(bool isActive) {
    int status = GuiButton(rect, text);
    if (isActive) {
        DrawRectangleRec(rect, ColorAlpha(BLUE, 0.2));
    }
    return status;
}

Font MyUI::initFont(const char *fontPath, int fontSize) {
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

MyUI::MyUI(const char *fontPath, const char *iconsPath) {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(1024, 700, "Зеркaльная комната");

    font = initFont(fontPath, fontSize);
    fileDialog = FileDialog();

    updateSize();

    SetWindowMinSize(600, 460);
    SetTargetFPS(60);
    SetExitKey(-1);

    GuiSetFont(font);
    GuiSetStyle(DEFAULT, TEXT_SIZE, fontSize);
    GuiLoadIcons(iconsPath, false);

    GuiSetStyle(DEFAULT, TEXT_LINE_SPACING, 20);
}

Room *MyUI::openFIle(Room *room) {
    fs::path filePath = fileDialog.filePath();
    if (!fs::exists(filePath)) {
        throw std::runtime_error(
            "Файл не найден: " + filePath.filename().string()
        );
    }

    if (!fs::is_regular_file(filePath)) {
        throw runtime_error(
            "Указанный путь не является файлом: " + filePath.filename().string()
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
            "Ошибка формата файла: " + filePath.filename().string()
        );
    }

    if (file.bad()) {
        throw runtime_error(
            "Ошибка чтения файла: " + filePath.filename().string()
        );
    }

    if (!file.eof() && file.fail()) {
        throw runtime_error(
            "Ошибка формата файла: " + filePath.filename().string()
        );
    }

    file.close();

    Room *newRoom;

    try {
        newRoom = new Room(j);
    } catch (const Room::RoomException &e) {
        throw runtime_error(
            "Некорректные данные в файле: " + filePath.filename().string()
        );
    } catch (...) {
        throw runtime_error(
            "Ошибка формата файла: " + filePath.filename().string()
        );
    }

    delete room;

    room = newRoom;

    showHint(TextFormat(
        "Комната успешно загружена из файла %s",
        fileDialog.filePath().filename().c_str()
    ));
    return room;
}

void MyUI::saveFile(Room *room) {
    fs::path filePath = fileDialog.filePath();

    if (filePath.extension().empty()) {
        filePath.replace_extension(".json");
    }

    if (!fs::exists(filePath.parent_path())) {
        throw runtime_error(
            "Некоррректное имя файла: " + filePath.filename().string()
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
            "Нет прав на запись в файл: " + filePath.filename().string()
        );
    }

    file.close();

    showHint(TextFormat(
        "Файл %s успешно экспортирован",
        fileDialog.filePath().filename().c_str()
    ));
}

void MyUI::showHint(const char *message) {
    currentHint = message;
    hintTimer = 0;
    hintActive = true;
}

void MyUI::showPanel(bool active, WallRound *wallRound) {
    panelActive = active;
    MyUI::wallRound = wallRound;
}

void MyUI::drawPanel() {
    GuiPanel(panel, "Свойства");

    Rectangle label = Rectangle{panel.x + 10, panel.y + 40, panel.width, 50};

    if (panelActive) {
        Rectangle button = {panel.x + 20, panel.y + 100, 260, 30};
        if (GuiButton(button, "Изменить выпуклость")) {
            wallRound->toggleOrient();
        }

        // Ползунок
        float sliderValue = wallRound->getRadiusCoef();
        float newSliderValue = sliderValue;
        Rectangle slider = {panel.x + 100, panel.y + 50, 135, 25};
        GuiSliderBar(
            slider, "Кривизна", TextFormat("%.0f%%", sliderValue),
            &newSliderValue, 0, 100
        );
        if (sliderValue != newSliderValue) {
            wallRound->setRadiusCoef(newSliderValue);
        }
    } else if (mode == UI_NORMAL) {
        GuiLabel(label, "Кликните на объект, чтобы \nизменить его свойства");
    } else if (mode == UI_ADD_LINE) {
        GuiLabel(
            label,
            "Кликните на пустое место на \nполе, чтобы добавить плоское "
            "\nзеркало"
        );
    } else if (mode == UI_ADD_ROUND) {
        GuiLabel(
            label,
            "Кликните на пустое место на \nполе, чтобы добавить \nсферическое "
            "зеркало"
        );
    } else if (mode == UI_ADD_RAY) {
        GuiLabel(label, "Кликните на зеркало, чтобы \nотложить луч света");
    }
}

void MyUI::setMode(UIMode newMode) {
    switch (newMode) {
    case UI_IMPORT: {
        mode = UI_IMPORT;
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        fileDialog.show(FileDialog::FILE_DIALOG_OPEN);
        break;
    }
    case UI_EXPORT: {
        mode = UI_EXPORT;
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        fileDialog.show(FileDialog::FILE_DIALOG_SAVE);
        break;
    }
    case UI_NORMAL: {
        mode = UI_NORMAL;
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        break;
    }
    case UI_ADD_LINE: {
        mode = UI_ADD_LINE;
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        break;
    }
    case UI_ADD_ROUND: {
        mode = UI_ADD_ROUND;
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        break;
    }
    case MyUI::UI_EDIT_ROUND: {
        mode = UI_EDIT_ROUND;
        SetMouseCursor(MOUSE_CURSOR_ARROW);
        break;
    }
    case MyUI::UI_ADD_RAY: {
        mode = UI_ADD_RAY;
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        break;
    }
    case UI_CLEAR: {
        mode = UI_CLEAR;
        panelActive = false;
        wallRound = nullptr;
        break;
    }
    }
}

void MyUI::handleButtons(bool isClosed) {
    if (importButton.draw()) {
        setMode(UI_IMPORT);
    }

    if (exportButton.draw()) {
        setMode(UI_EXPORT);
    }

    if (normalButton.draw(getMode() == UI_NORMAL)) {
        setMode(UI_NORMAL);
    }

    GuiLabel(addLineButton.rect, "#24#");
    if (addLineButton.draw(getMode() == UI_ADD_LINE)) {
        if (isClosed) {
            showHint("Комната замкнута");
        } else {
            setMode(UI_ADD_LINE);
        }
    }

    if (addRoundButton.draw(getMode() == UI_ADD_ROUND)) {
        if (isClosed) {
            showHint("Комната замкнута");
        } else {
            setMode(UI_ADD_ROUND);
        }
    }

    if (addRayButton.draw(getMode() == UI_ADD_RAY)) {
        setMode(UI_ADD_RAY);
    }

    if (clearButton.draw()) {
        setMode(UI_CLEAR);
    }
}

void MyUI::updateSize() {
    screen.x = GetScreenWidth();
    screen.y = GetScreenHeight();

    canvas.x = 0;
    canvas.y = 40;
    canvas.width = screen.x - rightPanelWidth;
    canvas.height = screen.y - 40;

    panel.x = canvas.width;
    panel.y = 0;
    panel.width = rightPanelWidth;
    panel.height = screen.y;

    hintPosition = {5, screen.y - 25};
    hintBar = {0, screen.y - 30, canvas.width, 30};
}

void MyUI::updateHint() {
    if (hintActive) {
        hintTimer += GetFrameTime();

        if (hintTimer >= hintDuration) {
            hintActive = false;
            currentHint = "";
        }

        DrawRectangleRec(hintBar, Fade(LIGHTGRAY, 0.5f));

        DrawTextEx(
            font, currentHint.c_str(), hintPosition, fontSize, 0, DARKGRAY
        );
    }
}
