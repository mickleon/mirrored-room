#include "raygui.h"
#include "raylib.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <vector>

#include "MyUI.h"

namespace fs = std::filesystem;
using std::string, std::vector;

FileDialog::FileDialog() {
    currentPath = fs::current_path().string();
    refreshDirectory();
}

string FileDialog::file() const {
    return selectedFile;
}

void FileDialog::show(FileDialogMode mode) {
    showDialog = true;
    dialogMode = mode;
    dialogBounds.x = 10;
    dialogBounds.y = 50;
    currentPath = fs::current_path().string();
    refreshDirectory();

    // Очищаем буфер имени файла при открытии диалога сохранения
    if (mode == DIALOG_SAVE) {
        memset(fileNameBuffer, 0, sizeof(fileNameBuffer));
    }
}

void FileDialog::update() {
    if (!showDialog) {
        return;
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        showDialog = false;
    }

    if (IsKeyPressed(KEY_ENTER) && !fileNameEditMode) {
        if (selectedIndex >= 0) {
            handleItemSelection(selectedIndex);
        } else if (dialogMode == DIALOG_SAVE && fileNameBuffer[0] != '\0') {
            // Сохранение с введенным именем файла
            confirmSelection();
        }
    }
}

void FileDialog::draw() {
    if (!showDialog) {
        return;
    }

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0, 0, 0, 128});

    const char *title =
        (dialogMode == DIALOG_OPEN) ? "Выберите файл" : "Сохранить файл";
    GuiWindowBox(dialogBounds, title);

    Rectangle listBounds = {
        dialogBounds.x + 10, dialogBounds.y + 50, dialogBounds.width - 20,
        dialogBounds.height - (dialogMode == DIALOG_OPEN ? 100 : 135)
    };

    float buttonY = dialogBounds.y + dialogBounds.height - 40;

    if (dialogMode == DIALOG_SAVE) {
        Rectangle fileNameRect = {
            dialogBounds.x + 10, buttonY - 40, dialogBounds.width - 20, 30
        };
        if (GuiTextBox(fileNameRect, fileNameBuffer, 255, fileNameEditMode)) {
            fileNameEditMode = !fileNameEditMode;
        }
    }

    Rectangle openButton = {
        dialogBounds.x + dialogBounds.width - 120, buttonY, 100, 30
    };
    Rectangle cancelButton = {dialogBounds.x + 10, buttonY, 100, 30};
    Rectangle pathBounds = {
        dialogBounds.x + 10, dialogBounds.y + 25, dialogBounds.width - 20, 20
    };

    string displayPath = currentPath;
    int pathWidth =
        MeasureText(displayPath.c_str(), GuiGetStyle(DEFAULT, TEXT_SIZE)) *
        0.75;
    int maxWidth = pathBounds.width;

    if (pathWidth > maxWidth) {
        string ellipsis = "...";
        int ellipsisWidth =
            MeasureText(ellipsis.c_str(), GuiGetStyle(DEFAULT, TEXT_SIZE)) *
            0.75;

        fs::path pathObj = fs::path(currentPath);
        vector<string> parts;

        for (const auto &part : pathObj) {
            parts.push_back(part.string());
        }

        size_t startIndex = 0;
        while (startIndex < parts.size() &&
               pathWidth + ellipsisWidth > maxWidth) {
            startIndex++;

            fs::path tempPath;
            for (size_t i = startIndex; i < parts.size(); i++) {
                tempPath /= parts[i];
            }
            displayPath = tempPath.string();
            pathWidth = MeasureText(
                            displayPath.c_str(), GuiGetStyle(DEFAULT, TEXT_SIZE)
                        ) *
                        0.75;
        }

        if (startIndex > 0) {
            displayPath = ellipsis + displayPath;
        }
    }

    GuiLabel(pathBounds, displayPath.c_str());

    Rectangle contentBounds = {
        0, 0, listBounds.width - 20,
        (float)((directories.size() + files.size()) * 30 + 10)
    };
    Rectangle scrollBar = {0};
    GuiScrollPanel(
        listBounds, NULL, contentBounds, &scrollPosition, &scrollBar
    );

    BeginScissorMode(
        listBounds.x, listBounds.y, listBounds.width, listBounds.height
    );

    float yPos = listBounds.y + scrollPosition.y;
    int itemIndex = 0;

    for (size_t i = 0; i < directories.size(); i++) {
        Rectangle itemRect = {
            listBounds.x + 5, yPos, listBounds.width - 30, 25
        };

        if (itemIndex == selectedIndex) {
            DrawRectangleRec(itemRect, {100, 100, 100, 128});
        }

        string displayName = (i == 0 ? "#03# " : "#01# ") + directories[i];
        if (GuiButton(itemRect, displayName.c_str()) &
            CheckCollisionPointRec(GetMousePosition(), listBounds)) {
            handleItemSelection(itemIndex);
        }

        yPos += 30;
        itemIndex++;
    }

    for (size_t i = 0; i < files.size(); i++) {
        Rectangle itemRect = {
            listBounds.x + 5, yPos, listBounds.width - 30, 25
        };

        if (itemIndex == selectedIndex) {
            DrawRectangleRec(itemRect, {100, 100, 100, 128});
        }

        string displayName = "#10# " + files[i];
        if (GuiButton(itemRect, displayName.c_str()) &
            CheckCollisionPointRec(GetMousePosition(), listBounds)) {
            handleItemSelection(itemIndex);
        }

        yPos += 30;
        itemIndex++;
    }

    EndScissorMode();

    const char *actionButtonText =
        (dialogMode == DIALOG_OPEN) ? "Открыть" : "Сохранить";
    if (GuiButton(openButton, actionButtonText)) {
        confirmSelection();
    }

    if (GuiButton(cancelButton, "Отмена")) {
        showDialog = false;
    }
}

void FileDialog::clearSelection() {
    selectedFile.clear();
}

void FileDialog::refreshDirectory() {
    directories.clear();
    files.clear();
    selectedIndex = -1;

    try {
        if (currentPath != fs::path(currentPath).root_path().string()) {
            directories.push_back("..");
        }

        for (const auto &entry : fs::directory_iterator(currentPath)) {
            if (entry.is_directory()) {
                directories.push_back(entry.path().filename().string());
            } else if (entry.is_regular_file()) {
                files.push_back(entry.path().filename().string());
            }
        }

        std::sort(directories.begin(), directories.end());
        std::sort(files.begin(), files.end());

    } catch (const std::exception &e) {
        printf(
            "%s: Ошибка чтения директории: %s", currentPath.c_str(), e.what()
        );
    }
}

void FileDialog::handleItemSelection(int index) {
    if (index < (int)directories.size()) {
        string dirName = directories[index];

        if (dirName == "..") {
            fs::path parentPath = fs::path(currentPath).parent_path();
            if (!parentPath.empty()) {
                currentPath = parentPath.string();
            }
        } else {
            currentPath = (fs::path(currentPath) / dirName).string();
        }

        refreshDirectory();
        selectedIndex = -1;

    } else if (index < (int)(directories.size() + files.size())) {
        int fileIndex = index - directories.size();

        if (dialogMode == DIALOG_OPEN) {
            selectedFile = (fs::path(currentPath) / files[fileIndex]).string();
            showDialog = false;
        } else {
            selectedIndex = index;
            strncpy(
                fileNameBuffer, files[fileIndex].c_str(),
                sizeof(fileNameBuffer) - 1
            );
            fileNameBuffer[sizeof(fileNameBuffer) - 1] = '\0';
        }
    }
}

void FileDialog::confirmSelection() {
    if (dialogMode == DIALOG_OPEN) {
        if (selectedIndex >= 0 && selectedIndex >= (int)directories.size()) {
            int fileIndex = selectedIndex - directories.size();
            selectedFile = (fs::path(currentPath) / files[fileIndex]).string();
            showDialog = false;
        }
    } else { // DIALOG_SAVE
        if (fileNameBuffer[0] != '\0') {
            string fileName = fileNameBuffer;

            fs::path filePath(fileName);
            if (!filePath.has_extension()) {
                fileName += ".json";
            }

            selectedFile = (fs::path(currentPath) / fileNameBuffer).string();
            showDialog = false;
        }
    }
}

string FileDialog::getItemName(int index) const {
    if (index < (int)directories.size()) {
        return directories[index];
    } else if (index < (int)(directories.size() + files.size())) {
        int fileIndex = index - directories.size();
        return files[fileIndex];
    }
    return "";
}

bool Button::draw() {
    return GuiButton(rect, text);
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

MyUI::MyUI(const char *fontPath) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(1024, 700, "Зеркaльная комната");

    fontSize = 20;
    font = initFont(fontPath, fontSize);
    screen = Vector2{1024, 700};
    rightPanelWidth = 300;
    canvas = Rectangle{0, 40, screen.x - rightPanelWidth, screen.y - 40};
    panel = Rectangle{canvas.width, 0, rightPanelWidth, screen.y};

    importButton = {Rectangle{5, 5, 30, 30}, "#01#"};
    exportButton = {Rectangle{40, 5, 30, 30}, "#02#"};
    normalButton = {Rectangle{90, 5, 30, 30}, "#21#"};
    addLineButton = {Rectangle{125, 5, 30, 30}, "#23#"};
    addRoundButton = {Rectangle{160, 5, 30, 30}, "#23#"};

    fileDialog = FileDialog();

    mode = Normal;

    updateSize();

    currentHint = "";
    hintTimer = 0;
    hintDuration = 3.0f;
    hintActive = false;

    SetWindowMinSize(500, 450);
    SetTargetFPS(60);
    SetExitKey(-1);

    GuiSetFont(font);
    GuiSetStyle(DEFAULT, TEXT_SIZE, fontSize);
}

void MyUI::showHint(const char *message) {
    currentHint = message;
    hintTimer = 0;
    hintActive = true;
}

void MyUI::drawPanel() {
    GuiPanel(panel, "Панель 1");
}

void MyUI::handleButtons() {
    UIMode newMode = mode;
    if (importButton.draw()) {
        newMode = Import;
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        fileDialog.show(DIALOG_OPEN);
    }

    if (exportButton.draw()) {
        newMode = Export;
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        fileDialog.show(DIALOG_SAVE);
    }

    if (normalButton.draw()) {
        newMode = Normal;
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }

    if (addLineButton.draw()) {
        newMode = AddLine;
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    }

    if (addRoundButton.draw()) {
        newMode = AddRound;
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    }

    mode = newMode;
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
