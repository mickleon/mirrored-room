/*******************************************************************************************
 *
 *   Window File Dialog v1.2 - Modal file dialog to open/save files
 *
 *   LICENSE: zlib/libpng
 *
 *   Copyright (c) 2019-2024 Ramon Santamaria (@raysan5)
 *
 *   This software is provided "as-is", without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software.
 *
 *   Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *     1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software in a
 * product, an acknowledgment in the product documentation would be appreciated
 * but is not required.
 *
 *     2. Altered source versions must be plainly marked as such, and must not
 * be misrepresented as being the original software.
 *
 *     3. This notice may not be removed or altered from any source
 * distribution.
 *
 **********************************************************************************************/

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

    void update();

    bool isActive() const { return windowActive; }

    bool isFileSelected() const { return SelectFilePressed; }

    fs::path filePath();

    void show();

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
