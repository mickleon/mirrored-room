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

#include <filesystem>
#include <string.h>
#include <string>
#include <vector>

#include "raygui.h"

#include "FileDialog.h"

namespace fs = std::filesystem;

FileDialog::FileDialog():
    windowActive(false),
    supportDrag(true),
    dragMode(false),
    dirPathEditMode(false),
    filesListActive(-1),
    filesListScrollIndex(0),
    fileNameEditMode(false),
    SelectFilePressed(false),
    CancelFilePressed(false),
    prevFilesListActive(-1),
    itemFocused(-1),
    dialogMode(FILE_DIALOG_OPEN),
    dirFilesIcon(MAX_DIRECTORY_FILES) {
    // Init window data
    windowBounds = {10, 40, 500, 400};
    panOffset = {0, 0};

    memset(dirPathText, 0, MAX_PATH_LENGTH);
    memset(fileNameText, 0, MAX_FILENAME_LENGTH);

    // Custom variables initialization
    fs::path dirPath;
    fs::path fileName;

    fs::path initPath = fs::current_path();

    if (!initPath.empty()) {
        if (fs::exists(initPath)) {
            if (fs::is_directory(initPath)) {
                dirPath = fs::absolute(initPath);
                fileName.clear();
            } else {
                dirPath = fs::absolute(initPath.parent_path());
                fileName = initPath.filename();
            }
        } else {
            dirPath = fs::path(GetWorkingDirectory());
        }
    } else {
        dirPath = fs::path(GetWorkingDirectory());
    }

    std::string dirPathStr = dirPath.string();
    strncpy(dirPathText, dirPathStr.c_str(), MAX_PATH_LENGTH - 1);

    std::string fileNameStr = fileName.string();
    strncpy(fileNameText, fileNameStr.c_str(), MAX_FILENAME_LENGTH - 1);

    dirPathTextCopy = dirPath;
    fileNameTextCopy = fileName;

    filterExt.clear();
    dirFiles.count = 0;
    dirFiles.capacity = 0;
    dirFiles.paths = nullptr;
}

FileDialog::~FileDialog() {
    if (dirFiles.paths != nullptr) {
        UnloadDirectoryFiles(dirFiles);
    }
}

fs::path FileDialog::filePath() {
    if (strlen(fileNameText) == 0) {
        SelectFilePressed = false;
        return fs::path();
    }

    fs::path fullPath = fs::path(dirPathText) / fileNameText;
    SelectFilePressed = false;
    return fullPath;
}

void FileDialog::update() {
    if (windowActive) {
        // update window dragging
        if (supportDrag) {
            Vector2 mousePosition = GetMousePosition();

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(
                        mousePosition,
                        {windowBounds.x, windowBounds.y, windowBounds.width, 20}
                    )) {
                    dragMode = true;
                    panOffset.x = mousePosition.x - windowBounds.x;
                    panOffset.y = mousePosition.y - windowBounds.y;
                }
            }

            if (dragMode) {
                windowBounds.x = mousePosition.x - panOffset.x;
                windowBounds.y = mousePosition.y - panOffset.y;

                if (windowBounds.x < 0) {
                    windowBounds.x = 0;
                } else if (windowBounds.x >
                           (GetScreenWidth() - windowBounds.width)) {
                    windowBounds.x = GetScreenWidth() - windowBounds.width;
                }

                if (windowBounds.y < 0) {
                    windowBounds.y = 0;
                } else if (windowBounds.y >
                           (GetScreenHeight() - windowBounds.height)) {
                    windowBounds.y = GetScreenHeight() - windowBounds.height;
                }

                if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                    dragMode = false;
                }
            }
        }

        // Load current directory files
        if (dirFiles.paths == nullptr) {
            ReloadDirectoryFiles();
        }

        // Draw window and controls
        const char *windowTitle = (dialogMode == FILE_DIALOG_SAVE) ?
                                      "Сохранить файл" :
                                      "Открыть файл";

        windowActive = !GuiWindowBox(windowBounds, windowTitle);

        // Draw previous directory button + logic
        if (GuiButton(
                {windowBounds.x + windowBounds.width - 48,
                 windowBounds.y + 24 + 12, 40, 24},
                "< .."
            )) {
            fs::path currentPath(dirPathText);
            if (currentPath.has_parent_path()) {
                fs::path newPath = currentPath.parent_path();
                std::string newPathStr = newPath.string();
                strncpy(dirPathText, newPathStr.c_str(), MAX_PATH_LENGTH - 1);
                dirPathTextCopy = newPath;
            }
            ReloadDirectoryFiles();

            filesListActive = -1;
            memset(fileNameText, 0, MAX_FILENAME_LENGTH);
            fileNameTextCopy.clear();
        }

        // Draw current directory text box
        if (GuiTextBox(
                {windowBounds.x + 8, windowBounds.y + 24 + 12,
                 windowBounds.width - 48 - 16, 24},
                dirPathText, MAX_PATH_LENGTH, dirPathEditMode
            )) {
            if (dirPathEditMode) {
                fs::path newPath(dirPathText);
                if (fs::exists(newPath) && fs::is_directory(newPath)) {
                    dirPathTextCopy = fs::absolute(newPath);
                    std::string dirPathStr = dirPathTextCopy.string();
                    strncpy(
                        dirPathText, dirPathStr.c_str(), MAX_PATH_LENGTH - 1
                    );
                    ReloadDirectoryFiles();
                } else {
                    std::string dirPathStr = dirPathTextCopy.string();
                    strncpy(
                        dirPathText, dirPathStr.c_str(), MAX_PATH_LENGTH - 1
                    );
                }
            }
            dirPathEditMode = !dirPathEditMode;
        }

        // List view
        int prevTextAlignment = GuiGetStyle(LISTVIEW, TEXT_ALIGNMENT);
        int prevElementsHeight = GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT);
        GuiSetStyle(LISTVIEW, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
        GuiSetStyle(LISTVIEW, LIST_ITEMS_HEIGHT, 24);

        std::vector<const char *> iconCStrings;
        iconCStrings.reserve(dirFiles.count);
        for (unsigned int i = 0; i < dirFiles.count; i++) {
            iconCStrings.push_back(dirFilesIcon[i].c_str());
        }

        GuiListViewEx(
            {windowBounds.x + 8, windowBounds.y + 48 + 20,
             windowBounds.width - 16, windowBounds.height - 60 - 16 - 48 - 10},
            (const char **)iconCStrings.data(), dirFiles.count,
            &filesListScrollIndex, &filesListActive, &itemFocused
        );
        GuiSetStyle(LISTVIEW, TEXT_ALIGNMENT, prevTextAlignment);
        GuiSetStyle(LISTVIEW, LIST_ITEMS_HEIGHT, prevElementsHeight);

        // Check if a directory has been selected
        if ((filesListActive >= 0) &&
            (filesListActive != prevFilesListActive)) {
            fs::path selectedPath = fs::path(dirFiles.paths[filesListActive]);
            fs::path fileNamePath = selectedPath.filename();

            std::string fileNameStr = fileNamePath.string();
            strncpy(fileNameText, fileNameStr.c_str(), MAX_FILENAME_LENGTH - 1);

            fs::path fullPath = fs::path(dirPathText) / fileNamePath;
            if (fs::exists(fullPath) && fs::is_directory(fullPath)) {
                if (fileNamePath == "..") {
                    fs::path currentPath(dirPathText);
                    if (currentPath.has_parent_path()) {
                        fs::path newPath = currentPath.parent_path();
                        std::string newPathStr = newPath.string();
                        strncpy(
                            dirPathText, newPathStr.c_str(), MAX_PATH_LENGTH - 1
                        );
                        dirPathTextCopy = newPath;
                    }
                } else {
                    std::string newPathStr = fullPath.string();
                    strncpy(
                        dirPathText, newPathStr.c_str(), MAX_PATH_LENGTH - 1
                    );
                    dirPathTextCopy = fullPath;
                }

                ReloadDirectoryFiles();

                filesListActive = -1;
                memset(fileNameText, 0, MAX_FILENAME_LENGTH);
                fileNameTextCopy.clear();
            } else {
                // Сохраняем копию для имени файла
                fileNameTextCopy = fileNamePath;
            }

            prevFilesListActive = filesListActive;
        }

        // Draw bottom controls
        GuiLabel(
            {windowBounds.x + 8, windowBounds.y + windowBounds.height - 48, 95,
             24},
            "Имя файла"
        );

        if (GuiTextBox(
                {windowBounds.x + 107,
                 windowBounds.y + windowBounds.height - 48,
                 windowBounds.width - 325, 24},
                fileNameText, MAX_FILENAME_LENGTH, fileNameEditMode
            )) {
            if (strlen(fileNameText) > 0) {
                if (dialogMode == FILE_DIALOG_SAVE) {
                    fileNameTextCopy = fs::path(fileNameText);
                } else {
                    fs::path fullPath = fs::path(dirPathText) / fileNameText;
                    if (fs::exists(fullPath) && !fs::is_directory(fullPath)) {
                        fs::path fileNamePath(fileNameText);
                        for (unsigned int i = 0; i < dirFiles.count; i++) {
                            fs::path currentFile(dirFiles.paths[i]);
                            if (currentFile.filename() == fileNamePath) {
                                filesListActive = i;
                                fileNameTextCopy = fileNamePath;
                                break;
                            }
                        }
                    }
                }
            }
            fileNameEditMode = !fileNameEditMode;
        }

        const char *selectButtonText =
            (dialogMode == FILE_DIALOG_SAVE) ? "Сохранить" : "Открыть";

        SelectFilePressed = GuiButton(
            {windowBounds.x + windowBounds.width - 208,
             windowBounds.y + windowBounds.height - 48, 96, 24},
            selectButtonText
        );

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
            SelectFilePressed = true;
        }

        if (GuiButton(
                {windowBounds.x + windowBounds.width - 104,
                 windowBounds.y + windowBounds.height - 48, 96, 24},
                "Отмена"
            )) {
            windowActive = false;
        }

        // Exit on file selected
        if (SelectFilePressed) {
            windowActive = false;
        }

        // File dialog has been closed
        if (!windowActive) {
            UnloadDirectoryFiles(dirFiles);
            dirFiles.count = 0;
            dirFiles.capacity = 0;
            dirFiles.paths = nullptr;
        }
    }
}

void FileDialog::ReloadDirectoryFiles() {
    UnloadDirectoryFiles(dirFiles);

    const char *filter = filterExt.empty() ? nullptr : filterExt.c_str();
    dirFiles = LoadDirectoryFilesEx(dirPathText, filter, false);
    itemFocused = 0;

    for (auto &icon : dirFilesIcon) {
        icon.clear();
    }

    for (unsigned int i = 0; i < dirFiles.count; i++) {
        std::string filename = GetFileName(dirFiles.paths[i]);

        if (IsPathFile(dirFiles.paths[i])) {
            if (IsFileExtension(
                    dirFiles.paths[i],
                    ".png;.bmp;.tga;.gif;.jpg;.jpeg;.psd;.hdr;.qoi;.dds;.pkm;." "ktx;.pvr;.astc"
                )) {
                dirFilesIcon[i] = "#12#" + filename;
            } else if (IsFileExtension(
                           dirFiles.paths[i],
                           ".wav;.mp3;.ogg;.flac;.xm;.mod;.it;.wma;.aiff"
                       )) {
                dirFilesIcon[i] = "#11#" + filename;
            } else if (IsFileExtension(
                           dirFiles.paths[i],
                           ".txt;.info;.md;.nfo;.xml;.json;.c;.cpp;.cs;.lua;." "py;.glsl;.vs;.fs"
                       )) {
                dirFilesIcon[i] = "#10#" + filename;
            } else if (IsFileExtension(
                           dirFiles.paths[i], ".exe;.bin;.raw;.msi"
                       )) {
                dirFilesIcon[i] = "#200#" + filename;
            } else {
                dirFilesIcon[i] = "#218#" + filename;
            }
        } else {
            dirFilesIcon[i] = "#1#" + filename;
        }
    }
}

void FileDialog::show(Mode mode) {
    windowActive = true;
    dialogMode = mode;
    windowBounds = {10, 50, 500, 400};
}
