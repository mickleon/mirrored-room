#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "raylib.h"

namespace fs = std::filesystem;

#define MAX_DIRECTORY_FILES 2048
#define MAX_ICON_PATH_LENGTH 512
#define MAX_PATH_LENGTH 1024
#define MAX_FILENAME_LENGTH 128

typedef std::string FileInfo;

class FileDialog {
public:
    enum Mode { FILE_DIALOG_OPEN, FILE_DIALOG_SAVE };

    FileDialog();
    ~FileDialog();

    bool update();

    bool isActive() const { return windowActive; }

    bool isFileSelected() const { return SelectFilePressed; }

    fs::path filePath();

    void show(Mode mode);

private:
    bool windowActive;
    Rectangle windowBounds;
    Vector2 panOffset;
    bool dragMode;
    bool supportDrag;

    bool dirPathEditMode;
    char dirPathText[MAX_PATH_LENGTH];

    int filesListScrollIndex;
    bool filesListEditMode;
    int filesListActive;

    bool fileNameEditMode;
    char fileNameText[MAX_FILENAME_LENGTH];
    bool SelectFilePressed;
    bool CancelFilePressed;
    int itemFocused;

    FilePathList dirFiles;
    std::string filterExt;
    fs::path dirPathTextCopy;
    fs::path fileNameTextCopy;

    int prevFilesListActive;
    Mode dialogMode;

    std::vector<std::string> dirFilesIcon;

    void ReloadDirectoryFiles();
};
