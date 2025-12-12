#pragma once

#include <string>
#include <vector>

#include "raylib.h"

using std::string, std::vector;

enum FileDialogMode { DIALOG_OPEN, DIALOG_SAVE };

class FileDialog {
private:
    bool showDialog = false;
    FileDialogMode dialogMode = DIALOG_OPEN;
    string currentPath;
    string selectedFile;
    vector<string> directories;
    vector<string> files;
    Vector2 scrollPosition = {0, 0};
    Rectangle dialogBounds = {50, 80, 400, 400};
    int selectedIndex = -1;

    char fileNameBuffer[256] = {0};
    bool fileNameEditMode = false;

public:
    FileDialog();

    string file() const;

    void show(FileDialogMode mode);

    void update();

    void draw();

    void clearSelection();

private:
    void refreshDirectory();

    void handleItemSelection(int index);

    void confirmSelection();

    string getItemName(int index) const;
};

class Button {
public:
    Rectangle rect;
    const char *text;

    bool draw();
};

enum UIMode { Normal, AddLine, AddRound, Import, Export };

class MyUI {
private:
    Font font;
    int fontSize;

    Font initFont(const char *fontPath, int fontSize);
    float rightPanelWidth;

    std::string currentHint;
    float hintTimer;
    float hintDuration;
    bool hintActive;

    Vector2 screen;
    Rectangle panel;
    Rectangle canvas;
    Vector2 hintPosition;
    Rectangle hintBar;

    Button importButton;
    Button exportButton;
    Button normalButton;
    Button addLineButton;
    Button addRoundButton;

public:
    UIMode mode;
    FileDialog fileDialog;

    MyUI(const char *fontPath);
    void updateSize();
    void updateHint();

    Rectangle getCanvas() { return canvas; }

    void showHint(const char *message);
    void drawPanel();
    void handleButtons();
};
